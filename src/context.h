#ifndef CONTEXT_H
#define CONTEXT_H
#include "common.h"
#include "datastructures/grammar.h"
#include "datastructures/vit.h"

class Context
{
	private:		
		char graphFile[BUFFER_SIZE];
		char grammarFile[BUFFER_SIZE];
		int numPartitions;
		unsigned long long int memBudget;
		int numThreads;

	public:
		Grammar grammar;
		Vit vit;

		Context(int argc,char **argv);
		void clear();

		inline char *getGraphFile() {return graphFile;}
		char *getGrammarFile() {return grammarFile;}
		int getNumPartitions() {return numPartitions;}
		unsigned long long int getMemBudget() {return memBudget;}
		int getNumThreads() {return numThreads;}
};


#endif
