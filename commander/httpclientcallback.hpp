#ifndef HTTPCLIENTCALLBACK_HPP
#define HTTPCLIENTCALLBACK_HPP

#include <Poco/Event.h>
#include <easyhttpcpp/EasyHttp.h>
#include <optional>
#include <functional>
#include <json/json.h>

namespace bangkong {

using MessageWithButton = std::function<void(std::string_view, std::optional<Json::Value>)>;
using MessageOnly = std::function<void(std::string&&)>;


class HttpClientCallback : public easyhttpcpp::ResponseCallback {
public:
    typedef Poco::AutoPtr<HttpClientCallback> Ptr;

    HttpClientCallback(MessageOnly&& result): m_result{result} {

    }

    virtual ~HttpClientCallback() {};

    // called when response was returned by the remote server.
    void onResponse(easyhttpcpp::Response::Ptr pResponse) {

        std::string res = "";
        if (pResponse->getRequest().get()->getMethod() == easyhttpcpp::Request::HttpMethodGet) {
            if (!pResponse->isSuccessful()) {
                res = "<pre>Data tidak dapat ditampilkan</pre>";
            } else {
                if (pResponse->getCode() == 200) {
                    res = pResponse->getBody()->toString();
                }
                else {
                    res = "<pre>Data tidak dapat ditampilkan</pre>";
                }
            }
        }

        m_completionWaiter.set();
        m_result(std::move(res));
    }

    // called when any error occurred in connecting with remote server or any other internal error.
    void onFailure(easyhttpcpp::HttpException::Ptr) {
        m_completionWaiter.set();
        m_result("<pre>Data tidak dapat ditampilkan</pre>");
    }

    bool waitForCompletion() {
        return m_completionWaiter.tryWait(10 * 1000 /* milliseconds*/);
    }

private:
    Poco::Event m_completionWaiter;
    MessageOnly m_result;
};
}

#endif // HTTPCLIENTCALLBACK_HPP
