#include "sender.hpp"
#include <fmt/format.h>
#include <iostream>

#include "../models/message.hpp"

#include <cpprest/json.h>
#include <cpprest/uri.h>

#include <boost/date_time.hpp>
#include <boost/algorithm/string/split.hpp>

bangkong::Sender::Sender(AbsMessage* chat): m_chat{chat}, m_status_send {false} {
    m_path_command = fmt::format("{}{}", URI, get_env("TOKEN"));
}

bangkong::Sender::~Sender() {

}

std::string_view bangkong::Sender::get_env(std::string_view var) noexcept {
    char* val = std::getenv(var.data());
    return val == NULL ? "": val;
}
bangkong::Sender& bangkong::Sender::sendMessage() noexcept {
    m_path_command.append("/sendMessage");
    web::http::client::http_client client {m_path_command};
    web::json::value message_json;

    if (m_chat->get_command() == "/start") {
        message_json =  Message {m_chat}
                .build_for_start()
                .parse_html()
                .get_message();
    }
    else if (m_chat->get_command() == "/menu") {
        message_json = Message {m_chat}
                .build_for_menu()
                .parse_html()
                .get_message();
    }
    else if (m_chat->get_command() == "/total") {
        message_json = Message {m_chat}
                .build_for_total()
                .parse_html()
                .get_message();
    }
    else if (m_chat->get_command() == "/internasional") {
        message_json = Message {m_chat}
                .build_for_international()
                .parse_html()
                .get_message();
    }
    else if (m_chat->get_command() == "/nasional") {
        message_json = Message(m_chat)
                .build_for_national()
                .parse_html()
                .get_message();
    }
    else if (m_chat->get_command() == "/rs") {
        message_json = Message(m_chat)
                .build_for_all_hospital()
                .parse_html()
                .get_message();
    }
    else if(m_chat->get_command() == "/hoax") {
        message_json = Message(m_chat)
                .build_for_hoaxs()
                .parse_html()
                .get_message();
    }
    else if (m_chat->get_command() == "/nasehat") {
        message_json = Message(m_chat)
                .build_for_article(bangkong::TypeArticle::NASEHAT)
                .parse_html()
                .get_message();
    }
    else if(m_chat->get_command() == "/akhbar") {
        message_json = Message(m_chat)
                .build_for_article(bangkong::TypeArticle::AKHBAR)
                .parse_html()
                .get_message();
    }
    else if(m_chat->get_command() == "/bnpb") {
        message_json = Message(m_chat)
                .build_for_article(bangkong::TypeArticle::BNPB)
                .parse_html()
                .get_message();
    }
    else if(m_chat->get_command() == "/covid19goid") {
        message_json = Message(m_chat)
                .build_for_article(bangkong::TypeArticle::COVIDGOID)
                .parse_html()
                .get_message();
    }
    else if (m_chat->get_command() == "/artikel_islami") {
        message_json = Message(m_chat)
                .build_for_article(bangkong::TypeArticle::ASYSYARIAH)
                .parse_html()
                .get_message();
    }
    else if (m_chat->get_command() == "/ciamis") {
        message_json = Message(m_chat)
                .build_for_ciamis()
                .parse_html()
                .get_message();
    }
    else {
        if (m_chat->get_command().length() == 4 ) {
            message_json = Message {m_chat}
                    .build_for_nation_request(m_chat->get_command())
                    .parse_html()
                    .get_message();
        }

        else if (m_chat->get_data() == "provinsi") {
            message_json = Message{m_chat}
                    .build_for_all_province()
                    .parse_html()
                    .get_message();

        }
        else {
            std::vector<std::string> seq;
            boost::algorithm::split(seq, m_chat->get_data(), boost::is_any_of(","));
            if (seq.size() == 5) {
                message_json = Message{m_chat}
                        .build_for_province(std::move(seq))
                        .parse_html()
                        .get_message();
            }
            else if (m_chat->get_command() != "/hoax") {
                message_json = Message{m_chat}
                        .build_for_hospital(m_chat->get_data())
                        .parse_html()
                        .get_message();
            }
        }
    }
    client.request(web::http::methods::POST, U(""), message_json)
            .then([](const web::http::http_response& response){
                return response.extract_json();
             })
            .then([this](const pplx::task<web::json::value>& task){
                web::json::value json = task.get();
                //std::cout << json << std::endl;
                m_status_send = json.at("ok").as_bool();
            })
            .wait();
    return *this;
}
