#include <cstdlib>
#include <time.h>
#include <unistd.h>

#include "compute.h"
#include "arraystomerge.h"
#include "../../algorithm/myalgorithm.h"

namespace myarray {

Compute::Compute() {
	newTotalEdges = 0;	
	isNewp = isNewq = false;
}

bool Compute::scheduler(partitionid_t &p,partitionid_t &q,Context &c) {
	return c.ddm.scheduler(p,q);
}

void Compute::adjustDDM(partitionid_t p,bool isNewp,partitionid_t q,bool isNewq,Context &c) {
	c.ddm.adjust(p,isNewp,q,isNewq);	
}

long Compute::startCompute(Context &c)  {
	
	// create threadpool for parallel computing
	boost::asio::io_service ioServ;
	boost::thread_group threadPool;
	boost::asio::io_service::work work(ioServ);
	for(int i = 0;i < c.getNumThreads();++i) 
		threadPool.create_thread(boost::bind(&boost::asio::io_service::run,&ioServ));	

	partitionid_t pid,qid;
	int roundId = 0;
	// TODO: better schedular algorithm 
	while(scheduler(pid,qid,c)) {
		Partition p;
		p.loadFromFile(pid,c);
		Partition q;
		q.loadFromFile(qid,c);

		cout << "=====STARTING ROUND" << roundId++  << "=====" << endl;
		cout << "P = " << pid << " , Q = " << qid << endl;  

		// check partitions
		if(p.check() || q.check()) {
			cout << "partition duplication happened!" << endl;
			exit(-1);
		}

		// init compset
		ComputationSet compset;
		initComputationSet(compset,p,q,c);

		// compute 
		newTotalEdges += computeOneRound(compset,c,ioServ);
		
		// update Partitions
		updatePartitions(compset,p,q,c);
		adjustDDM(pid,isNewp,qid,isNewq,c);
		
		// write Partitions to File
		p.writeToFile(pid,c);
		q.writeToFile(qid,c);

		// return resources
		p.clear();
		q.clear();
		compset.clear();
	}

	return newTotalEdges;	
}

void Compute::initComputationSet(ComputationSet &compset,Partition &p,Partition &q,Context &c) {
	compset.init(p,q,c);
}

long Compute::computeOneRound(ComputationSet &compset,Context &c,boost::asio::io_service &ioServ) {
	// TODO: parallel computing
	long thisRoundEdges = 0;
	int numThreads = c.getNumThreads();

	int segsize = compset.getSize() / 64 + 1;
	int nSegs = compset.getSize() / segsize + 1;

	int iterId = 0;
	while(1) {
		clock_t start_t,end_t;
		start_t = clock();
		computeOneIteration(compset,segsize,nSegs,c,ioServ);
		postProcessOneIteration(compset);
		long realIterEdges = compset.getDeltasTotalNumEdges();
		thisRoundEdges += realIterEdges;
		end_t = clock();
		cout << "===== STARTING ITERATION " << iterId++ << "=====" << endl;
		cout << "EDGES THIS ITER: " << realIterEdges << endl;
		cout << "NEW EDGES TOTAL: " << thisRoundEdges << endl;	
		cout << "time: " << (end_t - start_t) / CLOCKS_PER_SEC << "s" << endl << endl;

		if(realIterEdges == 0)
			break;	
	}
	return thisRoundEdges;	
}

void Compute::computeOneIteration(ComputationSet &compset,int segsize,int nSegs,Context &c,boost::asio::io_service &ioServ) {
	int lower,upper;
	numFinished = 0;
	compFinished = false; 
	int compSize = compset.getSize();

	long thisIterEdges = 0;
	for(int i = 0;i < nSegs;++i) {
		lower = i * segsize;
		upper = (lower + segsize < compSize) ? lower + segsize : compSize;
		ioServ.post(boost::bind(&myarray::Compute::runUpdates,this,lower,upper,nSegs,compset,c));
	}
	std::unique_lock<std::mutex> lck(comp_mtx);
	while (!compFinished) cv.wait(lck);
}

void Compute::runUpdates(int lower,int upper,int nSegs,ComputationSet &compset,Context &c) {
	for(int i = lower;i < upper;++i)
		computeOneVertex(i,compset,c);			

	std::unique_lock<std::mutex> lck(add_edges);
	++numFinished;
	if(numFinished == nSegs) {
		compFinished = true;
		cv.notify_one();
	}	
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

void Compute::updatePartitions(ComputationSet &compset,Partition &p,Partition &q,Context &c) {
	// TODO: refactor this method.
	vertexid_t psize = compset.getPsize();	
	vertexid_t numEdges,numVertices;
	vertexid_t *edges; char *labels;
	vertexid_t *addr; vertexid_t *index;

	// update p
	
	numVertices = psize;
	numEdges = compset.getPNumEdges();
	isNewp = false;
	if(numEdges > c.vit.getDegree(p.getId())) {
		isNewp = true;	
		edges = new vertexid_t[numEdges];
		labels = new char[numEdges];
		addr = new vertexid_t[numVertices];	
		index = new vertexid_t[numVertices];
		for(int i = 0;i < numVertices;++i)
			index[i] = 0;

		vertexid_t cur_addr = 0;
		for(vertexid_t i = 0;i < numVertices;++i) {
			addr[i] = cur_addr;
			index[i] = compset.getOldsNumEdges(i);
			if(index[i]) {
				memcpy(edges + addr[i],compset.getOldsEdges(i),sizeof(vertexid_t) * index[i]);
				memcpy(labels + addr[i],compset.getOldsLabels(i),sizeof(char) * index[i]);
			}
			cur_addr += index[i];
		}
		p.update(numVertices,numEdges,edges,labels,addr,index);
		delete[] edges; delete[] labels; delete[] addr; delete[] index;
		c.vit.setDegree(p.getId(),numEdges);
	}
	
	// update q
	isNewq = false;
	numVertices = compset.getQsize();
	numEdges = compset.getQNumEdges();
	if(numEdges > c.vit.getDegree(q.getId())) {
		isNewq = true;	
		edges = new vertexid_t[numEdges];
		labels = new char[numEdges];
		addr = new vertexid_t[numVertices];
		index = new vertexid_t[numVertices];
		for(int i = 0;i < numVertices;++i)
			index[i] = 0;

		vertexid_t cur_addr = 0;
		for(vertexid_t i = 0;i < numVertices;++i) {
			addr[i] = cur_addr;
			index[i] = compset.getOldsNumEdges(i + psize);
			if(index[i]) {
				memcpy(edges + addr[i],compset.getOldsEdges(i + psize),sizeof(vertexid_t) * index[i]);
				memcpy(labels + addr[i],compset.getOldsLabels(i + psize),sizeof(char) * index[i]);
			}
				cur_addr += index[i];
		}
		q.update(numVertices,numEdges,edges,labels,addr,index);
		delete[] edges; delete[] labels; delete[] addr; delete[] index;
		c.vit.setDegree(q.getId(),numEdges);
	}
}

}

