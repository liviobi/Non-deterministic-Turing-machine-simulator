cmake_minimum_required(VERSION 3.10)
project(FSM C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -fopenmp)
set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")

add_executable(FSM main.c)