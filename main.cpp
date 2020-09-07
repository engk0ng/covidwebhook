#include <iostream>

#include "controller/requesthandler.hpp"

#include "utils/utils.hpp"

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include "crow_all.h"

using namespace std;

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/update").methods(crow::HTTPMethod::Post)([](const crow::request& req, crow::response& res){
        RequestHandler request_handler;
        request_handler.update(req);
        res.set_header("Content-Type", "text/plain");
        res.write("");
        res.end();
    });

    CROW_ROUTE(app, "/")([](){
        return "<div><h1>Halo</h1></div>";
    });
    std::string_view port_str = bangkong::get_env("PORT");
    uint64_t port = boost::lexical_cast<uint64_t>(port_str != "" ? port_str: "18000");
    app.loglevel(crow::LogLevel::WARNING);
    app.port(port).multithreaded().run();
}
