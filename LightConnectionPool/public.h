#pragma once
#include <iostream>

using namespace std;

/*
	������Ŀ�Ĺ�����Դ
*/

#define LOG(str) \
	cout << __FILE__ << " : " << __LINE__ << " " << __TIMESTAMP__ \
		<< " : " << str << endl;