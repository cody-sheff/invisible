To build and run this code:
Navigate to the root of your project folder and run the following commands:

1. mkdir build
2. cd build
3. cmake ..
4. make
5. ./takehome <absolute path to data directory>

Some design decisions that were made:

- The maximum number of frames is set as a constant. Based on the data files and how communications are likely to be handled, this seems reasonable and allows for some minor optimizations.
- Frame state information is tracked using vectors equal to the previously mentioned constant. The index is used for which frame the data corresponds to.
- Number of frames observed and number of valid frames are both tracked, which allows us to easily extract the heuristics of interest.
- We use a thread pool with a number of threads equal to the hardware concurrency to ensure we don't get degraded performance with a large number of files.
