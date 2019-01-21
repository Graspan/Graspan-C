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
		inline char *getGrammarFile() {return grammarFile;}
		inline int getNumPartitions() {return numPartitions;}
		inline unsigned long long int getMemBudget() {return memBudget;}
		inline int getNumThreads() {return numThreads;}
};


#endif
