#ifndef ARRAY2_ARRAYSTOMERGE_H
#define ARRAY2_ARRAYSTOMERGE_H
#include "../../common.h"
#include "../containerstomerge.h"

namespace myarray2 {
// 两两合并，结果存放在resEdges,resLabels
class ArraysToMerge : public ContainersToMerge {	
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
	void mergeTwoArray();
	void setRes(int size,vertexid_t *edges,char *labels);

	// virtual functions
	virtual void addOneContainer();
	virtual void addOneEdge(vertexid_t vid,char label);
	virtual int getNumEdges() {return numEdges;}
	virtual void merge();
	virtual vertexid_t* getEdgesFirstAddr() {return resEdges;}
	virtual char* getLabelsFirstAddr() {return resLabels;}
	virtual void clear();
};

}
#endif
