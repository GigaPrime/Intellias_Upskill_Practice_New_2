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

class Timer 
{
private:
	std::mutex mutex_;
	std::condition_variable condition_;

	std::shared_ptr<TaskData> taskData_;
	std::unique_ptr<ThreadPool> threadPool_;

	// To avoid locking for a primitive type
	std::atomic<taskId> nextId_{1};

	bool running_ = true;
	
	std::chrono::steady_clock::time_point getNextTaskTime(
		const std::chrono::steady_clock::duration delay) const;

	// Same as in TaskData: either const or nexId_ should be mutable
	taskId generateTaskId();

public:
	explicit Timer(const std::size_t threadCount = 4);

	taskId addTask(std::function<void()> action, 
		const std::chrono::steady_clock::duration delay);
	TaskState getTaskState(const taskId id) const;
	bool cancelTask(const taskId id);
	bool shutdown();
};


// return value handling (later on)


// If the Timer should accept std::function<void*>() most likely I won't need any return value handling
// In other way the redesign should be considered: return values (as std::any<T>) + taskId + TasState