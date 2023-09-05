#pragma once
#include <string>
#include <ctime>

using namespace std;

/*
	���ݿ���ɾ�Ĳ���ز���
*/

#include <mysql.h>
#include <string>
#include "public.h"

using namespace std;

class Connection {
public:
	// ��ʼ�����ݿ�����
	Connection();
	// �ͷ����ݿ�������Դ
	~Connection();
	// �������ݿ�
	bool connect(
		string ip, 
		unsigned short port, 
		string user, 
		string password,
		string dbname
	);
	// ���²��� insert��delete��update
	bool update(string sql);
	// ��ѯ���� select
	MYSQL_RES* query(string sql);

	// ˢ�¿��е���ʼʱ���
	void refreshAliveTime() { _alivetime = clock(); }
	// ���ش��ʱ��
	clock_t getAliveTime()const { return clock() - _alivetime; }

private:
	MYSQL *_conn;
	clock_t _alivetime; // ��¼�������״̬��������У���ʱ��

};