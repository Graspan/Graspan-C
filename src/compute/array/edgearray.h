#ifndef ARRAY_EDGEARRAY_H
#define ARRAY_EDGEARRAY_H
#include "../../common.h"

namespace myarray {

class EdgeArray {		
	private:		
		vertexid_t *edges;
		char *labels;
		int size;

	public:
		EdgeArray();
		EdgeArray(int size,vertexid_t *edges,char *labels);
		void clear();

		// getters and setters
		inline bool isEmpty() {return !size;}
		inline vertexid_t *getEdges() {return edges;}
		inline char* getLabels() {return labels;}
		inline int getSize() {return size;}
	
		//void reset(int newSize);	// alloc and init
		void set(int size,vertexid_t *edges,char *labels);

		void print();
};		
}

#endif
