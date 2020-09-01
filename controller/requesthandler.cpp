#include "requesthandler.hpp"
#include <iostream>

#include "../utils/jsonconverter.hpp"

#include "../models/chat.hpp"
#include "../commander/sender.hpp"

void RequestHandler::update(const drogon::HttpRequestPtr &req,
                            std::function<void (const drogon::HttpResponsePtr &)> &&callback) const {
    std::pair<Json::Value, JSONCPP_STRING> data = bangkong::JsonConverter(req->getBody())
            .build_json()
            .data_json();

    if (data.second == "") {
        Json::Value root = std::move(data.first);
        bangkong::Chat chat;

        chat.set_id(root["message"]["chat"].get("id", 0).asInt64());
        chat.set_first_name(root["message"]["chat"].get("first_name", "").asString());
        chat.set_last_name(root["message"]["chat"].get("last_name", "").asString());
        chat.set_command(root["message"].get("text", "").asString());

        const Json::Value entities = root["message"]["entities"];
        Json::Value::const_iterator it;
        for (it = entities.begin(); it != entities.end(); ++it) {
            chat.set_type((*it)["type"].asString());
        }
        if (chat.get_type() == "bot_command") {
            auto sender = bangkong::Sender(&chat);
            sender.sendMessage();
        }
    }
    else {
        std::cout << data.second << std::endl;
    }

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody("");
    callback(resp);
}
