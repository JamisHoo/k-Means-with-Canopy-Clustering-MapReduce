#!/usr/bin/env python3
###############################################################################
 #  Copyright (c) 2015 Jamis Hoo
 #  Distributed under the MIT license 
 #  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 #  
 #  Project: 
 #  Filename: select_k_centers.py 
 #  Version: 1.0
 #  Author: Jamis Hoo
 #  E-mail: hjm211324@gmail.com
 #  Date: Jul 19, 2015
 #  Time: 20:30:01
 #  Description: 
###############################################################################

import sys
import math
import random

if len(sys.argv) != 3:
    print("Expected 2 arguments. ")
    exit(1)

input_file = open(sys.argv[1])
output_file = open(sys.argv[2], "w")


n = sum(1 for line in input_file)
k = math.sqrt(n / 2)

print("Select", int(k), "lines from", n, "lines")

bitmap = list((False for x in range(n)))

for i in range(int(k)):
    while True:
        rnd = random.randint(0, n - 1)
        if bitmap[rnd] == False:
            bitmap[rnd] = True
            break


input_file.seek(0, 0)

i = 0
for line in input_file:
    if bitmap[i]:
        colon_pos = line.find(':')
        line = line[: colon_pos] + '\t' + line[colon_pos + 1: ]
        output_file.write(line)
    i += 1

