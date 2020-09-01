#include <iostream>

#include <drogon/drogon.h>

#include "controller/requesthandler.hpp"

using namespace std;

int main()
{
    auto req_handler = std::make_shared<RequestHandler>();
    drogon::app().registerController(req_handler);

    drogon::app().setLogPath("./")
            .setLogLevel(trantor::Logger::kWarn)
            .addListener("0.0.0.0", 7887)
            .setThreadNum(16)
            .run();
}
