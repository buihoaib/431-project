#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>

#define DEFAULT_GRANULARITY "1"

struct ThreadInfo {
    uint thread_id;
    uintV num_vertices;
    uintE num_edges;
    double totalTime;
};
void calculateBfsDynamic(ThreadInfo * thread_info,uint n_threads ,Graph * g, int level ,std::queue<uintV> * currentFrontier, std::queue<uintV> * nextFrontier, int * visited, CustomBarrier * barrier, std::mutex * mtx_currF, std::mutex * mtx_nextF, std::mutex * mtx_visited ) {
    

    //parallel BFS algorithm


    while(!currentFrontier->empty()){
      int granularity = currentFrontier->size() / n_threads;
      for (int j = 0; j < granularity; j ++){
        mtx_currF->lock();
        int currVertex = currentFrontier->front();
        currentFrontier->pop();
        mtx_currF->unlock();
        //node_count++;

        uintE out_degree = g->vertices_[currVertex].getOutDegree();
        for (int i = 0; i < out_degree; i++){
          uintV neighbour = g->vertices_[currVertex].getOutNeighbor(i);
          if(visited[neighbour]==-1){
            visited[neighbour] = level;

            mtx_nextF->lock();
            nextFrontier->push(neighbour);
            mtx_nextF->unlock();
          }
        }
      }
      barrier->wait();
      //Swap 2 queues
      if (thread_info->thread_id == 0) {
        std::queue<uintV> * temp = currentFrontier;
        currentFrontier = nextFrontier;
        nextFrontier =  temp;

        level++;
      }
      barrier->wait();
    }
}

void breadthFirstSearch(Graph &g, int init_node, uint n_threads){
    std::thread threads[n_threads];
    ThreadInfo thread_info[n_threads];
    CustomBarrier barrier(n_threads);
    std::atomic<uintV> granularityIndex{0};
    uint n = g.n_;


    std::queue<uintV> * currentFrontier = new std::queue<uintV>();
    std::queue<uintV> * nextFrontier = new std::queue<uintV>();
    int *visited = new int[n];
    int level = 1;
    std::mutex * mtx_currF = new std::mutex();
    std::mutex * mtx_nextF = new std::mutex();
    std::mutex * mtx_visited = new std::mutex();
    
    //timer
    timer t1;
    double time_taken = 0.0;
    t1.start();
    uint node_count = 0;
    currentFrontier->push(init_node);

    for (int i = 0; i < n; i++)
      visited[i] = -1;
    visited[init_node] = level;
    node_count++;

    for (uint id = 0; id < n_threads; id++) {
        thread_info[id].thread_id = id;
        thread_info[id].num_edges = 0;
        thread_info[id].num_vertices = 0;
        thread_info[id].totalTime = 0;

        // Thread does work
        threads[id] = std::thread(calculateBfsDynamic, &thread_info[id],n_threads, &g, level ,currentFrontier, nextFrontier, visited, &barrier, mtx_currF, mtx_nextF, mtx_visited);
    }
    for (uint id = 0; id < n_threads; id++) {
        threads[id].join();
    }


    time_taken = t1.stop();
    std::cout << "thread_id, time_taken" << std::endl;
    std::cout << "0, " << time_taken << std::endl;
    std::cout << "Total number of nodes: "<< n <<std::endl;
    std::cout << "Number of nodes counted: "<< node_count <<std::endl;

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
  breadthFirstSearch(g, init_node, n_threads);

  return 0;
}