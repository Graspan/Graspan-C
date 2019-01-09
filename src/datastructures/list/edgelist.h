#ifndef LIST_EDGELIST_H
#define LIST_EDGELIST_H
#include "../../common.h"

typedef struct edgeNode {
	char label;
	vertexid_t vid;
	struct edgeNode *next;	
}EdgeNode;

class EdgeList {
	private:
		vertexid_t vid;
		int numOutEdges;
		EdgeNode *head;
		EdgeNode *tail;

	public:
		EdgeList(vertexid_t vid);
		~EdgeList();
	
		void setVid(vertexid_t vid);
		void addEdge(vertexid_t vid,char label);	
		void clear();
		void print();
};

#endif
