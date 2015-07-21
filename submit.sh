#!/bin/sh
###############################################################################
 #  Copyright (c) 2015 Jamis Hoo
 #  Distributed under the MIT license 
 #  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 #  
 #  Project: 
 #  Filename: submit.sh 
 #  Version: 1.0
 #  Author: Jamis Hoo
 #  E-mail: hjm211324@gmail.com
 #  Date: Jul 19, 2015
 #  Time: 10:02:14
 #  Description: 
###############################################################################

HADOOP_HOME=~/Desktop/hadoop-2.7.1/
ORIGINAL_INPUT_PATH=canopy_input
CANOPY_CENTERS=canopy_centers
CANOPY_LABELED=canopy_labeled
K_MEANS_INITIAL_CENTERS=k_means_initial_centers
K_MEANS_CENTERS=k_means_centers
K_MEANS_RESULT=k_means_result
k_ITERATIONS=4

# Step 0: update exec and data
${HADOOP_HOME}/bin/hdfs dfs -mkdir bin/
${HADOOP_HOME}/bin/hdfs dfs -put canopy_clustering bin/
${HADOOP_HOME}/bin/hdfs dfs -put label_data bin/
${HADOOP_HOME}/bin/hdfs dfs -put k_means_iteration bin/
${HADOOP_HOME}/bin/hdfs dfs -put k_means_last_iteration bin/
${HADOOP_HOME}/bin/hdfs dfs -mkdir $ORIGINAL_INPUT_PATH
for i in data/${ORIGINAL_INPUT_PATH}_blocks/*; do
    ${HADOOP_HOME}/bin/hdfs dfs -put $i $ORIGINAL_INPUT_PATH/
done

echo "Step 0 success. "

# Step 1: get canopy centers
${HADOOP_HOME}/bin/mapred pipes \
-conf src/conf.xml \
-D mapred.reduce.tasks=1 \
-input $ORIGINAL_INPUT_PATH \
-output $CANOPY_CENTERS \
-program bin/canopy_clustering

if (($?)); then
    echo "Step 1 failed. "
    exit 1
fi

echo "Step 1 success. "

rm -rf data/$CANOPY_CENTERS
${HADOOP_HOME}/bin/hdfs dfs -get $CANOPY_CENTERS/part-00000 data/$CANOPY_CENTERS

# Step 2: label all data with canopy centers
${HADOOP_HOME}/bin/mapred pipes \
-conf src/conf.xml \
-files data/$CANOPY_CENTERS \
-input $ORIGINAL_INPUT_PATH \
-output $CANOPY_LABELED \
-program bin/label_data

if (($?)); then
    echo "Step 2 failed. "
    exit 1
fi

echo "Step 2 success. "

 Step 3: k means iteration
rm -rf data/$CANOPY_LABELED
${HADOOP_HOME}/bin/hdfs dfs -get $CANOPY_LABELED/part-00000 data/$CANOPY_LABELED
if [ -e data/$CANOPY_LABELED ]; then
    ${HADOOP_HOME}/bin/hdfs dfs -rm $CANOPY_LABELED/*
fi
rm -rf data/${CANOPY_LABELED}_blocks
python3 data/split_blocks.py data/$CANOPY_LABELED data/${CANOPY_LABELED}_blocks/
for i in data/${CANOPY_LABELED}_blocks/*; do
    ${HADOOP_HOME}/bin/hdfs dfs -put $i $CANOPY_LABELED/
done


cp data/$K_MEANS_INITIAL_CENTERS data/$K_MEANS_CENTERS

for ((i = 0;i < $(($k_ITERATIONS - 1)); ++i)); do
    ${HADOOP_HOME}/bin/mapred pipes \
    -conf src/conf.xml \
    -files data/$CANOPY_CENTERS,data/$K_MEANS_CENTERS \
    -input $CANOPY_LABELED \
    -output $K_MEANS_CENTERS \
    -program bin/k_means_iteration

    if (($?)); then
        echo "Step 3 failed in loop $i. "
        exit 1
    fi

    rm -rf data/${K_MEANS_CENTERS}_tmp
    ${HADOOP_HOME}/bin/hdfs dfs -get $K_MEANS_CENTERS/part-00000 data/${K_MEANS_CENTERS}_tmp
    if [ ! -e data/${K_MEANS_CENTERS}_tmp ]; then
        echo "Step 3 failed when downloading files. "
        exit 1
    fi
    mv data/${K_MEANS_CENTERS}_tmp data/$K_MEANS_CENTERS
    if [ -e data/$K_MEANS_CENTERS ]; then
        ${HADOOP_HOME}/bin/hdfs dfs -rm -r $K_MEANS_CENTERS
    else 
        echo "Step 3 failed when moving files. "
        exit 1
    fi
done

# Step 4: last k means iteration, output clustering result rather than k-means centers

${HADOOP_HOME}/bin/mapred pipes \
-conf src/conf.xml \
-files data/$CANOPY_CENTERS,data/$K_MEANS_CENTERS \
-input $CANOPY_LABELED \
-output $K_MEANS_RESULT \
-program bin/k_means_last_iteration

if (($?)); then
    echo "Step 4 failed. "
    exit 1
fi


