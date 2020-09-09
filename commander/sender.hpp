#ifndef SENDER_HPP
#define SENDER_HPP

#include <vector>
#include "../models/chat.hpp"

namespace bangkong {
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
    bangkong::AbsMessage* m_chat;
    bool m_status_send;
    std::vector<std::string> m_commands;
};
}

#endif // SENDER_HPP
