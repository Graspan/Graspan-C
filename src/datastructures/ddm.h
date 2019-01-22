#ifndef DDM_H
#define DDM_H
#include "../common.h"
#define DDM_SIZE 25

class DDM {
	private:
		int matrix[DDM_SIZE][DDM_SIZE];
		int numPartitions;

	public:
		DDM();
		inline void setNumPartitions(int numPartitions) {this->numPartitions = numPartitions;}
		void adjust(partitionid_t p,bool isNewp,partitionid_t q,bool isNewq);
		bool scheduler(partitionid_t &p,partitionid_t &q);
		void print();			
};

#endif
