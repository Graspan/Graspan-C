#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include "compute.h"
#include "array/arraystomerge.h"
#include "../algorithm/myalgorithm.h"
#include "../context.h"
#include <malloc.h>

Compute::Compute() {
	oneRoundEdges = numAddedEdgesP = numAddedEdgesQ = totalAddedEdges = 0;
}

//TODO : modify this method.
void Compute::loadTwoPartition(Partition &p,Partition &q,partitionid_t new_pid,partitionid_t new_qid,Context &c) {
	partitionid_t old_pid = p.getId();	
	partitionid_t old_qid = q.getId();
	if(old_pid != new_pid) {
		if(old_pid != -1) {
			p.writeToFile(old_pid,c);
			p.clear();
		}
		if(new_pid == old_qid) {
			q.writeToFile(old_qid,c);
			q.clear();
		}	
		p.loadFromFile(new_pid,c);
	}
	if(old_qid != new_qid) {
		if(q.getId() != -1) {
			q.writeToFile(old_qid,c);
			q.clear();
		}
		q.loadFromFile(new_qid,c);
	}
}

bool Compute::scheduler(partitionid_t &p,partitionid_t &q,Context &c) {
	return c.ddm.scheduler(p,q);
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

	long oldNumEdges = c.vit.getTotalNumEdges();
	partitionid_t pid,qid;
 	int roundId = 0;
	Partition *p = new Partition();
	Partition *q = new Partition();
	while(scheduler(pid,qid,c)) {
		// load partition p,q from file or Memory
		loadTwoPartition(*p,*q,pid,qid,c);

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
		// update Partition p and q,adjust VIT
		updatePartitions(*compset,*p,*q,isFinished,c);
		cout << "totalAddedEdges: " << totalAddedEdges << endl;
		// return compset resources
		compset->clear(); delete compset;
		// repart p and q if necessary,update VIT and DDM
		repartAndUpdateDDM(*p,*q,c,isFinished);
		c.ddm.print();
	}
	if(p->getId() != -1) p->writeToFile(p->getId(),c);
	p->clear(); delete p;
	if(q->getId() != -1) q->writeToFile(q->getId(),c); 
	q->clear(); delete q;

	writeAllPartitionsToTxtFile(c);

	long newNumEdges = c.vit.getTotalNumEdges();
	return newNumEdges - oldNumEdges;	
}

void Compute::initComputationSet(ComputationSet &compset,Partition &p,Partition &q,Context &c) {
	compset.init(p,q,c);
}

long Compute::computeOneRound(ComputationSet &compset,Context &c,boost::asio::io_service &ioServ,bool &isFinished) {
	isFinished = true;
	numAddedEdgesP = numAddedEdgesQ = 0;
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
	if(oneRoundEdges) { 
		updateSinglePartition(compset,p,isFinished,c,true);
		updateSinglePartition(compset,q,isFinished,c,false);
	}
}

void Compute::updateSinglePartition(ComputationSet &compset,Partition &p,bool isFinished,Context &c,bool isP) {
	
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
	
	if(isP) {
		numAddedEdgesP = realNumEdges - p.getNumEdges();
		totalAddedEdges += numAddedEdgesP;
		if(numAddedEdgesP < 0) {
			cout << "error happened in updateParition P!" << endl;
			exit(-1);
		}	
	}
	else {
		numAddedEdgesQ = realNumEdges - p.getNumEdges();
		totalAddedEdges += numAddedEdgesQ;
		if(numAddedEdgesQ < 0) {
			cout << "error happened in updateParition Q! " << endl;
			exit(-1);
		}
	}
	p.update(numVertices,realNumEdges,edges,labels,addr,index);
	delete[] edges; delete[] labels; delete[] addr; delete[] index;
	c.vit.setDegree(p.getId(),realNumEdges);
}

void Compute::repartAndUpdateVIT(Partition &p,Context &c,bool &compIsFinished,int &numSubPartition,bool isP) {
	if(compIsFinished) return;

	partitionid_t pid = p.getId();
	long numEdges = p.getNumEdges();
	vertexid_t numVertices = p.getNumVertices();
	vertexid_t numRealVertices = p.getNumRealVertices();
	/* two partition used Memory: 5 * numEdges + 12 * numVertices
	 * compset used Memory: 0.55 * numEdges + 139 * numRealVertices (numRealVertices < numVertices)
	 * total used Memory: 5.55 * numEdges + 12 * numVertices + 139 * numRealVertices
	 */ 
	unsigned long int memoryBuf = c.getMemBudget() * 0.4;
	unsigned long int memory = (unsigned long int)139 * numRealVertices + (unsigned long int)12 * numVertices + 5.55 * (unsigned long int)numEdges;
	numSubPartition = (memory > memoryBuf) ? (memory / memoryBuf) : 1;
	if(numSubPartition) {
		vertexid_t oldStart = c.vit.getStart(pid);
		vertexid_t oldEnd = c.vit.getEnd(pid);
		vertexid_t segSize = (oldEnd - oldStart) / (numSubPartition + 1);
		numEdges = 0;
		for(vertexid_t i = 0;i < segSize;++i) {
			numEdges += p.getIndex(i);	
		}
		// update VIT[pid]
		c.vit.setVitValue(pid,oldStart,oldStart+segSize-1,numEdges);
		// update VIT[n],VIT[n+1],..,VIT[n+numSubPartition-1]
		for(partitionid_t i = 0;i < numSubPartition;++i) {
			vertexid_t v_start,v_end;
			v_start = oldStart + (i + 1) * segSize;
			v_end = (i != numSubPartition - 1) ? (v_start + segSize - 1) : oldEnd;
			numEdges = 0;
			for(vertexid_t i = v_start-oldStart;i <= v_end-oldStart;++i) {
				numEdges += p.getIndex(i);	
			}
			c.vit.add(v_start,v_end,numEdges);
			c.ddm.add();
		}
	}
}

