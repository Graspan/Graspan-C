#ifndef ARRAY2_ARRAYSTOMERGE_H
#define ARRAY2_ARRAYSTOMERGE_H
#include "../../common.h"

namespace myarray2 {
// 两两合并，结果存放在resEdges,resLabels
class ArraysToMerge {	
private:
	vertexid_t *resEdges;
	char *resLabels;
	int numEdges;

	vertexid_t *edges;
	char *labels;
	int size;
	int capacity;
		
public:
	ArraysToMerge();

	// getters and setters
	inline int getNumEdges() {return numEdges;}
	inline vertexid_t* getEdgesFirstAddr() {return resEdges;}
	inline char* getLabelsFirstAddr() {return resLabels;}

	void addOneArray();
	void addOneEdge(vertexid_t edge,char label);
	void merge();
	void setRes(int size,vertexid_t *edges,char *labels);
	void clear();
};

}
#endif
