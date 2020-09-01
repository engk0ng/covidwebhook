cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME covidwebhook)
project(${PROJECT_NAME})

project(covidwebhook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(3rdparty/drogon)

add_executable(covidwebhook
    main.cpp
    controller/requesthandler.cpp
    controller/requesthandler.hpp
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        drogon
        )
