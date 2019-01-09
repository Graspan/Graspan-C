#ifndef PREPROC_H
#define PREPROC_H
#include "../context.h"
#include "../datastructures/vit.h"

class Preproc {
	private:
		int totalNumEdges;
		int totalNumVertex;
		int maxVid;
		int *numEdges;        // degree of each vertex. size = maxVid+1

	public:
		Preproc();
		~Preproc();
		void loadGrammar(Context &c);
		void setNumEdges(Context &c);
		void setVIT(Context &c);
		void savePartitions(Context &c);
		
};

void quickSort3Way(vertexid_t *A,char *B,int l,int r);
void insertSort(vertexid_t *A,char *B,int l,int r);
int getPivot(vertexid_t *A,char *B,int l,int r);

#endif
