#pragma once
#ifndef FORMAT_HPP
#define FORMAT_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <utility>

namespace Stringf {
    namespace detail {
        template<typename T>

        void append(std::stringstream& ss, const char* fmt, int& offset, const T& value) {
            while (fmt[offset] != '\0') {
                if (fmt[offset] == '%') {
                    if (fmt[offset + 1] == '%') {
                        ss << '%';
                        offset += 2;
                        continue;
                    }
                    break;
                }
                ss << fmt[offset++];
            }
            if (fmt[offset++] == '%') {
                if (fmt[offset] == '\0') return;
                switch (fmt[offset++]) {
                    case 's': ss << value; break;
                    case 'd': ss << std::dec << value; ss.unsetf(std::ios::fixed); break;
                    case 'b': ss << std::boolalpha << value; ss.unsetf(std::ios::fixed); break;
                    case '.':
                    case '<':
                        if (fmt[offset] >= '0' && fmt[offset] <= '9') {
                            char* endPtr = nullptr;
                            const auto precision = strtol(fmt + offset, &endPtr, 10);
                            const auto len = endPtr - (fmt + offset);
                            if (fmt[offset + len] == 'f') {
                                ss << std::fixed << std::setprecision(precision) << value;
                                ss.unsetf(std::ios::fixed);
                                offset += len + 1;
                            } else {
                                ss << value;
                            }
                        } else {
                            ss << value;
                        }
                        break;
                    default: ss << value;
                }
            }
        }
    } // namespace detail

    template<typename... Args>
    std::string format(const std::string &format, Args &&... args) {
        std::stringstream ss;
        const auto & fmt = format.c_str();
        int writeOffset = 0;

        // 展开格式化
        (detail::append(ss, fmt, writeOffset, std::forward<Args>(args)), ...);

        // 剩余部分
        detail::append(ss, fmt, writeOffset, "");
        return ss.str();
    }
} // namespace Stringf

#endif // FORMAT_HPP