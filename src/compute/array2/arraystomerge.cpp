#include "arraystomerge.h"
#include "../../algorithm/myalgorithm.h"

namespace myarray2 {

ArraysToMerge::ArraysToMerge() {
	resEdges = NULL; resLabels = NULL; numEdges = 0;
	edges = NULL; labels = NULL; size = capacity = 0;
}	

void ArraysToMerge::mergeTwoArray() {
	if(size) {	
		int len = 0;
		vertexid_t *edge_v = new vertexid_t[numEdges+size];
		char *edge_l = new char[numEdges+size];
		myalgo::unionTwoArray(len,edge_v,edge_l,numEdges,resEdges,resLabels,size,edges,labels);
		setRes(len,edge_v,edge_l);
		delete[] edge_v; delete[] edge_l;
	}
}

void ArraysToMerge::setRes(int size,vertexid_t *edges,char *labels) {
	if(!size)
		return;
	if(!numEdges) {
		numEdges = size;
		resEdges = new vertexid_t[size];
		resLabels = new char[size];
		memcpy(resEdges,edges,sizeof(vertexid_t)*size);
		memcpy(resLabels,labels,sizeof(char)*size);
	}
	else {
		if(numEdges != size) {
			if(resEdges) delete[] resEdges;
			if(resLabels) delete[] resLabels;
			resEdges = new vertexid_t[size];
			resLabels = new char[size];
		}
		numEdges = size;
		memcpy(resEdges,edges,sizeof(vertexid_t)*size);
		memcpy(resLabels,labels,sizeof(char)*size);
	}
}

void ArraysToMerge::addOneContainer() {
	mergeTwoArray();
	size = 0;
	if(!capacity) {
		capacity = 8;
		edges = new vertexid_t[capacity];
		labels = new char[capacity];
		for(int i = 0;i < capacity;++i) {
			edges[i] = -1;
			labels[i] = (char)127;
		}
	}	
}

void ArraysToMerge::addOneEdge(vertexid_t edge,char label) {
	if(size >= capacity) {
		capacity *= 2;
		myrealloc(edges,size,capacity);
		myrealloc(labels,size,capacity);
		for(int i = size;i < capacity;++i) {
			edges[i] = -1;
			labels[i] = (char)127;
		}
	}
	edges[size] = edge;
	labels[size++] = label;
}

void ArraysToMerge::clear() {
	if(numEdges) {
		if(resEdges) {delete[] resEdges; resEdges = NULL;}
		if(resLabels) {delete[] resLabels; resLabels = NULL;}
		numEdges = 0;
	}
	if(capacity) {
		if(edges) {delete[] edges; edges = NULL;}
		if(labels) {delete[] labels; labels = NULL;}
		size = capacity = 0;
	}			
}
void ArraysToMerge::merge() {
	mergeTwoArray();	
}

}
