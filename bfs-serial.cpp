#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <queue>


void breadthFirstSearch(Graph &g, int init_node){
    //Declare
    uintV n = g.n_;
    std::queue<uintV> * currentFrontier = new std::queue<uintV>();
    std::queue<uintV> * nextFrontier = new std::queue<uintV>();
    int *visited = new int[n];
    int level = 1;
    
    //timer
    for (int i = 0; i < g.n_; i ++){
        if (visited[i]!=-1){
        }
    }
    timer t1;
    double time_taken = 0.0;
    t1.start();

    uint node_count = 0;
    currentFrontier->push(init_node);

    //serial BFS algorithm
    for (int i = 0; i < n; i++)
      visited[i] = -1;
    visited[init_node] = level;
    currentFrontier->push(init_node);

    while(!currentFrontier->empty()){
      for (int j = 0; j < currentFrontier->size(); j ++){

        int currVertex = currentFrontier->front();
        currentFrontier->pop();

        uintE out_degree = g.vertices_[currVertex].getOutDegree();
        for (int i = 0; i < out_degree; i++){
          uintV neighbour = g.vertices_[currVertex].getOutNeighbor(i);
          if(visited[neighbour]==-1){
            visited[neighbour] = level;
            nextFrontier->push(neighbour);
          }
        }
      }

      //Swap 2 queues
      std::queue<uintV> * temp = currentFrontier;
      currentFrontier = nextFrontier;
      nextFrontier =  temp;

      level++;
    }
    for (int i = 0; i < g.n_; i ++){
        if (visited[i]!=-1){
          node_count++;
        }
    }
    time_taken = t1.stop();
    std::cout << "thread_id, time_taken" << std::endl;
    std::cout << "Total time taken: " << time_taken << std::endl;
    std::cout << "Total number of nodes: "<< n <<std::endl;
    std::cout << "Number of nodes counted: "<< node_count <<std::endl;
    std::cout << "====================================================="<<std::endl;

    //clean up allocated memories
    delete currentFrontier;
    delete nextFrontier;
    delete[] visited;
}

int main(int argc, char *argv[]) {
  cxxopts::Options options(
      "bfs",
      "Traverse through graphs using serial and parallel execution");
  options.add_options(
      "",
      {
          {"nThreads", "Number of Threads",
           cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_THREADS)},
          {"nInitiator", "Initiator node",
           cxxopts::value<uint>()->default_value(DEFAULT_INITIATOR)},
          {"inputFile", "Input graph file path",
           cxxopts::value<std::string>()->default_value(
               "/scratch/input_graphs/roadNet-CA")},
      });

  auto cl_options = options.parse(argc, argv);
  uint n_threads = cl_options["nThreads"].as<uint>();
  uint init_node = cl_options["nInitiator"].as<uint>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();

  std::cout << "Serial BFS using FIFO queue data structure" << std::endl;
#ifdef USE_INT
  std::cout << "Using INT\n";
#else
  std::cout << "Using DOUBLE\n";
#endif
  std::cout << std::fixed;
  std::cout << "Number of Threads : " << n_threads << std::endl;
  std::cout << "Initiator node: " << init_node << std::endl;

  Graph g;
  std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  std::cout << "Created graph\n";
  breadthFirstSearch(g, init_node);

  return 0;
}