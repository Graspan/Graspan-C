#include "context.h"
#include "preproc/run_pre.h"
#include "compute/run_computation.h"
#include <iostream>
#include <time.h>
using std::cout;
using std::endl;

int main(int argc,char **argv)
{	
	Context c(argc,argv);

	clock_t start_t,end_t;
	long int prepTime,compTime,totalTime;
	cout << "===========PREPROCESS BEGIN==========" << endl;
	start_t = clock();
	preprocess(c);
	end_t = clock();
	prepTime = (end_t - start_t) / CLOCKS_PER_SEC;
	cout << "===========PREPROCESS END============" << endl;

	cout << "===========COMPUTE BEGIN=============" << endl;
	start_t = clock();
	cout << "NEW EDGES TOTAL: " << run_computation(c) << endl;
	end_t = clock();
	compTime = (end_t - start_t) / CLOCKS_PER_SEC;
	cout << "===========COMPUTE END===============" << endl;
	
	cout << "PREP TIME: " << prepTime << "s" << endl;
	cout << "COMP TIME: " << compTime << "s" << endl;
	cout << "TOTAL TIME: " << prepTime + compTime << "s" << endl;
	c.clear();

	return 0;
}
