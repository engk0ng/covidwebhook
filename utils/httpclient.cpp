#include "httpclient.hpp"

#include <string_view>

#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <boost/date_time/gregorian/greg_calendar.hpp>
#include <boost/bind.hpp>
#include <boost/fiber/all.hpp>
#include <boost/move/move.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "Poco/Redis/Redis.h"
#include "Poco/Redis/AsyncReader.h"
#include "Poco/Redis/Command.h"
#include "Poco/Redis/PoolableConnectionFactory.h"
#include "Poco/Redis/Array.h"

#include <fmt/format.h>
#include <chrono>

#include "money.h"
#include "jsonconverter.hpp"
#include "../models/article_model.hpp"
#include "../utils/utils.hpp"

#include <Poco/Event.h>

#include "../commander/httpclientcallback.hpp"
#include "../commander/redirectmessage.hpp"

#define REDIS_HOST bangkong::get_env("REDIS_HOST")
#define REDIS_PORT boost::lexical_cast<int>(bangkong::get_env("REDIS_PORT"))
#define REDIS_USR bangkong::get_env("REDIS_USR")
#define REDIS_PASS bangkong::get_env("REDIS_PASS")

static constexpr const char* url_total = "https://covid19.mathdro.id/api";
static constexpr const char* date_format = "%A, %d %B %Y";
static constexpr const char* country_url = "https://covid19.mathdro.id/api/countries";
static constexpr const char* nasional_url = "https://dekontaminasi.com/api/id/covid19/stats";
static constexpr const char* timestamp_url = "https://dekontaminasi.com/api/id/covid19/stats.timestamp";
static constexpr const char* hoaxs_url = "https://dekontaminasi.com/api/id/covid19/hoaxes";
static constexpr const char* rs_url = "https://dekontaminasi.com/api/id/covid19/hospitals";
static constexpr const char* api_article = "https://cryptic-brushlands-41995.herokuapp.com/";
static constexpr const char* hint_message = "<b>Jangan lupa PAKAI MASKER dan jalankan protokol kesehatan!</b>";

using namespace bangkong;

namespace bangkong {

static const std::string PROV_KEY = "__provinsi__";
static const std::string RS_KEY = "__rs__";
static const std::string COUNTRY_KEY = "___country____";

static std::string months[12]{"Januari", "Februari", "Maret", "April",
"Mei", "Juni", "Juli", "Agustus", "September", "Oktober",
"November", "Desember"};
static std::string weekdays[7]{"Ahad", "Senin", "Selasa",
"Rabu", "Kamis", "Jum\'at", "Sabtu"};

using packaged_task_t = typename boost::shared_ptr<boost::fibers::packaged_task<std::pair<bool, std::string>()>>;

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

