# 431-project
## How to compile:
Run this command: 
```
make all
```
or 
```
make fileName
```

## Run the file:
To run the compiled serial and parallel files, use format:
```
./fileName --nThreads \# --nInitiators \# --inputFile /path/to/input/file
```
--nThreads is number of threads. Default value is 1

--nInitiator is the init node. Default value is 0

--inputFile is the path to input graph

For example:
```
./bfs-parallel-queue --nThreads 4 --nInitiator 5  --inputFile /scratch/input_graphs/roadNet-CA
```
------

* Note that we provided the bash files to run on the cluster:

submit.sh: contains commands to run bfs-serial, bfs-parallel-queue, bfs-parallel-bag

submit-distributed: contains command to run bfs-distributed
