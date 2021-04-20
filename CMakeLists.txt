cmake_minimum_required(VERSION 3.10.2)
project(myLSM)

set(CMAKE_CXX_STANDARD 14)

set(LSM_SRC DiskManager.cc kvstore.cc LevelStorage.cc MergeBuffer.cc SkipList.cc SSTable.cc global.cc)

add_executable(correctness correctness.cc ${LSM_SRC})
add_executable(persistence persistence.cc ${LSM_SRC})


