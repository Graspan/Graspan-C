#ifndef CONTEXT_H
#define CONTEXT_H
#include "common.h"
#include "datastructures/grammar.h"
#include "datastructures/vit.h"
#include "datastructures/ddm.h"

class Context
{
	private:		
		char graphFile[BUFFER_SIZE];
		char grammarFile[BUFFER_SIZE];
		int numPartitions;
		unsigned long int memBudget;
		int numThreads;

	public:
		Grammar grammar;
		Vit vit;
		DDM ddm;

		Context(int argc,char **argv);
		void clear();

		// getters and setters
		inline char *getGraphFile() {return graphFile;}
		inline char *getGrammarFile() {return grammarFile;}
		inline int getNumPartitions() {return numPartitions;}
		inline unsigned long int getMemBudget() {return memBudget;}
		inline int getNumThreads() {return numThreads;}

		inline void setNumPartitions(int numPartitions) {this->numPartitions = numPartitions;}
};


#endif
