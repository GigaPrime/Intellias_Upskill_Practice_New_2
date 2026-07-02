#pragma once

#include "Task.h"

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>

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
		std::unique_ptr<T> result_;
	};

	std::unordered_map<taskId, CompletedTask> completedTasks_;
	bool stopping_ = false;

	// Helper: generate random timeout between 10 and 500 ms
	std::chrono::milliseconds getRandomDelay() const;

public:
	TaskData() = default;

	void addTask(const std::chrono::steady_clock::time_point& time, std::unique_ptr<Task<T>> task);

	std::unique_ptr<Task<T>> popTaskReadyForExecution();

	void shutdown();

	TaskState getTaskState(const taskId id) const;
	std::unique_ptr<T> getTaskResult(const taskId id);

	bool cancelTask(const taskId id);

	void markCompleted(const taskId id, TaskState state, std::unique_ptr<T> result = nullptr);
};