    boost::local_time::time_zone_ptr jkt = tz_db.time_zone_from_region("Asia/Jakarta");
    boost::gregorian::date in_date(tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday);
    boost::posix_time::time_duration td(tm_time.tm_hour + 7, tm_time.tm_min, tm_time.tm_sec);
    boost::local_time::local_date_time jkt_time(in_date, td, jkt, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    std::string res = jkt_time.to_string();
    boost::algorithm::replace_all(res, "UTC", "WIB");
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
    boost::local_time::time_zone_ptr jkt = tz_db.time_zone_from_region("Asia/Jakarta");
    boost::gregorian::date in_date(tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday);
    boost::posix_time::time_duration td(tm_time.tm_hour + 7, tm_time.tm_min, tm_time.tm_sec);
    boost::local_time::local_date_time jkt_time(in_date, td, jkt, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    std::string res = jkt_time.to_string();
    boost::algorithm::replace_all(res, "UTC", "WIB");
    std::vector<std::string> seq;
    boost::algorithm::split(seq, res, boost::is_any_of(" "));
    std::string result = fmt::format("{}, {} {}", date_day_format(jkt_time.date()), seq.at(1), seq.at(2));
    return result;
}

std::pair<bool, std::string> checkking_code_country(const std::string& str, const std::string& json_str) {
    std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
            .set_body(json_str)
            .build_json_from_string()
            .data_json();
    bool status = false;
    std::string name = "";
    if (pair.second == "") {
        Json::Value json_arr = pair.first;
        Json::Value::const_iterator it;
        for (it = json_arr.begin(); it != json_arr.end(); ++it) {
            if (str == (*it)["alpha-3"].asString()) {
                name = (*it)["name"].asString();
                status = true;
                break;
            }
        }
    }
    return std::make_pair(status, name);
}

void exec_check_country(packaged_task_t pt) {
    boost::fibers::fiber(boost::move(*pt)).join();
}

boost::fibers::future<std::pair<bool, std::string>> check_asyn(const std::string& str, const std::string& json_str) {
    packaged_task_t pt(new boost::fibers::packaged_task<std::pair<bool, std::string>()>(boost::bind(checkking_code_country, str, json_str)));
    boost::fibers::future<std::pair<bool, std::string>> f(pt->get_future());
    boost::thread(boost::bind(exec_check_country, pt)).detach();
    return boost::move(f);
}

}

easyhttpcpp::Call::Ptr HttpClient::call_get_request(std::string_view url) {
    easyhttpcpp::Request::Builder request_builder;
    easyhttpcpp::Request::Ptr p_request = request_builder.setUrl(url.data()).build();
    easyhttpcpp::Call::Ptr p_call = p_http_client->newCall(p_request);
    return p_call;
}

void HttpClient::post_message_only(Json::Value &&messages) {
    RedirectMessage(std::move(messages)).send_data();
}

void HttpClient::send_data_total(Json::Value &&messages) {
    easyhttpcpp::Call::Ptr p_call = call_get_request(url_total);
    MessageOnly p_send = [&messages](std::string&& res) {
        std::string result = "";
        std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                .set_body(res)
                .build_json_from_string()
                .data_json();
        if (pair.second == "") {
            Json::Value json_obj = pair.first;
            int64_t terkonfirmasi = json_obj["confirmed"]["value"].asInt64();
            int64_t sembuh = json_obj["recovered"]["value"].asInt64();
            int64_t meninggal = json_obj["deaths"]["value"].asInt64();
            int64_t aktif = terkonfirmasi - (sembuh + meninggal);
            std::string update = json_obj["lastUpdate"].asString();

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
            result = text;
        }
        else {
            result = "<b>Tidak dapat menampilkan data</b>";
        }

        messages["text"] = result;
        RedirectMessage(std::move(messages))
                .send_data();
    };
    HttpClientCallback::Ptr p_response_cb = new HttpClientCallback(std::move(p_send));
    p_call->executeAsync(p_response_cb);
    p_response_cb->waitForCompletion();
}

void HttpClient::send_data_nations(Json::Value&& messages) {
    easyhttpcpp::Call::Ptr p_call = call_get_request(country_url);
    MessageOnly p_send = [&messages](std::string&& res) {
        std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                .set_body(res)
                .build_json_from_string()
                .data_json();
        std::string result = "";
        if (pair.second == "") {
            auto array = pair.first["countries"];
            std::string country_str = "<b>Daftar Negara</b>\n\n"
                                      "Click link yang berada di sebelah <strong>nama negara</strong> untuk menampilkan data.\n\n";
            Json::Value::const_iterator it;
            int i = 1;
            for(it = array.begin(); it != array.end(); ++it) {
                if (it->size() >= 3) {
                    country_str.append(std::to_string(i));
                    country_str.append(". ");
                    country_str.append((*it)["name"].asString());
                    country_str.append(" ");
                    country_str.append("/");
                    country_str.append((*it)["iso3"].asString());
                    country_str.append("\n");
                }
                ++i;
            }
            result = std::move(country_str);
        }
        else {
            result = "<b>Tidak dapat menampilkan data</b>";
        }
        messages["text"] = result;
        RedirectMessage(std::move(messages)).send_data();
    };
    HttpClientCallback::Ptr p_response_cb = new HttpClientCallback(std::move(p_send));
    p_call->executeAsync(p_response_cb);
    p_response_cb->waitForCompletion();
}

void HttpClient::send_nation(Json::Value&& messages, std::string_view code) {
    std::string cd = code.data();
    boost::algorithm::replace_all(cd, "/", "");
    std::string json_str = bangkong::country_json_str;
    boost::fibers::future<std::pair<bool, std::string>> f = check_asyn(cd, json_str);
    std::pair<bool, std::string> result = f.get();
    if (result.first == true) {
        std::string pth = fmt::format("{}{}",country_url, code);
        std::shared_ptr<std::string> name = std::make_shared<std::string>(result.second);
        easyhttpcpp::Call::Ptr p_call = call_get_request(pth);
        MessageOnly p_send = [&messages, name](std::string&& res) {
            std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                    .set_body(res)
                    .build_json_from_string()
                    .data_json();
            if (pair.second == "") {
                Json::Value json_obj = pair.first;
                int64_t terkonfirmasi = json_obj["confirmed"]["value"].asInt64();
                int64_t sembuh = json_obj["recovered"]["value"].asInt64();
                int64_t meninggal = json_obj["deaths"]["value"].asInt64();
                int64_t aktif = terkonfirmasi - (sembuh + meninggal);
                std::string update = json_obj["lastUpdate"].asString();

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
                                               *name,
                                               konfirm_str,
                                               aktif_str,
                                               sembuh_str,
                                               meninggal_str,
                                               hint_message,
                                               bangkong::to_date_valid(std::move(update)));
                messages["text"] = text;
            }
            else {
                messages["text"] = "<b>Data tidak dapat ditampilkan</b>";
            }
            RedirectMessage(std::move(messages)).send_data();
        };
        HttpClientCallback::Ptr p_cb = new HttpClientCallback(std::move(p_send));
        p_call->executeAsync(p_cb);
        p_cb->waitForCompletion();
    }
    else {
        messages["text"] = "<b>Data tidak ditemukan</b>";
        RedirectMessage(std::move(messages)).send_data();
    }
}

