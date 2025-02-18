/*************************************************************************
    > File Name: buffer_id.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月20日 星期五 18时26分20秒
 ************************************************************************/

#ifndef __COURING_BUFFER_ID__
#define __COURING_BUFFER_ID__

#include <stdint.h>
#include <memory>

namespace eular {

class BufferId
{
    friend class BufferRegistration;
    friend class BufferProvide;
public:
    ~BufferId() = default;

    BufferId(const BufferId &other) :
        m_isRegistrationBuffer(false),
        m_gid(-1),
        m_bid(-1),
        m_size(0),
        m_buffer(nullptr)
    {
        if (this != std::addressof(other)) {
            m_isRegistrationBuffer = other.m_isRegistrationBuffer;
            m_gid = other.m_gid;
            m_bid = other.m_bid;
            m_size = other.m_size;
            m_buffer = other.m_buffer;
        }
    }

    BufferId &operator=(const BufferId &other)
    {
        if (this != std::addressof(other)) {
            m_isRegistrationBuffer = other.m_isRegistrationBuffer;
            m_gid = other.m_gid;
            m_bid = other.m_bid;
            m_size = other.m_size;
            m_buffer = other.m_buffer;
        }

        return *this;
    }

    void *buffer() const { return m_buffer; }
    int32_t gid() const { return m_gid; }
    int32_t bid() const { return m_bid; }
    uint32_t size() const { return m_size; }

private:
    BufferId() :
        m_isRegistrationBuffer(false),
        m_gid(-1),
        m_bid(-1),
        m_size(0),
        m_buffer(nullptr)
    {
    }

private:
    bool        m_isRegistrationBuffer;
    int32_t     m_gid; // provide buffer's group id
    int32_t     m_bid;
    uint32_t    m_size;
    void*       m_buffer;
};

} // namespace eular

#endif // __COURING_BUFFER_ID__
