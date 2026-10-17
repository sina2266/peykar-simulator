#include "gtest/gtest.h"
#include <vector>
#include <iostream>
#include <LoopIterator.h>
using namespace std;
using namespace vcell_util; 


TEST(loopiter,basic) {
	vector<int> stuff;
	stuff.push_back(1);
	stuff.push_back(2);
	stuff.push_back(3);
	stuff.push_back(4);

	const vector<int> cstuff = stuff;
	ConstLoopIterator<const vector<int>> iter(cstuff); 
	ConstLoopIterator<const vector<int>> copyiter(iter);
	for (int i = 0; i < 10; i++) {
		cout << *(iter++) << endl;
	}
	cout << "break" << endl;
	for (int i = 0; i < 12; i++) {
		cout << *(++iter) << endl;
	}
	cout << "break" << endl;
	for (int i = 0; i < 12; i++) {
		cout << *(--iter) << endl;
	}
	cout << "break" << endl;
	for (int i = 0; i < 12; i++) {
		cout << *(iter++) << endl;
	}
	cout << "break" << endl;
	LoopIterator<vector<int>> iter2(stuff,false); 
	for (int i = 0; i < 5; i++) {
		cout << *(iter2++) << endl;
	}
	LoopIterator<vector<int>> viter(stuff); 
	*viter = 4;
	//*iter = 4;
}