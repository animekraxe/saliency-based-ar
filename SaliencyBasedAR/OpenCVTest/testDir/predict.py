#! /usr/bin/python
from __future__ import print_function
import os
import sys
import subprocess

if len(sys.argv) != 3:
    print("Missing file")
    sys.exit()

p = subprocess.Popen(['./darknet', 'detect', 'cfg/yolo.cfg', 'yolo.weights', '-thresh', sys.argv[2], sys.argv[1]],
                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)

out, err = p.communicate()

out = out.split('\n')
out_filename = os.path.splitext(sys.argv[1])[0] + '_predictions.txt'
output_file = open(out_filename, 'w')
trigger = False
for line in out:
    if line.strip() == ">>>filedata":
        trigger = True
        continue
    if trigger:
        if len(line.split()) != 0:
            lab_start = line.find("Lab:") + 4
            lab_end = line.find("Lef:") - 1
            label = line[lab_start:lab_end].replace(' ', '_')
            line = line.replace(line[lab_start:lab_end], label)
            output_file.write(line + '\n')
output_file.close()

