#include "run_pre.h"
#include "preproc.h"
#include "time.h"

void preprocess(Context &c) {
	//clock_t start_t,end_t;
	Preproc pre;
	pre.loadGrammar(c);
	//start_t = clock();	
	pre.setVIT(c);
	//end_t = clock();
	//cout << "setVit time: " << (double)(end_t - start_t) / CLOCKS_PER_SEC << "s" << endl;
	//start_t = clock();
	pre.savePartitions(c);
	//end_t = clock();
	//cout << "savePartitions time: " << (double)(end_t - start_t) / CLOCKS_PER_SEC << "s" << endl;
	pre.test(c);
}
