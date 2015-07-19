#!/usr/bin/env python3
###############################################################################
 #  Copyright (c) 2015 Jamis Hoo
 #  Distributed under the MIT license 
 #  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 #  
 #  Project: 
 #  Filename: split_blocks.py 
 #  Version: 1.0
 #  Author: Jamis Hoo
 #  E-mail: hjm211324@gmail.com
 #  Date: Jul 19, 2015
 #  Time: 10:18:41
 #  Description: 
###############################################################################

import sys
import os

if len(sys.argv) != 3:
    print("Expect two arguments. ")
    exit(1)

if not os.path.exists(sys.argv[1]):
    print(sys.argv[1] + " does not exist. ")
    exit(1)

if os.path.exists(sys.argv[2]):
    print(sys.argv[2] + " already exists. ")
    exit(1)

os.mkdir(sys.argv[2])

block_size = 128 * 1024 * 1024

input_file = open(sys.argv[1])

output_file_counter = 0
output_file_length = 0
output_file = open(sys.argv[2] + "/part_" + format(output_file_counter, "08"), "w")

for line in input_file:
    output_file_length += len(line)

    if output_file_length > block_size:
        output_file.close
        output_file_counter += 1
        output_file = open(sys.argv[2] + "/part_" + format(output_file_counter, "08"), "w")
        output_file_length = len(line)

    output_file.write(line)

