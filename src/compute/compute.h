#ifndef COMPUTE_H
#define COMPUTE_H

#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include "computationset.h"
#include "array/arraystomerge.h"
#include "list/liststomerge.h"
#include "../datastructures/partition.h"
#include "../common.h"

class Compute {
private:
	long newTotalEdges;
	bool isNewp;
	bool isNewq;

	std::mutex comp_mtx;
	std::condition_variable cv;
	short numFinished;
	bool compFinished;

public:
	Compute();
	void initComputationSet(ComputationSet &compset,Partition &p,Partition &q,Context &c);
	bool scheduler(partitionid_t &p,partitionid_t &q,Context &c);
	
	long computeOneRound(ComputationSet &compset,Context &c,boost::asio::io_service &ioServ,bool &isFinished);
	void computeOneIteration(ComputationSet &compset,int segsize,int nSegs,Context &c,boost::asio::io_service &ioServ);
	void runUpdates(int lower,int upper,int nSegs,ComputationSet &compset,Context &c);
	long computeOneVertex(vertexid_t index,ComputationSet &compset,Context &c);		
	void postProcessOneIteration(ComputationSet &compset);

	void getEdgesToMerge(vertexid_t index,ComputationSet &compset,bool oldEmpty,bool deltaEmpty,myarray::ArraysToMerge &arrays,Context &c);
	void getEdgesToMerge(vertexid_t index,ComputationSet &compset,bool oldEmpty,bool deltaEmpty,mylist::ListsToMerge &lists,Context &c);
	void genS_RuleEdges(vertexid_t index,ComputationSet &compset,myarray::ArraysToMerge &arrays,Context &c);
	void genS_RuleEdges(vertexid_t index,ComputationSet &compset,mylist::ListsToMerge &lists,Context &c);
	void genD_RuleEdges(vertexid_t index,ComputationSet &compset,myarray::ArraysToMerge &arrays,Context &c,bool isOld);
	void genD_RuleEdges(vertexid_t index,ComputationSet &compset,mylist::ListsToMerge &lists,Context &c,bool isOld);
	void checkEdges(vertexid_t dstInd,char dstVal,ComputationSet &compset,myarray::ArraysToMerge &arrays,Context &c,bool isOld);
	void checkEdges(vertexid_t dstInd,char dstVal,ComputationSet &compset,mylist::ListsToMerge &lists,Context &c,bool isOld);

	void updatePartitions(ComputationSet &compset,Partition &p,Partition &q,bool isFinished,Context &c);
	void updateSinglePartition(ComputationSet &compset,Partition &p,bool isFinished,Context &c,bool isP);

	void adjustDDM(partitionid_t p,bool isNewp,partitionid_t q,bool isNewq,bool isFinished,Context &c);
	void needRepart(ComputationSet &compset,Partition &p,Partition &q,bool &repart_p,bool &repart_q,bool isFinished,Context &c);

	long startCompute(Context &c);	// return newTotalEdges;
};
#endif
