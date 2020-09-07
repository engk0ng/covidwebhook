#ifndef SENDER_HPP
#define SENDER_HPP

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
};
}

#endif // SENDER_HPP
