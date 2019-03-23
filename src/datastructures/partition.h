#ifndef PARTITION_H
#define PARTITION_H
#include "../common.h"
#include "../context.h"

class Partition {
private:
	partitionid_t id;	
	vertexid_t numVertices;
	long numEdges;

	vertexid_t *vertices;
	char *labels;
	long *addr;
	vertexid_t *index;
public:		
	Partition();
	Partition(partitionid_t id);
	void clear();

	// getters and setters
	inline partitionid_t getId() {return id;}
	inline vertexid_t getNumVertices() {return numVertices;}
	inline long getNumEdges() {return numEdges;}
	inline vertexid_t getIndex(vertexid_t id) {return index[id];}
	inline vertexid_t* getEdgesFirstAddr(vertexid_t id) {return vertices + addr[id];}
	inline char* getLabelsFirstAddr(vertexid_t id) {return labels + addr[id];}
	inline void setId(partitionid_t id) {this->id = id;}

	vertexid_t getNumRealVertices();
	bool check();
	void loadFromFile(partitionid_t id,Context &c);
	void writeToFile(partitionid_t id,Context &c);
	void update(vertexid_t numVertices,long numEdges,vertexid_t *vertices,char *labels,long *addr,vertexid_t *index);

	void print(Context &c);
};

#endif
