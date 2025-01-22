# Parallel Fractals

This project generates **Julia and Mandelbrot fractals**. It supports both **sequential execution** and **parallel execution using MPI** for the latter.

## Compilation and Execution

### 1️⃣ Running the **Sequential Version**
If you want to run the code **without MPI**, you need to:
- **Comment out MPI-specific code** in `main.cpp`.
- **Ensure `main()` calls the sequential function**.
- **Compile and run using `g++`**:
  ```sh
  g++ main.cpp -o main
  ./main
  ```

### 2️⃣ Running the **MPI (Parallel) Version**
To enable parallel execution, do not comment out the MPI code, and compile using `mpicxx`:
  ```sh
  mpicxx -o main main.cpp
  ```
Then, run it using `mpiexec`:
  ```sh
  mpiexec -n 4 ./main
  ```
The `-n 4` option tells MPI to use **4 parallel processes**.

### 3️⃣ Running on a **Cluster of Machines (Distributed Execution)**
If you are running this on a **cluster of VMs or multiple machines**:
- **Create a host file** (named `mpi_file` in this example) listing the machines:
  ```
  node1
  node2
  node3
  node4
  ```
Then, run the program across the machines:
  ```sh
  mpiexec -n 4 --hostfile mpi_file ./main
  ```
 If you are interested in running it on a cluster of virtual machines but don't know where to start, I highly recommend: https://mpitutorial.com/tutorials/

#### **How MPI Splits Workload Across Machines**
- If you have **4 processes (`-n 4`)** and **4 machines**, MPI will **assign 1 process per machine**.
- If you have **8 processes (`-n 8`)** and **4 machines**, MPI will **assign 2 processes per machine**.
- If there are **fewer machines than processes**, MPI will distribute the load accordingly.

Example:
- `-n 8` with 4 machines → **Each machine gets 2 processes**.
- `-n 16` with 4 machines → **Each machine gets 4 processes**.

### 4️⃣ Running **MPI on a Single Machine**
Even if you don’t have multiple machines, you can **run MPI locally**:
  ```sh
  mpiexec -n 4 ./main
  ```
🔹 This will **split execution into 4 separate parallel processes** on your single machine.  
🔹 Useful for **testing before running on a cluster**.

### 5️⃣ **Increasing Image Resolution for Zooming**
To **improve image detail** when zooming in, modify these parameters in `main.cpp`:

- **Increase `width` and `height`**:
  
  This makes the image sharper when zoomed in.

- **Increase `max_iter`**:
 
  More iterations allow for finer fractal details.

⚠️ **Warning:**  

  Higher resolutions and iterations will **increase computational load**

  Generated image will **take up more space**

## Notes
- You must **set up MPI and passwordless SSH for multi-machine execution**.
- If you're unfamiliar with the .PPM format, I highly recommend this: https://netpbm.sourceforge.net/doc/ppm.html
