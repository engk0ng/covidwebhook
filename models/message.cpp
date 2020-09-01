#include "message.hpp"

#include <fmt/format.h>

using namespace bangkong;

Message& Message::build_for_start() noexcept {
    m_message = fmt::format("بسم الله الرحمن الرحيم\n\n Ahlan wa Sahlan <b>{}</b>\n"
                            "<pre>Ini adalah bot yang menyediakan data dan informasi lainnya tentang Covid-19.\n"
                            "Oleh karena itu diharapkan anda dapat mengambil manfaat darinya.\n"
                            "Semoga Allah segera mengangkat wabah ini dari negara kita dan negara kaum muslimin lainnya.</pre>\n\n"
                            "<b>@_abumuhammad_</b>", m_chat->get_full_name());
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
    json["parse_mode"] = web::json::value::string(m_mode.data());
    return json;
}
