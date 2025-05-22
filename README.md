Build and run:

```sh
mkdir build
cd build
cmake ..
make
./app
```

A `results.csv` file will be placed in the CWD. Plot results with:

```sh
mkdir plots
python3 graph.py results.csv plots/
```
