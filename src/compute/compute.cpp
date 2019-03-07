#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include "compute.h"
#include "array/arraystomerge.h"
#include "../algorithm/myalgorithm.h"
#include "../context.h"
#include <malloc.h>

Compute::Compute() {
	newTotalEdges = 0; oneRoundEdges = 0;
}

void loadTwoPartitionFromFile(Partition *p,Partition *q,partitionid_t new_pid,partitionid_t new_qid,Context &c) {
	// new_pid < new_qid,old_pid < old_qid
	partitionid_t old_pid = p->getId();	
	partitionid_t old_qid = q->getId();
	if(old_pid != new_pid) {
		if(old_pid != -1) {
			p->writeToFile(old_pid,c);
			p->clear();
		}
		p->loadFromFile(new_pid,c);
	}
	if(old_qid != new_qid) {
		if(old_qid != -1) {
			q->writeToFile(old_qid,c);
			q->clear();
		}	
		q->loadFromFile(new_qid,c);
	}
}

bool Compute::scheduler(partitionid_t &p,partitionid_t &q,Context &c) {
	return c.ddm.scheduler(p,q);
}

void Compute::adjustDDM(Partition &p,Partition &q,bool isFinished,Context &c) {
	partitionid_t pid = p.getId();
	partitionid_t qid = q.getId();

	// if new edges has been added to partition p and q
	if(oneRoundEdges) {
		for(int i = 0;i < c.getNumPartitions();++i) {
			c.ddm.setNeedCalc(pid,i,true);
			c.ddm.setNeedCalc(qid,i,true);
		}
	}
	if(isFinished)
		c.ddm.setNeedCalc(pid,qid,false);	
}

long Compute::startCompute(Context &c)  {
	// create threadpool for parallel computing
	boost::asio::io_service ioServ;
	boost::thread_group threadPool;
	boost::asio::io_service::work work(ioServ);
	if(c.getNumThreads() > 1) {
		for(int i = 0;i < c.getNumThreads();++i) 
			threadPool.create_thread(boost::bind(&boost::asio::io_service::run,&ioServ));	
	}

	partitionid_t pid,qid;
	int roundId = 0;
	Partition *p = new Partition();
	Partition *q = new Partition();

	// TODO: better schedular algorithm 
	while(scheduler(pid,qid,c)) {
		cout << "USED MEMORY: " << myalgo::getUsedMemory(getpid()) << endl;	
		// load partition p,q from file or Memory
		loadTwoPartitionFromFile(p,q,pid,qid,c);
		
		cout << "=====STARTING ROUND" << roundId++  << "=====" << endl;
		cout << "P = " << pid << " , Q = " << qid << endl;  
		// check partitions
		if(p->check() || q->check()) {
			cout << "partition p or q duplication happened!" << endl;
			exit(-1);
		}
		// init compset
		ComputationSet *compset = new ComputationSet();
		initComputationSet(*compset,*p,*q,c);
		// compute one round 
		bool isFinished = false;
		oneRoundEdges = computeOneRound(*compset,c,ioServ,isFinished);
		newTotalEdges += oneRoundEdges;
		// update Partitions and adjust VIT and DDM
		updatePartitions(*compset,*p,*q,isFinished,c);
		// return compset resources
		compset->clear(); delete compset;

		// repart if out of memory,adjust VIT and DDM
		bool repartP = false; bool repartQ = false; 
		Partition *p_2 = new Partition();
		Partition *q_2 = new Partition();
		needRepart(*p,*q,repartP,repartQ,isFinished,c);

		if(repartP) p->repart(*p_2,c);
		if(repartQ) q->repart(*q_2,c);
		bool value = (isFinished == true) ? false : true; 
		if(repartP) {
			c.ddm.setNeedCalc(pid,p_2->getId(),value);	
			c.ddm.setNeedCalc(qid,p_2->getId(),value);	
			if(repartQ) {
				c.ddm.setNeedCalc(pid,q_2->getId(),value);
				c.ddm.setNeedCalc(qid,q_2->getId(),value);
				c.ddm.setNeedCalc(p_2->getId(),q_2->getId(),value);
			}
		}
		else {
			if(repartQ) {
				c.ddm.setNeedCalc(pid,q_2->getId(),value);
				c.ddm.setNeedCalc(qid,q_2->getId(),value);
			}
		}
		// check repart partitions
		if(p_2->check() || q_2->check()) {
			cout << "REPA p_2 or q_2 duplication happened!" << endl;
			exit(-1);
		}
		// write p_2 and q_2 to File
		if(repartP) {p_2->writeToFile(p_2->getId(),c);} 
		p_2->clear(); delete p_2;
		if(repartQ) {q_2->writeToFile(q_2->getId(),c);} 
		q_2->clear(); delete q_2;
	}
	if(p->getId() != -1) p->writeToFile(p->getId(),c);
	p->clear(); delete p;
	if(q->getId() != -1) q->writeToFile(q->getId(),c); 
	q->clear(); delete q;
	return newTotalEdges;	
}

