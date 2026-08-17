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
struct CompletedTask
{
	TaskState taskState_;
	std::unique_ptr<T> result_;
};

// TaskData class is not exposed anywhere except TaskManager. it is not necessary to make it thread-safe
// std::mutex and std::condition_variable removed
template<typename T>
class TaskData
{
private:
	std::multimap<std::chrono::steady_clock::time_point, std::unique_ptr<Task<T>>> tasks_;
	std::unordered_map<taskId, CompletedTask<T>> completedTasks_;

public:
	TaskData() = default;

	std::multimap<std::chrono::steady_clock::time_point, std::unique_ptr<Task<T>>>& getTasks() const;
	std::unordered_map<taskId, CompletedTask<T>>& getCompletedTasks() const;
};