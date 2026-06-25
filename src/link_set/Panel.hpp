#pragma once
#ifndef SETCALCULATOR_Panel_HPP
#define SETCALCULATOR_Panel_HPP

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

#include "Format.hpp"
#include "UIModule.hpp"   // 确保包含 LineBase、IHorizontalAlignable、IPaddable 等定义

// ---------- Panel 配置结构 ----------
struct PanelOptions {
    int width = 2;
    int height = -1;
    std::string title = "";
    int padding = -1;
    Horizontal items_horizontal_alignment = Horizontal::Null;
    Vertical vertical_alignment = Vertical::Null;
    int items_padding = -1;
};

// ---------- Panel 类 ----------
class Panel {
protected:
    std::string title;
    int width = 2;
    int height = -1;
    int padding = -1;
    Horizontal items_horizontal_alignment = Horizontal::Null;
    Vertical vertical_alignment = Vertical::Null;
    int items_padding = -1;
    std::vector<std::unique_ptr<LineBase>> lines;

public:
    // 构造函数：接受 Options 结构
    explicit Panel(const PanelOptions& opts = {})
        : width(opts.width),
          height(opts.height),
          title(opts.title),
          padding(opts.padding),
          items_horizontal_alignment(opts.items_horizontal_alignment),
          vertical_alignment(opts.vertical_alignment),
          items_padding(opts.items_padding) {}

    // 便捷构造函数
    Panel(int width = 2, int height = -1, const std::string& title = "")
        : Panel(PanelOptions{width, height, title, -1, Horizontal::Null, Vertical::Null, -1}) {}

    void addLine(const LineBase& line, int count = 1) {
        if (count <= 0) return;
        for (int i = 0; i < count; ++i) {
            lines.push_back(line.clone());   // 多态克隆
        }
    }

    void addLine(int count = 1) {
        Line defaultLine(width, '=');
        addLine(defaultLine, count);
    }

    void setAlLWidth() const {
        for (auto &line : lines) {
            line->setLength(width);
        }
    }

    void setALLHroizntalAlign() const {
        if (items_horizontal_alignment == Horizontal::Null) {
            return;
        }
        for (auto &line : lines) {
            if (auto* alignable = dynamic_cast<IHorizontalAlignable*>(line.get())) {
                alignable->setHorizontalAlign(items_horizontal_alignment);
            }
        }
    }

    void setALLPadding() const {
        if (items_padding == -1) {
            return;
        }
        for (auto &line : lines) {
            if (auto* p = dynamic_cast<IPaddable*>(line.get())) {
                p->setPadding(items_padding);
            }
        }
    }

    // 固定高度布局（带标题处理）
    void fixed_height_show() const {
        int panel_interior_height = height - 2;
        if (title != "") {
            Line(width).show();
            Textline(title, width, Horizontal::Center).show();
            panel_interior_height -= 2;
        }
        Line(width).show();
        if (lines.size() > static_cast<size_t>(panel_interior_height)) {
            throw std::overflow_error("lines overflow");
        }
        for (auto &line : lines) {
            line->show();
        }
        Line(width).show();
    }

    // 主显示函数
    void show() const {
        setAlLWidth();
        setALLHroizntalAlign();
        setALLPadding();
        if (height != -1) {
            fixed_height_show();
            return;
        }
        if (title != "") {
            Line(width).show();
            Textline(title, width, Horizontal::Center).show();
        }
        Line(width).show();
        for (auto &line : lines) {
            line->show();
        }
        Line(width).show();

    }

    void clearLines() { lines.clear(); }



};

#endif // SETCALCULATOR_Panel_HPP