void Compute::initComputationSet(ComputationSet &compset,Partition &p,Partition &q,Context &c) {
	compset.init(p,q,c);
}

long Compute::computeOneRound(ComputationSet &compset,Context &c,boost::asio::io_service &ioServ,bool &isFinished) {
	isFinished = true;
	long thisRoundEdges = 0;
	int segsize = compset.getSize() / 64 + 1;
	int nSegs = compset.getSize() / segsize + 1;
 
	int iterId = 0;
	while(1) {
		unsigned long int usedMemory = myalgo::getUsedMemory(getpid());
		if(usedMemory >= (unsigned long int)(0.8 * c.getMemBudget())) {
			malloc_trim(0);
			usedMemory = myalgo::getUsedMemory(getpid());
		}

		if(usedMemory >= (unsigned long int)(0.8 * c.getMemBudget())) {
			cout << "usedMemory: " << usedMemory << " memBudget: " << (unsigned long int)(0.8 * c.getMemBudget()) << endl;
			isFinished = false;
			break;
		}
		time_t start_t,end_t;
		start_t = time(NULL);
		computeOneIteration(compset,segsize,nSegs,c,ioServ);
		postProcessOneIteration(compset);
		long realIterEdges = compset.getDeltasTotalNumEdges();
		thisRoundEdges += realIterEdges;
		end_t = time(NULL);
		cout << "===== STARTING ITERATION " << iterId++ << "=====" << endl;
		cout << "EDGES THIS ITER: " << realIterEdges << endl;
		cout << "NEW EDGES TOTAL: " << thisRoundEdges << endl;	
		cout << "time: " << end_t - start_t << "s" << endl << endl;
	
		if(realIterEdges == 0)
			break;	
	}
	return thisRoundEdges;	
}

void Compute::computeOneIteration(ComputationSet &compset,int segsize,int nSegs,Context &c,boost::asio::io_service &ioServ) {
	int lower,upper;
	int compSize = compset.getSize();
	if(c.getNumThreads() == 1) {
		lower = 0; upper = compSize;
		for(int i = lower;i < upper;++i)
			computeOneVertex(i,compset,c);	
	}
	else {
		numFinished = 0;
		compFinished = false; 
		for(int i = 0;i < nSegs;++i) {
			lower = i * segsize;
			upper = (lower + segsize < compSize) ? lower + segsize : compSize;
			ioServ.post(boost::bind(&Compute::runUpdates,this,lower,upper,nSegs,compset,c));
		}
		std::unique_lock<std::mutex> lck(comp_mtx);
		while (!compFinished) cv.wait(lck);
	}
}

void Compute::runUpdates(int lower,int upper,int nSegs,ComputationSet &compset,Context &c) {
	for(int i = lower;i < upper;++i)
		computeOneVertex(i,compset,c);			

	std::unique_lock<std::mutex> lck(comp_mtx);
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
		compset.clearDeltas(i);
	}
	// deltasV <- newsV - oldsV,newV <- empty set
	for(int i = 0;i < compset.getSize();++i) {
		bool newEmpty = compset.newEmpty(i);
		if(!newEmpty) {
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
	ContainersToMerge *containers;
	if(c.getDatastructure() == ARRAY)
		containers = new myarray::ArraysToMerge();
	else if(c.getDatastructure() == LIST)
		containers = new mylist::ListsToMerge();
	else
		containers = new myarray2::ArraysToMerge();

	getEdgesToMerge(index,compset,oldEmpty,deltaEmpty,*containers,c);	// find new edges to containers
	containers->merge();	// merge and sort edges,remove duplicate edges.
	newEdgesNum = containers->getNumEdges();
	if(newEdgesNum)
		compset.setNews(index,newEdgesNum,containers->getEdgesFirstAddr(),containers->getLabelsFirstAddr());
	else
		compset.clearNews(index);	

	containers->clear(); delete containers;
	return newEdgesNum;
}

void Compute::getEdgesToMerge(vertexid_t index,ComputationSet &compset,bool oldEmpty,bool deltaEmpty,ContainersToMerge &containers,Context &c) {
	// add s-rule edges	
	if(!deltaEmpty) 	
		genS_RuleEdges(index,compset,containers,c);
	// add d-rule edges merge Ov of only Dv
	if(!oldEmpty) 
		genD_RuleEdges(index,compset,containers,c,true);
	// add d-rule edges merge Dv of (Ov and Dv)
	if(!deltaEmpty)
		genD_RuleEdges(index,compset,containers,c,false);
}

void Compute::genS_RuleEdges(vertexid_t index,ComputationSet &compset,ContainersToMerge &containers,Context &c) {
	
	vertexid_t numEdges = compset.getDeltasNumEdges(index);
	vertexid_t *edges = compset.getDeltasEdges(index);
	char *labels = compset.getDeltasLabels(index);

	char newLabel;
	bool added = false;
	for(vertexid_t i = 0;i < numEdges;++i) {
		newLabel = c.grammar.checkRules(labels[i]);
		if(newLabel != (char)127) {
			if(!added) { 
				containers.addOneContainer();
				added = true;
			}
			containers.addOneEdge(edges[i],newLabel);
		}
	}
}

void Compute::genD_RuleEdges(vertexid_t index,ComputationSet &compset,ContainersToMerge &containers,Context &c,bool isOld) {
	
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
		// if target vertice is in memory
		if(indexInCompset != -1) 
			checkEdges(indexInCompset,labels[i],compset,containers,c,isOld);
	}	
}

