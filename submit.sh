#!/bin/bash
#
#SBATCH --cpus-per-task=8
#SBATCH --time=02:00
#SBATCH --mem=5G
#SBATCH --partition=slow


srun ./bfs-serial --nThreads 8 --nInitiator 0 --inputFile /scratch/input_graphs/web-Google
srun ./bfs-parallel-queue --nThreads 8 --nInitiator 0 --nGranularity 10000 --inputFile /scratch/input_graphs/web-Google
srun ./bfs-parallel-bag --nThreads 8 --nInitiator 0 --nGranularity 10000 --inputFile /scratch/input_graphs/web-Google