void HttpClient::send_data_national(Json::Value&& messages) {
    uint64_t timestamp = get_timestamp();

    easyhttpcpp::Call::Ptr p_call_data = call_get_request(nasional_url);
    MessageOnly p_send = [timestamp, &messages](std::string&& res) {
        std::pair<Json::Value, JSONCPP_STRING> json_value = JsonConverter()
                .set_body(res)
                .build_json_from_string()
                .data_json();

        if (json_value.second == "") {
            std::string negara = json_value.first["name"].asString();

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
            messages["text"] = text;
            messages["reply_markup"] = root.toStyledString();
         }
        else {
            messages["text"] = json_value.second;
        }
        RedirectMessage(std::move(messages)).send_data();
    };
    HttpClientCallback::Ptr p_cb_c = new HttpClientCallback(std::move(p_send));
    p_call_data->executeAsync(p_cb_c);
    p_cb_c->waitForCompletion();
}

void HttpClient::send_all_province(Json::Value &&messages) {
    easyhttpcpp::Call::Ptr p_call = call_get_request(nasional_url);
    MessageOnly p_send = [&messages](std::string&& res) {
        std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                .set_body(res)
                .build_json_from_string()
                .data_json();

        if (pair.second == "") {
            auto json_obj = pair.first;
            std::string list = "<b>Data Covid19 Per Provinsi (Klik nama provinsi untuk menampilkan data!): </b>\n\n";
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
            messages["text"] = list;
            messages["reply_markup"] = base.toStyledString();
        }
        else {
            messages["text"] = "<b>Data tidak dapat ditampilkan</b>";
        }
        RedirectMessage(std::move(messages)).send_data();
    };
    HttpClientCallback::Ptr p_cb = new HttpClientCallback(std::move(p_send));
    p_call->executeAsync(p_cb);
    p_cb->waitForCompletion();
}

void HttpClient::send_province(Json::Value &&messages) {
    RedirectMessage(std::move(messages)).send_data();
}

void HttpClient::send_all_hospital(Json::Value &&messages) {
    easyhttpcpp::Call::Ptr p_call = call_get_request(rs_url);
    MessageOnly p_send = [&messages](std::string&& res) {
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
            messages["text"] = "<b>Rumah sakit rujukan per provinsi (Klik nama provinsi untuk menampilkan data!)</b>\n\n";
            messages["reply_markup"] = base.toStyledString();
        }
        else {
            messages["text"]= "<b>Data tidak dapat ditampilkan</b>";
        }
        RedirectMessage(std::move(messages)).send_data();
    };
    HttpClientCallback::Ptr p_cb = new HttpClientCallback(std::move(p_send));
    p_call->executeAsync(p_cb);
    p_cb->waitForCompletion();
}

