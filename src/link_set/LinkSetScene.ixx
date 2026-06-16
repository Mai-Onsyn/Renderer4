module;
#include <cstring>
#include <vector>
#include "LinkSet.hpp"
export module LinkSetScene;
import Scene2D;
import Types;
import FramebufferWindow;
import InputManager;
import Logger;
import Box;
import Text;
import Graphics2D;
import Color;
import LinkSetAppStatics;
import Format;
import Vectors;
import Functions;

using namespace Graphics2D;

enum class OpState : UInt8 {
    INPUT_A, INPUT_B, FIND, INSERT, INTERSECT, UNION, DIFFERENCE, SYMMETRIC_DIFF, NONE
};

enum class OpTarget : UInt8 {
    SET_A, SET_B, RESULT
};

export class LinkSetScene final : public Scene2D {
    OpState opState = OpState::FIND;
    OpTarget opTarget = OpTarget::SET_A;
    LinkSet<Int32> setA{15, 23, 67, 89, 120, 45, 176, 3, 99, 144, 200, 34, 78, 12, 155, 188, 43, 91, 134, 7, 62, 109};
    LinkSet<Int32> setB{67, 99, 3, 144, 78, 12, 155, 188, 43, 7, 52, 82, 110, 171, 30, 196, 5, 87, 147, 163, 92, 40, 18, 193};
    LinkSet<Int32> setOptResult;

    Int16 frameFactor = 0;

    List<String> opNames{"插入", "删除", "覆盖","查找", "求交集", "求并集", "求差集", "对称差"};

    void drawButton(
        Float width,
        Float height,
        Float x,
        Float y,
        const std::string& text,
        const std::string& keyText,
        bool active
    ) {
        Box base{
            Rect2D{{x, y}, {width, height}},
            Alignment::TopLeft,
            active ? LinkSetTheme::NavBaseContainerActive
                   : LinkSetTheme::NavBaseContainer
        };

        Vector2D center{x + width / 2, y + height / 2};
        Text optName{
            text,
            center - Vector2D{0, LinkSetTheme::BodyTextPx / 2 - 2},
            Alignment::Center,
            LinkSetTheme::BodyTextPx,
            LinkSetTheme::OnNaveBase
        };

        Text optKey{
            keyText,
            center + Vector2D{0, LinkSetTheme::BodyTextPx / 2 + 2},
            Alignment::Center,
            LinkSetTheme::BodyTextPx - 4,
            LinkSetTheme::OnNaveBase
        };

        boxes.push_back(std::move(base));
        texts.push_back(std::move(optName));
        texts.push_back(std::move(optKey));
    }
    /**
     * 占用150像素高度
     * @param windowWidth 窗口宽度
     */
    void drawNavigation(const Float windowWidth) {
        Box navBase{
            Rect2D{{0, 0}, {windowWidth, 150}},
            Alignment::TopLeft,
            LinkSetTheme::NavBase
        };
        Text navTitle{
            "链表集合运算演示",
            {30, 10},
            Alignment::TopLeft,
            LinkSetTheme::TitleFontPx,
            LinkSetTheme::OnNaveBase
        };
        boxes.push_back(move(navBase));
        texts.push_back(move(navTitle));

        const Float btnWidth = 120;
        const Float btnHeight = 70;
        const Float btnPadding = 4;
        const Float btnStartY = 20 + LinkSetTheme::TitleFontPx;
        const Float btnStartX = 20;
        for (Int32 i = 0; i < opNames.size(); i++) {
            const Vector2D topLeft{btnStartX + i * (btnWidth + btnPadding), btnStartY};
            drawButton(
                btnWidth,
                btnHeight,
                topLeft.x,
                topLeft.y,
                opNames[i],
                format("F%d", i + 1),
                static_cast<Int32>(opState) == i
            );
        }
        String opTargetNames[2] = {"对A操作", "对B操作"};
        for (Int32 i = 0; i < 2; i++) {
            const Vector2D topLeft{btnStartX + (i + opNames.size()) * (btnWidth + btnPadding) + 20, btnStartY};
            drawButton(
                btnWidth,
                btnHeight,
                topLeft.x,
                topLeft.y,
                opTargetNames[i],
                format("F%d", i + 1 + opNames.size()),
                static_cast<Int32>(opTarget) == i
            );
        }
    }

