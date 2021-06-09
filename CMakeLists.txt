cmake_minimum_required(VERSION 3.10.2)
project(myLSM)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "-g -O2 -Wall")

set(LSM_SRC MergeBuffer.cc DiskRepo.cc kvstore.cc Level.cc SkipList.cc SSTable.cc global.cc)

add_executable(correctness correctness.cc ${LSM_SRC})
add_executable(persistence persistence.cc ${LSM_SRC})
add_executable(efficiency efficiency.cc ${LSM_SRC})
add_executable(hard hard.cc ${LSM_SRC})
add_executable(gotkey gotkey.cc ${LSM_SRC})

