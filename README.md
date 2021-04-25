## myLSM: KVStore using Log-structured Merge Tree

Current version of myLSM can pass all correctness / persistence test on Linux, 
without any memory-leak. However, trying to run test on Windows would cause unpredictable 
problems. I will fix this issue 2 weeks later.

Explanation of each class / file:

```text
.
├── CMakeList.txt  // build file of myLSM using cmake
├── README.md // This readme file
├── data      // Data directory used in test
├── kvstore     // Top level implementation for LSM tree
├── SkipList     // Data structure of MemTable
├── DiskRepo   // Manage all levels stored in disk, handling compaction
├── Level    // Store all ssTables in the same level
├── SSTable     // Maintain metadata of a stored sorted table
├── MergeBuffer   // Linear structure for generating SSTs when merging
├── global      // Definitions of generic constants, functions and structs
├── kvstore_api.h  // A defined interface of key-value pair store program
├── utils.h         // Provides some cross-platform file/directory interface
├── MurmurHash3.h  // Provides murmur3 hash function
├── correctness.cc // Correctness test
├── persistence.cc // Persistence test
└── test.h         // Base class for testing
```

### TODOs:
+ Fixes the issue that doesn't work properly under Windows
+ Optimize compact operation by handling one overflowed sst a time  

