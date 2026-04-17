#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <format>
#include <iostream>

namespace lang
{
    enum class DiagnosticLevel : uint8_t
    {
        WARNING,
        ERROR,
        COUNT,
    };

    static_assert(static_cast<std::size_t>(DiagnosticLevel::COUNT) == 2,
                  "DiagnosticLevel added — update flush()");

    struct Diagnostic
    {
        DiagnosticLevel level;
        std::string     message;
        std::size_t     pos;
    };

    struct DiagnosticEngine
    {
        std::vector<Diagnostic> diagnostics;
        bool                    has_errors = false;

        void error(const std::string& msg, std::size_t pos = 0)
        {
            diagnostics.push_back({DiagnosticLevel::ERROR, msg, pos});
            has_errors = true;
        }

        void warn(const std::string& msg, std::size_t pos = 0)
        {
            diagnostics.push_back({DiagnosticLevel::WARNING, msg, pos});
        }

        void flush(const std::string& src)
        {
            for (auto& d : diagnostics)
            {
                int         line       = 1;
                std::size_t line_start = 0;
                for (std::size_t i = 0; i < d.pos && i < src.size(); ++i)
                {
                    if (src[i] == '\n')
                    {
                        line++;
                        line_start = i + 1;
                    }
                }
                int              col   = static_cast<int>(d.pos - line_start) + 1;
                std::string_view level = d.level == DiagnosticLevel::ERROR ? "error" : "warn";
                std::cerr << std::format("[{}] {}:{}: {}\n", level, line, col, d.message);
            }
        }
    };
}
