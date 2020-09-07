#ifndef HTTPCLIENT_HPP
#define HTTPCLIENT_HPP

#include <functional>
#include <string_view>
#include <json/json.h>
#include <optional>
#include <easyhttpcpp/EasyHttp.h>
#include <Poco/Redis/Client.h>

namespace bangkong {

enum TypeArticle {
    NASEHAT = 0,
    AKHBAR,
    ASYSYARIAH,
    BNPB,
    COVIDGOID
};

class AbsHttpClient {
public:
    virtual ~AbsHttpClient() {};
    virtual void post_message_only(Json::Value&&) = 0;
    virtual void send_data_total(Json::Value&&) = 0;
    virtual void send_data_nations(Json::Value&&) = 0;
    virtual void send_nation(Json::Value&&, std::string_view) = 0;
    virtual void send_data_national(Json::Value&&) = 0;
    virtual void send_all_province(Json::Value&&) = 0;
    virtual void send_province(Json::Value&&) = 0;
    virtual void send_all_hospital(Json::Value&&) = 0;
    virtual void send_hospital(Json::Value&&, std::string_view) = 0;
    virtual void send_hoaxs(Json::Value&&) = 0;
    virtual void send_article(Json::Value&&, bangkong::TypeArticle) = 0;
    virtual void send_ciamis(Json::Value&&) = 0;
};

class HttpClient: AbsHttpClient
{
public:
    HttpClient()  {
        p_http_client = easyhttpcpp::EasyHttp::Builder().build();
    }
    ~HttpClient() {
        if (p_http_client.get() == nullptr) {
            p_http_client.reset(nullptr);
        }
    }
    void post_message_only(Json::Value &&) override;
    void send_data_total(Json::Value&&) override;
    void send_data_nations(Json::Value&&) override;
    void send_nation(Json::Value&&, std::string_view) override;
    void send_data_national(Json::Value&&) override;
    void send_all_province(Json::Value&&) override;
    void send_province(Json::Value&&) override;
    void send_all_hospital(Json::Value&&) override;
    void send_hospital(Json::Value&&, std::string_view) override;
    void send_hoaxs(Json::Value&&) override;
    void send_article(Json::Value&&, bangkong::TypeArticle) override;
    void send_ciamis(Json::Value&&) override;
private:
    uint64_t get_timestamp();
    std::string build_and_parse_message(Json::Value&&json_obj);
    easyhttpcpp::Call::Ptr call_get_request(std::string_view);
    void set_redis_key(const std::string& key, std::string&& value);
    std::string get_redis_key(const std::string& key);
    bool redis_auth(Poco::Redis::Client& client, const std::string& usr, const std::string& pass);
private:
    easyhttpcpp::EasyHttp::Ptr p_http_client;
};
}

#endif // HTTPCLIENT_HPP
