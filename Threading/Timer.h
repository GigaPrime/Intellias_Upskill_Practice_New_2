#pragma once

#include "Task.h"
#include "ThreadPool.h"
#include "TaskData.h"

#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <atomic>

using taskId = std::size_t;

template<typename T>
class Timer 
{
private:
	std::mutex mutex_;
	std::condition_variable condition_;

	std::shared_ptr<TaskData<T>> taskData_;
	std::unique_ptr<ThreadPool<T>> threadPool_;

	mutable std::atomic<taskId> nextId_{1};

	bool running_ = true;
	
	std::chrono::steady_clock::time_point getNextTaskTime(
		const std::chrono::steady_clock::duration delay) const;

	taskId generateTaskId() const;

public:
	explicit Timer(const std::size_t threadCount = 4);

	taskId addTask(std::function<void()> action, 
		const std::chrono::steady_clock::duration delay);
	TaskState getTaskState(const taskId id) const;
	std::unique_ptr<T> getResult(const taskId id) const;
	bool cancelTask(const taskId id);
	bool shutdown();
};


// Object State that stores task callbacks and current task state
// TaskData would not need two containers anymore but would operate a single container with States incapsulating tasks and taskState
// This would allow decoupling the return type of the callable from the callable flow management