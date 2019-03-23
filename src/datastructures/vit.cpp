#include "vit.h"
#include <stdlib.h>

Vit::Vit(int size,vertexid_t *start,vertexid_t *end,long *degrees) {
	this->size = size;
	this->capacity = 2 * size ;
	p = (VitNode*)calloc(capacity,sizeof(VitNode));
	for(int i = 0;i < size;++i) {
		p[i].start = start[i];
		p[i].end = end[i];
		p[i].degree = degrees[i];
	}
}

Vit::Vit() {
	this->size = 0;
	this->capacity = 8;
	p = (VitNode*)calloc(capacity,sizeof(VitNode));
}

long Vit::getTotalNumEdges() {
	long res = 0;
	for(int i = 0;i < size;++i) {
		res += p[i].degree;	
	}
	return res;
}

void Vit::clear() {
	if(p) {
		free(p);
		p = NULL;
	}	
}

void Vit::setVitValue(int vitId,vertexid_t start,vertexid_t end,long numEdges) {
	if(vitId >= size || vitId < 0)
		cout << "Invalid vitId!" << endl;
	else {
		p[vitId].start = start;
		p[vitId].end = end;
		p[vitId].degree = numEdges;
	}
}

void Vit::add(vertexid_t start,vertexid_t end,long numEdges) {
	if(size == capacity) {
		capacity *= 2;
		p = (VitNode*)realloc(p,sizeof(VitNode)*capacity);
	}
	if(!p) {
		cout << "vit realloc failed!" << endl;
		exit(-1);
	}
	p[size].start = start;
	p[size].end = end;
	p[size].degree = numEdges;
	++size;
}

partitionid_t Vit::getPartitionId(vertexid_t vid) {
	for(partitionid_t i = 0;i < size;++i) {
		if(vid >= p[i].start && vid <= p[i].end)
			return i;	
	}
	return -1;
}


void Vit::print() {
	cout << "==========VIT TABLE===================" << endl;
	cout << "size:" << size << " capacity:" << capacity << endl;
	for(int i = 0;i < size;++i) {
		cout << "(" << p[i].start << "," << p[i].end << "," << p[i].degree << ") -> ";
	}
	cout << "end" << endl;
	cout << "==========VIT TABLE===================" << endl;
	cout << endl;
}

