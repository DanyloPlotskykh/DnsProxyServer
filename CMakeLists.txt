cmake_minimum_required(VERSION 3.22)

project(dnsServer)

SET(SERVER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/handleRequest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/loadConfig.c
)

add_executable(${PROJECT_NAME} ${SERVER_SRC})