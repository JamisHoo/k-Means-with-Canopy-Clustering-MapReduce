#!/usr/bin/env python3
###############################################################################
 #  Copyright (c) 2015 Jamis Hoo
 #  Distributed under the MIT license 
 #  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 #  
 #  Project: 
 #  Filename: preprocess.py 
 #  Version: 1.0
 #  Author: Jamis Hoo
 #  E-mail: hoojamis@gmail.com
 #  Date: Jul 18, 2015
 #  Time: 21:21:16
 #  Description: 
###############################################################################
import os

input_data_path = "netflix/training_set"
output_data_path = "canopy_input"

files = os.listdir(input_data_path)

output_handler = open(output_data_path, "w")

movie_id = None
user_ratings = ""

counter = 0

# for each file in this dir
for filename in files:
    # if is a data file
    if filename[0: 3] != "mv_" or filename[-4: ] != ".txt":
        continue

    input_handler = open(input_data_path + "/" + filename)

    # extract movie id
    movie_id = format(int(input_handler.readline()[: -2]), 'x')

    # extract user_ids and corresponding ratings
    for user_line in input_handler.readlines():
        user_id_rating_date = user_line.split(",")
        user_id = format(int(user_id_rating_date[0]), 'x')
        rating = format(int(user_id_rating_date[1]), 'x')
        user_ratings += str(user_id) + "," + str(rating) + ","
    
    output_handler.write(str(movie_id) + ":" + user_ratings[: -1] + "\n")
    user_ratings = ""

    counter = counter + 1
    if counter % 128 == 0:
        print(counter, "/", len(files))

