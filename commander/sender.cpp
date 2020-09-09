#include "sender.hpp"
#include <iostream>

#include "../models/message.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../utils/utils.hpp"
#include "../utils/jsonconverter.hpp"

bangkong::Sender::Sender(AbsMessage* chat): m_chat{chat}, m_status_send {false}, m_commands {} {
    // create http client
    m_commands.reserve(14);
    m_commands.emplace_back("/start");
    m_commands.emplace_back("/menu");
    m_commands.emplace_back("/total");
    m_commands.emplace_back("/internasional");
    m_commands.emplace_back("/nasional");
    m_commands.emplace_back("/rs");
    m_commands.emplace_back("/hoax");
    m_commands.emplace_back("/nasehat");
    m_commands.emplace_back("/akhbar");
    m_commands.emplace_back("/bnpb");
    m_commands.emplace_back("/covid19goid");
    m_commands.emplace_back("/artikel_islami");
    m_commands.emplace_back("/ciamis");
    m_commands.emplace_back("/sumber");
}

bangkong::Sender::~Sender() {
}

bangkong::Sender& bangkong::Sender::sendMessage() noexcept {
    if (m_chat->get_command() != "") {
        auto fi = std::find(m_commands.begin(), m_commands.end(), m_chat->get_command());
        if (fi != std::end(m_commands)) {
            if (m_chat->get_command() == "/start") {
                Message(m_chat)
                        .build_for_start();
            }
            else if (m_chat->get_command() == "/menu") {
                Message {m_chat}.build_for_menu();
            }
            else if (m_chat->get_command() == "/total") {
                Message {m_chat}
                        .build_for_total();
            }
            else if (m_chat->get_command() == "/internasional") {
                Message {m_chat}
                        .build_for_international();
            }
            else if (m_chat->get_command() == "/nasional") {
                Message(m_chat)
                        .build_for_national();
            }
            else if (m_chat->get_command() == "/rs") {
                Message(m_chat)
                        .build_for_all_hospital();
            }
            else if(m_chat->get_command() == "/hoax") {
                Message(m_chat)
                        .build_for_hoaxs();
            }
            else if (m_chat->get_command() == "/nasehat") {
                Message(m_chat)
                        .build_for_article(bangkong::TypeArticle::NASEHAT);
            }
            else if(m_chat->get_command() == "/akhbar") {
                Message(m_chat)
                        .build_for_article(bangkong::TypeArticle::AKHBAR);
            }
            else if(m_chat->get_command() == "/bnpb") {
                Message(m_chat)
                        .build_for_article(bangkong::TypeArticle::BNPB);
            }
            else if(m_chat->get_command() == "/covid19goid") {
                Message(m_chat)
                        .build_for_article(bangkong::TypeArticle::COVIDGOID);
            }
            else if (m_chat->get_command() == "/artikel_islami") {
                Message(m_chat)
                        .build_for_article(bangkong::TypeArticle::ASYSYARIAH);
            }
            else if (m_chat->get_command() == "/ciamis") {
                Message(m_chat)
                        .build_for_ciamis();
            }
            else if (m_chat->get_command() == "/sumber") {
                Message(m_chat)
                        .build_for_credit();
            }
        }
        else {

            std::string cmd = m_chat->get_command();
            if (cmd.find("/") != std::string::npos) {
                if (m_chat->get_command().length() == 4 ) {
                    Message {m_chat}
                            .build_for_nation_request(m_chat->get_command());
                }
                else {
                    Message{m_chat}
                            .build_for_all_province();
                }
            }
            else {
                std::size_t found = m_chat->get_command().find("RS di");
                if (found != std::string::npos) {
                    std::string command = m_chat->get_command();
                    boost::algorithm::replace_all(command, "RS di ", "");
                    Message{m_chat}
                            .build_for_hospital(command);
                }
                else {
                    std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter("../prov.json")
                            .build_json_from_file()
                            .data_json();
                    if (pair.second == "") {
                        Json::Value json_obj = pair.first;
                        Json::Value::const_iterator it;
                        for (it = json_obj.begin(); it != json_obj.end(); ++it) {
                            if ((*it)["name"] == m_chat->get_command()) {
                                 Message{m_chat}.build_covid_for_province(m_chat->get_command(), *it);
                            }
                        }
                    }
                }
            }
        }
    }
    return *this;
}
