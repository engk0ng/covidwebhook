#include "message.hpp"

#include <fmt/format.h>

using namespace bangkong;

Message& Message::build_for_start() noexcept {
    m_message = fmt::format("بسم الله الرحمن الرحيم\n\nAhlan wa Sahlan <b>{}</b>\n\n"
                            "Ini adalah bot yang menyediakan data dan informasi lainnya tentang Covid-19.\n"
                            "Oleh karena itu diharapkan anda dapat mengambil manfaat darinya.\n\n"
                            "Tetap berdo'a kepada Allah Azza wa Jalla agar segera mengangkat wabah ini dan tempuhlah sebab-sebab"
                            " agar terhindar dari Covid-19 dengan memakai MASKER dan menjalankan protokol kesehatan!\n\n"
                            "Semoga Allah segera mengangkat wabah ini dari negara kita tercinta dan negara kaum muslimin lainnya. Aamiin\n"
                            "Tulis /menu untuk melihat menu di bot Khobar Covid\n\n"
                            "-----------------am----------------------", m_chat->get_full_name());
    return *this;
}

Message& Message::build_for_menu() noexcept {
    m_message = fmt::format("<b>{}</b>\nBerikut adalah menu yang tersedia di bot Khobar Covid.\n\n"
                            "Menampilkan data kasus Covid-19\n"
                            "1. Di seluruh dunia /total\n"
                            "2. Per negara /internasional\n"
                            "3. Secara nasional /nasional\n"
                            "4. Ciamis /ciamis\n\n"
                            "Menampilkan artikel\n"
                            "1. Artikel islam /artikel_islami\n"
                            "2. Nasehat /nasehat\n\n"
                            "Menampilkan berita\n"
                            "1. Akhbar(tanggapcovid19.com) Covid-19 /akhbar\n"
                            "2. BNPB /bnpb\n"
                            "3. Covid19.go.id /covid19goid\n\n"
                            "<b>AWAS HOAX!!!</b>\n"
                            "1. Hoax /hoax\n\n"
                            "Semoga bermanfaat.\n\n"
                            "<b>Jangan lupa PAKAI MASKER dan jalankan protokol kesehatan!</b>",
                            m_chat->get_full_name());
    return *this;
}

Message& Message::build_for_international() noexcept {
    m_http_client->get_data_nations([this](std::string_view res){
                                        m_message = res;
                                    });
    return *this;
}

Message& Message::build_for_total() noexcept {
    m_http_client->get_data_total([this](std::string_view res){
        m_message = res;
    });
    return *this;
}

Message& Message::build_for_nation_request(std::string_view code) noexcept {
    m_http_client->req_nation([this](std::string_view res){
        m_message = res;
    }, code);
    return *this;
}

Message& Message::build_for_national() noexcept {
    m_http_client->get_data_national([this](std::string_view res, std::optional<Json::Value> button){
        m_message = res;
        if (button != std::nullopt) {
            Json::Value btn_json = button.value();
            //std::cout << btn_json.toStyledString() << std::endl;
            m_replay_markup = btn_json.toStyledString();
        }
    });
    return *this;
}

Message& Message::build_for_all_province() noexcept {
    m_http_client->get_all_province([this](std::string_view res, std::optional<Json::Value> button){
        m_message = res;
        if (button != std::nullopt) {
            Json::Value btn_json = button.value();
            //std::cout << btn_json.toStyledString() << std::endl;
            m_replay_markup = btn_json.toStyledString();
        }
    });
    return *this;
}

Message& Message::build_for_province(std::vector<std::string>&& vec) noexcept {
    std::string prov = fmt::format("<b>Provinsi {} </b>:\n<pre>"
                                   "Terkonfirmasi: {} orang\n"
                                   "Positif Aktif: {} orang\n"
                                   "Sembuh: {} orang\n"
                                   "Meninggal: {} orang\n</pre>\n\n{}",
                                   vec.at(0), vec.at(1), vec.at(2), vec.at(3), vec.at(4),
                                   "<b>Jangan lupa PAKAI MASKER dan jalankan protokol kesehatan!</b>");
    m_message = prov;
    return *this;
}

Message& Message::build_for_all_hospital() noexcept {
    m_http_client->get_all_hospital([this](std::string_view res, std::optional<Json::Value> buttons){
        m_message = res;
        if (buttons != std::nullopt) {
            Json::Value btn_json = buttons.value();
            //std::cout << btn_json.toStyledString() << std::endl;
            m_replay_markup = btn_json.toStyledString();
        }
    });
    return *this;
}

Message& Message::build_for_hospital(std::string_view prov) noexcept {
    m_http_client->get_hospital([this](std::string_view res){
        m_message = res;
    }, prov);
    return *this;
}

Message& Message::build_for_hoaxs() noexcept {
    m_http_client->get_hoaxs([this](std::string_view res){
        m_message = res;
    });
    return *this;
}

Message& Message::build_for_article(bangkong::TypeArticle type) noexcept {
    m_http_client->get_article([this](std::string_view res){
        m_message = res;
    }, type);
    return *this;
}

Message& Message::build_for_ciamis() noexcept {
    m_http_client->get_ciamis([this](std::string_view res){
        m_message = res;
    });
    return *this;
}

Message& Message::parse_html() noexcept {
    m_mode = "html";
    return *this;
}

Message& Message::parse_markdown() noexcept {
    m_mode = "markdown";
    return *this;
}

web::json::value Message::get_message() {
    web::json::value json;
    json["chat_id"] = m_chat->get_id();
    json["text"] = web::json::value::string(m_message);
    if (m_replay_markup != std::nullopt) {
        //std::cout << m_replay_markup.value() << std::endl;
        json["reply_markup"] = web::json::value::string(m_replay_markup.value());
    }
    json["parse_mode"] = web::json::value::string(m_mode.data());
    return json;
}
