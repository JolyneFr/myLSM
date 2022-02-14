cmake_minimum_required(VERSION 3.10.2)
project(myLSM)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "-g -O2 -Wall")


set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)

include_directories(${INCLUDE_DIR})

file(GLOB LSM_SRC ${SOURCE_DIR}/*.cc)

add_executable(correctness ${TEST_DIR}/correctness.cc ${LSM_SRC})
add_executable(persistence ${TEST_DIR}/persistence.cc ${LSM_SRC})
add_executable(hard ${TEST_DIR}/hard.cc ${LSM_SRC})
add_executable(gotkey ${TEST_DIR}/gotkey.cc ${LSM_SRC})

