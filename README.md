# Benchmark on Sequential and Parallel Graph Triangle Counter Problem
Comparison between Sequential and Parallel version of the Graph Triangle Counter problem.

## Startup
In order to run the benchmark you have to run these command in your console:
```
sudo apt update
sudo apt install build-essential
sudo apt-get install libboost-all-dev
```

After that, to compile the project, you have to jump into the */app/src* folder and type the following line in your console:

```
g++ -std=c++2a -o ../bin/app Main.cpp
```

For compier optimization instead type:
```
g++ -std=c++2a -Ofast -o ../bin/app Main.cpp
```

The *.exe* file will be inserted into the */app/bin* directory.
