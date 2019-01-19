#include "run_computation.h"
#include "array/compute.h"
#include "time.h"

long run_computation(Context &c) {
	myarray::Compute comp;
	return comp.startCompute(c);
}
