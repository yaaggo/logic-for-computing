cmake_minimum_required(VERSION 3.13)

project(logic-eval C)

include_directories(include)

file(GLOB LIBSOURCES "include/*.c")
file(GLOB SRCSOURCES "src/*.c")

add_executable(logic-eval ${LIBSOURCES} ${SRCSOURCES})