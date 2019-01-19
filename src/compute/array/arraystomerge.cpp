#include "arraystomerge.h"

namespace myarray {
	
ArraysToMerge::ArraysToMerge() {
	size = capacity = arraySize = arrayCapacity = 0;
}

void ArraysToMerge::clear() {
	if(capacity) {
		if(edges) {	
			delete[] edges;
			edges = NULL;
		}
		if(labels) {
			delete[] labels;
			labels = NULL;
		}
		capacity = size = 0;
	}	
	if(arrayCapacity) {
		if(index) {
			delete[] index;
			index = NULL;
		}	
		if(addr) {
			delete[] addr;
			addr = NULL;
		}
		arrayCapacity = arraySize = 0;
	}
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
	cout << "number of arraystomerge: " << arraySize << endl;
	for(int i = 0;i < arraySize;++i) {
		cout << "array " << i << ":";
		for(int j = 0;j < index[i];++j) {
			cout << "(" << edges[addr[i] + j] << "," << (int)labels[addr[i] + j] << ") -> ";
		}
		cout << "end" << endl;
	}
}

}
