#include "edgelist.h"
#include <iostream>
#include <cstdlib>
using std::cout;
using std::endl;

EdgeList::EdgeList(vertexid_t vid) {
	head = (EdgeNode*)malloc(sizeof(EdgeNode));
	tail = head;
	head->next = NULL;
	this->vid = vid;
	numOutEdges = 0;
}

EdgeList::~EdgeList() {
	if(head)
		clear();
}

void EdgeList::addEdge(vertexid_t vid,char label) {
	EdgeNode *p = (EdgeNode*)malloc(sizeof(EdgeNode));
	p->vid = vid;
	p->label = label;
	p->next = NULL;
	tail->next = p;
	tail = p;
	++numOutEdges;
}

void EdgeList::clear() {
	EdgeNode *p,*next;
	p = head;
	while(p) {
		next = p->next;
		free(p);
		p = next;
	}
	numOutEdges = 0;
}

void EdgeList::print() {

	cout << sizeof(EdgeNode) << endl; 

	if(!head || !head->next)
		cout << "empty EdgeList!" << endl;
	else {
		cout << "EdgeList:" << endl;
		cout << "(" << vid << "," << numOutEdges << "): ";
		EdgeNode *cur = head->next;
		while(cur) {
			cout << "(" << cur->vid << "," << cur->label << ")" << " -> ";
			cur = cur->next;
		}
		cout << "end" << endl;
	}
}

