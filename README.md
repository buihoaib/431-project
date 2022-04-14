# 431-project
## How to compile:
Run this command: 
```
make
```
or 
```
make fileName
```

## Run the file:
To run the compiled file, use format:
```
./fileName --nThreads \# --nInitiaor \# --inputFile /path/to/input/file
```
--nThreads is number of threads. Default value is 1

--nInitiaor is the init node. Default value is 0

--inputFile is the path to in put graph

For example:
```
./flooding --nThreads 4 --nInitiator 5  --inputFile /scratch/input_graphs/roadNet-CA
```
