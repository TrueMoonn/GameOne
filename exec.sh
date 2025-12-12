#!/bin/bash

copy_plugins() {
    echo "Copying plugins..."

    mkdir -p ./plugins_server
    mkdir -p ./plugins_client
    
    # You have to select the plugin you want for each side (Client/Server)

    # Copy plugins for server (no display/sfml needed)
    cp -f ./TrueEngine/plugins/interaction.so ./plugins_server/ 2>/dev/null || true
    cp -f ./TrueEngine/plugins/physic.so ./plugins_server/ 2>/dev/null || true
    cp -f ./TrueEngine/plugins/entity_spec.so ./plugins_server/ 2>/dev/null || true
    
    # Copy plugins for client (all plugins)
    cp -f ./TrueEngine/plugins/interaction.so ./plugins_client/ 2>/dev/null || true
    cp -f ./TrueEngine/plugins/physic.so ./plugins_client/ 2>/dev/null || true
    cp -f ./TrueEngine/plugins/entity_spec.so ./plugins_client/ 2>/dev/null || true
    cp -f ./TrueEngine/plugins/display.so ./plugins_client/ 2>/dev/null || true
    cp -f ./TrueEngine/plugins/sfml.so ./plugins_client/ 2>/dev/null || true
    
    echo "Plugins copied successfully!"
}

if [[ $1 == "--build" || $1 == "-b" ]]
then
    echo "------------BUILD------------"
    if [ ! -d "./build/" ]
    then
        mkdir ./build/ && cd ./build/
        cmake ..
        cd ..
    fi
    cmake --build ./build/ -j
    copy_plugins
    echo "------------END------------"

elif [[ $1 == "--re-build" || $1 == "-rb" ]]
then
    clear
    echo "------------RE-BUILD------------"
    rm -rf ./build/ r-type_server r-type_client
    rm -rf ./TrueEngine/*.a ./TrueEngine/plugins/*.so
    mkdir ./build/ && cd ./build/
    cmake ..
    cmake --build . -j
    cd ..
    copy_plugins
    echo "------------END------------"

# elif [[ $1 == "--build-tests" || $1 == "-t" ]]
# then
#     clear
#     echo "------------TESTS------------"
#     rm -rf ./build/ r-type_server r-type_client
#     rm -rf ./TrueEngine/*.a ./TrueEngine/plugins/*.so
#     cmake .. -DENABLE_TE_TESTS=ON -DENABLE_TE_COVERAGE=ON
#     cmake --build . -j
#     ctest --output-on-failure
#     gcovr --root .. \
#         --filter '../src/' \
#         --filter '../include/' \
#         --html --html-details -o coverage.html
#     xdg-open coverage.html
#     cd ..
#     echo "------------END------------"

elif [[ $1 == "--debug-build" || $1 == "-d" ]]
then
    echo "------------DEBUG------------"
    rm -rf ./build/ r-type_server r-type_client
    rm -rf ./TrueEngine/*.a ./TrueEngine/plugins/*.so
    mkdir ./build/ && cd ./build/
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build . -v -j
    cd ..
    copy_plugins
    echo "------------END------------"

elif [[ $1 == "--clear" || $1 == "-c" ]]
then
    rm -rf ./build/ r-type_server r-type_client
    rm -rf ./TrueEngine/*.a ./TrueEngine/plugins/*.so

elif [[ $1 == "--style-check" || $1 == "-cs" ]]
then
    echo "------------CS CHECKER------------"
    pip install cpplint
    cpplint --recursive .
    echo "------------END------------"

elif [[ $1 == "--doxygen" || $1 == "-doc" ]]
then
    echo "------------DOXYGEN------------"
    rm -rf doxygen/
    doxygen
    xdg-open doxygen/html/index.html
    echo "------------END------------"

elif [[ $1 == "--help" || $1 == "-h" ]]
then
    echo "To use this executer you must use a flag:
    --build, -b             Build the program with CMake
    --style-check, -cs      Check for coding style using cpplint
    --help, -h              More information about this script
    --doxygen, -doc   Create local documentation site using Doxygen

    << This section delete the old files created by the compilation >>
    --re-build, -rb         Build the program with CMake
    --debug-build, -d       Build the program with debug and verbose
    --clear, -c             Clear files created by the compilation and more
    # --build-test, -t        Launch unit tests with coverage using GTest
"
else
    echo "error: missing or wrong flag
To use this executer you must use a flag:
    --build, -b             Build the program with CMake
    --style-check, -cs      Check for coding style using cpplint
    --help, -h              More information about this script
    --doxygen, -doc   Create local documentation site using Doxygen

    << This section delete the old files created by the compilation >>
    --re-build, -rb         Build the program with CMake
    --debug-build, -d       Build the program with debug and verbose
    --clear, -c             Clear files created by the compilation and more
    # --build-test, -t        Launch unit tests with coverage using GTest
"
fi