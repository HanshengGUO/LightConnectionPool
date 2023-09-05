#include <iostream>
#include <string>
#include <queue>

#include "public.h"
#include "LightConnectionPool.h"

using namespace std;
// 线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool; // 对于静态变量，编译器自动上锁
	return &pool;
}

// 从配置文件中加载
bool ConnectionPool::loadConfigFile() {
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file does not exist!");
		return false;
	}

	while (!feof(pf)) {
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find("=", 0);
		if (idx == -1) // 无效的配置项
		{
			continue;
		}
		int endidx = str.find("\n", idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);

		if (key == "ip") {
			_ip = value;
		}
		else if (key == "port") {
			_port = atoi(value.c_str());
		}
		else if (key == "username") {
			_username = value;
		}
		else if (key == "password") {
			_password = value;
		}
		else if (key == "dbname") {
			_dbname = value;
		}
		else if (key == "initSize") {
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize") {
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime") {
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeOut") {
			_connectionTimeout = atoi(value.c_str());
		}
	}
	return true;
}

// 构造连接池实例
ConnectionPool::ConnectionPool() {
	if (!loadConfigFile()) {
		LOG("Failed to read from config file, default to mysql.ini");
		return;
	}



	// 创建初始数量连接
	for (int i = 0; i < _initSize; i++) {
		Connection* p = new Connection();
		if (!p->connect(_ip, _port, _username, _password, _dbname)) {
			LOG("Connection failed to mysql database!");
			return;
		}
		p->refreshAliveTime();
		_ConnectionQueue.push(p);
		_connectionCnt++;
	}

	// 启动一个线程作为连接生产者
	// 绑定对象和方法
	// 这个方法可以方便的访问成员变量，同时方便被外部调用
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();

	// 启动一个新的定时线程，扫描空闲连接的存活时间
	thread scanner(std::bind(&ConnectionPool::scanConnectionTask, this));
	scanner.detach();
}

void ConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(_queueMutex);
		while (!_ConnectionQueue.empty()) {
			cv.wait(lock);
		}

		// 连接数量没有到达上限，继续创建新的连接
		if (_connectionCnt < _maxSize) {
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();
			_ConnectionQueue.push(p);
			_connectionCnt++;
		}

		// 通知所有（消费者线程），可以消费连接
		cv.notify_all();
	}
}

void ConnectionPool::scanConnectionTask() {
	for (;;) {
		// 通过sleep实现定时
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		// 扫描队列，释放多余连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize) {
			Connection* p = _ConnectionQueue.front();
			// 换算毫秒
			if (p->getAliveTime() >= _maxIdleTime * 1000) {
				_ConnectionQueue.pop();
				_connectionCnt--;
				delete p; // 释放连接
			}
			else { break; } // 只需要检查队头
		}
	}
}

shared_ptr<Connection> ConnectionPool::getConnection() {
	unique_lock<mutex> lock(_queueMutex);
	while (_ConnectionQueue.empty()) {
		// sleep
		// 等待queue中出现可用的连接
		if (cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)) == cv_status::timeout) {
			if (_ConnectionQueue.empty()) {
				LOG("Failed: TimeOut on waiting for connection process");
				return nullptr;
			}
		}
	}
	/*
		shared_ptr智能指针析构时，会把connection资源自动删除，相当于调用connection的析构函数
		这里需要自定义shared_ptr的释放资源方式，把connection归还到queue中。
	*/
	shared_ptr<Connection> sp(_ConnectionQueue.front(),
		[&](Connection* pcon) {
			// 这里是在服务器线程调用，考虑线程安全 
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();
			_ConnectionQueue.push(pcon);
		}
	);
	_ConnectionQueue.pop();
	cv.notify_all(); // 消费结束后通知其他(生产者)线程
	
	return sp;
}