#compiler setup
CXX = g++
CXXFLAGS = -std=c++14  -pthread -O3 

#Utility Files
COMMON=

SERIAL= file_serial 
PARALLEL= file_parallel 
DISTRIBUTED= file_distributed
ALL= $(SERIAL) $(PARALLEL) $(DISTRIBUTED)

all : $(ALL)

serial : $(SERIAL)

parallel : $(PARALLEL)

distributed : $(DISTRIBUTED)

$(SERIAL): %: %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(PARALLEL): %: %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(DISTRIBUTED):  %: %.cpp $(COMMON)
	$(MPICXX) $(CXXFLAGS) -o $@ $<


.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)