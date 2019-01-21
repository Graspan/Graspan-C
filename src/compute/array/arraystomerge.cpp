#include "arraystomerge.h"
#include "../../algorithm/myalgorithm.h"

namespace myarray {
	
ArraysToMerge::ArraysToMerge() {
	size = capacity = arraySize = arrayCapacity = numEdges = 0;
}

void ArraysToMerge::mergeAndSort() {
	if(size)	
		sort1();
}

void ArraysToMerge::sort1() {
	// TODO: better Sort Algorithm
	myalgo::quickSort(this->edges,this->labels,0,size-1);
	// remove duplicate edges
	vertexid_t *edge_v = new vertexid_t[size];
	char *edge_l = new char[size];
	int len = 0;
	myalgo::removeDuple(len,edge_v,edge_l,size,edges,labels);

	memcpy(edges,edge_v,sizeof(vertexid_t)*len);
	memcpy(labels,edge_l,sizeof(char)*len);
	numEdges = len;
	delete[] edge_v;
	delete[] edge_l;
}

void ArraysToMerge::clear() {
	if(capacity) {
		if(edges) {	delete[] edges; edges = NULL; }
		if(labels) { delete[] labels; labels = NULL; }
		capacity = size = 0;
	}	
	if(arrayCapacity) {
		if(index) { delete[] index; index = NULL; }	
		if(addr) { delete[] addr; addr = NULL; }
		arrayCapacity = arraySize;
	}
	numEdges = 0;
}

void ArraysToMerge::addOneArray() {
	if(arraySize == 0) {
		arrayCapacity = _CAPACITY_VALUE;
		capacity = CAPACITY_VALUE;
		edges = new vertexid_t[capacity];
		labels = new char[capacity];
		index = new vertexid_t[arrayCapacity];
		addr = new vertexid_t[arrayCapacity];
		for(int i = 0;i < capacity;++i) {
			edges[i] = -1;
			labels[i] = (char)127;
		}
		for(int i = 0;i < arrayCapacity;++i)
			index[i] = addr[i] = 0;	
	}
	else {
		if(arraySize >= arrayCapacity) {
			arrayCapacity *= 2;	
			myrealloc(index,arraySize,arrayCapacity);
			myrealloc(addr,arraySize,arrayCapacity);
			for(int i = arraySize;i < arrayCapacity;++i)
				index[i] = addr[i] = 0;	
		}
	}
	// add one empty array
	++arraySize;
}

void ArraysToMerge::addOneEdge(vertexid_t edge,char label) {
	if(arraySize) {
		if(size >= capacity) {
			capacity *= 2;
			myrealloc(edges,size,capacity);
			myrealloc(labels,size,capacity);
			for(int i = size;i < capacity;++i) {
				edges[i] = -1;
				labels[i] = (char)127;
			}	
		}
		// add edge
		if(index[arraySize-1] == 0) {
			addr[arraySize-1] = size;		
		}
		edges[size] = edge;
		labels[size] = label;
		++index[arraySize-1];
		++size;
	}
	else {
		cout << "add edge failed! " << endl;	
	}
}

void ArraysToMerge::print() {
	//cout << "number of arraystomerge: " << arraySize << endl;
	/*
	for(int i = 0;i < arraySize;++i) {
		cout << "array " << i << ":";
		for(int j = 0;j < index[i];++j) {
			cout << "(" << edges[addr[i] + j] << "," << (int)labels[addr[i] + j] << ") -> ";
		}
		cout << "end" << endl;
	}
	*/
	if(numEdges) {
		for(int i = 0;i < numEdges;++i) {
			cout << "(" << edges[i] << "," << (int)labels[i] << ") -> "; 	
		}
		cout << "end" << endl;
	}	
}

}
