#ifndef VIT_H
#define VIT_H
#include "../common.h"

typedef struct vitnode {
	vertexid_t start;
	vertexid_t end;
	int degree;
}VitNode;

class Vit {
	private:
		VitNode *p;
		int size;
		int capacity;
	
	public:
		Vit();
		Vit(int size,vertexid_t *start,vertexid_t *end,int *degrees);
		void clear();

		// getter and setter
		inline int getSize() {return size;}
		inline int getStart(int vitId){return p[vitId].start;}
		inline int getEnd(int vitId){return p[vitId].end;}
		inline int getDegree(int vitId){return p[vitId].degree;}
	
		inline void setDegree(int vitId,int numEdges) {p[vitId].degree = numEdges;}

		void setVitValue(int vitId,vertexid_t start,vertexid_t end,int numEdges);
		void add(vertexid_t start,vertexid_t end,int numEdges);
		void print();
};

#endif
