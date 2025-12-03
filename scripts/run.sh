#!/bin/bash

if [[ -z DOCKER_IMG ]]; then
    export DOCKER_IMG=colbra:latest
    echo "DOCKER_IMG env var not set, using default: ${DOCKER_IMG}"
fi

echo "Starting docker container"
if [ "$#" -gt 0 ]; then
  docker run -w /app/ns-3-dev ${DOCKER_IMG} ./ns3 run "mapreduce-sim --input_file=/app/$1"
  else
  docker run -w /app/ns-3-dev ${DOCKER_IMG} ./ns3 run "mapreduce-sim"
fi