void Compute::checkEdges(vertexid_t dstInd,char dstVal,ComputationSet &compset,ContainersToMerge &containers,Context &c,bool isOld) {
	
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
				containers.addOneContainer();
				added = true;
			}	
			containers.addOneEdge(edges[i],newVal);
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
					containers.addOneContainer();
					added = true;
				}	
				containers.addOneEdge(edges[i],newVal);
			}
		}
	}
}

void Compute::updatePartitions(ComputationSet &compset,Partition &p,Partition &q,bool isFinished,Context &c) {
	// update partition p and q
	if(oneRoundEdges) { 
		updateSinglePartition(compset,p,isFinished,c,true);
		updateSinglePartition(compset,q,isFinished,c,false);
	}
	// update DDM
	adjustDDM(p,q,isFinished,c); 
}

void Compute::updateSinglePartition(ComputationSet &compset,Partition &p,bool isFinished,Context &c,bool isP) {
	if(!oneRoundEdges) return;
	
	vertexid_t numVertices,offset;
	long numEdges;
	long realNumEdges = 0;
	vertexid_t *edges; char *labels; long *addr; vertexid_t *index;
	if(isP) {
		numVertices = compset.getPsize();
		numEdges = compset.getPNumEdges();
		offset = 0;
	}
	else {
		numVertices = compset.getQsize();
		numEdges = compset.getQNumEdges();
		offset = compset.getPsize();
	}
	edges = new vertexid_t[numEdges];
	labels = new char[numEdges];
	addr = new long[numVertices];
	index = new vertexid_t[numVertices];

	// new edges only in Ov
	if(isFinished) {
		long cur_addr = 0;	
		for(vertexid_t i = 0;i < numVertices;++i) {
			addr[i] = cur_addr;
			index[i] = compset.getOldsNumEdges(i + offset);
			if(compset.getOldsNumEdges(i + offset)) {
				memcpy(edges + addr[i],compset.getOldsEdges(i + offset),sizeof(vertexid_t) * index[i]);
				memcpy(labels + addr[i],compset.getOldsLabels(i + offset),sizeof(char) * index[i]);
			}	
			cur_addr += index[i];
		}	
		realNumEdges = cur_addr;
	}
	// new edges in Ov and Dv
	else {
		memset(edges,0,sizeof(vertexid_t) * numEdges);
		memset(labels,0,sizeof(char) * numEdges);
		long cur_addr = 0;
		for(vertexid_t i = 0;i < numVertices;++i) {
			addr[i] = cur_addr;
			int n1 = compset.getOldsNumEdges(i + offset); int n2 = compset.getDeltasNumEdges(i + offset);
			vertexid_t *tmpEdges = new vertexid_t[n1 + n2];
			char *tmpLabels = new char[n1 + n2];
			myalgo::unionTwoArray(index[i],tmpEdges,tmpLabels,n1,compset.getOldsEdges(i + offset),compset.getOldsLabels(i + offset),n2,compset.getDeltasEdges(i + offset),compset.getDeltasLabels(i + offset));
			if(index[i]) {
				memcpy(edges + addr[i],tmpEdges,sizeof(vertexid_t) * index[i]);
				memcpy(labels + addr[i],tmpLabels,sizeof(char) * index[i]);
			}
			delete[] tmpEdges; delete[] tmpLabels;
			cur_addr += index[i];
			realNumEdges += index[i];
		}
	}
	p.update(numVertices,realNumEdges,edges,labels,addr,index);
	delete[] edges; delete[] labels; delete[] addr; delete[] index;
	c.vit.setDegree(p.getId(),realNumEdges);
}

void Compute::needRepart(Partition &p,Partition &q,bool &repart_p,bool &repart_q,bool isFinished,Context &c) {
	if(isFinished)
		repart_p = repart_q = false;
	else {
		long numPVertices = p.getNumVertices();
		long numQVertices = q.getNumVertices();
		if(numPVertices > 2 * numQVertices) {
			repart_p = true; repart_q = false;
		}
		else if(2 * numPVertices > numQVertices) { 
			repart_p = true; repart_q = true;
		}
		else { 
			repart_p = false; repart_q = true;	
		}
	}
}
