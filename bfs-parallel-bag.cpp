#include "core/graph.h"
#include "parallel_util/bag.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <queue>
#include <thread>
#include <mutex>

#define DEFAULT_GRANULARITY "1"
#define DEFAULT_INPUT "./input_graphs/roadNet-CA"

uint n_granularity;

struct ThreadInfo {
    Graph * g;
    Bag* currBag;
    Bag* nextBag;
    int * visited;
    uint level;
    uint granularity;
    std::mutex * mtx_currF;
};


void processLayer(Graph * g,Bag* currBag, Bag* nextBag, int * visited,uint level){
        if (currBag->size() < n_granularity){

            std::stack<Node *> nodes;
            for (int i = 0; i <= currBag->max_index; i++){

                if (currBag->backbone[i] != NULL){
                    nodes.push(currBag->backbone[i]->root);
                    while (nodes.size() > 0){
                        Node* current = nodes.top();
                        nodes.pop();
                        if (current->left != NULL){
                            nodes.push(current->left);
                        }
                        if (current->right != NULL){
                            nodes.push(current->right);
                        }
                        uintE out_degree = g->vertices_[current->vertex].getOutDegree();
                        for (int j = 0; j < out_degree; j++){
                            uintV neighbour = g->vertices_[current->vertex].getOutNeighbor(j);
                            if (visited[neighbour] == -1){
                                nextBag->insert_vertex(neighbour);
                                visited[neighbour] = level;
                            }

                        }
                    } 
                }
            }
        } else {
          Bag * currBag2 = currBag->split();
          Bag * nextBag2 = new Bag();



          //std::cout<< currentFrontier->size() << std::endl;
          std::thread t1(processLayer,g ,currBag2, nextBag2, visited, level);
          std::thread t2(processLayer,g ,currBag, nextBag, visited, level);
          
          t1.join();
          t2.join();

          nextBag->merge(nextBag2);

        }

    
}

void breadthFirstSearch(Graph &g, int init_node, uint n_threads){
    std::thread threads[n_threads];
    ThreadInfo thread_info[n_threads];
    CustomBarrier barrier(n_threads);
    std::atomic<uintV> granularityIndex{0};
    uint n = g.n_;

    Bag* currBag = new Bag();
    Bag* nextBag = new Bag();
    int *visited = new int[n];
    int level = 1;
    std::mutex * mtx_currF = new std::mutex();
    for (int i = 0; i < n; i++)
      visited[i] = -1;


    //timer
    timer t1;
    double time_taken = 0.0;
    t1.start();
    
    uint node_count = 0;
    currBag->insert_vertex(init_node);
    visited[init_node] = level;


    while (!currBag->empty()){
      processLayer(&g,currBag, nextBag, visited, level);
      currBag->clear();
      Bag* tempBag = currBag;
      currBag = nextBag;
      nextBag = tempBag;
      level ++;

    }



    time_taken = t1.stop();
    for (int i = 0; i < g.n_; i ++){
        if (visited[i]!=-1){
          node_count++;
        }
    }
    std::cout << "thread_id, time_taken" << std::endl;
    std::cout << "Total time taken: " << time_taken << std::endl;
    std::cout << "Total number of nodes: "<< n <<std::endl;
    std::cout << "Number of nodes counted: "<< node_count <<std::endl;
    std::cout << "====================================================="<<std::endl;

    //clean up allocated memories
    delete currBag;
    delete nextBag;
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
  n_granularity = cl_options["nGranularity"].as<uint>();
  uint init_node = cl_options["nInitiator"].as<uint>();

  std::string input_file_path = cl_options["inputFile"].as<std::string>();

  std::cout << "Parallel BFS using bag data structure" << std::endl;
#ifdef USE_INT
  std::cout << "Using INT\n";
#else
  std::cout << "Using DOUBLE\n";
#endif
  std::cout << std::fixed;
  std::cout << "Number of Threads : " << n_threads << std::endl;
  std::cout << "Initiator node: " << init_node << std::endl;
  std::cout << "Granularrity: " << n_granularity << std::endl;

  Graph g;
  std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  std::cout << "Created graph\n";
  breadthFirstSearch(g, init_node, n_threads);

  return 0;
}