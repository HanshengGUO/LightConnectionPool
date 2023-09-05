#pragma once
#include <iostream>

using namespace std;

/*
	整个项目的公共资源
*/

#define LOG(str) \
	cout << __FILE__ << " : " << __LINE__ << " " << __TIMESTAMP__ \
		<< " : " << str << endl;