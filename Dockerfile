FROM ubuntu:22.04

WORKDIR /app

COPY ./mapreduce-sim.cc /app/ns-3-dev/scratch/mapreduce-sim.cc
COPY ./ns-3-dev /app/ns-3-dev
COPY ./entrypoint.sh /app/entrypoint.sh
COPY ./mapping-algorithm/*.txt /app/

RUN apt-get update && \
    apt-get install -y --no-install-recommends python3 \
    python3-pip python3-dev python3-venv cmake g++ make \
    git xutils libssl-dev

RUN python3 -m venv /app/.env && \
    . /app/.env/bin/activate && \
    pip install --upgrade pip

RUN cd /app/ns-3-dev && \
    ./ns3 configure --enable-modules=core,network,internet,csma,applications && \
    ./ns3 build && \
    cp /app/mapreduce-sim.cc /app/ns-3-dev/scratch/ && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

EXPOSE 5000

ENTRYPOINT [ "/app/entrypoint.sh" ]

CMD ["python", "--version"]
