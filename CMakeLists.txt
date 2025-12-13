cmake_minimum_required(VERSION 3.10)
project(r-type)

########## SETUP ##########
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(RT_SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(RT_HDR_DIR "${PROJECT_SOURCE_DIR}/include")
set(TE_PLUGIN_PATH "${PROJECT_SOURCE_DIR}/TrueEngine/plugins")

########## TRUE ENGINE ##########
add_subdirectory(TrueEngine)

########## GAME ##########
add_subdirectory(server)
add_subdirectory(client)
