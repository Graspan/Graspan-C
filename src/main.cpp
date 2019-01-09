#include "context.h"
#include "preproc/run_pre.h"

int main(int argc,char **argv)
{
	Context c(argc,argv);
	preprocess(c);
	
	return 0;
}
