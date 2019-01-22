# DiskSpa
A Disk-based Highly Parallel Interprocedural Static Analysis Engine.

## Getting Started
DiskSpa is simple to use,with a very straight-forward complication procedure.

### Compiling DiskSpa
```
git clone https://github.com/z-zhiqiang/DiskSpa.git
cd src
make
```

### Running DiskSpa
```
cd src
./comp <graph_file> <grammar_file> <number_partitions> <memory_budget> <num_threads>
```

## 待实现的功能
```
1.多线程并行计算。
2.根据内存大小设置划分数，内存不足进行重划分。
3.更高效的调度算法。
4.设计其他的数据结构，与当前数组效率进行对比。
5.优化多个有序数组合并去重算法。
```
