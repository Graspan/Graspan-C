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
	~Partition();
	void loadFromFile(partitionid_t id,Context &c);
	void writeToFile();
	void print(Context &c);
};

#endif
