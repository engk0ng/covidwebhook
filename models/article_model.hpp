#ifndef ARTICLE_MODEL_HPP
#define ARTICLE_MODEL_HPP

#include <string>

struct Article {
    std::string m_judul;
    std::string m_url;
    Article(const std::string& judul, const std::string& url):
        m_judul{judul}, m_url{url} {}

    Article(const Article&) = default;
    Article& operator=(const Article&) = default;

    Article(Article&& other) {
        m_judul = std::move(other.m_judul);
        m_url = std::move(other.m_url);
    }

    Article& operator=(Article&& other) {
        m_judul = std::move(other.m_judul);
        m_url = std::move(other.m_url);
        return *this;
    }

    bool operator==(const Article& other) {
        if (m_url == other.m_url) {
            return true;
        }
        else
            return false;
    }
};

#endif // ARTICLE_MODEL_HPP
