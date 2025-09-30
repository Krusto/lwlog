#pragma once

namespace lwlog::details
{
    template<typename BufferLimits>
    argument_buffers_pool<BufferLimits>::argument_buffers_pool()
        : m_args_buffers_free_top{ BufferLimits::pool_size }
    {
        for (std::uint8_t i = 0; i < BufferLimits::pool_size; ++i)
        {
            m_args_buffers_free_indices[i] = i;
        }
    }

    template<typename BufferLimits>
    std::uint8_t argument_buffers_pool<BufferLimits>::acquire_args_buffer()
    {
        std::uint8_t top{ m_args_buffers_free_top.load(std::memory_order_acquire) };
        while (top > 0) 
        {
            if (m_args_buffers_free_top.compare_exchange_weak(
                top, static_cast<std::uint8_t>(top - 1),
                std::memory_order_acq_rel, std::memory_order_acquire))
            {
                const std::uint8_t slot_index{ m_args_buffers_free_indices[top - 1] };
                const std::uint8_t slot_handle{ static_cast<std::uint8_t>(slot_index + 1) };

                return slot_handle;
            }
        }

        return 0;
    }


    template<typename BufferLimits>
    void argument_buffers_pool<BufferLimits>::release_args_buffer(std::uint8_t slot_handle)
    {
        const std::uint8_t slot_index{ static_cast<std::uint8_t>(slot_handle - 1) };

        std::uint8_t top{ m_args_buffers_free_top.load(std::memory_order_acquire) };

        while (top < static_cast<std::uint8_t>(BufferLimits::pool_size)) 
        {
            m_args_buffers_free_indices[top] = slot_index;

            if (m_args_buffers_free_top.compare_exchange_weak(
                top, static_cast<std::uint8_t>(top + 1),
                std::memory_order_acq_rel, std::memory_order_acquire))
            {
                break;
            }
        }
    }

    template<typename BufferLimits>
    char(&argument_buffers_pool<BufferLimits>::get_args_buffer(std::uint8_t slot_index))
        [BufferLimits::arg_count][BufferLimits::argument]
    {
        return m_args_buffers[slot_index];
    }

    template<typename BufferLimits>
    const char(&argument_buffers_pool<BufferLimits>::get_args_buffer(std::uint8_t slot_index) const)
        [BufferLimits::arg_count][BufferLimits::argument]
    {
        return m_args_buffers[slot_index];
    }
}