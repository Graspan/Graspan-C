#include "run_computation.h"
#include "compute.h"
#include "time.h"

long run_computation(Context &c) {
	Compute comp;
	return comp.startCompute(c);
}
