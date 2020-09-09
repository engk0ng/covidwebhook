#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <optional>

#include "chat.hpp"
#include "../utils/httpclient.hpp"
#include <json/json.h>

namespace bangkong {
class Builder {
public:

};

class Message: public Builder
{
public:
    Message(AbsMessage* chat):
        m_chat{chat}, m_message {""}, m_replay_markup {std::nullopt} {
        m_http_client = (AbsHttpClient*)new HttpClient();
    }
    ~Message() {
        if (m_http_client != nullptr) {
            delete  m_http_client;
            m_http_client = nullptr;
        }
    }
    void build_for_start() noexcept;
    void build_for_menu() noexcept;
    void build_for_total() noexcept;
    void build_for_international() noexcept;
    void build_for_nation_request(std::string_view) noexcept;
    void build_for_national() noexcept;
    void build_for_all_province() noexcept;
    void build_for_province(std::vector<std::string>&&) noexcept;
    void build_for_all_hospital() noexcept;
    void build_for_hospital(std::string_view) noexcept;
    void build_for_hoaxs() noexcept;
    void build_for_article(bangkong::TypeArticle) noexcept;
    void build_for_credit() noexcept;
    void build_for_ciamis() noexcept;
    void build_covid_for_province(std::string_view, const Json::Value&) noexcept;
    Message& parse_html() noexcept;
    Message& parse_markdown() noexcept;
    void build_message() noexcept;
private:
    AbsMessage* m_chat;
    std::string m_message;
    std::optional<std::string> m_replay_markup;
    std::string_view m_mode;
    Json::Value m_message_json;
    AbsHttpClient* m_http_client;
};
}

#endif // MESSAGE_HPP
