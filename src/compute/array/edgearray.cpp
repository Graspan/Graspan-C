#include "edgearray.h"
#include "../../preproc/preproc.h"
#include "../../algorithm/myalgorithm.h"

namespace myarray {

EdgeArray::EdgeArray() {
	this->size = 0;				
}

EdgeArray::EdgeArray(int size,vertexid_t *edges,char *labels) {
	this->size = size;
	if(size) {
		this->edges = new vertexid_t[size];
		this->labels = new char[size];
		memcpy(this->edges,edges,sizeof(vertexid_t)*size);
		memcpy(this->labels,labels,sizeof(char)*size);
	}
}

void EdgeArray::set(int size,vertexid_t *edges,char *labels) {
	if(!size)
		return;	

	if(!this->size) {
		this->size = size;
		this->edges = new vertexid_t[size];
		this->labels = new char[size];
		memcpy(this->edges,edges,sizeof(vertexid_t)*size);
		memcpy(this->labels,labels,sizeof(char)*size);
	}
	else {

		if(this->size != size) {	
			if(this->edges)	delete[] this->edges; 
			if(this->labels) delete[] this->labels;
			this->edges = new vertexid_t[size];
			this->labels = new char[size];
		}
		this->size = size;
		memcpy(this->edges,edges,sizeof(vertexid_t)*size);
		memcpy(this->labels,labels,sizeof(char)*size);
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