void HttpClient::send_hospital(Json::Value &&messages, std::string_view prov) {
    easyhttpcpp::Call::Ptr p_call = call_get_request(rs_url);
    MessageOnly p_send = [prov, &messages](std::string&& res) {
        std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                .set_body(res)
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
            messages["text"] = rs;
        }
        else {
            messages["text"] = "<b>Data tidak dapat ditampilkan</b>";
        }
        RedirectMessage(std::move(messages)).send_data();
    };
    HttpClientCallback::Ptr p_cb = new HttpClientCallback(std::move(p_send));
    p_call->executeAsync(p_cb);
    p_cb->waitForCompletion();
}

void HttpClient::send_hoaxs(Json::Value &&messages) {
    easyhttpcpp::Call::Ptr p_call = call_get_request(hoaxs_url);
    MessageOnly p_send = [&messages](std::string&& res) {
        std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                .set_body(res)
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
            messages["text"] = hoaxs;
        }
        else {
            messages["text"] = "<b>Data tidak dapat ditampilkan</b>";
        }
        RedirectMessage(std::move(messages)).send_data();
    };
    HttpClientCallback::Ptr p_cb = new HttpClientCallback(std::move(p_send));
    p_call->executeAsync(p_cb);
    p_cb->waitForCompletion();
}

void HttpClient::send_article(Json::Value &&messages, bangkong::TypeArticle type) {
    std::string_view path = "nasehat";
    if (type == TypeArticle::NASEHAT) {
        path = "nasehat";
    }
    else if(type == TypeArticle::AKHBAR) {
        path = "akhbar";
    }
    else if (type == TypeArticle::ASYSYARIAH) {
        path = "asysyariah";
    }
    else if (type == TypeArticle::BNPB) {
        path = "bnpb";
    }
    else if (type == TypeArticle::COVIDGOID) {
        path = "coviggov";

    }
    std::string pth = fmt::format("{}{}", api_article, path);
    easyhttpcpp::Call::Ptr p_call = call_get_request(pth);
    MessageOnly p_send = [this, &messages, type](std::string&& res) {
        std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                .set_body(res)
                .build_json_from_string()
                .data_json();

        if (pair.second == "") {
            Json::Value json_obj = pair.first;
            std::string result = "";
            if (type == TypeArticle::NASEHAT) {
                result.append("<b>Berikut adalah judul nasehat yang bisa anda ambil faedahnya: </b>\n\n"
                                             "<pre>Klik judulnya ambil faedahnya</pre>\n\n");
            }
            else if(type == TypeArticle::AKHBAR) {
                result.append("<b>Berikut adalah judul akhbar yang bisa anda ambil faedahnya: </b>\n\n"
                                                     "<pre>Klik judulnya ambil faedahnya</pre>\n\n");
            }
            else if (type == TypeArticle::ASYSYARIAH) {
                result.append("<b>Berikut adalah judul artikel islami yang bisa anda ambil faedahnya: </b>\n\n"
                                                     "<pre>Klik judulnya ambil faedahnya</pre>\n\n");
            }
            else if (type == TypeArticle::BNPB) {
                result.append("<b>Berikut adalah judul berita dari BNPB yang bisa anda ambil faedahnya: </b>\n\n"
                                                             "<pre>Klik judulnya ambil faedahnya</pre>\n\n");
            }
            else if (type == TypeArticle::COVIDGOID) {
                result.append("<b>Berikut adalah judul berita dari covid19.go.id yang bisa anda ambil faedahnya: </b>\n\n"
                                                             "<pre>Klik judulnya ambil faedahnya</pre>\n\n");
            }
            std::string r = build_and_parse_message(std::move(json_obj));
            result.append(r);
            messages["text"] = result;
        }
        else {
            messages["text"] = "<b>Data tidak dapat ditampilkan</b>";
        }

        RedirectMessage(std::move(messages)).send_data();
    };
    HttpClientCallback::Ptr p_cb = new HttpClientCallback(std::move(p_send));
    p_call->executeAsync(p_cb);
    p_cb->waitForCompletion();
}

