#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <optional>

#include <cpprest/json.h>

#include "chat.hpp"
#include "../utils/httpclient.hpp"

namespace bangkong {
class Builder {
public:
    virtual web::json::value get_message() = 0;
};

class Message: public Builder
{
public:
    Message(AbsMessage* chat): m_chat{chat}, m_replay_markup {std::nullopt}, m_mode{""} {
        m_http_client = (AbsHttpClient*)new HttpClient();
    }
    ~Message() {
        if (m_http_client != nullptr) {
            delete  m_http_client;
            m_http_client = nullptr;
        }
    }
    Message& build_for_start() noexcept;
    Message& build_for_menu() noexcept;
    Message& build_for_total() noexcept;
    Message& build_for_international() noexcept;
    Message& build_for_nation_request(std::string_view) noexcept;
    Message& build_for_national() noexcept;
    Message& build_for_all_province() noexcept;
    Message& build_for_province(std::vector<std::string>&&) noexcept;
    Message& build_for_all_hospital() noexcept;
    Message& build_for_hospital(std::string_view) noexcept;
    Message& build_for_hoaxs() noexcept;
    Message& build_for_article(bangkong::TypeArticle) noexcept;
    Message& build_for_ciamis() noexcept;
    Message& parse_html() noexcept;
    Message& parse_markdown() noexcept;
    web::json::value get_message() override;
private:
    AbsMessage* m_chat;
    std::string m_message;
    std::optional<std::string> m_replay_markup;
    std::string_view m_mode;

    AbsHttpClient* m_http_client;
};
}

#endif // MESSAGE_HPP
