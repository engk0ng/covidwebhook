cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME covidwebhook)
project(${PROJECT_NAME})

project(covidwebhook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS *)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules/)

find_package(OpenSSL REQUIRED)
find_package(Poco REQUIRED Foundation Data DataSQLite Net NetSSL Redis Crypto CONFIG)
find_package(easyhttpcpp REQUIRED CONFIG)
find_package(fmt REQUIRED)
find_package(Boost COMPONENTS system date_time thread fiber REQUIRED)

find_package(jsoncpp REQUIRED)
list(APPEND INCLUDE_DIRS_FOR_DYNAMIC_VIEW ${JSONCPP_INCLUDE_DIRS})

set(DATE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/3rdparty/date/include)

#find_library(cpprestsdk-lib cpprest)
#message(STATUS "cpprestsdk-lib = ${cpprestsdk-lib}")


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
    models/article_model.hpp
    utils/utils.hpp
    commander/httpclientcallback.hpp
    commander/redirectmessage.cpp
    commander/redirectmessage.hpp
    )

target_include_directories(covidwebhook
        PUBLIC
        ${DATE_INCLUDE_DIRS}
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        Jsoncpp_lib
        fmt::fmt
        easyhttpcpp::easyhttp
        Poco::Foundation
        Poco::Data Poco::DataSQLite
        Poco::Net Poco::NetSSL Poco::Crypto
        OpenSSL::SSL OpenSSL::Crypto
        Poco::Redis
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
        )
