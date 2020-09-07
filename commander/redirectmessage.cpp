#include "redirectmessage.hpp"

#include "../utils/utils.hpp"

#include <string>
#include <string_view>
#include <fmt/format.h>

#include "httpclientcallback.hpp"

using namespace bangkong;

constexpr static const char* URI = "https://api.telegram.org/bot";

easyhttpcpp::Call::Ptr RedirectMessage::call_post_request(std::string_view url, std::string_view data) {
    easyhttpcpp::MediaType::Ptr p_media_type(new easyhttpcpp::MediaType("application/json"));
    easyhttpcpp::RequestBody::Ptr p_request_body = easyhttpcpp::RequestBody::create(p_media_type, Poco::SharedPtr<std::string>(new std::string(data)));
    easyhttpcpp::Request::Builder request_builder;
    easyhttpcpp::Request::Ptr p_request = request_builder.setUrl(url.data()).httpPost(p_request_body).build();
    easyhttpcpp::Call::Ptr p_call = p_http_client->newCall(p_request);
    return p_call;
}

void RedirectMessage::send_data() {
    std::string json_str = m_json.toStyledString();
    std::string url_path = fmt::format("{}{}/sendMessage", URI, bangkong::get_env("TOKEN"));
    easyhttpcpp::Call::Ptr p_call = call_post_request(url_path, json_str);
    MessageOnly res = [](std::string&&) {
        //std::cout << str << std::endl;
    };
    HttpClientCallback::Ptr p_response = new HttpClientCallback(std::move(res));
    p_call->executeAsync(p_response);
    p_response->waitForCompletion();
}
