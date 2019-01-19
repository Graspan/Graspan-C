#ifndef ARRAY_COMPUTE_H
#define ARRAY_COMPUTE_H

#include "computationset.h"
#include "arraystomerge.h"
#include "../../datastructures/partition.h"
#include "../../common.h"

namespace myarray {

class Compute {
private:
	long newTotalEdges;

public:
	Compute();
	void initComputationSet(ComputationSet &compset,Partition &p,Partition &q,Context &c);
	bool scheduler(partitionid_t &p,partitionid_t &q,Context &c);
	
	long computeOneRound(ComputationSet &compset,Context &c);
	long computeOneIteration(ComputationSet &compset,Context &c);
	long computeOneVertex(vertexid_t index,ComputationSet &compset,Context &c);
	
	void getEdgesToMerge(vertexid_t index,ComputationSet &compset,bool oldEmpty,bool deltaEmpty,ArraysToMerge &arrays,Context &c);
	void genS_RuleEdges(vertexid_t index,ComputationSet &compset,ArraysToMerge &arrays,Context &c);
	void genD_RuleEdges(vertexid_t index,ComputationSet &compset,ArraysToMerge &arrays,Context &c,bool isOld);
	void checkEdges(vertexid_t dstInd,char dstVal,ComputationSet &compset,ArraysToMerge &arrays,Context &c,bool isOld);
	long startCompute(Context &c);	// return newTotalEdges;
};
}
#endif
