#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <drogon/HttpController.h>

using namespace drogon;

class RequestHandler: public drogon::HttpController<RequestHandler, false>
{
public:
    METHOD_LIST_BEGIN
    METHOD_ADD(RequestHandler::update, "/", Post);
    METHOD_LIST_END

    explicit RequestHandler() {}

    void update(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;
};

#endif // REQUESTHANDLER_HPP
