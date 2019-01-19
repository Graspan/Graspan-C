#include "edgearray.h"

namespace myarray {

EdgeArray::EdgeArray() {
	this->size = 0;				
}

EdgeArray::~EdgeArray() {
}

EdgeArray::EdgeArray(int size,vertexid_t *edges,char *labels) {
	this->size = size;
	if(size) {
		this->edges = new vertexid_t[size];
		this->labels = new char[size];
		for(int i = 0;i < size;++i) {
			this->edges[i] = edges[i];
			this->labels[i] = labels[i];
		}
	}
}

void EdgeArray::print() {
	if(!size)
		cout << "empty edgearray! " << endl;
	else {
		for(int i = 0;i < size;++i) {
			cout << edges[i] << ", " << (int)labels[i] << " -> ";
		}
	cout << "end" << endl;
}
}

void EdgeArray::clear() {
	if(size) {
		if(edges) {
			delete[] edges;
			edges = NULL;
		}
		if(labels) {
			delete[] labels;
			labels = NULL;
		}
		size = 0;
	}	
}
}
