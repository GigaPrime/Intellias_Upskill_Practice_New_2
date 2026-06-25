#include "ThreadPool.h"

#include <iostream>
#include <memory>

template <typename T>
void ThreadPool<T>::run()
{
	while (true)
	{
		if (!taskData_) return;

		std::unique_ptr<Task<T>> task = taskData_->popReadyTask();
		if (!task) return;

		try
		{
			(*task)();
			taskData_->markCompleted(task->getId(), TaskState::Completed, task->getResult());
		}
		catch (...)
		{
			taskData_->markCompleted(task->getId(), TaskState::Failed);
			std::cerr << "Exception caught in thread pool task" << std::endl;
		}
	}
}

template <typename T>
ThreadPool<T>::ThreadPool(std::shared_ptr<TaskData<T>> taskData, const std::size_t threadCount) : taskData_(std::move(taskData)), running_(true)
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

	if (taskData_)
	{
		taskData_->shutdown();
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
