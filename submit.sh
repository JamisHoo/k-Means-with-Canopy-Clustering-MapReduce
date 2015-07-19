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

# Step 1: get canopy centers
:'
${HADOOP_HOME}/bin/mapred pipes \
-conf src/conf.xml \
-D mapred.reduce.tasks=1 \
-input jamis_canopy_input/ \
-output jamis_canopy_output/ \
-program bin/jamis_canopy_clustering
'

# Step 2: label all data with canopy centers
${HADOOP_HOME}/bin/mapred pipes \
-conf src/conf.xml \
-input jamis_canopy_input/ \
-output jamis_canopy_labeled/ \
-program bin/jamis_labeled_data
