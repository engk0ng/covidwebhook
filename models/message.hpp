#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cpprest/json.h>

#include "chat.hpp"

namespace bangkong {
class Builder {
public:
    virtual web::json::value get_message() = 0;
};

class Message: public Builder
{
public:
    Message(AbsMessage* chat): m_chat{chat}, m_mode{""} {}
    Message& build_for_start() noexcept;
    Message& parse_html() noexcept;
    Message& parse_markdown() noexcept;
    web::json::value get_message() override;
private:
    AbsMessage* m_chat;
    std::string m_message;
    std::string_view m_mode;
};
}

#endif // MESSAGE_HPP
