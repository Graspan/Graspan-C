#include "context.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
using std::cout;
using std::endl;

Context::Context(int argc, char **argv) 
{
	if(argc != 6)
	{
		cout << "usage: [graph_file] [grammar_file] [num_partitions] [memory_budget] [num_threads]"	<< endl;
		exit(0);		
	}
	else
	{
		strcpy(graphFile,argv[1]);
		strcpy(grammarFile,argv[2]);
		numPartitions = atoi(argv[3]);
		memBudget = (unsigned long long int)atoi(argv[4]) * (unsigned long long int)GB;
		numThreads = atoi(argv[5]);		
		cout << graphFile << " " << grammarFile << " " << numPartitions << " " << memBudget << " " << numThreads << endl;
	}
}

char* Context::getGraphFile() {
	return graphFile;
}

char* Context::getGrammarFile() {
	return grammarFile;	
}

int Context::getNumPartitions() {
	return numPartitions;	
}

unsigned long long int Context::getMemBudget() {
	return memBudget;
}

int Context::getNumThreads() {
	return numThreads;	
}
