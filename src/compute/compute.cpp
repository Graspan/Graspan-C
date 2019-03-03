#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include "compute.h"
#include "array/arraystomerge.h"
#include "../algorithm/myalgorithm.h"
#include "../context.h"

Compute::Compute() {
	newTotalEdges = 0;	
	isNewp = isNewq = false;
}

bool Compute::scheduler(partitionid_t &p,partitionid_t &q,Context &c) {
	return c.ddm.scheduler(p,q);
}

void Compute::adjustDDM(partitionid_t p,bool isNewp,partitionid_t q,bool isNewq,bool isFinished,Context &c) {
	c.ddm.adjust(p,isNewp,q,isNewq,isFinished);	
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
		cout << "USED MEMORY: " << myalgo::getUsedMemory(getpid()) << endl;	
		// load partition p,q from file
		Partition *p = new Partition();
		Partition *q = new Partition();
		p->loadFromFile(pid,c);
		q->loadFromFile(qid,c);

		cout << "=====STARTING ROUND" << roundId++  << "=====" << endl;
		cout << "P = " << pid << " , Q = " << qid << endl;  
		// check partitions
		if(p->check() || q->check()) {
			cout << "partition p or q duplication happened!" << endl;
			exit(-1);
		}
		// init compset
		unsigned long int m1 = myalgo::getUsedMemory(getpid());
		ComputationSet *compset = new ComputationSet();
		initComputationSet(*compset,*p,*q,c);
		unsigned long int m2 = myalgo::getUsedMemory(getpid());
		cout << "compset memory: " << (m2 - m1) << endl;

		// compute one round 
		bool isFinished = false;
		newTotalEdges += computeOneRound(*compset,c,ioServ,isFinished);
		// update Partitions and adjust VIT and DDM
		updatePartitions(*compset,*p,*q,isFinished,c);

		m1 = myalgo::getUsedMemory(getpid());
		compset->clear(); delete compset;
		m2 = myalgo::getUsedMemory(getpid());
		cout << "return compset memory: " << (m1 - m2) << endl;

		// repart if out of memory,adjust VIT and DDM
		bool repartP = false; bool repartQ = false; 
		Partition *p_2 = new Partition();
		Partition *q_2 = new Partition();
		needRepart(*p,*q,repartP,repartQ,isFinished,c);

		if(repartP) p->repart(*p_2,c);
		if(repartQ) q->repart(*q_2,c);
		int value = (isFinished == true) ? 0 : 1; 
		if(repartP) {
			c.ddm.setValue(pid,p_2->getId(),value);	
			c.ddm.setValue(qid,p_2->getId(),value);	
			if(repartQ) {
				c.ddm.setValue(pid,q_2->getId(),value);
				c.ddm.setValue(qid,q_2->getId(),value);
				c.ddm.setValue(p_2->getId(),q_2->getId(),value);
			}
		}
		else {
			if(repartQ) {
				c.ddm.setValue(pid,q_2->getId(),value);
				c.ddm.setValue(qid,q_2->getId(),value);
			}
		}
		// check repart partitions
		if(p_2->check() || q_2->check()) {
			cout << "REPA p_2 or q_2 duplication happened!" << endl;
			exit(-1);
		}
		// write Partitions to File
		m1 = myalgo::getUsedMemory(getpid());
		if(repartP) {p_2->writeToFile(p_2->getId(),c);} //p_2->print(c);}
		p_2->clear(); delete p_2;
		if(repartQ) {q_2->writeToFile(q_2->getId(),c);} //q_2->print(c);}
		q_2->clear(); delete q_2;
		m2 = myalgo::getUsedMemory(getpid());
		cout << "return p_2 and q_2 memory: " << (m1 - m2) << endl;

		m1 = myalgo::getUsedMemory(getpid());
		p->writeToFile(p->getId(),c); p->clear(); delete p;
		q->writeToFile(q->getId(),c); q->clear(); delete q;
		m2 = myalgo::getUsedMemory(getpid());
		cout << "return p and q memory: " << (m1 - m2) << endl;
	}
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
			cout << "MEMORY IS OUT! " << endl;
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
	numFinished = 0;
	compFinished = false; 
	int compSize = compset.getSize();

	long thisIterEdges = 0;
	for(int i = 0;i < nSegs;++i) {
		lower = i * segsize;
		upper = (lower + segsize < compSize) ? lower + segsize : compSize;
		ioServ.post(boost::bind(&Compute::runUpdates,this,lower,upper,nSegs,compset,c));
	}
	std::unique_lock<std::mutex> lck(comp_mtx);
	while (!compFinished) cv.wait(lck);
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

	if(c.getDatastructure() == ARRAY) {
		myarray::ArraysToMerge arrays;
		getEdgesToMerge(index,compset,oldEmpty,deltaEmpty,arrays,c);	// find new edges to arrays.
		arrays.mergeAndSort(); // merge and sort edges,remove duplicate edges.
		// add edges to News
		newEdgesNum = arrays.getNumEdges(); 
		if(newEdgesNum)
			compset.setNews(index,newEdgesNum,arrays.getEdgesFirstAddr(),arrays.getLabelsFirstAddr());
		else
			compset.clearNews(index);
		arrays.clear();
	}
	else if(c.getDatastructure() == LIST) {
		mylist::ListsToMerge lists;
		getEdgesToMerge(index,compset,oldEmpty,deltaEmpty,lists,c);
		lists.mergeAndSort();
		newEdgesNum = lists.getNumEdges();
		if(newEdgesNum)
			compset.setNews(index,newEdgesNum,lists.getEdgesFirstAddr(),lists.getLabelsFirstAddr());	
		else
			compset.clearNews(index);
		lists.clear();
	}
	else {
		myarray2::ArraysToMerge arrays;
		getEdgesToMerge(index,compset,oldEmpty,deltaEmpty,arrays,c);
		arrays.merge();
		newEdgesNum = arrays.getNumEdges();
		if(newEdgesNum)
			compset.setNews(index,newEdgesNum,arrays.getEdgesFirstAddr(),arrays.getLabelsFirstAddr());
		else
			compset.clearNews(index);
		arrays.clear();
	}
	return newEdgesNum;	
}

