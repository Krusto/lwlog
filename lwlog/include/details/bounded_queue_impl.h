#pragma once

namespace lwlog::details
{
    template<std::size_t Capacity, typename T, typename OverflowPolicy, typename ConcurrencyModelPolicy>
    bool bounded_queue<Capacity, T, OverflowPolicy, ConcurrencyModelPolicy>::is_full(std::size_t next_write, 
        std::atomic_size_t& read_idx) const
    {
        return next_write == read_idx.load(std::memory_order_acquire);
    }

    template<std::size_t Capacity, typename T,
        typename OverflowPolicy, typename ConcurrencyModelPolicy>
    void bounded_queue<Capacity, T, OverflowPolicy, ConcurrencyModelPolicy>::enqueue(T&& item)
    {
        this->enqueue(std::move(item), ConcurrencyModelPolicy{});
    }

    template<std::size_t Capacity, typename T,
        typename OverflowPolicy, typename ConcurrencyModelPolicy>
    void bounded_queue<Capacity, T, OverflowPolicy, ConcurrencyModelPolicy>::enqueue(T&& item,
        [[maybe_unused]] spsc_model_policy)
    {
        const std::size_t current_write_index{ m_write_index.load(std::memory_order_relaxed) };
        const std::size_t next_write_index{ (current_write_index + 1) % ring_size };

        while (this->is_full(next_write_index, m_read_index))
        {
            OverflowPolicy::handle_overflow();
            if (OverflowPolicy::should_discard()) 
            {
                return;
            }
        }

        m_storage[current_write_index] = std::move(item);
        m_write_index.store(next_write_index, std::memory_order_release);
    }

    template<std::size_t Capacity, typename T,
        typename OverflowPolicy, typename ConcurrencyModelPolicy>
    void bounded_queue<Capacity, T, OverflowPolicy, ConcurrencyModelPolicy>::enqueue(T&& item,
        [[maybe_unused]] mpsc_model_policy)
    {
        while (m_mpsc_lock.test_and_set(std::memory_order_acquire))
        {
            LWLOG_CPU_PAUSE();
        }

        const std::size_t current_write_index{ m_write_index.load(std::memory_order_relaxed) };
        const std::size_t next_write_index{ (current_write_index + 1) % ring_size };

        while (this->is_full(next_write_index, m_read_index))
        {
            m_mpsc_lock.clear(std::memory_order_release);
            OverflowPolicy::handle_overflow();

            if (OverflowPolicy::should_discard())
            {
                return;
            }

            while (m_mpsc_lock.test_and_set(std::memory_order_acquire))
            {
                LWLOG_CPU_PAUSE();
            }
        }

        m_storage[current_write_index] = std::move(item);
        m_write_index.store(next_write_index, std::memory_order_release);
        m_mpsc_lock.clear(std::memory_order_release);
    }

    template<std::size_t Capacity, typename T,
        typename OverflowPolicy, typename ConcurrencyModelPolicy>
    T bounded_queue<Capacity, T, OverflowPolicy, ConcurrencyModelPolicy>::dequeue()
    {
        std::size_t current_read_index{ m_read_index.load(std::memory_order_relaxed) };

        while (current_read_index == m_write_index.load(std::memory_order_acquire))
        {
            OverflowPolicy::handle_underflow();
        }

        T out{ std::move(m_storage[current_read_index]) };

        const std::size_t next{ (current_read_index + 1) % ring_size };
        m_read_index.store(next, std::memory_order_release);

        return out;
    }

    template<std::size_t Capacity, typename T,
        typename OverflowPolicy, typename ConcurrencyModelPolicy>
    bool bounded_queue<Capacity, T, OverflowPolicy, ConcurrencyModelPolicy>::is_empty() const
    {
        return m_read_index.load(std::memory_order_relaxed) 
            == m_write_index.load(std::memory_order_relaxed);
    }
}