void Compute::repartAndUpdateDDM(Partition &p,Partition &q,Context &c,bool &compIsFinished) {
	int numSubP = 0;
	int numSubQ = 0;
	int n = c.vit.getSize();
	partitionid_t pid = p.getId();
	partitionid_t qid = q.getId();

	// repart p and q if out of memory
	repartAndUpdateVIT(p,c,compIsFinished,numSubP,true);
	repartAndUpdateVIT(q,c,compIsFinished,numSubQ,false);
	
	// calculate DDM matrix
	if(!(numAddedEdgesP + numAddedEdgesQ) && compIsFinished) {
		c.ddm.setDDM(pid,pid,0);
		c.ddm.setDDM(pid,qid,0);
		c.ddm.setDDM(qid,pid,0);
		c.ddm.setDDM(qid,qid,0);
	}
	else {
		int size = c.vit.getSize();
		for(partitionid_t i = 0;i < size;++i) {
			if(i == pid || i == qid || (i >= n && i < n+numSubP) || (i >= n+numSubP && i < size)) { // if Parition pi in memory
					for(partitionid_t j = 0;j < size;++j)
					c.ddm.setDDM(i,j,0);		
			}
		}	

		for(partitionid_t i = 0;i < size;++i) {
			if(i == pid || (i >= n && i < n+numSubP)) {
				vertexid_t id_begin = c.vit.getStart(i) - c.vit.getStart(pid); 	
				vertexid_t id_end = c.vit.getEnd(i) - c.vit.getStart(pid);
				for(vertexid_t j = id_begin;j <= id_end;++j) {
					if(p.getIndex(j)) {
						vertexid_t *edges = p.getEdgesFirstAddr(j);	
						for(vertexid_t k = 0;k < p.getIndex(j);++k) {
							vertexid_t dstVid = *(edges + k);	
							partitionid_t dstPid = c.vit.getPartitionId(dstVid);
							if(dstPid == pid || (dstPid >= n && dstPid < n+numSubP)) {
								if(!compIsFinished)	{
									int degree = p.getIndex(dstVid - c.vit.getStart(pid));
									c.ddm.setDDM(i,dstPid,c.ddm.getDDM(i,dstPid) + degree);	
								}	
							}
							else if(dstPid == qid || (dstPid >= n+numSubP && dstPid < size)) {
								if(!compIsFinished) {
									int degree = q.getIndex(dstVid - c.vit.getStart(qid));
									c.ddm.setDDM(i,dstPid,c.ddm.getDDM(i,dstPid) + degree);
								}
							}
							else
								c.ddm.addDDM(i,dstPid);
						}

					}
				}
				for(partitionid_t j = 0;j < size;++j) {
					if(!(j == pid || j == qid || (j >= n && j < n+numSubP) || (j >= n+numSubP && j < size))) {
						long tmp = c.ddm.getDDM(i,j) * c.vit.getDegree(j) / (c.vit.getEnd(j) - c.vit.getStart(j) + 1);
						if(c.ddm.getDDM(i,j) && !tmp) tmp = 1;
						c.ddm.setDDM(i,j,tmp);
					}
				}	
			}		
			else if(i == qid || (i >= n+numSubP && i < size)) {
				vertexid_t id_begin = c.vit.getStart(i) - c.vit.getStart(qid);
				vertexid_t id_end = c.vit.getEnd(i) - c.vit.getStart(qid);
				for(vertexid_t j = id_begin;j <= id_end;++j) {
					if(q.getIndex(j)) {
						vertexid_t *edges = q.getEdgesFirstAddr(j);
						for(vertexid_t k = 0;k < q.getIndex(j);++k) {
							vertexid_t dstVid = *(edges + k);
							partitionid_t dstPid = c.vit.getPartitionId(dstVid);
							if(dstPid == pid || (dstPid >= n && dstPid < n+numSubP)) {
								if(!compIsFinished) {
									int degree = p.getIndex(dstVid - c.vit.getStart(pid));
									c.ddm.setDDM(i,dstPid,c.ddm.getDDM(i,dstPid) + degree);
								}	
							}
							else if(dstPid == qid || (dstPid >= n+numSubP && dstPid < size)) {
								if(!compIsFinished) {
									int degree = q.getIndex(dstVid - c.vit.getStart(qid));
									c.ddm.setDDM(i,dstPid,c.ddm.getDDM(i,dstPid) + degree);
								}	
							}
							else 
								c.ddm.addDDM(i,dstPid);	
						}
					}		
				}
				for(partitionid_t j = 0;j < size;++j) {
					if(!(j == pid || j == qid || (j >= n && j < n+numSubP) || (j >= n+numSubP && j < size))) {
						long tmp = c.ddm.getDDM(i,j) * c.vit.getDegree(j) / (c.vit.getEnd(j) - c.vit.getStart(j) + 1);
						if(c.ddm.getDDM(i,j) && !tmp) tmp = 1;
						c.ddm.setDDM(i,j,tmp);	
					}
				}
			}
			else { // Partition pi out of memory
				long oldValueP = c.ddm.getDDM(i,pid);
				long oldValueQ = c.ddm.getDDM(i,qid);
				long resP = oldValueP/(numSubP+1);
				long resQ = oldValueQ/(numSubQ+1);
				if(!resP && oldValueP) resP = 1;
				if(!resQ && oldValueQ) resQ = 1;
				for(partitionid_t j = 0;j < size;++j) {
					if(j == pid || (j >= n && j < n+numSubP))
						c.ddm.setDDM(i,j,resP);
					if(j == qid || (j >= n+numSubP && j < size))
						c.ddm.setDDM(i,j,resQ);	
				}
			}
		}
		// write all rePartitions to file
		if(numSubP) 
			writeRepartitionsToFile(p,c,n,n+numSubP);	
		if(numSubQ)
			writeRepartitionsToFile(q,c,n+numSubP,size);	
	}

}