void Compute::getEdgesToMerge(vertexid_t index,ComputationSet &compset,bool oldEmpty,bool deltaEmpty,myarray::ArraysToMerge &arrays,Context &c) {
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

void Compute::getEdgesToMerge(vertexid_t index,ComputationSet &compset,bool oldEmpty,bool deltaEmpty,myarray2::ArraysToMerge &arrays,Context &c) {
	if(!deltaEmpty) 	
		genS_RuleEdges(index,compset,arrays,c);
	if(!oldEmpty) 
		genD_RuleEdges(index,compset,arrays,c,true);
	if(!deltaEmpty)
		genD_RuleEdges(index,compset,arrays,c,false);
}

void Compute::getEdgesToMerge(vertexid_t index,ComputationSet &compset,bool oldEmpty,bool deltaEmpty,mylist::ListsToMerge &lists,Context &c) {
	if(!deltaEmpty)
		genS_RuleEdges(index,compset,lists,c);
	if(!oldEmpty)
		genD_RuleEdges(index,compset,lists,c,true);
	if(!deltaEmpty)
		genD_RuleEdges(index,compset,lists,c,false);	
}

void Compute::genS_RuleEdges(vertexid_t index,ComputationSet &compset,myarray::ArraysToMerge &arrays,Context &c) {
	
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

void Compute::genS_RuleEdges(vertexid_t index,ComputationSet &compset,myarray2::ArraysToMerge &arrays,Context &c) {
	
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

void Compute::genS_RuleEdges(vertexid_t index,ComputationSet &compset,mylist::ListsToMerge &lists,Context &c) {
	
	vertexid_t numEdges = compset.getDeltasNumEdges(index);
	vertexid_t *edges = compset.getDeltasEdges(index);
	char *labels = compset.getDeltasLabels(index);

	char newLabel;
	bool added = false;
	for(vertexid_t i = 0;i < numEdges;++i) {
		newLabel = c.grammar.checkRules(labels[i]);
		if(newLabel != (char)127) {
			if(!added) { 
				lists.addOneList();
				added = true;
			}
			lists.addOneEdge(edges[i],newLabel);
		}
	}
}

void Compute::genD_RuleEdges(vertexid_t index,ComputationSet &compset,myarray::ArraysToMerge &arrays,Context &c,bool isOld) {
	
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
			checkEdges(indexInCompset,labels[i],compset,arrays,c,isOld);
	}	
}

void Compute::genD_RuleEdges(vertexid_t index,ComputationSet &compset,myarray2::ArraysToMerge &arrays,Context &c,bool isOld) {
	
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
		if(indexInCompset != -1) 
			checkEdges(indexInCompset,labels[i],compset,arrays,c,isOld);
	}	
}

void Compute::genD_RuleEdges(vertexid_t index,ComputationSet &compset,mylist::ListsToMerge &lists,Context &c,bool isOld) {
	
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
			checkEdges(indexInCompset,labels[i],compset,lists,c,isOld);
	}	
}

