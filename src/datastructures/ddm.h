#ifndef DDM_H
#define DDM_H
#include "../common.h"
#define DDM_SIZE 256

class DDM {
	private:
		long ddm[DDM_SIZE][DDM_SIZE];			
		int numPartitions;

	public:
		DDM();
		// getters and setters
		inline void setNumPartitions(int numPartitions) {this->numPartitions = numPartitions;}
		inline void addDDM(partitionid_t pid,partitionid_t qid) {++ddm[pid][qid];}	
		inline void setDDM(partitionid_t pid,partitionid_t qid,long value) {ddm[pid][qid] = value;}	
		inline long getDDM(partitionid_t pid,partitionid_t qid) {return ddm[pid][qid];}
		inline void add(){++numPartitions;}

		bool scheduler(partitionid_t &p,partitionid_t &q);
		void print();			
};

#endif
