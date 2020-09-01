#ifndef CHAT_HPP
#define CHAT_HPP

#include <string_view>
#include <string>

namespace bangkong {
class AbsMessage {
public:
    virtual ~AbsMessage() {}
    virtual std::string get_full_name() noexcept = 0;
    virtual long long get_id() const = 0;
    virtual std::string get_command() const = 0;
    virtual std::string get_type() const = 0;
};


class Chat: public AbsMessage
{
public:
    Chat();
    ~Chat();
    virtual std::string get_full_name() noexcept override;

    void set_id(long long id) {
        m_id = id;
    }

    long long get_id() const override {
        return m_id;
    }

    std::string get_command() const override {
        return m_command;
    }

    std::string get_type() const override {
        return m_type;
    }

    void set_first_name(const std::string& first_name) {
        m_first_name = first_name;
    }

    void set_last_name(const std::string& last_name) {
        m_last_name = last_name;
    }

    void set_command(const std::string& command) {
        m_command = command;
    }

    void set_type(const std::string& type) {
        m_type = type;
    }

    Chat(const Chat& other) {
        m_id = other.m_id;
        m_first_name = other.m_first_name;
        m_last_name = other.m_last_name;
        m_command = other.m_command;
        m_type = other.m_type;
    }
    Chat& operator=(const Chat& other) {
        m_id = other.m_id;
        m_first_name = other.m_first_name;
        m_last_name = other.m_last_name;
        m_command = other.m_command;
        m_type = other.m_type;

        return *this;
    }

    Chat(Chat&& other) {
        m_id = std::move(other.m_id);
        m_first_name = std::move(other.m_first_name);
        m_last_name = std::move(other.m_last_name);
        m_command = std::move(other.m_command);
        m_type = std::move(other.m_type);
    }

    Chat& operator=(Chat&& other) {
        m_id = std::move(other.m_id);
        m_first_name = std::move(other.m_first_name);
        m_last_name = std::move(other.m_last_name);
        m_command = std::move(other.m_command);
        m_type = std::move(other.m_type);
        return *this;
    }

private:
    long long m_id;
    std::string m_first_name;
    std::string m_last_name;
    std::string m_command;
    std::string m_type;
};
}

#endif // CHAT_HPP
