#include <memory>
#include <dns_resolve.h>

ev::EventLoop::Ptr g_eventLoop = std::make_unique<ev::EventLoop>();

int main()
{
    ev::DNSManager::Ptr manager = std::make_unique<ev::DNSManager>(g_eventLoop.get());
    ev::DNSResolver::Ptr resolver = std::make_unique<ev::DNSResolver>(manager.get());
    resolver->setResolveTimeout(3000);
    resolver->resolve("www.example.com", [](int32_t code, const ev::DNSResultVec &resultVec) {
        g_eventLoop->breakLoop();
        if (code != DNS_SUCCESS) {
            printf("resolve www.example.com error\n");
            return;
        }

        printf("www.example.com resolution successful\n");
        for (const auto &it : resultVec) {
            if (it.isIPv6) {
                printf("\tIPv6: %s, ttl: %u\n", it.address.c_str(), it.ttl);
            } else {
                printf("\tIPv4: %s, ttl: %u\n", it.address.c_str(), it.ttl);
            }
        }
    });

    g_eventLoop->dispatch();
    return 0;
}