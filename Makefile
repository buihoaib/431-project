ifdef USE_INT
MACRO = -DUSE_INT
endif

#compiler setup
CXX = g++
CXXFLAGS = -std=c++14  -pthread -O3 

#Utility Files
COMMON = core/utils.h core/cxxopts.h core/get_time.h core/graph.h core/quick_sort.h


SERIAL= bfs-serial
PARALLEL-BAG= bfs-parallel-bag
PARALLEL-QUEUE= bfs-parallel-queue
DISTRIBUTED= bfs-distributed
ALL= $(SERIAL) $(PARALLEL-BAG) $(PARALLEL-QUEUE) $(DISTRIBUTED) 

all : $(ALL)

serial : $(SERIAL)

parallel-bag : $(PARALLEL-BAG)

parallel-queue : $(PARALLEL-QUEUE)

distributed : $(DISTRIBUTED)

$(SERIAL): %: %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(PARALLEL-BAG): %: %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(PARALLEL-QUEUE): %: %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(DISTRIBUTED):  %: %.cpp $(COMMON)
	$(MPICXX) $(CXXFLAGS) -o $@ $<


.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)