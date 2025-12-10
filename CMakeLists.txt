cmake_minimum_required(VERSION 3.10)
project(r-type)

########## SETUP ##########
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(RT_SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(RT_HDR_DIR "${PROJECT_SOURCE_DIR}/include")
include(FetchContent)

########## OPTIONS ##########
option(ENABLE_TE_TESTS "Build tests along with the library" OFF)
option(ENABLE_TE_COVERAGE "Enable coverage reporting" OFF)

########## COVERAGE ##########
if (ENABLE_TE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage -lgcov")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
endif()

########## TRUE ENGINE ##########
add_subdirectory(TrueEngine)

########## GAME ##########

# SERVER
add_executable( ${PROJECT_NAME}_server
    ${RT_SRC_DIR}/server_main.cpp
    ${RT_SRC_DIR}/RtypeServer.cpp
)

target_include_directories(${PROJECT_NAME}_server
    PUBLIC
        ${RT_HDR_DIR}
        ${TE_HDR_DIR}
)

target_link_libraries(${PROJECT_NAME}_server
    PRIVATE
        TrueEngine
        physic
)

# CLIENT
add_executable( ${PROJECT_NAME}_client
    ${RT_SRC_DIR}/client_main.cpp
    ${RT_SRC_DIR}/RtypeClient.cpp
)

target_include_directories(${PROJECT_NAME}_client
    PUBLIC
        ${RT_HDR_DIR}
        ${TE_HDR_DIR}
)

target_link_libraries(${PROJECT_NAME}_client
    PRIVATE
        TrueEngine
        physic
)

########## TESTS ##########
if (ENABLE_TE_TESTS)
  enable_testing()
  add_subdirectory(tests/unit_tests)
endif ()
