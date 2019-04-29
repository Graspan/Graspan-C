# DiskSpa
A Disk-based Highly Parallel Interprocedural Static Analysis Engine.

## Getting Started
DiskSpa is simple to use,with a very straight-forward complication procedure.

### Required Libraries
Ensure that you have a recent version of the Boost library installed in your system. You can obtain the library from [here](http://www.boost.org/users/history/version_1_62_0.html).

### Compiling DiskSpa
First, download the entire DiskSpa source code into your machine. Next, edit the src/makefile to set the paths to the Boost library include files and lib files in your machine. Do the same for the src/run file. Finally, run the makefile in the src folder using make. DiskSpa should now be compiled and ready to run.

### Running DiskSpa
```
cd src
./run <graph_file> <grammar_file> <number_partitions> <memory_budget> <num_threads>
```

