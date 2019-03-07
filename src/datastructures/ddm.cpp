#include "ddm.h"
#include "../common.h"

DDM::DDM() {
	numPartitions = 0;
	for(int i = 0;i < DDM_SIZE;++i) {
		for(int j = 0;j < DDM_SIZE;++j)	{
			needCalc[i][j] = true;
			ddm[i][j] = 0;
		}		
	}	
}

bool DDM::scheduler(partitionid_t &p,partitionid_t &q) {
	p = q = -1;
	for(int i = 0;i < numPartitions;++i) {
		for(int j = i+1;j < numPartitions;++j) {
			if(i != j) {
				if(needCalc[i][j] == true) {
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
			cout << needCalc[i][j] << " ";			
		}	
		cout << endl;
	}
	cout << endl;	
	for(int i = 0;i < numPartitions;++i) {
		for(int j = 0;j < numPartitions;++j)
			cout << ddm[i][j] << " ";
		cout << endl;
	}
}

