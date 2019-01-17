#ifndef PREPROC_H
#define PREPROC_H
#include "../common.h"
#include "../context.h"
#include "../datastructures/vit.h"

/*
typedef struct graphFile {
	vertexid_t src;
	vertexid_t dst;
	char rawLabel[GRAMMAR_STR_LEN];
}Graph;
*/
// typedef int vertexid_t 
class Preproc {
	private:
		vertexid_t totalNumEdges;
		vertexid_t totalDuplicateEdges;
		vertexid_t maxVid;
		vertexid_t *numEdges;        // degree of each vertex. size = maxVid+1

	public:
		Preproc();
		~Preproc();
		void loadGrammar(Context &c);
		void setNumEdges(Context &c);
		void setVIT(Context &c);
		void savePartitions(Context &c);
		void test(Context &c);
};

void quickSort3Way(vertexid_t *A,char *B,int l,int r);
void insertSort(vertexid_t *A,char *B,int l,int r);
int getPivot(vertexid_t *A,char *B,int l,int r);


#endif
