#ifndef COMMON_H
#define COMMON_H
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>

using std::cout;
using std::endl;

#define VMRSS_LINE 21	// in ubuntu os,vmrss_line = 21. TODO: modify this number if necessary.
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
