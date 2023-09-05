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
	���ݿ����ӳع���ģ��
*/

class ConnectionPool {
public:
	// ��̬��������ȡ���ӳض���ʵ��
	static ConnectionPool* getConnectionPool();
	// �����ӳ��л��һ�����õĿ�������
	// �ͷ�ʱ�����Զ�����������黹ָ��
	shared_ptr<Connection> getConnection();

private:
	ConnectionPool(); // ����#1 ���캯��˽�л�

	bool loadConfigFile(); // ���������ļ�

	// �����ڶ������߳��У�ר�Ÿ�������������
	void produceConnectionTask();

	// ɨ������߳�
	void scanConnectionTask();

	string _ip;
	unsigned short _port;
	string _username;
	string _password;
	string _dbname;
	int _initSize; // ���ӳس�ʼ������
	int _maxSize; // ���ӳ����������
	int _maxIdleTime; // ���ӳ�������ʱ��
	int _connectionTimeout; // ��ȡ���ӵĳ�ʱʱ��

	queue<Connection*> _ConnectionQueue; // �洢mysql���ӵĶ���
	mutex _queueMutex; // ά�����Ӷ��е��̰߳�ȫ������
	atomic_int _connectionCnt; // ��¼����������������
	condition_variable cv; // �����������������������̺߳����������̵߳�ͨ��
};