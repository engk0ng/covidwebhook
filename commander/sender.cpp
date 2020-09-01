#include "sender.hpp"
#include <fmt/format.h>

#include "../models/message.hpp"

#include <cpprest/json.h>

bangkong::Sender::Sender(AbsMessage* chat): m_chat{chat} {
    m_path_command = fmt::format("{}{}", URI, get_env("TOKEN"));
}

bangkong::Sender::~Sender() {

}

std::string_view bangkong::Sender::get_env(std::string_view var) noexcept {
    char* val = std::getenv(var.data());
    return val == NULL ? "": val;
}

void bangkong::Sender::sendMessage() noexcept {
    m_path_command.append("/sendMessage");
    web::http::client::http_client client {m_path_command};
    web::json::value message_json;
    if (m_chat->get_command() == "/start") {
        message_json =  Message {m_chat}
                .build_for_start()
                .parse_html()
                .get_message();
    }
    client.request(web::http::methods::POST, U(""), message_json);
}
