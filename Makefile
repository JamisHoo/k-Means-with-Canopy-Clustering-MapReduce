
CXXFLAGS += -std=c++11 -Wextra -O2

HADOOP_SRC_PATH = ./include/
HADOOP_LIB_PATH = ./hadoop_libs/

vpath %.cc src $(HADOOP_SRC_PATH)/hadoop
vpath %.h src $(HADOOP_SRC_PATH)/hadoop
vpath %.a $(HADOOP_LIB_PATH)

all: canopy_clustering label_data k_means_iteration k_means_last_iteration

canopy_clustering: canopy_clustering.cc netflix_movie.h hadooppipes.a
	$(CXX) $(CXXFLAGS) $< -o$@ -I $(HADOOP_SRC_PATH) $(HADOOP_LIB_PATH)/hadooppipes.a -lcrypto -lpthread

label_data: label_data.cc netflix_movie.h hadooppipes.a
	$(CXX) $(CXXFLAGS) $< -o$@ -I $(HADOOP_SRC_PATH) $(HADOOP_LIB_PATH)/hadooppipes.a -lcrypto -lpthread

k_means_iteration: k_means_iteration.cc netflix_movie.h hadooppipes.a
	$(CXX) $(CXXFLAGS) $< -o$@ -I $(HADOOP_SRC_PATH) $(HADOOP_LIB_PATH)/hadooppipes.a -lcrypto -lpthread

k_means_last_iteration: k_means_iteration.cc netflix_movie.h hadooppipes.a
	$(CXX) $(CXXFLAGS) $< -o$@ -I $(HADOOP_SRC_PATH) $(HADOOP_LIB_PATH)/hadooppipes.a -lcrypto -lpthread -DLAST_ITERATION

hadooppipes.a: HadoopPipes.cc SerialUtils.cc StringUtils.cc
	mkdir -p $(HADOOP_LIB_PATH)
	$(CXX) $(CXXFLAGS) -c $(HADOOP_SRC_PATH)/hadoop/HadoopPipes.cc -o $(HADOOP_LIB_PATH)/HadoopPipes.o -I $(HADOOP_SRC_PATH)
	$(CXX) $(CXXFLAGS) -c $(HADOOP_SRC_PATH)/hadoop/SerialUtils.cc -o $(HADOOP_LIB_PATH)/SerialUtils.o -I $(HADOOP_SRC_PATH)
	$(CXX) $(CXXFLAGS) -c $(HADOOP_SRC_PATH)/hadoop/StringUtils.cc -o $(HADOOP_LIB_PATH)/StringUtils.o -I $(HADOOP_SRC_PATH)
	$(AR) rvs $(HADOOP_LIB_PATH)/hadooppipes.a $(HADOOP_LIB_PATH)/HadoopPipes.o $(HADOOP_LIB_PATH)/SerialUtils.o $(HADOOP_LIB_PATH)/StringUtils.o


clean:
	$(RM) canopy_clustering label_data k_means_iteration k_means_last_iteration
	$(RM) -r $(HADOOP_LIB_PATH)
