#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <mpi.h>

#define DEFAULT_GRAIN_SIZE "25"

// struct ThreadInfo {
//     uint thread_id;
//     uintV num_vertices;
//     uintE num_edges;
//     double totalTime;
// };

// void calculateBfsDynamic(ThreadInfo * thread_info,uint n_threads ,Graph * g, int level ,std::queue<uintV> * currentFrontier, std::queue<uintV> * nextFrontier, int * visited, CustomBarrier * barrier, std::mutex * mtx_currF, std::mutex * mtx_nextF, std::mutex * mtx_visited ) {
//     //parallel BFS algorithm
//     while(!currentFrontier->empty()){
//       int granularity = currentFrontier->size() / n_threads;
//       for (int j = 0; j < granularity; j ++){
//         mtx_currF->lock();
//         int currVertex = currentFrontier->front();
//         currentFrontier->pop();
//         mtx_currF->unlock();
//         //node_count++;

//         uintE out_degree = g->vertices_[currVertex].getOutDegree();
//         for (int i = 0; i < out_degree; i++){
//           uintV neighbour = g->vertices_[currVertex].getOutNeighbor(i);
//           if(visited[neighbour]==-1){
//             visited[neighbour] = level;

//             mtx_nextF->lock();
//             nextFrontier->push(neighbour);
//             mtx_nextF->unlock();
//           }
//         }
//       }
//       barrier->wait();
//       //Swap 2 queues
//       if (thread_info->thread_id == 0) {
//         std::queue<uintV> * temp = currentFrontier;
//         currentFrontier = nextFrontier;
//         nextFrontier =  temp;

//         level++;
//       }
//       barrier->wait();
//     }
// }

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

  // std::vector<int> recvCount (world_size, 0);

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

    // for (int i = 0; i < world_rank; i++) {
    //   uint sendCount = buffers[i].size();
    //   MPI_Reduce(&sendCount, &recvCount[i], 1, MPI_INT, MPI_SUM, i, MPI_COMM_WORLD);
    // }

    // gather buffer[world_rank] from all processes and combine it in recvBuffers[world_rank]
    // this is a mess - need modification
		MPI_Gather(&(buffers[world_rank]), buffers[world_rank].size(), MPI_INT, &recvBuffers, buffers[world_rank].size(), MPI_INT, world_rank, MPI_COMM_WORLD);

    // Merge recvBuffers into nextFrontier
    for (int i = 0; i < world_size; i++) {
      nextFrontier.insert(nextFrontier.end(), recvBuffers[i].begin(), recvBuffers[i].end()); // may contain duplicate vertices
    }

    nextFrontier.insert(nextFrontier.end(), recvBuffers.begin(), recvBuffers.end()); // may contain duplicate vertices

    // if (world_rank == 0) {
    //   printf("nextFrontier of rank 0: ");
    //   for (int i = 0; i < nextFrontier.size(); i++) {
    //     printf("%d ", world_rank);
    //   }
    //   printf("\n");
    // }

    currentFrontier.clear();

    for (int v = 0; v < nextFrontier.size(); v++) {
      if (distance_from_source[v] == -1) {
        distance_from_source[v] = level + 1;
        currentFrontier.push_back(v); // no duplicate of vertices here because of the condition
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

  // if (world_rank == 0) {
  //   std::cout << "Total number of nodes: " << n << "\n";
  //   std::cout << "Number of nodes counted: " << node_count << "\n";
  //   std::cout << "process_id, time_taken:\n";
	// }

  // std::cout << world_rank << ", " << time_taken << "\n";
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