#include "TaskData.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <type_traits>

template <typename T>
std::chrono::milliseconds TaskData<T>::getRandomDelay() const
{
	static thread_local std::mt19937 rng{ std::random_device{}() };
	std::uniform_int_distribution<int> dist(10, 500);
	return std::chrono::milliseconds(dist(rng));
}

template<typename T>
void TaskData<T>::addTask(const std::chrono::steady_clock::time_point& time, std::unique_ptr<Task<T>> task)
{
	std::lock_guard<std::mutex> lock(mutex_);
	tasks_.emplace(time, std::move(task));
	condition_.notify_all();
}

template <typename T>
std::unique_ptr<Task<T>> TaskData<T>::popTaskReadyForExecution()
{
	std::unique_lock<std::mutex> lock(mutex_);
	while (true)
	{
		if (stopping_)
		{
			return nullptr;
		}

		if (tasks_.empty())
		{
			condition_.wait(lock, [this]() { return stopping_ || !tasks_.empty(); });
			if (stopping_) return nullptr;
			continue;
		}

		auto it = tasks_.begin();
		auto now = std::chrono::steady_clock::now();

		if (it->first <= now && it->second->getState() == TaskState::Pending)
		{
			std::unique_ptr<Task<T>> task = std::move(it->second);
			tasks_.erase(it);
			return task;
		}

		// Wait until the next task's scheduled time or until shutdown/new task
		condition_.wait_until(lock, it->first);
		if (stopping_) return nullptr;
	}
}

template <typename T>
TaskState TaskData<T>::getTaskState(const taskId id) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const auto& p) { return p.second->getId() == id; });
	if (it != tasks_.end()) return it->second->getState();
	auto ct = completedTasks_.find(id);
	if (ct != completedTasks_.end()) return ct->second.taskState_;
	return TaskState::Invalid;
}

template<typename T>
std::unique_ptr<T> TaskData<T>::getTaskResult(const taskId id)
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto ct = completedTasks_.find(id);
	if (ct != completedTasks_.end())
	{
		// I assume if the task result was returned upon request, 
		// there's no need to store the task further
		auto result = std::move(ct->second.result_);
		completedTasks_.erase(ct);
		return result;
	}
	return nullptr;
}

template <typename T>
bool TaskData<T>::cancelTask(const taskId id)
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const auto& p) { return p.second->getId() == id; });
	if (it != tasks_.end() && it->second->getState() == TaskState::Pending)
	{
		completedTasks_.emplace(
			it->second->getId(),
			CompletedTask{ TaskState::Cancelled, nullptr });
		tasks_.erase(it);
		return true;
	}
	return false;
}

template <typename T>
void TaskData<T>::markCompleted(const taskId id, TaskState state, std::unique_ptr<T> result)
{
	std::lock_guard<std::mutex> lock(mutex_);

	// If the result type is callable, schedule a new task
	if constexpr (std::is_invocable_v<T&>)
	{
		T callable = std::move(*result);
		completedTasks_.emplace(id, CompletedTask{ TaskState::Completed, nullptr }); // I assume the return result of the current task should be nullptr since we are scheduling a new task

		// Schedule the callable as a new task with a random delay from now
		auto now = std::chrono::steady_clock::now();
		auto delay = getRandomDelay() + std::chrono::steady_clock::now();

		auto newTask = std::make_unique<Task<T>>(id, delay, [callable = std::move(callable)]() mutable -> T {
			std::invoke(callable);
			return T{}; });

		tasks_.emplace(delay, std::move(newTask));
		condition_.notify_all();
	}

	// If not - store the result
	completedTasks_.emplace(id, CompletedTask{ state, std::move(result) });
	condition_.notify_all();
}

template <typename T>
void TaskData<T>::shutdown()
{
	std::lock_guard<std::mutex> lock(mutex_);
	stopping_ = true;
	condition_.notify_all();
}

