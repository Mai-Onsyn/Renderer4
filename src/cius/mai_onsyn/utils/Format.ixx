module;
#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <tuple>

export module Format;
import Types; // 引入你的 String

// 内部实现细节，隐藏在命名空间中
namespace FormatInternal {

    // 函数 1: 处理单个参数的格式化输出
    template <typename T>
    void formatSingleArg(std::stringstream& ss, const T& arg, std::string_view spec) {
        using DecayedT = std::decay_t<T>;

        // 处理布尔值 %b
        if constexpr (std::is_same_v<DecayedT, bool>) {
            ss << (arg ? "true" : "false");
        }
        // 处理浮点数 %.xf
        else if constexpr (std::is_floating_point_v<DecayedT>) {
            if (spec.size() > 2 && spec[1] == '.') {
                int precision = 0;
                for (size_t i = 2; i < spec.size() - 1; ++i) {
                    if (spec[i] >= '0' && spec[i] <= '9') {
                        precision = precision * 10 + (spec[i] - '0');
                    }
                }
                ss << std::fixed << std::setprecision(precision) << arg;
            } else {
                ss << arg;
            }
        }
        // 处理其余通用类型 (int, string 等)
        else {
            ss << arg;
        }
    }

    // 函数 2: 动态运行时通过索引匹配解包参数
    template <typename... Args>
    void dispatchArgByIndex(std::stringstream& ss, size_t targetIndex, std::string_view spec, const std::tuple<Args...>& argsTuple) {
        std::apply([&](const auto&... pack) {
            size_t currentIndex = 0;
            // 折叠表达式：顺着参数列表数数，数到 targetIndex 时触发格式化
            ((currentIndex++ == targetIndex ? (formatSingleArg(ss, pack, spec), 0) : 0), ...);
        }, argsTuple);
    }
}

// =================================================================
// 外部导出接口
// =================================================================

export template<typename... Args>
String format(std::string_view fmt, Args&&... args) {
    std::stringstream ss;
    auto argsTuple = std::forward_as_tuple(args...);

    size_t argIndex = 0;
    const size_t argCount = sizeof...(args);
    size_t i = 0;

    while (i < fmt.size()) {
        // 遇到普通字符，直接写入
        if (fmt[i] != '%') {
            ss << fmt[i];
            i++;
            continue;
        }

        // 处理转义的 "%%"
        if (i + 1 < fmt.size() && fmt[i + 1] == '%') {
            ss << '%';
            i += 2;
            continue;
        }

        // 开始解析占位符说明符 (Specifier)
        size_t specStart = i;
        i++; // 跳过 '%'

        // 步进读取数字和点号 (如 .2f 中的 .2)
        while (i < fmt.size() && (fmt[i] == '.' || (fmt[i] >= '0' && fmt[i] <= '9'))) {
            i++;
        }

        // 此时 fmt[i] 应当是类型字符 (d, s, f, b)
        if (i < fmt.size()) {
            std::string_view spec = fmt.substr(specStart, i - specStart + 1);
            i++; // 跳过类型字符本身

            // 提取并分发对应的参数
            if (argIndex < argCount) {
                FormatInternal::dispatchArgByIndex(ss, argIndex, spec, argsTuple);
                argIndex++;
            } else {
                ss << "[Missing]";
            }
        }
    }

    return String(ss.str());
}