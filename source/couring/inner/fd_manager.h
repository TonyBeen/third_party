/*************************************************************************
    > File Name: fd_manager.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月17日 星期二 17时52分29秒
 ************************************************************************/

#ifndef __COURING_FD_MANAGER_H__
#define __COURING_FD_MANAGER_H__

#include <unordered_map>
#include <memory>

#include <sys/stat.h>
#include <utils/utils.h>
#include <utils/thread_local.h>

#define TAG_FD_MANAGER  "FdManager"

namespace eular {

class FdContext : public std::enable_shared_from_this<FdContext>
{
    DISALLOW_COPY_AND_ASSIGN(FdContext);
    friend class FdManager;

private:
    FdContext(int32_t fd);

public:
    using SP = std::shared_ptr<FdContext>;
    using WP = std::weak_ptr<FdContext>;

    bool socketFile() const { return S_ISSOCK(_type); }
    bool regularFile() const { return S_ISREG(_type); }
    bool pipeFile() const { return S_ISFIFO(_type); }

    void setTimeout(int32_t type, uint32_t ms);

public:
    bool        _isSysNonblock: 1;
    bool        _isUserNonblock: 1;

    int32_t     _fd;
    uint32_t    _type; // stat::st_mode
    uint32_t    _recvTimeoutMs;
    uint32_t    _sendTimeoutMs;
};

class FdManager final
{
public:
    FdManager() { }
    ~FdManager() { }
    FdManager(const FdManager &other) {
        if (this != std::addressof(other)) {
            _contextHashMap = other._contextHashMap;
        }
    }

    FdManager&operator=(const FdManager &other) {
        if (this != std::addressof(other)) {
            _contextHashMap = other._contextHashMap;
        }

        return *this;
    }

    FdContext::SP get(int32_t fd, bool needCreate = false);
    void remove(int32_t fd);

private:
    std::unordered_map<int32_t, FdContext::SP>  _contextHashMap;
};

template<typename T>
T *InstanceFromTLS(const std::string &key)
{
    using namespace eular;
    std::shared_ptr<eular::TLSSlot<T>> tlsSlot = eular::ThreadLocalStorage::Current()->get<T>(key);
    if (tlsSlot == nullptr) {
        tlsSlot = eular::ThreadLocalStorage::Current()->set<T>(key, T());
    }

    return tlsSlot->pointer();
}

} // namespace eular

#endif // __COURING_FD_MANAGER_H__
