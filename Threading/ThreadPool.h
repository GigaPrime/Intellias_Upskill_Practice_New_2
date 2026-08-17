#pragma once

#include <condition_variable>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <memory>

#include "Task.h"
#include "TaskManager.h"

template <typename T>
class ThreadPool
{
private:
	std::vector<std::thread> threads_;
	std::mutex mutex_;
	std::condition_variable condition_;

	std::shared_ptr<TaskManager<T>> taskManager_;

	bool running_ = false;

	void run();

public:
	explicit ThreadPool(std::shared_ptr<TaskManager<T>> taskManager, const std::size_t threadCount = 4);

	~ThreadPool();

	void shutdown();
};
