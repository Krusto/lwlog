#pragma once

#include "argument_format.h"

namespace lwlog::details
{
    template<typename BufferLimits>
    static void format_args(memory_buffer<BufferLimits::message>& msg,
        const char(&args)[BufferLimits::arg_count][BufferLimits::argument], std::uint8_t arg_count)
    {
		std::size_t pos{ 0 };
		std::size_t arg_i{ 0 };

		while (pos + 1 < msg.size())
		{
			if (msg[pos] == '{' && msg[pos + 1] == '}' && arg_i < arg_count)
			{
				const char* replacement{ args[arg_i] };
				std::size_t replacement_len{ std::char_traits<char>::length(replacement) };

				msg.replace(pos, 2, replacement, replacement_len);

				pos += replacement_len;
				++arg_i;
			}
			else
			{
				++pos;
			}
		}
    }
}