#ifndef ARRAYSTOMERGE_H
#define ARRAYSTOMERGE_H
#include "../../common.h"
#include "minheap.h"

#define _CAPACITY_VALUE 8	// TODO: modify this number
#define CAPACITY_VALUE 8	// TODO: modify this number

namespace myarray {
class ArraysToMerge {
	/* 	turn vector<vector> to 1D array 
	 * 	faster and smaller(RAM)
	 */ 
private:
	
	vertexid_t *edges;	// edges = new vertexid_t[capacity];
	char *labels;		// labels = new char[capacity];
	int size;			// size = total number of edges
	int capacity;		// capacity = 2 * size

	vertexid_t *index;	// calculate offset of each array. index = new vertexid_t[arrayCapacity];
	vertexid_t *addr;	// store the firstAddr of each array. addr = new vertexid_t[arrayCapacity];
	int arraySize; 		// arraySize = number of arrays 
	int arrayCapacity;	// arrayCapacity = 2 * arraySize

	int numEdges;		// exclude duplicate edges (numEdges <= size)
public:
	ArraysToMerge();
	
	// getters and setters
	inline bool isEmpty() {return !arraySize;}
	inline vertexid_t *getEdgesFirstAddr() {return edges;}
	inline char* getLabelsFirstAddr() {return labels;}
	inline int getNumEdges() {return numEdges;}

	void clear();
	void addOneArray();
	void addOneEdge(vertexid_t edge,char label);

	/* TODO: better Sort algorithm
	 * after mergeAndSort, index and addr is useless
	 */
	void mergeAndSort();
	void mergeKArrays();	// method 1
	void sort1();			// method 2

	void print();
};
}
#endif
