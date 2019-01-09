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

		char *getGraphFile();
		char *getGrammarFile();
		int getNumPartitions();
		unsigned long long int getMemBudget();
		int getNumThreads();
};


#endif
