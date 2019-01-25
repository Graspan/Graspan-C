#ifndef DDM_H
#define DDM_H
#include "../common.h"
#define DDM_SIZE 100

class DDM {
	private:
		int matrix[DDM_SIZE][DDM_SIZE];
		int numPartitions;

	public:
		DDM();
		// getters and setters
		inline void setNumPartitions(int numPartitions) {this->numPartitions = numPartitions;}
		inline void setValue(partitionid_t p,partitionid_t q,int value) {matrix[p][q] = matrix[q][p] = value;}
		inline void add(){++numPartitions;}

		void adjust(partitionid_t p,bool isNewp,partitionid_t q,bool isNewq,bool isFinished);
		bool scheduler(partitionid_t &p,partitionid_t &q);
		void print();			
};

#endif
