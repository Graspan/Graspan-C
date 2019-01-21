#include <cstdlib>
#include <time.h>
#include <unistd.h>
#include "compute.h"
#include "arraystomerge.h"
#include "../../algorithm/myalgorithm.h"

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
	Partition p;
	p.loadFromFile(pid,c);
	Partition q;
	q.loadFromFile(qid,c);
	// p.print(c); q.print(c);

	// check partitions
	if(p.check() || q.check()) {
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

	int iterId = 0;
	while(1) {
		long totalIterEdges = computeOneIteration(compset,c);
		postProcessOneIteration(compset);
		long realIterEdges = compset.getDeltasTotalNumEdges();
		thisRoundEdges += realIterEdges;
		/*
		cout << "===== STARTING ITERATION " << iterId++ << "=====" << endl;
		cout << "EDGES THIS ITER: " << realIterEdges << endl;
		cout << "NEW EDGES TOTAL: " << thisRoundEdges << endl << endl;	
		*/
		if(realIterEdges == 0)
			break;	
	}
	//compset.print();
	return thisRoundEdges;	
}

long Compute::computeOneIteration(ComputationSet &compset,Context &c) {
	// TODO: parellel computing
	long thisIterEdges = 0;
	for(int i = 0;i < compset.getSize();++i) {
		thisIterEdges += computeOneVertex(i,compset,c);		
	}
	return thisIterEdges;
}

void Compute::postProcessOneIteration(ComputationSet &compset) {
	// oldsV <- {oldsV,deltasV}
	for(int i = 0;i < compset.getSize();++i) {
		bool oldEmpty = compset.oldEmpty(i);
		bool deltaEmpty = compset.deltaEmpty(i);
		if(oldEmpty) {
			if(deltaEmpty)
				compset.clearOlds(i);
			else {
				compset.setOlds(i,compset.getDeltasNumEdges(i),compset.getDeltasEdges(i),compset.getDeltasLabels(i));	
			}
		}
		else {
			if(!deltaEmpty) {
				int len = 0; int n1 = compset.getOldsNumEdges(i); int n2 = compset.getDeltasNumEdges(i);
				vertexid_t *edges = new vertexid_t[n1+n2];
				char *labels = new char[n1+n2];
				myalgo::unionTwoArray(len,edges,labels,n1,compset.getOldsEdges(i),compset.getOldsLabels(i),n2,compset.getDeltasEdges(i),compset.getDeltasLabels(i));
				compset.setOlds(i,len,edges,labels);
				delete[] edges; delete[] labels;
			}	
		}
	}
	// deltasV <- newsV,newV <- empty set
	
	for(int i = 0;i < compset.getSize();++i) {
		bool newEmpty = compset.newEmpty(i);
		
		if(!newEmpty) {
			// DeltasV = NewsV - oldsV
			int len = 0; int n1 = compset.getNewsNumEdges(i); int n2 = compset.getOldsNumEdges(i);
			vertexid_t *edges = new vertexid_t[n1];
			char *labels = new char[n1];
			myalgo::minusTwoArray(len,edges,labels,n1,compset.getNewsEdges(i),compset.getNewsLabels(i),n2,compset.getOldsEdges(i),compset.getOldsLabels(i));
			if(len)
				compset.setDeltas(i,len,edges,labels);	
			else 
				compset.clearDeltas(i);
			delete[] edges; delete[] labels;
		}
		else
			compset.clearDeltas(i);
		compset.clearNews(i);
	}
}	


long Compute::computeOneVertex(vertexid_t index,ComputationSet &compset,Context &c) {

	long newEdgesNum = 0;

	bool oldEmpty = compset.oldEmpty(index);
	bool deltaEmpty = compset.deltaEmpty(index);
	if(oldEmpty && deltaEmpty) return 0;	// if this vertex has no edges, no need to merge.
	
	ArraysToMerge arrays;

	// find new edges to arrays
	getEdgesToMerge(index,compset,oldEmpty,deltaEmpty,arrays,c);

	// merge and sort edges, remove duplicate edges
	arrays.mergeAndSort();

	// add edges to News
	newEdgesNum = arrays.getNumEdges();

	
	if(newEdgesNum)
		compset.setNews(index,newEdgesNum,arrays.getEdgesFirstAddr(),arrays.getLabelsFirstAddr());
	else
		compset.clearNews(index);

	// return resources
	arrays.clear();

	return newEdgesNum;	
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
			//cout << "S-rule: " << edges[i] << "," << (int)labels[i] << " -> (" << edges[i] << "," << (int)newLabel << ")" << endl;
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
	numEdges = compset.getDeltasNumEdges(dstInd);
	edges = compset.getDeltasEdges(dstInd);
	labels = compset.getDeltasLabels(dstInd);
	
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
			//cout << "D-rule:" << (int)dstVal << " " << (int)labels[i] << " (" << edges[i] << "," << (int)newVal << ")" <<  endl;
		}
	}
	
	if(!isOld) {
		numEdges = compset.getOldsNumEdges(dstInd);
		edges = compset.getOldsEdges(dstInd);
		labels = compset.getOldsLabels(dstInd);
		added = false;
		for(vertexid_t i = 0;i < numEdges;++i) {
			newVal = c.grammar.checkRules(dstVal,labels[i]);
			if(newVal != (char)127) {
				if(!added) {
					arrays.addOneArray();
					added = true;
				}	
				arrays.addOneEdge(edges[i],newVal);
				//cout << "D-rule:" << (int)dstVal << " " << (int)labels[i] << " (" << edges[i] << "," << (int)newVal << ")" <<  endl;
			}
		}
	}
}

}
