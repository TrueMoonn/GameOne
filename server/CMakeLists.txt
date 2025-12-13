cmake_minimum_required(VERSION 3.10)
project(r-type_server)

########## SETUP ##########
set(RT_SERV_SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(RT_SERV_HDR_DIR "${PROJECT_SOURCE_DIR}/include")

########## GAME ##########
add_executable( ${PROJECT_NAME}
    # GLOBAL
    ${RT_SRC_DIR}/Game.cpp

    # LOCAL
    ${RT_SERV_SRC_DIR}/main.cpp
    ${RT_SERV_SRC_DIR}/RtypeServer.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${TE_HDR_DIR}
        ${RT_HDR_DIR}
        ${RT_SERV_HDR_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        TrueEngine
)

########## GET PLUGINS ##########
add_dependencies(${PROJECT_NAME} addon::physic)
add_dependencies(${PROJECT_NAME} addon::interaction)
add_dependencies(${PROJECT_NAME} addon::entity_spec)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/plugins
    COMMAND ${CMAKE_COMMAND} -E copy
        ${TE_PLUGIN_PATH}/physic.so
        ${CMAKE_CURRENT_SOURCE_DIR}/plugins/physic.so
    COMMAND ${CMAKE_COMMAND} -E copy
        ${TE_PLUGIN_PATH}/interaction.so
        ${CMAKE_CURRENT_SOURCE_DIR}/plugins/interaction.so
    COMMAND ${CMAKE_COMMAND} -E copy
        ${TE_PLUGIN_PATH}/entity_spec.so
        ${CMAKE_CURRENT_SOURCE_DIR}/plugins/entity_spec.so
)
