#ifndef VIT_H
#define VIT_H
#include "../common.h"

typedef struct vitnode {
	vertexid_t start;
	vertexid_t end;
	long degree;
}VitNode;

class Vit {
	private:
		VitNode *p;
		int size;
		int capacity;
	
	public:
		Vit();
		Vit(int size,vertexid_t *start,vertexid_t *end,long *degrees);
		void clear();

		// getter and setter
		inline int getSize() {return size;}
		inline int getStart(int vitId){return p[vitId].start;}
		inline int getEnd(int vitId){return p[vitId].end;}
		inline long getDegree(int vitId){return p[vitId].degree;}
		inline void setDegree(int vitId,long numEdges) {p[vitId].degree = numEdges;}
		partitionid_t getPartitionId(vertexid_t vid);

		void setVitValue(int vitId,vertexid_t start,vertexid_t end,long numEdges);
		void add(vertexid_t start,vertexid_t end,long numEdges);
		void print();
};

#endif
