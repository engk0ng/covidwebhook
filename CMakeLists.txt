cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME covidwebhook)
project(${PROJECT_NAME})

project(covidwebhook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpprestsdk REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(3rdparty/drogon)

add_executable(covidwebhook
    main.cpp
    controller/requesthandler.cpp
    controller/requesthandler.hpp
    models/chat.cpp
    models/chat.hpp
    utils/jsonconverter.cpp
    utils/jsonconverter.hpp
    commander/sender.cpp
    commander/sender.hpp
    models/message.cpp
    models/message.hpp
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        drogon
        PUBLIC
        fmt::fmt
        cpprestsdk::cpprest
        )
