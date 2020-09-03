#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <drogon/HttpController.h>

using namespace drogon;

namespace bangkong {
    class Chat;
}

namespace Json {
    class Value;
}

class RequestHandler: public drogon::HttpController<RequestHandler, false>
{
public:
    METHOD_LIST_BEGIN
    METHOD_ADD(RequestHandler::update, "/", Post);
    METHOD_LIST_END

    explicit RequestHandler() {}

    void update(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;
private:
    bangkong::Chat get_chat(Json::Value&&) const noexcept;
    void build_chat_from_message(Json::Value&&, bangkong::Chat&) const noexcept;
};

#endif // REQUESTHANDLER_HPP
