#include "computationset.h"

namespace myarray {

ComputationSet::ComputationSet() {
	psize = qsize = size = 0;				
}

void ComputationSet::clear() {
	if(size) {
		if(Olds) {
			for(int i = 0;i < size;++i)	
				Olds[i].clear();	
			delete[] Olds;
			Olds = NULL;
		}			
		if(Deltas) {
			for(int i = 0;i < size;++i)
				Deltas[i].clear();
			delete[] Deltas;
			Deltas = NULL;
		}
		if(Alls) {
			for(int i = 0;i < size;++i)
				Alls[i].clear();
			delete[] Alls;
			Alls = NULL;
		}
		size = 0;
	}	
}

void ComputationSet::init(Partition &p,Partition &q,Context &c) {
	
	psize = p.getNumVertices();
	qsize = q.getNumVertices();
	size = psize + qsize;
	Olds = new EdgeArray[size];
	Deltas = new EdgeArray[size];
	Alls = new EdgeArray[size];

	/* Combine the vertices of p and q into V
	 * Combine the edge lists of p and q into E
	 * for each edge list v:(e1,e2,...,en) in E do
	 * 		set Ov to ()
	 * 		set Dv to (e1,e2,...,en)
	 */
	for(int i = 0;i < psize;++i) {
		Olds[i] = EdgeArray();
		if(p.getlndex(i)) {
			Deltas[i] = EdgeArray(p.getlndex(i),p.getEdgesFirstAddr(i),p.getLabelsFirstAddr(i));		
			Alls[i] = EdgeArray(p.getlndex(i),p.getEdgesFirstAddr(i),p.getLabelsFirstAddr(i));
		}
		else {
			Deltas[i] = EdgeArray();	
			Alls[i] = EdgeArray();
		}
	}

	for(int i = 0;i < qsize;++i) {
	 	Olds[psize + i] = EdgeArray();
		if(q.getlndex(i)) {
			Deltas[psize + i] = EdgeArray(q.getlndex(i),q.getEdgesFirstAddr(i),q.getLabelsFirstAddr(i));	
			Alls[psize + i] = EdgeArray(q.getlndex(i),q.getEdgesFirstAddr(i),q.getLabelsFirstAddr(i));	
		}
		else {
			Deltas[psize + i] = EdgeArray();	
			Alls[psize + i] = EdgeArray();	
		}
	}
	
	interval.pFirstVid = c.vit.getStart(p.getId()); interval.pLastVid = interval.pFirstVid + psize + 1;
	interval.qFirstVid = c.vit.getStart(q.getId()); interval.qLastVid = interval.qFirstVid + qsize + 1;
	interval.pFirstIndex = 0; interval.pLastIndex = psize - 1;
	interval.qFirstIndex = psize; interval.qLastIndex = size - 1;

	/*
	for(int i = 0;i < size;++i) {
		Olds[i].print();
		Deltas[i].print();
		Alls[i].print();
	}
	*/
}

vertexid_t ComputationSet::getIndexInCompSet(vertexid_t vid) {
	if(vid >= interval.pFirstVid && vid <= interval.pLastVid) {
		return interval.pFirstIndex + (vid - interval.pFirstVid);	
	}
	else if(vid >= interval.qFirstVid && vid <= interval.qLastVid) {
		return interval.qFirstIndex + (vid - interval.qFirstVid);	
	}
	else {
		return -1;	// is out of memory
	}
}

}
