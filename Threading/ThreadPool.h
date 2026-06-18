#pragma once

#include <condition_variable>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <memory>

#include "Task.h"
#include "TaskData.h"

class ThreadPool
{
private:
	std::vector<std::thread> threads_;
	std::mutex mutex_;
	std::condition_variable condition_;

	std::shared_ptr<TaskData> taskData_;

	bool running_ = false;

	void run();

public:
	explicit ThreadPool(std::shared_ptr<TaskData> taskData, const std::size_t threadCount = 4);

	~ThreadPool();

	void shutdown();
};
