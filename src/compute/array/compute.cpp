#include <cstdlib>
#include <time.h>
#include <unistd.h>
#include "compute.h"
#include "arraystomerge.h"

namespace myarray {

Compute::Compute() {
	newTotalEdges = 0;		
}

bool Compute::scheduler(partitionid_t &p,partitionid_t &q,Context &c) {
	p = 0; q = 1;
	return true;
}

long Compute::startCompute(Context &c)  {
	
	// TODO: scheduler 
	partitionid_t pid,qid;	
	scheduler(pid,qid,c);
	cout << "p = " << pid << " , q = " << qid << endl;
	Partition p;
	p.loadFromFile(pid,c);
	Partition q;
	q.loadFromFile(qid,c);

	// check duple
	if(p.checkduple() || q.checkduple()) {
		cout << "partition duplication happened!" << endl;
		exit(-1);
	}
	
	// init compset
	ComputationSet compset;
	initComputationSet(compset,p,q,c);

	// compute 
	newTotalEdges += computeOneRound(compset,c);

	// return resources
	p.clear();
	q.clear();
	compset.clear();

	return newTotalEdges;	
}

void Compute::initComputationSet(ComputationSet &compset,Partition &p,Partition &q,Context &c) {
	compset.init(p,q,c);
}

long Compute::computeOneRound(ComputationSet &compset,Context &c) {
	// TODO: parallel computing
	long thisRoundEdges = 0;

	while(1) {
		long iterEdges = computeOneIteration(compset,c);
		thisRoundEdges += iterEdges;
		if(iterEdges == 0)
			break;
		// TODO: Ov <- {Ov,Dv}     Dv <- mergeResule - Ov 
	}

	return thisRoundEdges;	
}

long Compute::computeOneIteration(ComputationSet &compset,Context &c) {
	// TODO: parellel computing
	long thisIterEdges = 0;
	for(int i = 0;i < compset.getSize();++i) {
		long thisVertexEdges = computeOneVertex(i,compset,c);		
		thisIterEdges += thisVertexEdges;
	}
	return thisIterEdges;
}

long Compute::computeOneVertex(vertexid_t index,ComputationSet &compset,Context &c) {

	bool oldEmpty = compset.oldEmpty(index);
	bool deltaEmpty = compset.deltaEmpty(index);
	if(oldEmpty && deltaEmpty) return 0;	// if this vertex has no edges, no need to merge.
	
	ArraysToMerge arrays;

	// find new edges to arrays
	getEdgesToMerge(index,compset,oldEmpty,deltaEmpty,arrays,c);
		
	// TODO: mergeAndSort arrays

	arrays.clear();

	return 0;	
}

void Compute::getEdgesToMerge(vertexid_t index,ComputationSet &compset,bool oldEmpty,bool deltaEmpty,ArraysToMerge &arrays,Context &c) {
		
	// add s-rule edges	
	if(!deltaEmpty) 	
		genS_RuleEdges(index,compset,arrays,c);
	
	// add d-rule edges merge Ov of only Dv
	if(!oldEmpty) 
		genD_RuleEdges(index,compset,arrays,c,true);
	// add d-rule edges merge Dv of (Ov and Dv)
	if(!deltaEmpty)
		genD_RuleEdges(index,compset,arrays,c,false);
}

void Compute::genS_RuleEdges(vertexid_t index,ComputationSet &compset,ArraysToMerge &arrays,Context &c) {
	
	vertexid_t numEdges = compset.getDeltasNumEdges(index);
	vertexid_t *edges = compset.getDeltasEdges(index);
	char *labels = compset.getDeltasLabels(index);

	char newLabel;
	bool added = false;
	for(vertexid_t i = 0;i < numEdges;++i) {
		newLabel = c.grammar.checkRules(labels[i]);
		if(newLabel != (char)127) {
			if(!added) { 
				arrays.addOneArray();
				added = true;
			}
			arrays.addOneEdge(edges[i],newLabel);
		}
	}
}

void Compute::genD_RuleEdges(vertexid_t index,ComputationSet &compset,ArraysToMerge &arrays,Context &c,bool isOld) {
	
	vertexid_t numEdges;
	vertexid_t *edges;
	char *labels;
	if(isOld) {
		numEdges = compset.getOldsNumEdges(index);
		edges = compset.getOldsEdges(index);
		labels = compset.getOldsLabels(index);
	}
	else {
		numEdges = compset.getDeltasNumEdges(index);
		edges = compset.getDeltasEdges(index);
		labels = compset.getDeltasLabels(index);
	}

	vertexid_t indexInCompset;
	for(vertexid_t i = 0;i < numEdges;++i) {
		indexInCompset = compset.getIndexInCompSet(edges[i]);
		// target vertice is in memory
		if(indexInCompset != -1) 
			checkEdges(indexInCompset,labels[i],compset,arrays,c,isOld);
	}	
}

void Compute::checkEdges(vertexid_t dstInd,char dstVal,ComputationSet &compset,ArraysToMerge &arrays,Context &c,bool isOld) {
	
	vertexid_t numEdges;
	vertexid_t *edges;
	char *labels;
	if(isOld) {
		numEdges = compset.getDeltasNumEdges(dstInd);
		edges = compset.getDeltasEdges(dstInd);
		labels = compset.getDeltasLabels(dstInd);
	}
	else {
		numEdges = compset.getAllsNumEdges(dstInd);
		edges = compset.getAllsEdges(dstInd);
		labels = compset.getAllsLabels(dstInd);
	}
	
	char newVal;
	bool added = false;
	for(vertexid_t i = 0;i < numEdges;++i) {
		newVal = c.grammar.checkRules(dstVal,labels[i]);
		if(newVal != (char)127) {
			if(!added) {
				arrays.addOneArray();
				added = true;
			}	
			arrays.addOneEdge(edges[i],newVal);
		}
	}
}

}
