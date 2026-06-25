#pragma once
#ifndef SETCALCULATOR_MainPanel_HPP
#define SETCALCULATOR_MainPanel_HPP

#include <conio.h>

#include "Panel.hpp"
#include "UIModule.hpp"
#include "LinkSet.hpp"
#include <string>
#include <cstdlib>  // for system
#include <limits>

using namespace std::string_literals;

inline int getChoice() {
    return _getch();
}

static void inputSetFromConsole(LinkSet<int> &result);

class MainPanel {
private:
    Panel panel = Panel(PanelOptions{
        .width = 120,
        .title = "链表集合演示系统",
        .items_padding = 5
    });
    LinkSet<int> setA{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    std::string outputA = setA.toString();
    LinkSet<int> setB{2, 5, 6, 9, 12, 15, 17, 19, 21, 22, 25, 27, 28, 30, 32, 33};
    std::string outputB = setB.toString();
    std::string defaultOUL = "输入A(q)    输入B(w)    查找(e)     插入(r)     交集(t)";
    std::string optionsUpLine = defaultOUL;
    std::string defaultODL = "并集(a)     差集(s)     对称差(d)   清空(f)     退出(ESC)";
    std::string optionsDownLine = defaultODL;
    std::string output = "";

public:
    MainPanel() = default;

    // 重建面板内容（使用当前 setA 和 setB）
    void refreshUI() {
        panel.clearLines(); // 清空旧行
        panel.addLine(Spaceline(SpacelineOptions{}));
        panel.addLine(Textline(TextlineOptions{
            .text = "集合A:"s + outputA
        }));
        panel.addLine(Textline(TextlineOptions{
            .text = "集合B:"s + outputB
        }));
        panel.addLine(Spaceline(SpacelineOptions{}));
        panel.addLine(1);
        panel.addLine(Spaceline(SpacelineOptions{}));
        panel.addLine(Textline(TextlineOptions{
            .text = "操作（键位）:",
        }));
        panel.addLine(Textline(TextlineOptions{
            .text = optionsUpLine,
            .align = Horizontal::Start
        }));
        panel.addLine(Textline(TextlineOptions{
            .text = optionsDownLine,
            .align = Horizontal::Start
        }));
        panel.addLine(Spaceline(SpacelineOptions{}));
        panel.addLine(1);
        panel.addLine(Spaceline(SpacelineOptions{}));
        panel.addLine(Textline(TextlineOptions{
            .text = "output:"s + output
        }));
        panel.addLine(Spaceline(SpacelineOptions{}));
    }

    // 显示面板（清屏 + 重建 + 显示）
    void show() {
        system("cls");
        refreshUI();
        panel.show();
    }

    void setSetA() {
        inputSetFromConsole(setA);
        outputA = setA.toString();
    }

    void setSetB() {
        inputSetFromConsole(setB);
        outputB = setB.toString();
    }

    void indexOfSet() {
        std::cout << "请输入要查找的元素:" << std::endl;
        int value;
        std::cin >> value;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::string valueStr = std::to_string(value);

        int inA = setA.indexOf(value);
        int inB = setB.indexOf(value);

        outputA = (inA != -1) ? setA.toColoredString(inA) : setA.toString();
        outputB = (inB != -1) ? setB.toColoredString(inB) : setB.toString();

        std::string new_output;
        if (inA != -1 && inB != -1) {
            new_output = "集合A、B找到元素" + valueStr + "，已在集合中由绿色标出";
        } else if (inA != -1) {
            new_output = "集合A找到元素" + valueStr + "，已在集合中由绿色标出,集合B未找到";
        } else if (inB != -1) {
            new_output = "集合A未找到,集合B找到元素" + valueStr + "，已在集合中由绿色标出";
        } else {
            new_output = "集合A、B均未找到元素" + valueStr;
        }
        output = new_output;
    }

    void insertSet() {
        optionsUpLine = "在集合A中插入元素(A)     在集合B中插入元素(B)";
        optionsDownLine = "在A、B中均插入元素(C)    返回上一级(ESC)";
        show();
        int choice;
        do {
            choice = getChoice();
            choice = tolower(choice);
        } while (choice != 'a' && choice != 'b' && choice != 'c' && choice != 27);

        switch (choice) {
            case 'a': {
                int value;
                std::cout << "请输入要插入的元素：" << std::endl;
                std::cin >> value;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                int index = setA.insert(value);
                if (index != -1) {
                    outputA = setA.toColoredString(index);
                    outputB = setB.toString();
                    output = "插入成功，插入元素已用绿色高亮标出。";
                } else {
                    output = "元素"s + std::to_string(value) + "已存在！";
                }
                break;
            }
            case 'b': {
                int value;
                std::cout << "请输入要插入的元素：" << std::endl;
                std::cin >> value;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                int index = setB.insert(value);
                if (index != -1) {
                    outputB = setB.toColoredString(index);
                    outputA = setA.toString();
                    output = "插入成功，插入元素已用绿色高亮标出。";
                } else {
                    output = "元素"s + std::to_string(value) + "已存在！";
                }
                break;
            }
            case 'c': {
                int value;
                std::cout << "请输入要插入的元素：" << std::endl;
                std::cin >> value;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                int indexA = setA.insert(value);
                int indexB = setB.insert(value);
                if (indexA != -1 || indexB != -1) {
                    outputB = setB.toColoredString(indexB);
                    outputA = setA.toColoredString(indexA);
                    output = "插入成功，插入元素已用绿色高亮标出。";
                } else {
                    output = "元素"s + std::to_string(value) + "在A、B中已存在！";
                }
                break;
            }
            case 27: {
                break;
            }
            default: {
            }
        }
        optionsUpLine = defaultOUL;
        optionsDownLine = defaultODL;
    }

    void intersectionSet() {
        output = "A、B交集："s + LinkSet<int>::intersectionSet(setA, setB).toString();
    }

    void unionSet() {
        output = "A、B并集"s + LinkSet<int>::unionSet(setA, setB).toString();
    }

    void differenceSet() {
        output = "A、B差集"s + LinkSet<int>::differenceSet(setA, setB).toString();
    }

    void symmetricDifference() {
        output = "A、B对称差："s + LinkSet<int>::symmetricDifference(setA, setB).toString();
    }

    void clearSet() {
        optionsUpLine = "清空集合A(A)         清空集合B(B)";
        optionsDownLine = "清空集合A、B(C)     返回上一级(ESC)";
        show();
        int choice;
        do {
            choice = getChoice();
            choice = tolower(choice);
        } while (choice != 'a' && choice != 'b' && choice != 'c' && choice != 27);

        switch (choice) {
            case 'a': {
                setA.clear();
                break;
            }
            case 'b': {
                setB.clear();
                break;
            }
            case 'c': {
                setA.clear();
                setB.clear();
                break;
            }
            case 27: {
                break;
            }
            default: {
            }
        }
        outputA = setA.toString();
        outputB = setB.toString();
        optionsUpLine = defaultOUL;
        optionsDownLine = defaultODL;
        output = "";
    }

    // 更新集合的移动赋值

    void setOutput(std::string output) {
        this->output = output;
    }
};

static void inputSetFromConsole(LinkSet<int> &result) {
    std::cout << "请输入集合元素，用英文逗号分隔（例如：1,2,3）:" << std::endl;
    std::string line;
    std::getline(std::cin, line);

    result.clear();
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, ',')) {
        // 去除前后空格
        size_t start = token.find_first_not_of(" \t");
        size_t end = token.find_last_not_of(" \t");
        if (start == std::string::npos || end == std::string::npos)
            continue; // 空 token，跳过

        token = token.substr(start, end - start + 1);
        try {
            int value = std::stoi(token);
            result.insert(value);
        } catch (const std::exception &) {
            std::cerr << "警告：忽略无效元素 '" << token << "'" << std::endl;
        }
    }
}

#endif
