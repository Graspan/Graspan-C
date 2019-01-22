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
		if(News) {
			for(int i = 0;i < size;++i)
				News[i].clear();
			delete[] News;
			News = NULL;
		}
		size = 0;
	}	
}

void ComputationSet::print() {
	for(int i = 0;i < size;++i) {
	//cout << "Old " << i;
	//Olds[i].print();
	cout << "Deltas: " << i;
	Deltas[i].print();
	//	News[i].print();
	}	
}

void ComputationSet::init(Partition &p,Partition &q,Context &c) {
	
	psize = p.getNumVertices();
	qsize = q.getNumVertices();
	size = psize + qsize;
	Olds = new EdgeArray[size];
	Deltas = new EdgeArray[size];
	News = new EdgeArray[size];

	/* Combine the vertices of p and q into V
	 * Combine the edge lists of p and q into E
	 * for each edge list v:(e1,e2,...,en) in E do
	 * 		set Ov to ()
	 * 		set Dv to (e1,e2,...,en)
	 */
	for(int i = 0;i < psize;++i) {
		Olds[i] = EdgeArray();
		News[i] = EdgeArray();
		if(p.getlndex(i)) {
			Deltas[i] = EdgeArray(p.getlndex(i),p.getEdgesFirstAddr(i),p.getLabelsFirstAddr(i));		
		}
		else {
			Deltas[i] = EdgeArray();	
		}
	}

	for(int i = 0;i < qsize;++i) {
	 	Olds[psize + i] = EdgeArray();
		News[psize + i] = EdgeArray();
		if(q.getlndex(i)) {
			Deltas[psize + i] = EdgeArray(q.getlndex(i),q.getEdgesFirstAddr(i),q.getLabelsFirstAddr(i));	
		}
		else {
			Deltas[psize + i] = EdgeArray();	
		}
	}
	
	interval.pFirstVid = c.vit.getStart(p.getId()); interval.pLastVid = interval.pFirstVid + psize - 1;
	interval.qFirstVid = c.vit.getStart(q.getId()); interval.qLastVid = interval.qFirstVid + qsize - 1;
	interval.pFirstIndex = 0; interval.pLastIndex = psize - 1;
	interval.qFirstIndex = psize; interval.qLastIndex = size - 1;

	/*
	cout << "pFirstVid: " << interval.pFirstVid << endl;
	cout << "pLastVid: " << interval.pLastVid << endl;
	cout << "pFirstIndex: " << interval.pFirstIndex << endl;
	cout << "pLastIndex: " << interval.pLastIndex << endl;

	cout << "qFirstVid: " << interval.qFirstVid << endl;
	cout << "qLastVid: " << interval.qLastVid << endl;
	cout << "qFirstIndex: " << interval.qFirstIndex << endl;
	cout << "qLastIndex: " << interval.qLastIndex << endl;
	*/

	/*
	for(int i = 0;i < size;++i) {
		Olds[i].print();
		Deltas[i].print();
		News[i].print();
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

vertexid_t ComputationSet::getOldsTotalNumEdges() {
	vertexid_t num = 0;
	for(int i = 0;i < size;++i)
		num += getOldsNumEdges(i);
	return num;
}

vertexid_t ComputationSet::getDeltasTotalNumEdges() {
	vertexid_t num = 0;
	for(int i = 0;i < size;++i)
		num += getDeltasNumEdges(i);		
	return num;
}

vertexid_t ComputationSet::getNewsTotalNumEdges() {
	vertexid_t num = 0;
	for(int i = 0;i < size;++i)
		num += getNewsNumEdges(i);
	return num;
}

void ComputationSet::setOlds(vertexid_t index,int numEdges,vertexid_t *edges,char *labels) {
	Olds[index].set(numEdges,edges,labels);
}

void ComputationSet::setDeltas(vertexid_t index,int numEdges,vertexid_t *edges,char *labels) {
	Deltas[index].set(numEdges,edges,labels);	
}

void ComputationSet::setNews(vertexid_t index,int numEdges,vertexid_t *edges,char *labels) {
	News[index].set(numEdges,edges,labels);
}

void ComputationSet::clearOlds(vertexid_t index) {
	Olds[index].clear();	
}

void ComputationSet::clearDeltas(vertexid_t index) {
	Deltas[index].clear();	
}

void ComputationSet::clearNews(vertexid_t index) {
	News[index].clear();	
}

vertexid_t ComputationSet::getPNumEdges() {
	vertexid_t res = 0;
	for(int i = 0;i < psize;++i)
		res += getOldsNumEdges(i);
	return res;
}

vertexid_t ComputationSet::getQNumEdges() {
	vertexid_t res = 0;
	for(int i = psize;i < size;++i) {
		res += getOldsNumEdges(i);	
	}
	return res;
}

}


