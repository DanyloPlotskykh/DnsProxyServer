cmake_minimum_required(VERSION 3.22)

project(dnsServer)

add_executable(${PROJECT_NAME} main.c handleRequest.c loadConfig.c)