#include "requesthandler.hpp"
#include <iostream>

#include <json/json.h>

void RequestHandler::update(const drogon::HttpRequestPtr &req,
                            std::function<void (const drogon::HttpResponsePtr &)> &&callback) const {
    Json::Value json(req->body().data());
    std::cout << req->body() << std::endl;
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody("");
    callback(resp);
}
