#include "message.hpp"
#include <iostream>

#include <fmt/format.h>

using namespace bangkong;

void Message::build_for_start() noexcept {
    m_message = fmt::format("بسم الله الرحمن الرحيم\n\nAhlan wa Sahlan <b>{}</b>\n\n"
                            "Ini adalah bot yang menyediakan data dan informasi lainnya tentang Covid-19.\n"
                            "Oleh karena itu diharapkan anda dapat mengambil manfaat darinya.\n\n"
                            "Tetap berdo'a kepada Allah Azza wa Jalla agar segera mengangkat wabah ini dan tempuhlah sebab-sebab"
                            " agar terhindar dari Covid-19 dengan memakai MASKER dan menjalankan protokol kesehatan!\n\n"
                            "Semoga Allah segera mengangkat wabah ini dari negara kita tercinta dan negara kaum muslimin lainnya. Aamiin\n"
                            "Tulis /menu untuk melihat menu di bot Khobar Covid\n\n"
                            "---------------------------------------", m_chat->get_full_name());
    parse_html().build_message();
    m_http_client->post_message_only(std::move(m_message_json));
    return;
}

void Message::build_for_menu() noexcept {
    m_message = fmt::format("<b>{}</b>\nBerikut adalah menu yang tersedia di bot Khobar Covid:\n\n"
                            "Menampilkan data kasus Covid-19\n"
                            "1. Di seluruh dunia /total\n"
                            "2. Per negara /internasional\n"
                            "3. Secara nasional /nasional\n"
                            "4. Ciamis /ciamis\n\n"
                            "Rumah sakit rujukan Covid-19 di Indonesia: /rs\n\n"
                            "Menampilkan artikel\n"
                            "1. Artikel islam /artikel_islami\n"
                            "2. Nasehat /nasehat\n\n"
                            "Menampilkan berita\n"
                            "1. Akhbar(tanggapcovid19.com) Covid-19 /akhbar\n"
                            "2. BNPB /bnpb\n"
                            "3. Covid19.go.id /covid19goid\n\n"
                            "<b>AWAS HOAX!!!</b>\n"
                            "1. Hoax /hoax\n\n"
                            "Sumber /sumber\n\n"
                            "Semoga bermanfaat.\n\n"
                            "<b>Jangan lupa PAKAI MASKER dan jalankan protokol kesehatan!</b>",
                            m_chat->get_full_name());
    parse_html().build_message();
    m_http_client->post_message_only(std::move(m_message_json));
    return;
}

void Message::build_for_credit() noexcept {
    m_message = fmt::format("<b>{}</b>\n"
                            "Berikut adalah sumber data yang jadi acuan bot Khobar Covid:\n"
                            "1. https://covid19.mathdro.id/ (Muhammad Mustadi https://github.com/mathdroid/)\n"
                            "2. https://dekontaminasi.com/ (Ariya Hidayat https://github.com/ariya)\n\n"
                            "Sumber berita dan artikel:\n"
                            "1. https://www.tanggapcovid19.com/ (Ma'had Minhajul Atsar Jember)\n"
                            "2. https://asysyariah.com/ (Majalah Asy Syari'ah)\n"
                            "3. https://www.bnpb.go.id/ (BNPB)\n"
                            "4. https://covid19.go.id/ (Gugus Tugas Percepatan Penanganan Covid-19 Pusat\n"
                            "5. https://pikcovid19.ciamiskab.go.id (Gugus Tugas Percepatan Penanganan Covid-19 Ciamis\n\n",
                            m_chat->get_full_name());
    parse_html().build_message();
    m_http_client->post_message_only(std::move(m_message_json));
    return;
}

void Message::build_for_international() noexcept {
    parse_html().build_message();
    m_http_client->send_data_nations(std::move(m_message_json));
    return;
}

void Message::build_for_total() noexcept {
    parse_html().build_message();
    m_http_client->send_data_total(std::move(m_message_json));
    return;
}

void Message::build_for_nation_request(std::string_view code) noexcept {
    parse_html().build_message();
    m_http_client->send_nation(std::move(m_message_json), code);
    return;
}

void Message::build_for_national() noexcept {
    parse_html().build_message();
    m_http_client->send_data_national(std::move(m_message_json));
    return;
}

void Message::build_for_all_province() noexcept {
    parse_html().build_message();
    m_http_client->send_all_province(std::move(m_message_json));
    return;
}

void Message::build_for_province(std::vector<std::string>&& vec) noexcept {
    parse_html().build_message();
    std::string prov = fmt::format("<b>Provinsi {} </b>:\n<pre>"
                                   "Terkonfirmasi: {} orang\n"
                                   "Positif Aktif: {} orang\n"
                                   "Sembuh: {} orang\n"
                                   "Meninggal: {} orang\n</pre>\n\n{}",
                                   vec.at(0), vec.at(1), vec.at(2), vec.at(3), vec.at(4),
                                   "<b>Jangan lupa PAKAI MASKER dan jalankan protokol kesehatan!</b>");
    m_message_json["text"] = prov;
    m_http_client->send_province(std::move(m_message_json));
    return;
}

void Message::build_for_all_hospital() noexcept {
    parse_html().build_message();
    m_http_client->send_all_hospital(std::move(m_message_json));
    return;
}

void Message::build_for_hospital(std::string_view prov) noexcept {
    parse_html().build_message();
    m_http_client->send_hospital(std::move(m_message_json), prov);
    return;
}

void Message::build_for_hoaxs() noexcept {
    parse_html().build_message();
    m_http_client->send_hoaxs(std::move(m_message_json));
    return;
}

void Message::build_for_article(bangkong::TypeArticle type) noexcept {
    parse_html().build_message();
    m_http_client->send_article(std::move(m_message_json), type);
    return;
}

void Message::build_for_ciamis() noexcept {
    parse_html().build_message();
    m_http_client->send_ciamis(std::move(m_message_json));
    return;
}

Message& Message::parse_html() noexcept {
    m_mode = "html";
    return *this;
}

Message& Message::parse_markdown() noexcept {
    m_mode = "markdown";
    return *this;
}

void Message::build_message() noexcept {
    m_message_json["chat_id"] = Json::Int64(m_chat->get_id());
    m_message_json["text"] = m_message;
    if (m_replay_markup != std::nullopt) {
        //std::cout << m_replay_markup.value() << std::endl;
        m_message_json["reply_markup"] = m_replay_markup.value();
    }

    m_message_json["parse_mode"] = m_mode.data();
    return;
}

