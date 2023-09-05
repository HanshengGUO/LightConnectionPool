#pragma once
#include <string>
#include <ctime>

using namespace std;

/*
	数据库增删改查相关操作
*/

#include <mysql.h>
#include <string>
#include "public.h"

using namespace std;

class Connection {
public:
	// 初始化数据库连接
	Connection();
	// 释放数据库连接资源
	~Connection();
	// 连接数据库
	bool connect(
		string ip, 
		unsigned short port, 
		string user, 
		string password,
		string dbname
	);
	// 更新操作 insert、delete、update
	bool update(string sql);
	// 查询操作 select
	MYSQL_RES* query(string sql);

	// 刷新空闲的起始时间点
	void refreshAliveTime() { _alivetime = clock(); }
	// 返回存活时间
	clock_t getAliveTime()const { return clock() - _alivetime; }

private:
	MYSQL *_conn;
	clock_t _alivetime; // 记录进入空闲状态（进入队列）的时间

};