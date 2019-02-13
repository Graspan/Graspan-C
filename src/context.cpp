#include "context.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

Context::Context(int argc, char **argv) {
	if(argc != 7) {
		cout << "usage: [graph_file] [grammar_file] [num_partitions] [memory_budget] [num_threads] [datastructure]"	<< endl;
		exit(0);		
	}
	else {
		strcpy(graphFile,argv[1]);
		strcpy(grammarFile,argv[2]);
		numPartitions = atoi(argv[3]);
		memBudget = (unsigned long int)atoi(argv[4]) * (unsigned long int)GB;
		numThreads = atoi(argv[5]);		
		if(!strcmp(argv[6],"list")) {
			datastructure = LIST;
			cout << graphFile << " " << grammarFile << " " << numPartitions << " " << memBudget << " " << numThreads << " list" << endl;
		}
		else {
			datastructure = ARRAY;
			cout << graphFile << " " << grammarFile << " " << numPartitions << " " << memBudget << " " << numThreads << " array" << endl;
		}	
	}
}

void Context::clear() {
	vit.clear();	
}
