#include "Buffer.h"
#include "Consumer.h"
#include "Message.h"
#include "Producer.h"
#include "UdpStreamer.h"

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    constexpr std::size_t queueCapacity = 64;
    constexpr std::uint16_t udpPort = 40117;

    Buffer buffer(queueCapacity);
    Consumer consumer(udpPort, "messages.db", 25);
    UdpStreamer streamer(buffer, "127.0.0.1", udpPort);
    Producer producer(buffer);

    consumer.start();
    streamer.start();
    producer.start();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    producer.stop();

    while (!buffer.empty()) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));      // A kinda workaround not applicalbe in real over-network solutions
    }                                                                   // That's why I'm not covering it with tests

    streamer.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    consumer.stop();

    const auto producerStats = producer.getStats();
    const auto streamerStats = streamer.getStats();
    const auto consumerStats = consumer.getStats();

    std::cout << "Generated: " << producerStats.generatedCount << '\n'
              << "Produced: " << producerStats.producedCount << '\n'
              << "Dropped: " << producerStats.droppedCount << '\n'
              << "Streamed: " << streamerStats.streamedCount << '\n'
              << "Consumed: " << consumerStats.consumedCount << '\n';

    return 0;
}
