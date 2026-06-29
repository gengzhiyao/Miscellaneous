cmake_minimum_required(VERSION 3.12)

project(ModernCpp)

set(CMAKE_CXX_STANDARD 17) #设置C++标准

set(CMAKE_CXX_EXTENSIONS OFF) #禁用编译器扩展

add_definitions()

# add_subdirectory(src)

add_subdirectory(Item2)
add_subdirectory(Item3)
add_subdirectory(Item4)
add_subdirectory(Item5)
add_subdirectory(Logger)
add_subdirectory(SFINAE)
add_subdirectory(Exception)
add_subdirectory(New)
add_subdirectory(IniPaser)
add_subdirectory(ThreadPool)
add_subdirectory(DataStructure_Graph)

