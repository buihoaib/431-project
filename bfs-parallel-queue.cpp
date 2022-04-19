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
#define DEFAULT_INPUT "./input_graphs/roadNet-CA"

struct ThreadInfo {
    uint thread_id;
    uintV num_vertices;
    uintE num_edges;
    double totalTime;
};

void processLayer(Graph * g,std::queue<uintV> * currentFrontier, std::queue<uintV> * nextFrontier, int * visited,uint level,uint granularity, std::mutex * mtx_currF){
        if (currentFrontier->size() < granularity){



            while(!currentFrontier->empty()){
                int currVertex = currentFrontier->front();
                currentFrontier->pop();
                uintE out_degree = g->vertices_[currVertex].getOutDegree();
                for (int i = 0; i < out_degree; i++){
                    uintV neighbour = g->vertices_[currVertex].getOutNeighbor(i);
                    if(visited[neighbour]==-1){
                      visited[neighbour] = level;
                      mtx_currF->lock();
                      nextFrontier->push(neighbour);
                      mtx_currF->unlock();
                    }


                }
            }



        } else {
          std::queue<uintV>  tempQueue1;
          std::queue<uintV>  tempQueue2;
          int tempSize = currentFrontier->size();

          for (int i = 0 ; i <= tempSize/2; i++){

              tempQueue1.push(currentFrontier->front());
              currentFrontier->pop();
              //std::cout<<"popping 1"<< currentFrontier->size() << std::endl;

          }
          //std::cout<< currentFrontier->size() << std::endl;

          for (int i = tempSize/2 + 1; i < tempSize; i++){
              tempQueue2.push(currentFrontier->front());
              currentFrontier->pop();
              //std::cout<<"popping 2"<< currentFrontier->size() << std::endl;

          }

          //std::cout<< currentFrontier->size() << std::endl;
          std::thread t1(processLayer,g ,&tempQueue1, nextFrontier, visited, level, granularity, mtx_currF);
          std::thread t2(processLayer,g ,&tempQueue2, nextFrontier, visited, level, granularity, mtx_currF);
          

          t1.join();
          t2.join();



        }

    
}

void breadthFirstSearch(Graph &g, int init_node, uint n_threads, uint n_glanularity){
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

    

    uint node_count = 0;
    currentFrontier->push(init_node);

    for (int i = 0; i < n; i++)
      visited[i] = -1;
    visited[init_node] = level;

    //timer
    timer t1;
    double time_taken = 0.0;
    t1.start();


    while (!currentFrontier->empty()){
      processLayer(&g,currentFrontier, nextFrontier, visited, level, n_glanularity, mtx_currF);
      std::swap(*currentFrontier, *nextFrontier);

    }

    time_taken = t1.stop();
    for (int i = 0; i < n; i ++){
      if (visited[i]!=-1){
        node_count++;
      }
    }
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
          {"nGranularity", "Granularity",
           cxxopts::value<uint>()->default_value(DEFAULT_GRANULARITY)},
          {"inputFile", "Input graph file path",
           cxxopts::value<std::string>()->default_value(
               "./input_graphs/roadNet-CA")},
      });

  auto cl_options = options.parse(argc, argv);
  uint n_threads = cl_options["nThreads"].as<uint>();
  uint n_granularity = cl_options["nGranularity"].as<uint>();
  uint init_node = cl_options["nInitiator"].as<uint>();

  std::string input_file_path = cl_options["inputFile"].as<std::string>();

  std::cout << "Parallel BFS using FIFO queue data structure" << std::endl;
#ifdef USE_INT
  std::cout << "Using INT\n";
#else
  std::cout << "Using DOUBLE\n";
#endif
  std::cout << std::fixed;
  std::cout << "Number of Threads : " << n_threads << std::endl;
  std::cout << "Initiator node: " << init_node << std::endl;
  std::cout << "Glanularrity: " << n_granularity << std::endl;

  Graph g;
  std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  std::cout << "Created graph\n";
  breadthFirstSearch(g, init_node, n_threads, n_granularity);

  return 0;
}