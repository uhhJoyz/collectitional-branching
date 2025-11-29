# Welcome

Simulation infrastructure to evaluate the performance of device-aware mapping in the MapReduce algorithm.

## Links
- [Mapping Algorithm Readme](./mapping-algorithm/README.md)

## Notes

The setup script will clone the GitHub repository of ns-3 and perform initial setup. No changes made to the files in this directory will be tracked. In order to make changes to the simulation script, edit `mapreduce-sim.cc` with your favorite editor then run `./scripts/push.sh` to update the already-built docker container. Also, do *not* make a virtual Python environment in this directory! It will cause issues with the docker container.

TL;DR:
- Use the scripts in `./scripts/` to interact with the docker container
- **Do not** make a virtual environment called `.env` in this directory.

# Installing and Running

On MacOS:
```bash
cat ./scripts/setup.sh | sh
brew install colima docker
colima start
docker build -t colbra .
docker run colbra
```

Linux:
```bash
cat ./scripts/setup.sh | sh
sudo apt install -y colima docker-ce docker-ce-cli
colima start
docker build -t colbra .
docker run colbra
```

Windows:
```bash
no idea && skill issue && use WSL
```
