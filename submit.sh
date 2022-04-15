#!/bin/bash
#
#SBATCH --cpus-per-task=8
#SBATCH --time=02:00
#SBATCH --mem=5G
#SBATCH --partition=slow


srun ./bfs-serial --nThreads 4 --nInitiator 0  --inputFile /scratch/input_graphs/web-Google