#pragma once

#include "Task.h"

#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <unordered_map>

using taskId = std::size_t;

class TaskData
{
private:
	std::mutex mutex_;
	std::condition_variable condition_;
	std::multimap<std::chrono::steady_clock::time_point, std::unique_ptr<Task>> tasks_;
	std::unordered_map<taskId, TaskState> completedTasks_;
	bool stopping_ = false;

public:
	TaskData() = default;

	void addTask(const std::chrono::steady_clock::time_point& time, std::unique_ptr<Task> task);

	std::unique_ptr<Task> popReadyTask();

	void shutdown();

	// As a trivial getter it should be const but it is thread safe and uses mutex
	// So either I have to keep it non-const or set mutex_ as mutable
	TaskState getTaskState(const taskId id);

	bool cancelTask(const taskId id);

	void markCompleted(const taskId id, TaskState state);
};
