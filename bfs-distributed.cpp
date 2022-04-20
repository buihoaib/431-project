#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <mpi.h>

#define DEFAULT_GRAIN_SIZE "25"

int findOwner(int vertex, int world_size) {
  if (world_size == 1) return 0;
  return vertex % world_size;
}

void breadthFirstSearch(Graph &g, int init_node) {
  uint n = g.n_;
  int level = 0;
  uint node_count = 0;
  uint local_node_count = 0;
  uint local_frontierSize = 0;
  uint global_frontierSize = 0;

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int numberOfVerticesPerProcessor = n / world_size;

  std::vector<int> currentFrontier;
  std::vector<int> nextFrontier;
  std::vector<int> distance_from_source(n, -1);

  //timer
  timer t1;
  t1.start();

  // Put source into first frontier
  int owner = findOwner(init_node, world_size);
  if (owner == world_rank) {
    currentFrontier.push_back(init_node);
    distance_from_source[init_node] = 0;
    local_node_count++;
  }

  while (true) {
    // Local variables
    std::vector<std::vector<int>> buffers(world_size);
    std::vector<std::vector<int>> recvBuffers(world_size);

    for (int i = 0; i < currentFrontier.size(); i++) {
      // Processing each vertex in the currentFrontier
			int u = currentFrontier.at(i);

			for (int i = 0; i < g.vertices_[u].getOutDegree(); i++) {
				int v = g.vertices_[u].getOutNeighbor(i);
        int w = findOwner(v, world_size);
        // add it to local buffers, which will be send to the owner later
        buffers[w].push_back(v);
      }
    }

    // gathering all buffers from other processes and put it in recvBuffers
		for (int i = 0; i < world_size; i++) {
			MPI_Gather(buffers[i].data(), buffers[i].size(), MPI_INT, recvBuffers[i].data(), buffers[i].size(), MPI_INT, i, MPI_COMM_WORLD);
		}

    // Merge recvBuffers into nextFrontier
    for (int i = 0; i < world_size; i++) {
      nextFrontier.insert(nextFrontier.end(), recvBuffers[i].begin(), recvBuffers[i].end()); // may contain duplicate vertices
    }

    currentFrontier.clear();

    for (int v = 0; v < nextFrontier.size(); v++) {
      if (distance_from_source[v] == -1) {
        distance_from_source[v] = level + 1;
        currentFrontier.push_back(v); // no duplicate of vertices now because of the condition
        local_node_count++;
      }
    }

    nextFrontier.clear();
    level++;

    // Termination condition
    local_frontierSize = currentFrontier.size();
    MPI_Allreduce(&local_frontierSize, &global_frontierSize, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if (global_frontierSize == 0)
      break;
  }

  MPI_Reduce(&local_node_count, &node_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  double time_taken = t1.stop();

  if (world_rank == 0) {
    std::cout << "Total number of nodes: " << n << "\n";
    std::cout << "Number of nodes counted: " << node_count << "\n";
    std::cout << "process_id, time_taken:\n";
	}

  std::cout << world_rank << ", " << time_taken << "\n";
}

int main(int argc, char *argv[]) {
  // Initialize the MPI environment
  MPI_Init(NULL, NULL);

  // Initialize command line arguments
  cxxopts::Options options(
      "bfs",
      "Traverse through graphs using distributed execution");
  options.add_options(
      "",
      {
          {"nInitiator", "Initiator node",
           cxxopts::value<uint>()->default_value(DEFAULT_INITIATOR)},
          // {"minBagSize", "Minimum bag size",
          //  cxxopts::value<uint>()->default_value(MIN_BAGSIZE))},
          {"inputFile", "Input graph file path",
           cxxopts::value<std::string>()->default_value(
               "/scratch/input_graphs/roadNet-CA")},
      });

  auto cl_options = options.parse(argc, argv);
  uint init_node = cl_options["nInitiator"].as<uint>();
  // uint minBagSize = cl_options["minBagSize"].as<uint>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    std::cout << "Number of processes : "
              << world_size << "\n";
    std::cout << "Initiator node: " 
              << init_node << "\n";
    //std::cout << "Initiator Minimum bag size: " << minBagSize << std::endl;
  }

  Graph g;
  if (world_rank == 0) std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  if (world_rank == 0) std::cout << "Created graph\n";
  breadthFirstSearch(g, init_node);

  // Finalize the MPI environment
  MPI_Finalize();

  printf("Program completed.\n");
  return 0;
}