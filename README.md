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
g++ -std=c++2a -fopenmp -o ../bin/app Main.cpp
```

For compier optimization instead type:
```
g++ -std=c++2a -fopenmp -O3 -o ../bin/app Main.cpp
```

The *.exe* file will be inserted into the */app/bin* directory.

## Usage
Start the application by typing ```./app```. Next you have only to specify:
* If tou want to override the existing random generated graph and creatig other one:  _0_ or _1_

During the execution of the becnhmarck the application will run the TriangleCounter first on the Stanford graph by first reading the .csv file and creating the respecitve edegs list and the for each graph and for each thread it run the actual TriangleCounter. Next the same thing is done regarding the random gebnerated graphs.

Screen shot describing the output of the application:

## Results
These result was achived running the application on a HP ElideDesk G1 TW with 8 cores CPU:
![](summary_plots.png)
