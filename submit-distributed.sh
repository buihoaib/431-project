#!/bin/bash
#
#SBATCH --cpus-per-task=1
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --time=02:00
#SBATCH --mem=5G
#SBATCH --partition=slow


mpirun -n 4 ./bfs-distributed --nInitiator 0  --inputFile /scratch/input_graphs/web-Google