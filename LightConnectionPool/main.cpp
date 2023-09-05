#include <iostream>
#include "Connection.h"
#include "LightConnectionPool.h"

using namespace std;

int main()
{
    clock_t begin = clock();

    for (int i = 0; i < 1000; i++) {
        /*
        Connection conn;
        char sql[1024] = { 0 };
        sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
            "zhang san", 20, "male");
        conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
        conn.update(sql);
        */
        char sql[1024] = { 0 };
        sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
            "zhang san", 20, "male");

        ConnectionPool* cp = ConnectionPool::getConnectionPool();
        shared_ptr<Connection> sp = cp->getConnection();
        sp->update(sql);
    }
    clock_t end = clock();

    cout << (end - begin) << "ms" << endl;
    

    // ConnectionPool* cp = ConnectionPool::getConnectionPool();

    return 0;
}