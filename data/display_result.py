#!/usr/bin/env python3
###############################################################################
 #  Copyright (c) 2015 Jamis Hoo
 #  Distributed under the MIT license 
 #  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 #  
 #  Project: 
 #  Filename: display_result.py 
 #  Version: 1.0
 #  Author: Jamis Hoo
 #  E-mail: hjm211324@gmail.com
 #  Date: Jul 21, 2015
 #  Time: 21:47:53
 #  Description: 
###############################################################################

movie_file = "netflix/movie_titles.txt"
k_means_result = "k_means_result"

f = open(movie_file)

movie_titles = []
for line in f:
    title = line[line.find(',', line.find(',') + 1) + 1: ]
    movie_titles.append(title[: -1])

f.close()
f = open(k_means_result)

for line in f:
    movie_ids = [ int(x, 16) for x in line.split(',') ]
    for i in movie_ids:
        print(movie_titles[i - 1], end = ',')
    print()
