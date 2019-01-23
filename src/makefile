CC=g++ -std=c++0x

#use path according to boost installation directory in user machine
BOOST_PATH=/home/aftab/Downloads/boost_1.62_installed

export LD_LIBRARY_PATH=$(BOOST_PATH)/lib:$LD_LIBRARY_PATH
#use paths according to boost installation directory in user machine
CFLAGS= -I$(BOOST_PATH)/include -L$(BOOST_PATH)/lib -O2 -ggdb -lboost_thread -lpthread -lboost_system

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
	$(CC) main.cpp $(PREP) $(COMPUTE) $(DATASTRUCTURES) $(ALGORITHM) $(OTHERS) -o ../bin/comp $(CFLAGS)

