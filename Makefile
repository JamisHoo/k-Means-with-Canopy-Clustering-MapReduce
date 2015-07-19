vpath %.cc src
vpath %.h src

CXXFLAGS += -std=c++11 -Wextra -O2

HADOOP_SRC_PATH = ./include/

all: canopy_clustering

canopy_clustering: canopy_clustering.cc netflix_movie.h
	$(CXX) $(CXXFLAGS) $< -o$@ -I $(HADOOP_SRC_PATH) $(HADOOP_SRC_PATH)/hadoop/*.cc -lcrypto -lpthread
