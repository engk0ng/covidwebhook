#include "chat.hpp"

#include <fmt/format.h>

bangkong::Chat::Chat(): bangkong::AbsMessage()
{

}

bangkong::Chat::~Chat() {

}

std::string bangkong::Chat::get_full_name() noexcept {
    if (m_last_name != "") {
        return fmt::format("{} {}", m_first_name, m_last_name);
    }
    else
        return m_first_name;
}
