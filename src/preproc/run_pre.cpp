#include "run_pre.h"
#include "preproc.h"

void preprocess(Context &c) {
	Preproc pre;
	pre.loadGrammar(c);
	pre.setVIT(c);
	pre.savePartitions(c);
	pre.test(c);
}
