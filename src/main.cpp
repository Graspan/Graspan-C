#include "context.h"
#include "preproc/run_pre.h"
#include "compute/run_computation.h"
#include <iostream>
#include <ctime>
using std::cout;
using std::endl;

int main(int argc,char **argv)
{	
	Context c(argc,argv);

	time_t start_t,end_t;
	time_t prepTime,compTime,totalTime;
	cout << "===========PREPROCESS BEGIN==========" << endl;
	start_t = time(NULL);
	preprocess(c);
	end_t = time(NULL);
	prepTime = end_t - start_t;
	cout << "===========PREPROCESS END============" << endl;

	cout << "===========COMPUTE BEGIN=============" << endl;
	start_t = time(NULL);
	cout << "NEW EDGES TOTAL: " << run_computation(c) << endl;
	end_t = time(NULL);
	compTime = end_t - start_t;
	cout << "===========COMPUTE END===============" << endl;
	
	cout << "PREP TIME: " << prepTime << "s" << endl;
	cout << "COMP TIME: " << compTime << "s" << endl;
	cout << "TOTAL TIME: " << prepTime + compTime << "s" << endl;
	c.clear();

	return 0;
}
