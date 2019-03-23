#include "ddm.h"
#include "../common.h"

DDM::DDM() {
	numPartitions = 0;
	for(int i = 0;i < DDM_SIZE;++i) {
		for(int j = 0;j < DDM_SIZE;++j)	{
			ddm[i][j] = 0;
		}		
	}	
}

bool DDM::scheduler(partitionid_t &p,partitionid_t &q) {
	p = q = -1;
	long maxValue = 0;
	for(partitionid_t i = 0;i < numPartitions;++i) {
		for(partitionid_t j = i+1;j < numPartitions;++j) {
			if(i != j) {
				long value = ddm[i][j] + ddm[j][i] + ddm[i][i] + ddm[j][j];
				if(value > maxValue) {	
					p = i;
					q = j;
					maxValue = value;
				}
			}			
		}	
	}
	if(maxValue)
		return true;
	else
		return false;
}

void DDM::print() {
	cout << "=====DDM Table=====" << endl;
	cout << "numPartitions: " << numPartitions << endl;
	for(int i = 0;i < numPartitions;++i) {
		for(int j = 0;j < numPartitions;++j)
			cout << ddm[i][j] << " ";
		cout << endl;
	}
}

