#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

class UdpSocket 
{
public:
#ifdef _WIN32
    using SocketHandle = std::uintptr_t;
    static constexpr SocketHandle invalidSocket_ = static_cast<SocketHandle>(~0ULL);
#else
    using SocketHandle = int;
    static constexpr SocketHandle invalidSocket_ = -1;
#endif

    UdpSocket();
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    void openSender();
    void openReceiver(std::uint16_t listenPort);
    bool sendTo(const std::byte* data, std::size_t size, const std::string& host, std::uint16_t port);
    int receive(std::byte* data, std::size_t size);
    void close();

private:
    static void initializeNetwork();
    static void shutdownNetwork();
    void setNonBlocking();

    SocketHandle socket_{invalidSocket_};
};

