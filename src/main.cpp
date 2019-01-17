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
	
	cout << "===========PREPROCESS BEGIN==========" << endl;
	start_t = clock();
	preprocess(c);
	end_t = clock();
	cout << "preprocess time: " << (end_t - start_t) / CLOCKS_PER_SEC << "s" << endl;
	cout << "===========PREPROCESS END============" << endl;

	
	cout << "===========COMPUTE BEGIN=============" << endl;
	start_t = clock();
	cout << run_computation(c) << endl;
	end_t = clock();
	cout << "compute time: " << (end_t - start_t) / CLOCKS_PER_SEC << "s" << endl;
	cout << "===========COMPUTE END===============" << endl;
	
	return 0;
}
