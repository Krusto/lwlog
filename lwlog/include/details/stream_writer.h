#pragma once

#include <filesystem>

namespace lwlog::details
{
    enum class stream_mode
    {
        none = 0,
        append = 1,
        overwrite_empty = 2
    };

    template <typename FlushPolicy>
    class stream_writer
    {
    public:
        explicit stream_writer(std::FILE* stream, stream_mode mode = stream_mode::append);
        explicit stream_writer(std::string_view path, stream_mode mode = stream_mode::append);
        virtual ~stream_writer();

    public:
        void write(std::string_view message) const;
        std::FILE* handle() const;
        std::filesystem::path& filesystem_path();

    private:
        std::FILE* m_stream{nullptr};
        std::filesystem::path m_path{};
    };
}// namespace lwlog::details

#include "stream_writer_impl.h"