#include "requesthandler.hpp"
#include <iostream>

#include "../utils/jsonconverter.hpp"

#include "../models/chat.hpp"
#include "../commander/sender.hpp"

void RequestHandler::update(const drogon::HttpRequestPtr &req,
                            std::function<void (const drogon::HttpResponsePtr &)> &&callback) const {
    //std::cout << req->getBody() << std::endl;
    std::pair<Json::Value, JSONCPP_STRING> data = bangkong::JsonConverter(req->getBody())
            .build_json_from_string()
            .data_json();

    if (data.second == "") {
        Json::Value root = std::move(data.first);
        bangkong::Chat chat = get_chat(std::move(root));
        bangkong::Sender(&chat).sendMessage();
    }

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody("");
    callback(resp);
}

bangkong::Chat RequestHandler::get_chat(Json::Value&& root) const noexcept {
    bangkong::Chat chat;

    if (root["callback_query"].isNull() == false) {
        Json::Value message = root["callback_query"];
        chat.set_data(message["data"].asString());
        build_chat_from_message(std::move(message), chat);
    }
    else {
        build_chat_from_message(std::move(root), chat);
    }
    return chat;
}

void RequestHandler::build_chat_from_message(Json::Value &&root, bangkong::Chat& chat) const noexcept {
    chat.set_id(root["message"]["chat"].get("id", 0).asInt64());
    chat.set_first_name(root["message"]["chat"].get("first_name", "").asString());
    chat.set_last_name(root["message"]["chat"].get("last_name", "").asString());
    chat.set_command(root["message"].get("text", "").asString());

    //std::cout << root["message"]["chat"].get("id", 0).asInt64() << std::endl;
    //std::cout << chat.get_data() << std::endl;

    const Json::Value entities = root["message"]["entities"];
    Json::Value::const_iterator it;
    for (it = entities.begin(); it != entities.end(); ++it) {
        chat.set_type((*it)["type"].asString());
    }
}
