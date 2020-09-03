#ifndef HTTPCLIENT_HPP
#define HTTPCLIENT_HPP

#include <functional>
#include <string_view>
#include <cpp_redis/cpp_redis>
#include <cpprest/asyncrt_utils.h>
#include <cpprest/json.h>
#include <json/json.h>
#include <optional>

namespace bangkong {

using MessageWithButton = std::function<void(std::string_view, std::optional<Json::Value>)>;
using MessageOnly = std::function<void(std::string_view)>;

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
    virtual void get_data_total(MessageOnly&&) = 0;
    virtual void get_data_nations(MessageOnly&&) = 0;
    virtual void req_nation(MessageOnly&&, std::string_view) = 0;
    virtual void get_data_national(MessageWithButton&&) = 0;
    virtual void get_all_province(MessageWithButton&&) = 0;
    virtual void get_all_hospital(MessageWithButton&&) = 0;
    virtual void get_hospital(MessageOnly&&, std::string_view) = 0;
    virtual void get_hoaxs(MessageOnly&&) = 0;
    virtual void get_article(MessageOnly&&, bangkong::TypeArticle) = 0;
    virtual void get_ciamis(MessageOnly&&) = 0;
};

class HttpClient: AbsHttpClient
{
public:
    HttpClient();
    ~HttpClient() {}
    void get_data_total(std::function<void (std::string_view)> &&) override;
    void get_data_nations(std::function<void (std::string_view)> &&) override;
    void req_nation(std::function<void (std::string_view)> &&, std::string_view) override;
    void get_data_national(std::function<void (std::string_view, std::optional<Json::Value>)> &&) override;
    void get_all_province(std::function<void (std::string_view, std::optional<Json::Value>)> &&) override;
    void get_all_hospital(MessageWithButton &&) override;
    void get_hospital(MessageOnly &&, std::string_view prov) override;
    void get_hoaxs(MessageOnly &&) override;
    void get_article(MessageOnly&&, bangkong::TypeArticle) override;
    void get_ciamis(MessageOnly&&) override;
private:
    pplx::task<std::vector<unsigned char>> get_timestamp();
    pplx::task<std::vector<unsigned char>> get_hoaxs();
    void build_and_parse_message(web::json::value &&json_obj, std::string &result);
private:
    cpp_redis::client redis_client;
};
}

#endif // HTTPCLIENT_HPP
