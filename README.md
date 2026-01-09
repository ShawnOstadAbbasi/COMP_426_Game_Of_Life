# Comp 426 Multicore Programming Project
## 1. Description
The goal of this project was to recreate the "Game of Life Simulation" using ***multicore programming paradignms for enhanced computation*** as well as ***OpenGL for rendering*** on an array/window of size 1024x768. There are four iterations of this project, all of which demonstrate different forms of parallelization and optimizations with the goal of maximizing computation while maintaining correctness. ***Performance is measured in achievable frmaes per second (FPS)***
## Assignment 1 (STD Threads)
The first itertion of the project implements multithreading through the ***C++ Standard Library***. Using multithreading, work is split between precisecly 8 threads, all of which perform the computation for 128 rows in the 1024x768 window. While this approach offers enhanced performance compared to a pure sequential execution its performance relies on the number of cores, and processing power of the executing CPU.
## Assignment 2 (Intel Threading Building Blocks TBB)
### The necessary TBB libraries and header files are provided in the dependencies folder
The second iteration of the project improves the performance through the use of ***Intel's TBB library***. TBB provides increased performance with its aids in multicore programming with the use of it's work stealing scheduler, automatic grain size, and thread management. While this iteration showed improved results over the first assignment, performance is still hindered due to the CPU's trouble in ***data parallelism***.
## Assignment 3 (OpenCL)
Due to the data parallel nature of the projet using the GPU for the parallel computation provides significantly improved performance in comparion to using the CPU. Using OpenCL's device queue, kernels, and work-groups performance again was greatly improved, demonstrating great ***data parallelism*** on the GPU. 
## Assignment 4 (OpenCL OpenGL Interoperability)
The final optimizations made to the iterative project was to obtain interoperability between _OpenCL_ and _OpenGL_ by sharing a buffer to avoid having to copy data from in OpenCL from the GPU to the CPU and then again from the CPU to the GPU for OpenGL rendering. Through the use of a shared buffer data transfers would no longer be necessary, with OpenCL and OpenGL being capable of processing the same data. This optimization yields greater results than the previous iteration. Another optimization is to utilize OpenCL's possibiliy to utilize multiple devices. Through using the GPU device for the majority of the data-parallel computation and the CPU for simpler calculations we can obtain even greater performance.

## 2. Requirements
While most of the necessary libraries and headers are provided in the dependencies folder there _may_ be additional necessary downloads.
* [Intel OpenCL Runtime](https://www.intel.com/content/www/us/en/developer/articles/tool/opencl-drivers.html)
* [Intel Threading Building Blocks](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html)