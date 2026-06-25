#pragma once

#include "Task.h"

#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <unordered_map>
#include <set>

using taskId = std::size_t;

template<typename T>
class TaskData
{
private:
	mutable std::mutex mutex_;
	std::condition_variable condition_;
	std::multimap<std::chrono::steady_clock::time_point, std::unique_ptr<Task<T>>> tasks_;

	struct CompletedTask
	{
		TaskState taskState_;
		const std::unique_ptr<T> result_;
	};

	std::unordered_map<taskId, CompletedTask> completedTasks_;
	bool stopping_ = false;

public:
	TaskData() = default;

	void addTask(const std::chrono::steady_clock::time_point& time, std::unique_ptr<Task<T>> task);

	std::unique_ptr<Task<T>> popReadyTask();

	void shutdown();

	TaskState getTaskState(const taskId id) const;
	const std::unique_ptr<T> getTaskResult(const taskId id) const;

	bool cancelTask(const taskId id);

	void markCompleted(const taskId id, TaskState state, const std::unique_ptr<T> result);
};
