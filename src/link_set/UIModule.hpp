#pragma once
#ifndef SETCALCULATOR_UIModule_HPP
#define SETCALCULATOR_UIModule_HPP

#include <iostream>
#include <string>
#include <memory>          // 新增：用于 std::unique_ptr
#include <stdexcept>       // 新增：用于异常（虽然已有，但显式包含更安全）
// #include "Format.hpp"

int display_width(const std::string& str) {
    int width = 0;
    size_t i = 0;
    while (i < str.size()) {
        // 检测 ANSI 转义序列：\033[...字母
        if (str[i] == '\033' && i + 1 < str.size() && str[i + 1] == '[') {
            i += 2;
            // 跳过参数（数字、分号、问号等）
            while (i < str.size() && !( (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z') )) {
                ++i;
            }
            // 跳过结束字母
            if (i < str.size()) ++i;
            continue;  // 不计数
        }
        unsigned char ch = str[i];
        if (ch < 0x80) {
            width += 1;
        } else if ((ch & 0xC0) == 0xC0) {
            width += 2;
        }
        ++i;
    }
    return width;
}

enum class Horizontal {
    Start,
    End,
    Center,
    Null,
};

enum class Vertical {
    Center,
    Top,
    Bottom,
    Null,
};

class IHorizontalAlignable {
public:
    virtual void setHorizontalAlign(Horizontal align) = 0;
    virtual Horizontal getHorizontalAlign() const = 0;
    virtual ~IHorizontalAlignable() = default;
};

class IVerticalAlignable {
public:
    virtual void setVerticalAlign(Vertical align) = 0;
    virtual Vertical getVerticalAlign() const = 0;
    virtual ~IVerticalAlignable() = default;
};

class IPaddable {
public:
    virtual void setPadding(int pad) = 0;
    virtual int getPadding() const = 0;
    virtual ~IPaddable() = default;
};

// ---------- 基类 LineBase ----------
class LineBase {
protected:
    int length = 0;
    char border = '|';
public:
    LineBase(int len = 0, char b = '|') : length(len), border(b) {}
    virtual ~LineBase() = default;

    virtual void show() const = 0;

    // ---------- 新增克隆接口 ----------
    virtual std::unique_ptr<LineBase> clone() const = 0;

    void setLength(int len) { length = len; }
    void setBorder(char b) { border = b; }
    int getLength() const { return length; }
};

// ---------- Line ----------
struct LineOptions {
    int length = 0;
    char ch = '=';
};

class Line : public LineBase {
private:
    char ch = '=';
public:
    explicit Line(const LineOptions& opts = {})
        : LineBase(opts.length), ch(opts.ch) {}

    Line(int len = 0, char ch = '=')
        : Line(LineOptions{len, ch}) {}

    void show() const override {
        for (int i = 0; i < length; ++i) std::cout << ch;
        std::cout << std::endl;
    }

    // ---------- 实现 clone ----------
    std::unique_ptr<LineBase> clone() const override {
        return std::make_unique<Line>(*this);
    }
};

// ---------- Spaceline ----------
struct SpacelineOptions {
    int length = 0;
    char border = '|';
};

class Spaceline : public LineBase {
public:
    explicit Spaceline(const SpacelineOptions& opts = {})
        : LineBase(opts.length, opts.border) {}

    Spaceline(int len = 0, char b = '|')
        : Spaceline(SpacelineOptions{len, b}) {}

    void show() const override {
        // 防止 length<2 时出现负循环
        if (length < 2) {
            std::cout << border << border << std::endl;
            return;
        }
        std::cout << border;
        for (int i = 0; i < length - 2; ++i) std::cout << ' ';
        std::cout << border << std::endl;
    }

    // ---------- 实现 clone ----------
    std::unique_ptr<LineBase> clone() const override {
        return std::make_unique<Spaceline>(*this);
    }
};

// ---------- Textline ----------
struct TextlineOptions {
    std::string text = "";
    int length = 0;
    Horizontal align = Horizontal::Null;
    int padding = 0;
    char border = '|';
};

class Textline : public LineBase, public IHorizontalAlignable, public IPaddable {
private:
    std::string text;
    Horizontal align = Horizontal::Null;
    int padding = 0;
public:
    explicit Textline(const TextlineOptions& opts = {})
        : LineBase(opts.length, opts.border),
          text(opts.text),
          align(opts.align),
          padding(opts.padding) {}

    Textline(const std::string& txt = "", int len = 0,
             Horizontal horizontal = Horizontal::Null,
             int pad = 0, char b = '|')
        : Textline(TextlineOptions{txt, len, horizontal, pad, b}) {}

    void setHorizontalAlign(Horizontal a) override { align = a; }
    Horizontal getHorizontalAlign() const override { return align; }
    void setPadding(int p) override { padding = p; }
    int getPadding() const override { return padding; }

    void show() const override {
        int text_len = display_width(text);
        if (text_len + 2 * padding > length) {
            throw std::out_of_range("text too long");
        }
        std::cout << border;
        switch (align) {
            case Horizontal::Center: {
                int left = (length - 2 - text_len) / 2;
                int right = length - 2 - text_len - left;
                std::cout << std::string(left, ' ') << text << std::string(right, ' ');
                break;
            }
            case Horizontal::End: {
                int left = length - 2 - padding - text_len;
                std::cout << std::string(left, ' ') << text << std::string(padding, ' ');
                break;
            }
            default: {
                int right = length - 2 - padding - text_len;
                std::cout << std::string(padding, ' ') << text << std::string(right, ' ');
                break;
            }
        }
        std::cout << border << std::endl;
    }

    // ---------- 实现 clone ----------
    std::unique_ptr<LineBase> clone() const override {
        return std::make_unique<Textline>(*this);
    }
};

#endif