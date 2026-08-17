#include "UdpSocket.h"

#include <cstring>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {

#ifdef _WIN32
SOCKET toNative(UdpSocket::SocketHandle socketHandle)
{
    return static_cast<SOCKET>(socketHandle);
}
#endif

bool wouldBlock()
{
#ifdef _WIN32
    const int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK;
#else
    return errno == EWOULDBLOCK || errno == EAGAIN;
#endif
}

} // namespace

UdpSocket::UdpSocket()
{
    initializeNetwork();
}

UdpSocket::~UdpSocket()
{
    close();
    shutdownNetwork();
}

void UdpSocket::openSender()
{
    close();
    socket_ = static_cast<SocketHandle>(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (socket_ == invalidSocket_) {
        throw std::runtime_error("failed to create UDP sender socket");
    }
}

void UdpSocket::openReceiver(std::uint16_t listenPort)
{
    close();
    socket_ = static_cast<SocketHandle>(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (socket_ == invalidSocket_) {
        throw std::runtime_error("failed to create UDP receiver socket");
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(listenPort);

    if (::bind(
#ifdef _WIN32
            toNative(socket_),
#else
            socket_,
#endif
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)) < 0) {
        throw std::runtime_error("failed to bind UDP receiver socket");
    }

    setNonBlocking();
}

bool UdpSocket::sendTo(const std::byte* data, std::size_t size, const std::string& host, std::uint16_t port)
{
    sockaddr_in destination{};
    destination.sin_family = AF_INET;
    destination.sin_port = htons(port);

    if (::inet_pton(AF_INET, host.c_str(), &destination.sin_addr) != 1) {
        throw std::runtime_error("invalid destination IPv4 address");
    }

    const int sentBytes = ::sendto(
#ifdef _WIN32
        toNative(socket_),
#else
        socket_,
#endif
        reinterpret_cast<const char*>(data),
        static_cast<int>(size),
        0,
        reinterpret_cast<sockaddr*>(&destination),
        sizeof(destination));

    return sentBytes == static_cast<int>(size);
}

int UdpSocket::receive(std::byte* data, std::size_t size)
{
    sockaddr_in sender{};
#ifdef _WIN32
    int senderSize = sizeof(sender);
#else
    socklen_t senderSize = sizeof(sender);
#endif

    const int receivedBytes = ::recvfrom(
#ifdef _WIN32
        toNative(socket_),
#else
        socket_,
#endif
        reinterpret_cast<char*>(data),
        static_cast<int>(size),
        0,
        reinterpret_cast<sockaddr*>(&sender),
        &senderSize);

    if (receivedBytes < 0 && wouldBlock()) {
        return 0;
    }

    return receivedBytes;
}

void UdpSocket::close()
{
    if (socket_ == invalidSocket_) {
        return;
    }

#ifdef _WIN32
    closesocket(toNative(socket_));
#else
    ::close(socket_);
#endif
    socket_ = invalidSocket_;
}

void UdpSocket::initializeNetwork()
{
#ifdef _WIN32
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
}

void UdpSocket::shutdownNetwork()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

void UdpSocket::setNonBlocking()
{
#ifdef _WIN32
    u_long mode = 1;
    if (ioctlsocket(toNative(socket_), FIONBIO, &mode) != 0) {
        throw std::runtime_error("failed to set non-blocking UDP socket");
    }
#else
    const int flags = fcntl(socket_, F_GETFL, 0);
    if (flags < 0 || fcntl(socket_, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::runtime_error("failed to set non-blocking UDP socket");
    }
#endif
}

