/*************************************************************************
    > File Name: uring_event.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月18日 星期三 17时43分57秒
 ************************************************************************/

#ifndef __COURING_URING_EVENT_H__
#define __COURING_URING_EVENT_H__

#include <functional>
#include <memory>

#include <sys/types.h>
#include <sys/socket.h>

#include <liburing.h>

#include <utils/buffer.h>

namespace eular {
class UringLoop;
struct UserInfo;

class UringEvent
{
    friend class UringLoop;
public:
    using EvnetCB       = std::function<void(int32_t, const void *, uint32_t)>;
    using EvnetBuferCB  = std::function<void(int32_t, const ByteBuffer &)>;

    enum Event {
        None = IORING_OP_NOP, 
        Read = IORING_OP_READ,
        Readv = IORING_OP_READV,
        Recvmsg = IORING_OP_RECVMSG,
        Write,
        Writev,
        Sendmsg = IORING_OP_SENDMSG,
        Accepet = IORING_OP_ACCEPT,
    };

    UringEvent(int32_t fd, UringLoop *loop);
    ~UringEvent();

    /**
     * @brief 设置事件为持久状态, 如读写(默认为一次性事件)
     * 
     * @param persistent 
     */
    void setEventPersistent(bool persistent = true) { m_persistent = persistent; }

    /**
     * @brief 1、注册事件
     * 
     * @param ev io_uring_op
     * @param cb 回调
     * @return int32_t 
     */
    int32_t registerEvent(uint32_t ev, EvnetCB cb);
    int32_t registerEvent(uint32_t ev, EvnetBuferCB cb);

    /**
     * @brief 2、
     * 
     * @param size 
     */
    void setBufferSize(uint32_t size);

    /**
     * @brief 3、
     * 
     * @param gid 
     * @return int32_t 
     */
    int32_t provideBuffer(int32_t gid);

private:
    bool reRegisterEvent();
    void onCQECallback(struct io_uring_cqe *cqe);

private:
    int32_t         m_fd;
    uint32_t        m_ev;
    bool            m_persistent;
    int32_t         m_groupId;
    UringLoop*      m_loop;
    ByteBuffer      m_buffer;
    EvnetCB         m_cb;
    EvnetBuferCB    m_bufCB;
    struct msghdr   m_msg;

    std::shared_ptr<UserInfo>   m_provideInfo;
    std::shared_ptr<UserInfo>   m_eventInfo;
};

} // namespace eular

#endif // __COURING_URING_EVENT_H__
