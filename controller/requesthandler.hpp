#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../crow_all.h"

namespace bangkong {
    class Chat;
}

namespace Json {
    class Value;
}

class RequestHandler
{
public:
    explicit RequestHandler() {}

    void update(const crow::request& req);
private:
    bangkong::Chat get_chat(Json::Value&&) const noexcept;
    void build_chat_from_message(Json::Value&&, bangkong::Chat&) const noexcept;
};

#endif // REQUESTHANDLER_HPP
