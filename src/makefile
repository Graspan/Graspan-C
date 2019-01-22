##### source files #####
# preprocessing
PREP=preproc/preproc.cpp preproc/run_pre.cpp
# edge computation
COMPUTE=compute/run_computation.cpp compute/array/arraystomerge.cpp compute/array/computationset.cpp compute/array/compute.cpp compute/array/edgearray.cpp
# datastructures
DATASTRUCTURES=datastructures/grammar.cpp datastructures/partition.cpp datastructures/vit.cpp datastructures/ddm.cpp
# algorithm
ALGORITHM=algorithm/myalgorithm.cpp
# other source files
OTHERS=context.cpp 

comp: main.cpp $(PREP) $(COMPUTE) $(DATASTRUCTURES) $(ALGORITHM) $(OTHERS)
	g++ main.cpp $(PREP) $(COMPUTE) $(DATASTRUCTURES) $(ALGORITHM) $(OTHERS) -o comp


