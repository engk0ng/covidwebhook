cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME covidwebhook)
project(${PROJECT_NAME})

project(covidwebhook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpprestsdk REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system date_time REQUIRED)

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
    utils/money.h
    utils/httpclient.cpp
    utils/httpclient.hpp
    data/countries.json
    models/article_model.hpp
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        drogon
        PUBLIC
        /usr/local/lib/libcpp_redis.a
        /usr/local/lib/libtacopie.a
        fmt::fmt
        cpprestsdk::cpprest
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
        )
