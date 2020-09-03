#ifndef SENDER_HPP
#define SENDER_HPP

#include "../models/chat.hpp"

#include <cpprest/http_client.h>

namespace bangkong {
constexpr static const char* URI = "https://api.telegram.org/bot";
class Sender
{
public:
    Sender(AbsMessage* chat);
    ~Sender();
    Sender& sendMessage() noexcept;
    bool status() const {
        return m_status_send;
    }
private:
    std::string_view get_env(std::string_view var) noexcept;
private:
    bangkong::AbsMessage* m_chat;
    std::string m_path_command;
    bool m_status_send;
};
}

#endif // SENDER_HPP
