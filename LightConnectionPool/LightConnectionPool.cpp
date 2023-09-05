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
			_username = value;
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
		return;
	}

	// 创建初始数量连接
	for (int i = 0; i < _initSize; i++) {
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		_ConnectionQueue.push(p);
		_connectionCnt++;
	}

	// 启动一个线程作为连接生产者
	// 绑定对象和方法
	// 这个方法可以方便的访问成员变量，同时方便被外部调用
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));

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
			_ConnectionQueue.push(p);
			_connectionCnt++;
		}

		// 通知所有（消费者线程），可以消费连接
		cv.notify_all();
	}
}