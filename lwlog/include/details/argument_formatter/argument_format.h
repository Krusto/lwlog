#pragma once

#include <string>

#include "details/memory_buffer.h"

namespace lwlog::details
{
	template<typename BufferLimits>
	static void format_args(memory_buffer<BufferLimits::message>& msg,
		const char(&args)[BufferLimits::arg_count][BufferLimits::argument], std::uint8_t arg_count);
}

#include "argument_format_impl.h"