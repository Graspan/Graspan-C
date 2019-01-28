#ifndef MYALGORITHM_H
#define MYALGORITHM_H
#include "../common.h"

namespace myalgo {

inline int myCompare(vertexid_t a1,char b1,vertexid_t a2,char b2) {return (a1 == a2) ? (b1 - b2) : (a1 - a2);}

// sort algorithm
void quickSort(vertexid_t *A,char *B,int l,int r);
void insertSort(vertexid_t *A,char *B,int l,int r);
int split(vertexid_t *A,char *B,int l,int r);

// merge sorted arrays algorithm
void unionTwoArray(int &len,vertexid_t *dstA,char *dstB,int len1,vertexid_t *A1,char *B1,int len2,vertexid_t *A2,char *B2); // union set
void minusTwoArray(int &len,vertexid_t *dstA,char *dstB,int len1,vertexid_t *A1,char *B1,int len2,vertexid_t *A2,char *B2); // difference set

void removeDuple(int &len,vertexid_t *dstA,char *dstB,int srclen,vertexid_t *srcA,char *srcB); // remove duplicate edges
bool checkEdges(int len,vertexid_t *A,char *B); // check duplicate edges and sequence

unsigned long int getUsedMemory(const pid_t pid);

};
#endif
