#ifndef REDIRESTMESSAGE_HPP
#define REDIRESTMESSAGE_HPP

#include <iostream>
#include <easyhttpcpp/EasyHttp.h>
#include <json/json.h>

namespace bangkong {
class RedirectMessage
{
public:
    RedirectMessage(Json::Value&& json): m_json{json} {
        //std::cout << m_json << std::endl;
        p_http_client = easyhttpcpp::EasyHttp::Builder().build();
    }
    void send_data();
private:
    easyhttpcpp::Call::Ptr call_post_request(std::string_view, std::string_view);
private:
    Json::Value m_json;
    easyhttpcpp::EasyHttp::Ptr p_http_client;
};
}


#endif // REDIRESTMESSAGE_HPP
