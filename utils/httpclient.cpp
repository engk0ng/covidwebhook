#include "httpclient.hpp"

#include <string_view>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/http_listener.h>

#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <boost/date_time/gregorian/greg_calendar.hpp>

#include <fmt/format.h>
#include <chrono>

#include "money.h"
#include "jsonconverter.hpp"
#include "../models/article_model.hpp"

using namespace bangkong;
using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using namespace web::http::experimental::listener;

static constexpr const char* url_total = "https://covid19.mathdro.id/api";
static constexpr const char* date_format = "%A, %d %B %Y";
static constexpr const char* country_url = "https://covid19.mathdro.id/api/countries";
static constexpr const char* nasional_url = "https://dekontaminasi.com/api/id/covid19/stats";
static constexpr const char* timestamp_url = "https://dekontaminasi.com/api/id/covid19/stats.timestamp";
static constexpr const char* hoaxs_url = "https://dekontaminasi.com/api/id/covid19/hoaxes";
static constexpr const char* rs_url = "https://dekontaminasi.com/api/id/covid19/hospitals";
static constexpr const char* api_article = "https://cryptic-brushlands-41995.herokuapp.com/";
static constexpr const char* hint_message = "<b>Jangan lupa PAKAI MASKER dan jalankan protokol kesehatan!</b>";

