#include "ddm.h"
#include "../common.h"

DDM::DDM() {
	numPartitions = 0;
	for(int i = 0;i < DDM_SIZE;++i)
		for(int j = 0;j < DDM_SIZE;++j)	
			matrix[i][j] = 1;
}

void DDM::adjust(partitionid_t p,bool isNewp,partitionid_t q,bool isNewq,bool isFinished) {
	if(isNewp) {
		for(int i = 0;i < numPartitions;++i) {
			if(i != p)	
				matrix[i][p] = matrix[p][i] = 1;			
		}			
	}
	if(isNewq) {
		for(int i = 0;i < numPartitions;++i) {
			if(i != q)
				matrix[i][q] = matrix[q][i] = 1;	
		}	
	}
	if(isFinished)
		matrix[p][q] = matrix[q][p] = 0;
}

bool DDM::scheduler(partitionid_t &p,partitionid_t &q) {
	p = q = -1;
	for(int i = 0;i < numPartitions;++i) {
		for(int j = 0;j < numPartitions;++j) {
			if(i != j) {
				if(matrix[i][j] == 1)
				{
					p = i; q = j;
					return true;
				}	
			}		
		}			
	}	
	return false;
}

void DDM::print() {
	cout << "=====DDM Table=====" << endl;
	cout << "numPartitions: " << numPartitions << endl;
	for(int i = 0;i < numPartitions;++i) {
		for(int j = 0;j < numPartitions;++j) {
			cout << matrix[i][j] << " ";			
		}	
		cout << endl;
	}
	cout << endl;	
}

