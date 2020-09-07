#include "sender.hpp"
#include <iostream>

#include "../models/message.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>

#include "../utils/utils.hpp"

bangkong::Sender::Sender(AbsMessage* chat): m_chat{chat}, m_status_send {false} {
    // create http client
}

bangkong::Sender::~Sender() {
}

bangkong::Sender& bangkong::Sender::sendMessage() noexcept {
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
    else {
        if (m_chat->get_command().length() == 4 ) {
            Message {m_chat}
                    .build_for_nation_request(m_chat->get_command());
        }
        else if (m_chat->get_data() == "provinsi") {
            Message{m_chat}
                    .build_for_all_province();
        }
        else {
            std::vector<std::string> seq;
            std::string dta = m_chat->get_data();
            boost::split(seq, dta, boost::is_any_of(","));
            if (seq.size() == 5) {
                Message{m_chat}
                        .build_for_province(std::move(seq));
            }
            else if (m_chat->get_command() != "/hoax") {
                Message{m_chat}
                        .build_for_hospital(m_chat->get_data());
            }
        }
    }

    return *this;
}
