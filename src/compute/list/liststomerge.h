#ifndef LISTTOMERGE_H
#define LISTTOMERGE_H
#include "edgelist.h"

namespace mylist {

class ListsToMerge {
	private:
		EdgeList *lists;	
		int numOfLists;
		int capacityOfLists;
		int numEdges;	// exclude duplicate edges
		
		// store result
		vertexid_t *edges;
		char *labels;

	public:
		ListsToMerge();
		// getters and setters
		inline bool isEmpty() {return !numOfLists;}
		inline int getNumEdges() {return numEdges;}
		inline vertexid_t* getEdgesFirstAddr() {return edges;}
		inline char* getLabelsFirstAddr() {return labels;}

		void clear();
		void addOneList();
		void addOneEdge(vertexid_t edge,char label);

		void mergeAndSort();
		void mergeKLists();
		void print();
};	
}
#endif
