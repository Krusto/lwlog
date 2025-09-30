#pragma once

#include "argument_format.h"

namespace lwlog::details
{
    template<typename BufferLimits>
    static void format_args(memory_buffer<BufferLimits::message>& msg, 
        const char(&args)[BufferLimits::arg_count][BufferLimits::argument], std::uint8_t arg_count)
    {
        std::size_t pos{ 0 };
        std::size_t idx{ 0 };

        while (pos + 1 < msg.size()) 
        {
            if (msg[pos] == '{' && msg[pos + 1] == '}') 
            {
                if (idx < arg_count) 
                {
                    msg.replace(pos, 2, args[idx], std::strlen(args[idx]));
                    ++idx;
                }
                else 
                {
                    msg.replace(pos, 2, "", 0);
                }
            }
            ++pos;
        }
    }
}