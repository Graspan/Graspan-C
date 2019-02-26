#ifndef PREPROC_H
#define PREPROC_H
#include "../common.h"
#include "../context.h"
#include "../datastructures/vit.h"
#include "../datastructures/grammar.h"

class Preproc {
	private:
		long totalNumEdges;
		vertexid_t totalDuplicateEdges;
		vertexid_t maxVid;
		vertexid_t *numEdges;        // degree of each vertex. size = maxVid+1

	public:
		Preproc();
		~Preproc();

		inline vertexid_t getTotalDuplicateEdges() {return totalDuplicateEdges;}
		inline long getAddedEdgesNum(Context &c) {return (maxVid+1) * c.grammar.getNumErules();}

		void loadGrammar(Context &c);
		void setNumEdges(Context &c);
		void setVIT(Context &c);
		void savePartitions(Context &c);
		void test(Context &c);
};

#endif
