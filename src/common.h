#ifndef COMMON_H
#define COMMON_H
#include <iostream>

using std::cout;
using std::endl;

#define BUFFER_SIZE 256
#define GRAMMAR_STR_LEN 36
#define GB 1073741824

typedef int vertexid_t;
typedef int partitionid_t;

template <typename T>
void myrealloc(T* &arr,int size,int Tosize) {
	T* tmpArr;
	tmpArr = new T[Tosize];
	for(int i = 0;i < size;++i) {
		tmpArr[i] = arr[i];	
	}
	delete[] arr;
	arr = tmpArr;
}

#endif
