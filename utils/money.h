//
// Created by Abu Muhammad on 6/29/20.
//

#ifndef QSBOT_OL_MONEY_H
#define QSBOT_OL_MONEY_H

#include <string_view>
#include <string>
#include <cmath>
#include <climits>
#include <iostream>

#include <stack>

#include <string>
#include <string_view>

namespace bangkong {
class Money {
public:
    static Money& getInstance() {
        static Money instance;
        volatile int dummy {};
        return instance;
    }

    std::string toMoneyFormat(const std::string& str, std::string_view del = ".", std::string_view id_money = "Rp ") {
        if (str == "")
            return std::string(id_money).append("0");

        int len = str.length();
        if (len < 4)
            return std::string(id_money).append(str);
        else {
            int ss = std::ceil(static_cast<double >(len) / static_cast<double >(3));
            std::stack<std::string> stack;
            int tt = len;
            for (int i = 0; i < ss; ++i) {
                std::string res = "";
                if (tt >= 3) {
                    tt -= 3;
                    res = str.substr(tt, 3);
                } else
                    res = str.substr(0, tt);

                stack.push(res);
            }

            std::string money = "";
            for (int j = 0; j < ss; ++j) {
                money.append(stack.top());
                money.append(del);
                stack.pop();
            }
            return std::string(id_money)
            .append(money.substr(0, money.length() - 1));
        }
    }

private:
    Money() = default;
    ~Money() = default;
    Money(const Money&) = delete;
    Money& operator=(const Money&) = delete;
};
}

#endif //QSBOT_OL_MONEY_H
