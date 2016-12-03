#!/bin/bash
thresh=0.1
if [ $# -eq 2 ]
then
    thresh=$2
fi

x=$1
y=${x%.*}

echo "Resizing image..."
sips -Z 640 $1

echo "Generating saliency map..."
./BMS $1 salmap 8 13 1 1 0 0 1 

echo "Generating object detection predictions..."
./predict.py $1 $thresh

echo "Running..."
#./saliency_based_ar $y 1 0 0 0 1
./saliency_based_ar $y $3 $4 $5 $6 $7
