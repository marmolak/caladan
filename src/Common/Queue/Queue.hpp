#pragma once

namespace Common {

template <typename QueueMember, unsigned int QueueLen>
class Queue
{
    public:
        struct queue_ret
        {
            QueueMember member;
            bool        ret_code;
        };
        using queue_ret_t = struct queue_ret;

        bool push(const QueueMember member)
        {
            if (m_queue_index == (QueueLen - 1))
            {
                return false;
            }

            ++m_queue_index;
            m_queue[m_queue_index] = member;
            return true;
        }

        queue_ret pop()
        {
            static int head = 0;
            if (empty()) 
            {
                head = 0;
                return queue_ret {
                     // stupid but works in this compiler...
                     .member    = m_queue[0],
                     .ret_code  = false
                };
            }

            --m_queue_index;
            return queue_ret {
                .member     = m_queue[head++],
                .ret_code   = true
            };
        }

        bool empty() const
        {
            const bool is_empty = (m_queue_index == -1);
            return is_empty;
        }

        void flush()
        {
            m_queue_index = -1;
        }

    private:
        QueueMember m_queue[QueueLen];
        int m_queue_index { -1 };
};

template <typename T>
class QueueAutoFlush
{
    public:
        QueueAutoFlush(T &queue) : m_queue(queue) {};
        ~QueueAutoFlush() { m_queue.flush(); }

    private:
        T &m_queue;
};

} // end namespace