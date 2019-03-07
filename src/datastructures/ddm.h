#ifndef DDM_H
#define DDM_H
#include "../common.h"
#define DDM_SIZE 256

class DDM {
	private:
		bool needCalc[DDM_SIZE][DDM_SIZE];	// if p and q need computation, needCalc[p][q] = needCalc[q][p] = true
		long ddm[DDM_SIZE][DDM_SIZE];			
		int numPartitions;

	public:
		DDM();
		// getters and setters
		inline void setNumPartitions(int numPartitions) {this->numPartitions = numPartitions;}
		inline void addDDM(partitionid_t pid,partitionid_t qid) {++ddm[pid][qid];}	
		inline void setNeedCalc(partitionid_t p,partitionid_t q,int value) {needCalc[p][q] = needCalc[q][p] = value;}
		inline void add(){++numPartitions;}

		void adjust(partitionid_t p,bool isNewp,partitionid_t q,bool isNewq,bool isFinished);
		bool scheduler(partitionid_t &p,partitionid_t &q);
		void print();			
};

#endif