void Compute::checkEdges(vertexid_t dstInd,char dstVal,ComputationSet &compset,myarray::ArraysToMerge &arrays,Context &c,bool isOld) {
	
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
			}
		}
	}
}

void Compute::checkEdges(vertexid_t dstInd,char dstVal,ComputationSet &compset,myarray2::ArraysToMerge &arrays,Context &c,bool isOld) {
	
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
			}
		}
	}
}

void Compute::checkEdges(vertexid_t dstInd,char dstVal,ComputationSet &compset,mylist::ListsToMerge &lists,Context &c,bool isOld) {
	
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
				lists.addOneList();
				added = true;
			}	
			lists.addOneEdge(edges[i],newVal);
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
					lists.addOneList();
					added = true;
				}	
				lists.addOneEdge(edges[i],newVal);
			}
		}
	}
}

void Compute::updatePartitions(ComputationSet &compset,Partition &p,Partition &q,bool isFinished,Context &c) {
	// update partition p and q
	isNewp = isNewq = false;
	updateSinglePartition(compset,p,isFinished,c,true);
	updateSinglePartition(compset,q,isFinished,c,false);
	// update DDM
	adjustDDM(p.getId(),isNewp,q.getId(),isNewq,isFinished,c); 
}

void Compute::updateSinglePartition(ComputationSet &compset,Partition &p,bool isFinished,Context &c,bool isP) {
	vertexid_t numVertices,offset;
	long numEdges;
	long realNumEdges = 0;
	vertexid_t *edges; char *labels; vertexid_t *addr; vertexid_t *index;
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
	if(numEdges > c.vit.getDegree(p.getId())) {
		if(isP) 
			isNewp = true;
		else
			isNewq = true;
		edges = new vertexid_t[numEdges];
		labels = new char[numEdges];
		addr = new vertexid_t[numVertices];
		index = new vertexid_t[numVertices];

		// new edges only in Ov
		if(isFinished) {
			vertexid_t cur_addr = 0;	
			for(vertexid_t i = 0;i < numVertices;++i) {
				addr[i] = cur_addr;
				index[i] = compset.getOldsNumEdges(i + offset);
				if(compset.getOldsNumEdges(i + offset)) {
					memcpy(edges + addr[i],compset.getOldsEdges(i + offset),sizeof(vertexid_t) * index[i]);
					memcpy(labels + addr[i],compset.getOldsLabels(i + offset),sizeof(char) * index[i]);
				}	
				cur_addr += index[i];
			}	
			realNumEdges = numEdges;
		}
		// new edges in Ov and Dv
		else {
			memset(edges,0,sizeof(vertexid_t) * numEdges);
			memset(labels,0,sizeof(char) * numEdges);
			vertexid_t cur_addr = 0;
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
}

void Compute::needRepart(Partition &p,Partition &q,bool &repart_p,bool &repart_q,bool isFinished,Context &c) {
	if(isFinished)
		repart_p = repart_q = false;
	else
		repart_p = repart_q = true;	
}