namespace bangkong {

static constexpr const char* PROV_KEY = "__provinsi__";

static std::string months[12]{"Januari", "Februari", "Maret", "April",
"Mei", "Juni", "Juli", "Agustus", "September", "Oktober",
"November", "Desember"};
static std::string weekdays[7]{"Ahad", "Senin", "Selasa",
"Rabu", "Kamis", "Jum\'at", "Sabtu"};

static const std::string prov_key = "provinsi_key";
static const std::string rs_key = "rs_key";

std::string date_day_format(const boost::gregorian::date& date) {
    boost::gregorian::date d{date.year(), date.month(), date.day()};
    boost::gregorian::date_facet *df = new boost::gregorian::date_facet{date_format};
    df->long_month_names(std::vector<std::string>{months, months + 12});
    df->long_weekday_names(std::vector<std::string>{weekdays,
    weekdays + 7});
    std::cout.imbue(std::locale{std::cout.getloc(), df});
    std::ostringstream os;
    os.imbue(std::locale{std::cout.getloc(), df});
    os << d;
    return os.str();
}

std::string datetime_from_timestamp(uint64_t tm)
{

    auto millisecond = std::chrono::milliseconds(tm);
    using time_point = std::chrono::system_clock::time_point;
    time_point uptime_point {std::chrono::duration_cast<time_point::duration>(millisecond)};
    std::time_t time_t = std::chrono::system_clock::to_time_t(uptime_point);

    std::tm tm_time = boost::posix_time::to_tm(boost::posix_time::from_time_t(time_t));
    boost::local_time::tz_database tz_db;
    try {
          tz_db.load_from_file("../data/date_time_zonespec.csv");
        }catch(boost::local_time::data_not_accessible dna) {
          std::cerr << "Error with time zone data file: " << dna.what() << std::endl;
          exit(EXIT_FAILURE);
        }catch(boost::local_time::bad_field_count bfc) {
          std::cerr << "Error with time zone data file: " << bfc.what() << std::endl;
          exit(EXIT_FAILURE);
        }
    boost::local_time::time_zone_ptr jkt = tz_db.time_zone_from_region("Asia/Jakarta");
    boost::gregorian::date in_date(tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday);
    boost::posix_time::time_duration td(tm_time.tm_hour + 7, tm_time.tm_min, tm_time.tm_sec);
    boost::local_time::local_date_time jkt_time(in_date, td, jkt, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    std::string res = jkt_time.to_string();
    boost::algorithm::replace_all(res, "WIT", "WIB");
    std::vector<std::string> seq;
    boost::algorithm::split(seq, res, boost::is_any_of(" "));
    std::string result = fmt::format("{}, {} {}", date_day_format(jkt_time.date()), seq.at(1), seq.at(2));
    return result;
}

std::string to_date_valid(std::string&& str) {
    boost::algorithm::replace_all(str, "T", " ");
    boost::algorithm::replace_all(str, ".000Z", "");
    std::tm tm_time = boost::posix_time::to_tm(boost::posix_time::time_from_string(str));
    boost::local_time::tz_database tz_db;
    try {
          tz_db.load_from_file("../data/date_time_zonespec.csv");
        }catch(boost::local_time::data_not_accessible dna) {
          std::cerr << "Error with time zone data file: " << dna.what() << std::endl;
          exit(EXIT_FAILURE);
        }catch(boost::local_time::bad_field_count bfc) {
          std::cerr << "Error with time zone data file: " << bfc.what() << std::endl;
          exit(EXIT_FAILURE);
        }
    boost::local_time::time_zone_ptr jkt = tz_db.time_zone_from_region("Asia/Jakarta");
    boost::gregorian::date in_date(tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday);
    boost::posix_time::time_duration td(tm_time.tm_hour + 7, tm_time.tm_min, tm_time.tm_sec);
    boost::local_time::local_date_time jkt_time(in_date, td, jkt, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    std::string res = jkt_time.to_string();
    boost::algorithm::replace_all(res, "WIT", "WIB");
    std::vector<std::string> seq;
    boost::algorithm::split(seq, res, boost::is_any_of(" "));
    std::string result = fmt::format("{}, {} {}", date_day_format(jkt_time.date()), seq.at(1), seq.at(2));
    return result;
}

pplx::task<std::pair<bool, std::string>> checking_code_coutry(const Json::Value& jsn, std::string_view str) {
    bool status = false;
    std::string name = "";
    for (const auto& item: jsn) {
        if (str == item["alpha-3"].asString()) {
            name = item["name"].asString();
            status = true;
            break;
        }
    }
    return pplx::task_from_result<std::pair<bool, std::string>>(std::make_pair(status, name));
}

pplx::task<std::string> get_country_name(const Json::Value& jsn, std::string_view code) {
    std::string name = "";
    for (const auto& item: jsn) {
        if (code == item["alpha-3"].asString()) {
            name = item["name"].asString();
            break;
        }
    }
    return pplx::task_from_result<std::string>(name);
}
}

HttpClient::HttpClient()
{
    redis_client.connect("127.0.0.1", 6379, [](const std::string& host, std::size_t port, cpp_redis::connect_state status) {
        if (status == cpp_redis::connect_state::dropped) {
          std::cout << "client disconnected from " << host << ":" << port << std::endl;
        }
    });
}

void HttpClient::get_data_total(std::function<void (std::string_view)> &&callback) {
    auto request = http_client(U(url_total))
            .request(methods::GET, U(""))
            .then([&callback](http_response response){
                if (response.status_code() != 200) {
                    callback("<b>Terjadi kesalahan</b>");
                }
                return response.extract_json();
             })
            .then([&callback](json::value json_obj){
                std::cout << json_obj << std::endl;
                int64_t terkonfirmasi = json_obj.at("confirmed").as_object().at("value").as_number().to_int64();
                int64_t sembuh = json_obj.at("recovered").as_object().at("value").as_number().to_int64();
                int64_t meninggal = json_obj.at("deaths").as_object().at("value").as_number().to_int64();
                int64_t aktif = terkonfirmasi - (sembuh + meninggal);
                std::string update = json_obj.at("lastUpdate").as_string();

                std::string konfirm_str = Money::getInstance().toMoneyFormat(std::to_string(terkonfirmasi), ".", "");
                std::string sembuh_str = Money::getInstance().toMoneyFormat(std::to_string(sembuh), ".", "");
                std::string meninggal_str = Money::getInstance().toMoneyFormat(std::to_string(meninggal), ".", "");
                std::string aktif_str = Money::getInstance().toMoneyFormat(std::to_string(aktif), ".", "");

                std::string text = fmt::format("<b>Total kasus Covid-19 di seluruh dunia:</b>\n\n"
                                               "<pre>Terkonfirmasi: {} orang\n"
                                               "Positif Aktif: {} orang\n"
                                               "Sembuh: {} orang\n"
                                               "Meninggal: {} orang\n</pre>\n\n"
                                               "{}\n"
                                               "Update terakhir:\n<b>{}</b>",
                                               konfirm_str,
                                               aktif_str,
                                               sembuh_str,
                                               meninggal_str,
                                               hint_message,
                                               bangkong::to_date_valid(std::move(update)));
                callback(text);
             });
    try {
        request.wait();
    } catch (const std::exception& e) {
        callback("<b>Tidak dapat menampilkan data</b>");
    }
}

void HttpClient::get_data_nations(std::function<void (std::string_view)> &&callback) {
    auto request = http_client(U(country_url))
            .request(methods::GET, U(""))
            .then([&callback](http_response response){
                if (response.status_code() != 200) {
                    callback("<b>Terjadi kesalahan</b>");
                }
                return response.extract_json();
            })
            .then([&callback](json::value json_obj){
                auto array = json_obj.at("countries").as_array();
                std::string country_str = "<b>Daftar Negara</b>\n\n"
                                          "Click link yang berada di sebelah <strong>nama negara</strong> untuk menampilkan data.\n\n";
                int i = 1;
                for (const auto& item: array) {

                    if (item.size() >= 3) {
                        country_str.append(std::to_string(i));
                        country_str.append(". ");
                        country_str.append(item.at("name").as_string());
                        country_str.append(" ");
                        country_str.append("/");
                        country_str.append(item.at("iso3").as_string());
                        country_str.append("\n");
                    }
                    ++i;
                }
                callback(country_str);
            });
    try {
        request.wait();
    } catch (const std::exception& e) {
        callback("<b>Tidak dapat menampilkan data</b>");
    }
}

void HttpClient::req_nation(std::function<void (std::string_view)> && callback, std::string_view code) {
    std::string cd = code.data();
    boost::algorithm::replace_all(cd, "/", "");

    std::pair<Json::Value, JSONCPP_STRING> country_json = JsonConverter("../data/countries.json")
            .build_json_from_file()
            .data_json();
    if (country_json.second == "") {

        Json::Value& json = country_json.first;
        std::string name = "";
        bangkong::checking_code_coutry(json, cd)
                .then([&callback, cd, &name](std::pair<bool, std::string_view> pair) {

                    if (pair.first == true) {
                        name = pair.second;
                    }
                    else {
                        callback("<pre> Tidak dapat menampilkan data</pre>");
                    }
                }).wait();
        auto request_json = http_client(U(country_url))
                .request(methods::GET, uri_builder(U("")).append_path(U(cd.data())).to_string())
                .then([&callback](http_response response){
                    if (response.status_code() != 200) {
                        callback("<b>Terjadi kesalahan</b>");
                    }
                    return response.extract_json();
                })
                .then([&callback, &name](json::value json_obj){
                    int64_t terkonfirmasi = json_obj.at("confirmed").as_object().at("value").as_number().to_int64();
                    int64_t sembuh = json_obj.at("recovered").as_object().at("value").as_number().to_int64();
                    int64_t meninggal = json_obj.at("deaths").as_object().at("value").as_number().to_int64();
                    int64_t aktif = terkonfirmasi - (sembuh + meninggal);
                    std::string update = json_obj.at("lastUpdate").as_string();

                    std::string konfirm_str = Money::getInstance().toMoneyFormat(std::to_string(terkonfirmasi), ".", "");
                    std::string sembuh_str = Money::getInstance().toMoneyFormat(std::to_string(sembuh), ".", "");
                    std::string meninggal_str = Money::getInstance().toMoneyFormat(std::to_string(meninggal), ".", "");
                    std::string aktif_str = Money::getInstance().toMoneyFormat(std::to_string(aktif), ".", "");

                    std::string text = fmt::format("<b>Total kasus Covid-19 di {}:</b>\n\n"
                                                   "<pre>Terkonfirmasi: {} orang\n"
                                                   "Positif Aktif: {} orang\n"
                                                   "Sembuh: {} orang\n"
                                                   "Meninggal: {} orang\n</pre>\n\n"
                                                   "{}\n"
                                                   "Update terakhir:\n<b>{}</b>",
                                                   name,
                                                   konfirm_str,
                                                   aktif_str,
                                                   sembuh_str,
                                                   meninggal_str,
                                                   hint_message,
                                                   bangkong::to_date_valid(std::move(update)));

                    callback(text);
                });
        try {
            request_json.wait();
        } catch (const std::exception &e) {
            callback("<pre> Tidak dapat menampilkan data</pre>");
        }
    }
    else {
        callback("<pre> Tidak dapat menampilkan data</pre>");
    }
}

void HttpClient::get_data_national(std::function<void (std::string_view, std::optional<Json::Value>)> && callback) {
    auto request = http_client(U(nasional_url))
            .request(methods::GET, U(""))
            .then([&callback](http_response response){
                if (response.status_code() != 200) {

                    callback("<pre>Terjadi kesalahan</p>", std::nullopt);
                }
                return response.extract_string();
            })
            .then([this, &callback](std::string str){
                redis_client.set(bangkong::PROV_KEY, str);
                redis_client.sync_commit();

                std::pair<Json::Value, JSONCPP_STRING> json_value = JsonConverter()
                        .set_body(str)
                        .build_json_from_string()
                        .data_json();

                if (json_value.second == "") {
                    std::string negara = json_value.first["name"].asString();
                    uint64_t timestamp = 0;
                    get_timestamp()
                    .then([&timestamp](std::vector<unsigned char> v){
                        std::string ss = std::string(v.begin(), v.end());
                        timestamp = boost::lexical_cast<uint64_t>(ss);
                    })
                    .wait();
                    int64_t terkonfirmasi = json_value.first["numbers"]["infected"].asInt64();
                    int64_t sembuh = json_value.first["numbers"]["recovered"].asInt64();
                    int64_t meninggal = json_value.first["numbers"]["fatal"].asInt64();
                    int64_t aktif = terkonfirmasi - (sembuh + meninggal);

                    std::string konfirm_str = Money::getInstance().toMoneyFormat(std::to_string(terkonfirmasi), ".", "");
                    std::string sembuh_str = Money::getInstance().toMoneyFormat(std::to_string(sembuh), ".", "");
                    std::string meninggal_str = Money::getInstance().toMoneyFormat(std::to_string(meninggal), ".", "");
                    std::string aktif_str = Money::getInstance().toMoneyFormat(std::to_string(aktif), ".", "");

                    Json::Value root;
                    Json::Value array(Json::arrayValue);
                    Json::Value array1(Json::arrayValue);
                    Json::Value btn_one(Json::objectValue);
                    btn_one["text"] = "Lihat Per Provinsi";
                    btn_one["callback_data"] = "provinsi";
                    array1[0] = btn_one;
                    array[0] = array1;
                    root["inline_keyboard"] = array;

                    std::string text = fmt::format("<b>Total kasus Covid-19 di {}:</b>\n\n"
                                                   "<pre>Terkonfirmasi: {} orang\n"
                                                   "Positif Aktif: {} orang\n"
                                                   "Sembuh: {} orang\n"
                                                   "Meninggal: {} orang\n</pre>\n\n"
                                                   "{}\n"
                                                   "Update terakhir:\n<b>{}</b>",
                                                   negara, konfirm_str,
                                                   aktif_str, sembuh_str,
                                                   meninggal_str,
                                                   hint_message,
                                                   bangkong::datetime_from_timestamp(timestamp));
                    callback(text, root);
                }
                else {
                    callback("<pre>Data tidak dapat ditampilkan</pre>", std::nullopt);
                }
            });
    try {
        request.wait();
    } catch (const std::exception &e) {
       callback("<pre>Data tidak dapat ditampilkan</pre>", std::nullopt);
    }
}

void HttpClient::get_all_province(std::function<void (std::string_view, std::optional<Json::Value>)> &&callback) {
    auto get = redis_client.get(bangkong::PROV_KEY);
    redis_client.sync_commit();

    std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
            .set_body(get.get().as_string())
            .build_json_from_string()
            .data_json();

    if (pair.second == "") {
        auto json_obj = pair.first;
        std::string list = "<b>Data Covid19 Per Provinsi: </b>\n\n";
        int i = 0;
        Json::Value::const_iterator it;
        Json::Value regions = json_obj["regions"];
        Json::Value base;
        Json::Value array(Json::arrayValue);

        for (it = regions.begin(); it != regions.end(); ++it) {
            Json::Value btn_one;
            std::string callback_data = "";
            std::string name = (*it)["name"].asString();
            btn_one["text"] = name;
            int64_t terkonfirmasi = (*it)["numbers"]["infected"].asInt64();
            int64_t sembuh = (*it)["numbers"]["recovered"].asInt64();
            int64_t meninggal = (*it)["numbers"]["fatal"].asInt64();
            int64_t aktif = terkonfirmasi - (sembuh + meninggal);

            std::string konfirm_str = Money::getInstance().toMoneyFormat(std::to_string(terkonfirmasi), ".", "");
            std::string sembuh_str = Money::getInstance().toMoneyFormat(std::to_string(sembuh), ".", "");
            std::string meninggal_str = Money::getInstance().toMoneyFormat(std::to_string(meninggal), ".", "");
            std::string aktif_str = Money::getInstance().toMoneyFormat(std::to_string(aktif), ".", "");

            callback_data.append(name);
            callback_data.append(",");
            callback_data.append(konfirm_str);
            callback_data.append(",");
            callback_data.append(sembuh_str);
            callback_data.append(",");
            callback_data.append(meninggal_str);
            callback_data.append(",");
            callback_data.append(aktif_str);
            btn_one["callback_data"] = callback_data;
            Json::Value array1(Json::arrayValue);
            array1[0] = btn_one;
            array[i] = array1;
            ++i;
        }

        base["inline_keyboard"] = array;
        callback(list, base);
    }
    else {
        callback("<pre>Data tidak dapat ditampilkan</pre>", std::nullopt);
    }
}

void HttpClient::get_all_hospital(MessageWithButton &&callback) {
    auto request = http_client(U(rs_url))
            .request(methods::GET, U(""))
            .then([&callback](http_response response){
                if (response.status_code() != 200) {
                    callback("<pre> Terjadi kesalahan </pre>", std::nullopt);
                }
                return response.extract_string();
             })
            .then([this, &callback](std::string res){
                redis_client.set(bangkong::rs_key, res);
                redis_client.sync_commit();

                std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                        .set_body(res)
                        .build_json_from_string()
                        .data_json();
                if (pair.second == "") {
                    Json::Value json_obj = pair.first;
                    int i = 0;
                    Json::Value::const_iterator it;
                    Json::Value base;
                    Json::Value array(Json::arrayValue);

                    std::list<std::string> data;
                    for (it = json_obj.begin(); it != json_obj.end(); ++it) {
                        data.push_back((*it)["province"].asString());
                    }
                    data.unique();

                    BOOST_FOREACH(const std::string& item, data) {
                        Json::Value btn_one;
                        btn_one["text"] = item;
                        btn_one["callback_data"] = item;
                        Json::Value array1(Json::arrayValue);
                        array1[0] = btn_one;
                        array[i] = array1;
                        ++i;
                    }

                    base["inline_keyboard"] = array;
                    data.clear();
                    callback("<b>Rumah sakit rujukan per provinsi</b>\n\n", base);
                }
                else {
                    callback("<pre>Data tidak dapat ditampilkan</pre>", std::nullopt);
                }
            });
    try {
        request.wait();
    } catch (const std::exception &e) {
        callback("<pre>Data tidak dapat ditampilkan</pre>", std::nullopt);
    }
}

void HttpClient::get_hospital(MessageOnly &&callback, std::string_view prov) {
    auto get = redis_client.get(bangkong::rs_key);
    redis_client.sync_commit();

    std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
            .set_body(get.get().as_string())
            .build_json_from_string()
            .data_json();

    if (pair.second == "") {
        std::string result = "";
        Json::Value json = pair.first;
        Json::Value::const_iterator it;
        for (it = json.begin(); it != json.end(); ++it) {
            if ((*it)["province"].asString() == prov) {
                std::string address = (*it)["name"].asString();
                std::string link = fmt::format("<a href=\"https://www.google.com/maps/search/?api=1&query={}\">{}</a>", address, (*it)["name"].asString());
                result.append(link);
                result.append("\n");
                result.append((*it)["address"].asString());
                result.append("\n");
                result.append((*it)["region"].asString());
                result.append("\n");
                if ((*it)["phone"].isNull() == false) {
                    result.append((*it)["phone"].asString());
                    result.append("\n");
                }
                result.append("\n");
            }
        }
        std::string rs = fmt::format("<b>Berikut Rumah Sakit Rujukan Covid-19 di {}</b>\n\n{}\n\n{}", prov, result, hint_message);

        callback(rs);
    }
    else {
        callback("<pre>Data tidak dapat ditampilkan</pre>");
    }
}

void HttpClient::get_hoaxs(MessageOnly &&callback) {
    get_hoaxs()
            .then([&callback](std::vector<unsigned char> res){
                std::string json_str = std::string(res.begin(), res.end());
                std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                        .set_body(json_str)
                        .build_json_from_string()
                        .data_json();
                if (pair.second == "") {
                    Json::Value json = pair.first;
                    Json::Value::const_iterator it;
                    std::string result = "";
                    for (it = json.begin(); it != json.end(); ++it) {
                        result.append("- ");
                        result.append("<a href=\"");
                        result.append((*it)["url"].asString());
                        result.append("\">");
                        result.append((*it)["title"].asString());
                        result.append("</a>");
                        result.append("\n");
                    }
                    std::string hoaxs = fmt::format("<b>Awas berita hoaks</b>\n\n"
                                                    "<pre>Klik judulnya lihat faktanya</pre>\n\n "
                                                    "{}", result);
                    callback(hoaxs);
                }
                else {
                    callback("<pre>Tidak dapat menampilkan data</pre>");
                }
            }).wait();
}

void HttpClient::get_article(MessageOnly &&callback, bangkong::TypeArticle type) {
    std::string_view path = "nasehat";
    std::string result = "";
    if (type == TypeArticle::NASEHAT) {
        path = "nasehat";
        result = "<b>Berikut adalah judul nasehat yang bisa anda ambil faedahnya: </b>\n\n"
                                     "<pre>Klik judulnya ambil faedahnya</pre>\n\n";
    }
    else if(type == TypeArticle::AKHBAR) {
        path = "akhbar";
        result = "<b>Berikut adalah judul akhbar yang bisa anda ambil faedahnya: </b>\n\n"
                                             "<pre>Klik judulnya ambil faedahnya</pre>\n\n";
    }
    else if (type == TypeArticle::ASYSYARIAH) {
        result = "<b>Berikut adalah judul artikel islami yang bisa anda ambil faedahnya: </b>\n\n"
                                             "<pre>Klik judulnya ambil faedahnya</pre>\n\n";
        path = "asysyariah";
    }
    else if (type == TypeArticle::BNPB) {
        path = "bnpb";
        result = "<b>Berikut adalah judul berita dari BNPB yang bisa anda ambil faedahnya: </b>\n\n"
                                                     "<pre>Klik judulnya ambil faedahnya</pre>\n\n";
    }
    else if (type == TypeArticle::COVIDGOID) {
        path = "coviggov";
        result = "<b>Berikut adalah judul berita dari covid19.go.id yang bisa anda ambil faedahnya: </b>\n\n"
                                                     "<pre>Klik judulnya ambil faedahnya</pre>\n\n";
    }
    auto request = http_client(U(api_article))
            .request(methods::GET, U(path.data()))
            .then([&callback](http_response response){
                if (response.status_code() != 200) {
                    callback("<pre>Terjadi kesalahan</pre>");
                }
                return response.extract_json();
            })
            .then([this, &callback, &result](json::value json_obj){
                build_and_parse_message(std::move(json_obj), result);
                callback(result);
            });
    try {
        request.wait();
    } catch (const std::exception &e) {
        callback("<pre>Tidak dapat menampilkan data</pre>");
    }
}

void HttpClient::get_ciamis(MessageOnly &&callback) {
    auto request = http_client(U(api_article))
            .request(methods::GET, U("ciamis"))
            .then([&callback](http_response response){
                if (response.status_code() != 200) {
                    callback("<pre>Terjadi kesalahan</pre>");
                }
                return response.extract_json();
            })
            .then([&callback](json::value json_obj){
                std::string isOK = json_obj.at("status").as_string();
                if (isOK == "Ok") {
                    try {
                        auto data = json_obj.at("data").as_array();
                        std::string header = data[0].as_string();
                        std::string tgl = data[1].as_string();
                        std::string konfirmasi = fmt::format("{}\n{}\n{}\n{}\n{}",
                                                             data[3].as_string(),
                                data[4].as_string(),
                                data[7].as_string(),
                                data[8].as_string(),
                                data[9].as_string());
                        std::string opp = fmt::format("{}\n{}\n{}\n{}", data[10].as_string(),
                                data[11].as_string(), data[12].as_string(), data[13].as_string());
                        std::string suspek = fmt::format("{}\n{}\n{}\n{}",
                                                      data[14].as_string(),
                                                        data[15].as_string(),
                                                        data[16].as_string(),
                                                        data[19].as_string());
                        std::string kontak = fmt::format("{}\n{}\n{}\n{}",
                                                      data[20].as_string(),
                                                        data[21].as_string(),
                                                        data[22].as_string(),
                                                        data[24].as_string());
                        std::string url = "<a href=\"";
                        url.append("https://pikcovid19.ciamiskab.go.id/");
                        url.append("\">");
                        url.append("https://pikcovid19.ciamiskab.go.id/");
                        url.append("</a>");
                        std::string result = fmt::format("<b>{}</b>\n\n<i>{}</i>\n\n{}\n\n{}\n\n{}\n\n{}\n\n{}\n\n{}{}",
                                                         header,
                                                         tgl,
                                                         konfirmasi,
                                                         opp,
                                                         suspek,
                                                         kontak,
                                                         hint_message,
                                                         "Karena dihawatirkan terjadi perbuhan format penampilan data, "
                                                         "maka silakan buka tautan resmi ", url);
                        callback(result);
                    } catch (const std::exception &e) {
                         callback("<pre> Tidak dapat menampilkan data</pre>");
                    }

                }
                else {
                   callback("<pre> Tidak dapat menampilkan data</pre>");
                }
            });
    try {
        request.wait();
    } catch (const std::exception &e) {
        callback("<pre> Tidak dapat menampilkan data</pre>");
    }
}

void HttpClient::build_and_parse_message(web::json::value &&json_obj, std::string &result) {
    std::string isOK = json_obj.at("status").as_string();
    if (isOK == "Ok") {
        auto data = json_obj.at("data").as_array();
        std::vector<Article> articles;
        articles.reserve(data.size());
        BOOST_FOREACH(const json::value& item, data) {
            Article art {item.at("judul").as_string(), item.at("url").as_string()};
            auto fn = std::find(articles.begin(), articles.end(), art);
            if (fn == std::end(articles)) {
                articles.push_back(art);
                result.append("- ");
                result.append("<a href=\"");
                result.append(item.at("url").as_string());
                result.append("\">");
                result.append(item.at("judul").as_string());
                result.append("</a>");
                result.append("\n");
            }
        }
        articles.erase(articles.begin(), articles.end());
        articles.shrink_to_fit();
    }
    else {
        result.clear();
        result = "<pre> Tidak dapat menampilkan data</pre>";
    }
}


pplx::task<std::vector<unsigned char>> HttpClient::get_timestamp() {
    http_client client(timestamp_url);
    return client.request(methods::GET).then([](http_response response){
        return response.extract_vector();
    });
}

pplx::task<std::vector<unsigned char>> HttpClient::get_hoaxs() {
    http_client client(hoaxs_url);
    return client.request(methods::GET).then([](http_response response){
        return response.extract_vector();
    });
}
