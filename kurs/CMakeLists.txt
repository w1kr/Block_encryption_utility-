cmake_minimum_required(VERSION 3.15)

project(App CXX)

add_executable(App "src/magma.cpp"
                   "src/rw_file.cpp"
                   "src/main.cpp")
