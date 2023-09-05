#pragma once
#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include "Connection.h"

using namespace std;

/*
	数据库连接池功能模块
*/

class ConnectionPool {
public:
	// 静态方法：获取连接池对象实例
	static ConnectionPool* getConnectionPool();
	// 从连接池中获得一个可用的空闲连接
	// 释放时可以自动析构，无需归还指针
	shared_ptr<Connection> getConnection();

private:
	ConnectionPool(); // 单例#1 构造函数私有化

	bool loadConfigFile(); // 加载配置文件

	// 运行在独立的线程中，专门负责生产新连接
	void produceConnectionTask();

	// 扫描空闲线程
	void scanConnectionTask();

	string _ip;
	unsigned short _port;
	string _username;
	string _password;
	string _dbname;
	int _initSize; // 连接池初始连接量
	int _maxSize; // 连接池最大连接量
	int _maxIdleTime; // 连接池最大空闲时间
	int _connectionTimeout; // 获取连接的超时时间

	queue<Connection*> _ConnectionQueue; // 存储mysql连接的队列
	mutex _queueMutex; // 维护连接队列的线程安全互斥锁
	atomic_int _connectionCnt; // 记录创建的连接总数量
	condition_variable cv; // 设置条件变量，连接生产线程和连接消费线程的通信
};