void HttpClient::send_ciamis(Json::Value &&messages) {
    std::string pth = fmt::format("{}{}", api_article, "ciamis");
    easyhttpcpp::Call::Ptr p_call = call_get_request(pth);
    MessageOnly p_send = [&messages](std::string&& res) {
        std::pair<Json::Value, JSONCPP_STRING> pair = JsonConverter()
                .set_body(res)
                .build_json_from_string()
                .data_json();

        if (pair.second == "") {
            Json::Value json_obj = pair.first;

            std::string isOK = json_obj["status"].asString();
            if (isOK == "Ok") {
                auto data = json_obj["data"];
                std::string header = data[0].asString();
                std::string tgl = data[1].asString();
                std::string konfirmasi = fmt::format("{}\n{}\n{}\n{}\n{}",
                                                     data[3].asString(),
                        data[4].asString(),
                        data[7].asString(),
                        data[8].asString(),
                        data[9].asString());
                std::string opp = fmt::format("{}\n{}\n{}\n{}", data[10].asString(),
                        data[11].asString(), data[12].asString(), data[13].asString());
                std::string suspek = fmt::format("{}\n{}\n{}\n{}",
                                              data[14].asString(),
                                                data[15].asString(),
                                                data[16].asString(),
                                                data[19].asString());
                std::string kontak = fmt::format("{}\n{}\n{}\n{}",
                                              data[20].asString(),
                                                data[21].asString(),
                                                data[22].asString(),
                                                data[24].asString());
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
                messages["text"] = result;
            }
            else {
                messages["text"] = "<b>Data tidak dapat ditampilkan</b>";
            }
        }
        else {
            messages["text"] = "<b>Data tidak dapat ditampilkan</b>";
        }
        RedirectMessage(std::move(messages)).send_data();
    };
    HttpClientCallback::Ptr p_cb = new HttpClientCallback(std::move(p_send));
    p_call->executeAsync(p_cb);
    p_cb->waitForCompletion();
}

std::string HttpClient::build_and_parse_message(Json::Value &&json_obj) {
    std::string result = "";
    std::string isOK = json_obj["status"].asString();
    if (isOK == "Ok") {
        auto data = json_obj["data"];
        std::vector<Article> articles;
        articles.reserve(data.size());
        Json::Value::const_iterator it;
        for (it = data.begin(); it != data.end(); ++it) {
            Article art {(*it)["judul"].asString(), (*it)["url"].asString()};
            auto fn = std::find(articles.begin(), articles.end(), art);
            if (fn == std::end(articles)) {
                articles.push_back(art);
                result.append("- ");
                result.append("<a href=\"");
                result.append((*it)["url"].asString());
                result.append("\">");
                result.append((*it)["judul"].asString());
                result.append("</a>");
                result.append("\n");
            }
        }

        articles.erase(articles.begin(), articles.end());
        articles.shrink_to_fit();
    }
    else {
        result.clear();
        result = "<b>Data tidak dapat ditampilkan</b>";
    }
    return result;
}


uint64_t HttpClient::get_timestamp() {
    easyhttpcpp::EasyHttp::Ptr p_http = easyhttpcpp::EasyHttp::Builder().build();
    easyhttpcpp::Request::Builder request_builder;
    easyhttpcpp::Request::Ptr p_request = request_builder.setUrl(timestamp_url).build();
    easyhttpcpp::Call::Ptr p_call = p_http->newCall(p_request);
    easyhttpcpp::Response::Ptr p_response = p_call->execute();
    if (!p_response->isSuccessful()) {
        return 0;
    }
    else {
        if (p_response->getCode() == 200) {
            std::string res = p_response->getBody()->toString();
            return boost::lexical_cast<uint64_t>(res);
        }
        else {
            return 0;
        }
    }
    return 0;
}

void HttpClient::set_redis_key(const std::string& key, std::string&& value) {
    Poco::Redis::Client client;
    client.connect(REDIS_HOST.data());

    Poco::Redis::Command set_command = Poco::Redis::Command::set(key, value);
    client.execute<void>(set_command);
}

std::string HttpClient::get_redis_key(const std::string &key) {
    Poco::Redis::Client client;
    client.connect(REDIS_HOST.data());

    Poco::Redis::Command get_command = Poco::Redis::Command::get(key);
    try {
        Poco::Redis::BulkString result = client.execute<Poco::Redis::BulkString>(get_command);
        return result;
    } catch (Poco::Redis::RedisException& e) {
        return e.message();
    }
    catch (Poco::BadCastException& e) {
        return e.message();
    }
    return "";
}

bool HttpClient::redis_auth(Poco::Redis::Client &client, const std::string &usr, const std::string &pass) {
    Poco::Redis::Array cmd;
    std::string auth = fmt::format("{}{}", usr, pass);
    cmd << "AUTH" << auth;
    std::string response;
    try {
        response = client.execute<std::string>(cmd);
    }
    catch(...){
        return false;
    }
    return (response == "OK");
}
