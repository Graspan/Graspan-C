#ifndef ARRAY_COMPUTATIONSET_H
#define ARRAY_COMPUTATIONSET_H
#include "../../common.h"
#include "../../datastructures/partition.h"
#include "edgearray.h"
#include "../../context.h"

namespace myarray {		

// Partition p,q	
typedef struct interval {
	vertexid_t pFirstVid;	// firstVertexId(p)
	vertexid_t pLastVid;	// lastVertexId(p)
	vertexid_t qFirstVid;	// firstVertexId(q)
	vertexid_t qLastVid;	// lastVertexId(q)
	
	vertexid_t pFirstIndex;	// index in ComputationSet
	vertexid_t pLastIndex;
	vertexid_t qFirstIndex;
	vertexid_t qLastIndex;
}Interval;

class ComputationSet {
	private:		
		EdgeArray *Olds;	// Ov
		EdgeArray *Deltas;	// Dv
		EdgeArray *Alls;	// Ov + Dv

		vertexid_t size;	// numVertices(p) + numVertices(q)
		vertexid_t psize;	// numVertices(p)
		vertexid_t qsize;	// numVertices(q)

		Interval interval;

	public:
		ComputationSet();
		void clear();

		// getters and setters
		inline vertexid_t getSize() {return size;}
		inline bool oldEmpty(vertexid_t index) {return Olds[index].isEmpty();}
		inline bool deltaEmpty(vertexid_t index) {return Deltas[index].isEmpty();}
		inline bool allsEmpty(vertexid_t index) {return Alls[index].isEmpty();}
		
		inline vertexid_t getOldsNumEdges(vertexid_t index) {return Olds[index].getSize();}
		inline vertexid_t getDeltasNumEdges(vertexid_t index) {return Deltas[index].getSize();}
		inline vertexid_t getAllsNumEdges(vertexid_t index) {return Alls[index].getSize();}

		inline char* getOldsLabels(vertexid_t index) {return Olds[index].getLabels();}
		inline char* getDeltasLabels(vertexid_t index) {return Deltas[index].getLabels();}
		inline char* getAllsLabels(vertexid_t index) {return Alls[index].getLabels();}

		inline vertexid_t* getOldsEdges(vertexid_t index) {return Olds[index].getEdges();}
		inline vertexid_t* getDeltasEdges(vertexid_t index) {return Deltas[index].getEdges();}
		inline vertexid_t* getAllsEdges(vertexid_t index) {return Alls[index].getEdges();}

		void init(Partition &p,Partition &q,Context &c);
		
		vertexid_t getIndexInCompSet(vertexid_t vid); 
};		
}
#endif