void Compute::writeRepartitionsToFile(Partition &p,Context &c,partitionid_t p_start,partitionid_t p_end) {
	partitionid_t id = p.getId();
	vertexid_t v_start,v_end;
	vertexid_t offset = c.vit.getStart(id);
	// write all partitions to file
	for(partitionid_t i = 0;i < c.vit.getSize();++i) {
		if(i == id || (i >= p_start && i < p_end)) {
			char filename[256];
			sprintf(filename,"%d.part",i);
			FILE *f = fopen(filename,"wb");
			if(f == NULL) {
				cout << "can't open file: " << filename << endl;
				exit(-1);
			}
			v_start = c.vit.getStart(i);
			v_end = c.vit.getEnd(i);

			for(vertexid_t j = v_start;j <= v_end;++j) {
				vertexid_t pos = j - offset;
				vertexid_t degree = p.getIndex(pos);
				if(!degree)
					continue;
				fwrite((const void*)& j,sizeof(vertexid_t),1,f);
				fwrite((const void*)& degree,sizeof(vertexid_t),1,f);
				vertexid_t *edges = p.getEdgesFirstAddr(pos);
				char *labels = p.getLabelsFirstAddr(pos);
				for(vertexid_t k = 0;k < degree;++k) {
					fwrite((const void*)& *(edges + k),sizeof(vertexid_t),1,f);
					fwrite((const void*)& *(labels + k),sizeof(char),1,f);
				}
			}
			fclose(f);
		}	
	}
	p.clear();
}

void Compute::writeAllPartitionsToTxtFile(Context &c) {
	char finalFile[256] = "HELLO.txt";
	sprintf(finalFile,"%s.output",c.getGraphFile());
	FILE *fp = fopen(finalFile,"w+");
	cout << "finalFile: " << finalFile << endl;
	if(!fp) {
		cout << "can't write to file: " << finalFile << endl;
		exit(-1);		
	}	

	for(int i = 0;i < c.vit.getSize();++i) {
		char partFile[256];
		sprintf(partFile,"%d.part",i);
		FILE *f = fopen(partFile,"rb");
		if(!f) {
			cout << "can't load partition file: " << partFile << endl;
			exit(-1);
		}
		vertexid_t src,dst,degree;
		char label;
		size_t freadRes = 0; //clear warnings

		while(fread(&src,sizeof(vertexid_t),1,f) != 0) {
			freadRes = fread(&degree,sizeof(vertexid_t),1,f);
			int bufsize = (sizeof(vertexid_t) + sizeof(char)) * degree;
			char *buf = (char*)malloc(bufsize);
			freadRes = fread(buf,bufsize,1,f);
			for(vertexid_t i = 0;i < bufsize;i += 5) {
				dst = *((vertexid_t*)(buf + i));
				label = *((char*)(buf + 4 + i));
				char *rawLabel = c.grammar.getRawLabel(label);
				fprintf(fp,"%d\t%d\t%s\n",src,dst,rawLabel);
			}
			free(buf);
		}
		fclose(f);
	}
	fclose(fp);
}