    void drawSetRow(const Float startY, const Float rowHeight, const LinkSet<Int32> &set, const Color &primary, const String &name) {
        Float bodyHeight = 100;
        Float horizontalPadding = 40;
        Float bodyStartY = startY + (rowHeight - bodyHeight) / 2;
        Box mark{
            Rect2D{{horizontalPadding, bodyStartY}, {8, bodyHeight}},
            Alignment::TopLeft,
            primary
        };

        Text sectionTitle{
            name,
            {horizontalPadding + 20, bodyStartY + 4},
            Alignment::TopLeft,
            LinkSetTheme::BodyTitlePx,
            primary
        };

        Text sectionCount{
            format("(%d个元素)", set.getSize()),
            {horizontalPadding + 120, bodyStartY + 12},
            Alignment::TopLeft,
            LinkSetTheme::BodyTitlePx - 8,
            primary
        };

        Float contentStartX = horizontalPadding + 20;
        Float contentStartY = bodyStartY + 4 + LinkSetTheme::BodyTitlePx + 20;
        Float elementSize = 40;
        Float elementPadding = 12;

        if (!set.isEmpty()) {
            Box connectWire{
                Rect2D{{contentStartX, contentStartY + elementSize / 2 - 2}, {(elementSize + elementPadding) * (set.getSize() - 1), 4}},
                Alignment::TopLeft,
                primary
            };
            boxes.push_back(move(connectWire));

            set.forEachIndexed([&](const Int32& idx, const Int32& data) {
                Box elementBase{
                    Rect2D{{contentStartX, contentStartY}, {elementSize, elementSize}},
                    Alignment::TopLeft,
                    {222, 222, 222}
                };
                Text elementText{
                    toString(data),
                    {contentStartX + elementSize / 2, contentStartY + elementSize / 2},
                    Alignment::Center,
                    LinkSetTheme::BodyTextPx,
                    LinkSetTheme::OnBackground
                };
                boxes.push_back(move(elementBase));
                texts.push_back(move(elementText));
                contentStartX += elementSize + elementPadding;
            });
        } else {
            Text elementText{
                "集合为空",
                {contentStartX + elementSize / 2, contentStartY + elementSize / 2},
                Alignment::CenterLeft,
                LinkSetTheme::BodyTitlePx,
                LinkSetTheme::OnBackground
            };
            texts.push_back(move(elementText));
        }

        boxes.push_back(move(mark));
        texts.push_back(move(sectionTitle));
        texts.push_back(move(sectionCount));
    }

    void drawHorizontalDivider(const Float windowWidth, const Float padding, const Float centerY, const Float height) {
        Box divider{
            Rect2D{{padding, centerY - height / 2}, {windowWidth - padding * 2, height}},
            Alignment::TopLeft,
            LinkSetTheme::Outline
        };
        boxes.push_back(move(divider));
    }

    void drawBody(const Float windowWidth, const Float windowHeight) {
        Float verticalPadding = 0;
        Float brushStartY = 150 + verticalPadding;
        Float brushEndY = windowHeight - 200 - verticalPadding;
        Float heightPerRow = (brushEndY - brushStartY) / 3;

        drawSetRow(brushStartY, heightPerRow, setA, LinkSetTheme::PrimaryA, "集合A");
        drawHorizontalDivider(windowWidth, 40, brushStartY + heightPerRow, 2);
        drawSetRow(brushStartY + heightPerRow, heightPerRow, setB, LinkSetTheme::PrimaryB, "集合B");
        drawHorizontalDivider(windowWidth, 40, brushStartY + heightPerRow * 2, 2);
        drawSetRow(brushStartY + heightPerRow * 2, heightPerRow, setOptResult, LinkSetTheme::PrimaryC, "运算结果");
    }

    void drawFooter(const String& text, const Float windowWidth, const Float windowHeight) {
        Float footerStartY = windowHeight - 200;
        Box footer{
            {{0, footerStartY}, {windowWidth, 200}},
            Alignment::TopLeft,
            LinkSetTheme::FooterBase
        };
        boxes.push_back(move(footer));

        // 输入区域
        {
            Text inputBufferTip{
                "输入内容：",
                {100, footerStartY + 50},
                Alignment::CenterLeft,
                LinkSetTheme::BodyTextPx + 2,
                LinkSetTheme::OnBackground
            };
            texts.push_back(move(inputBufferTip));

            Text inputBuffer{
                Stringf::format("%s%s", text, frameFactor / 64 % 2 == 0 ? "_" : " "),
                {200, footerStartY + 50},
                Alignment::CenterLeft,
                LinkSetTheme::BodyTextPx + 2,
                {64, 64, 64}
            };
            texts.push_back(move(inputBuffer));
        }

        // 提示区域
        {
            Text tipLine1{
                "仅能输入数字，仅能在选择需要输入的选项时输入，按Enter提交内容",
                {100, footerStartY + 110},
                Alignment::CenterLeft,
                LinkSetTheme::BodyTextPx - 4,
                LinkSetTheme::NavBaseContainerActive
            };
            Text tipLine2{
                "集合元素用空格或者中文/英文逗号作为分割符，多个连续分割符视为一个",
                {100, footerStartY + 140},
                Alignment::CenterLeft,
                LinkSetTheme::BodyTextPx - 4,
                LinkSetTheme::NavBaseContainerActive
            };
            texts.push_back(move(tipLine1));
            texts.push_back(move(tipLine2));
        }

        drawHorizontalDivider(windowWidth, 0, footerStartY, 3);
    }
public:
    void update(InputManager* input, Int32 tps, Int32 windowWidth, Int32 windowHeight, FramebufferWindow* window) override {
        boxes.clear();
        texts.clear();

        Box backGround{
            Rect2D{{0, 0},{static_cast<Float>(windowWidth), static_cast<Float>(windowHeight)}},
            Alignment::TopLeft,
            LinkSetTheme::Background
        };
        boxes.push_back(move(backGround));

        drawNavigation(windowWidth);
        drawBody(windowWidth, windowHeight);
        drawFooter("111 22 3", windowWidth, windowHeight);

        frameFactor++;
    }
};