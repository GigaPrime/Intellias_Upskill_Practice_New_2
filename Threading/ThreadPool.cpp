#include "ThreadPool.h"

#include <iostream>
#include <memory>

template <typename T>
void ThreadPool<T>::run()
{
	while (true)
	{
		if (!taskManager_) return;

		std::unique_ptr<Task<T>> task = taskManager_->popTaskReadyForExecution();
		if (!task) return;

		try
		{
			(*task)();
			taskManager_->markCompleted(task->getId(), TaskState::Completed, task->getResult());
		}
		catch (...)
		{
			taskManager_->markCompleted(task->getId(), TaskState::Failed);
			std::cerr << "Exception caught in thread pool task" << std::endl;
		}
	}
}

template <typename T>
ThreadPool<T>::ThreadPool(std::shared_ptr<TaskManager<T>> taskData, const std::size_t threadCount) : taskManager_(std::move(taskData)), running_(true)
{
	threads_.reserve(threadCount);

	for (std::size_t i = 0; i < threadCount; ++i)
	{
		threads_.push_back(std::thread([this]() { run(); }));
	}
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
	shutdown();
}

template <typename T>
void ThreadPool<T>::shutdown()
{
	{
		std::lock_guard<std::mutex> lock(mutex_);
		running_ = false;
	}

	if (taskManager_)
	{
		taskManager_->shutdown();
	}

	// This is necessary for preventing threads to be terminated while the callables are still running there
	for (auto& t : threads_)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
}
