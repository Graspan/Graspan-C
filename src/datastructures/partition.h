#ifndef PARTITION_H
#define PARTITION_H
#include "../common.h"
#include "../context.h"

class Partition {
private:
	partitionid_t id;	
	vertexid_t numVertices;
	vertexid_t numEdges;

	vertexid_t *vertices;
	char *labels;
	vertexid_t *addr;
	vertexid_t *index;
public:		
	Partition();
	Partition(partitionid_t id);
	void clear();

	// getters and setters
	inline partitionid_t getId() {return id;}
	inline vertexid_t getNumVertices() {return numVertices;}
	inline vertexid_t getNumEdges() {return numEdges;}
	inline vertexid_t getlndex(vertexid_t id) {return index[id];}
	inline vertexid_t* getEdgesFirstAddr(vertexid_t id) {return vertices + addr[id];}
	inline char* getLabelsFirstAddr(vertexid_t id) {return labels + addr[id];}

	inline void setId(partitionid_t id) {this->id = id;}

	bool check();
	void loadFromFile(partitionid_t id,Context &c);
	void writeToFile(partitionid_t id,Context &c);
	void update(vertexid_t numVertices,vertexid_t numEdges,vertexid_t *vertices,char *labels,vertexid_t *addr,vertexid_t *index);
	void repart(Partition &p,Context &c);

	void print(Context &c);
};

#endif
