#!/bin/bash

if [[ -z DOCKER_IMG ]]; then
    export DOCKER_IMG=colbra:latest
    echo "DOCKER_IMG env var not set, using default: ${DOCKER_IMG}"
fi

echo "Creating temp container temp_colbr..."
docker run -d --name temp_colbr ${DOCKER_IMG} sleep infinity > /dev/null 2>&1
echo "Copying script to conatiner..."
docker cp ./mapreduce-sim.cc temp_colbr:/app/ns-3-dev/scratch/mapreduce-sim.cc > /dev/null 2>&1
DIR=./mapping-algorithm/build
F1=naive_mappings.txt
if [ -e "$DIR/$F1" ]; then
  echo "Copying $DIR/$F1 to /app/$F1"
  docker cp $F1 temp_colbr:/app/$F1 > /dev/null 2>&1
fi

F2=./mapping-algorithm/build/partition_bounded_mappings.txt
if [ -e "$DIR/$F2" ]; then
  echo "Copying $DIR/$F2 to /app/$F2"
  docker cp $F2 temp_colbr:/app/$F2 > /dev/null 2>&1
fi

F3="./mapping-algorithm/build/partition_hw_strict_mappings.txt"
if [ -e "$DIR/$F3" ]; then
  echo "Copying $DIR/$F3 to /app/$F3"
  docker cp $F3 temp_colbr:/app/$F3 > /dev/null 2>&1
fi

docker cp ./mapreduce-sim.cc temp_colbr:/app/ns-3-dev/scratch/mapreduce-sim.cc > /dev/null 2>&1
echo "Commiting changes to image ${DOCKER_IMG}..."
docker commit temp_colbr ${DOCKER_IMG} > /dev/null 2>&1
echo "Cleaning up temp container..."
docker stop temp_colbr > /dev/null 2>&1
docker rm temp_colbr > /dev/null 2>&1
