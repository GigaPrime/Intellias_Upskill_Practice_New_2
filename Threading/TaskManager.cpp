#include "TaskManager.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <type_traits>

template<typename T>
TaskManager<T>::TaskManager() : taskManager_(std::make_unique<TaskData<T>>()) {}

template <typename T>
std::chrono::milliseconds TaskManager<T>::getRandomDelay() const
{
	static thread_local std::mt19937 rng = std::mt19937{std::random_device{}()};
	std::uniform_int_distribution<int> dist(10, 500);
	return std::chrono::milliseconds(dist(rng));
}

template<typename T>
void TaskManager<T>::addTask(const std::chrono::steady_clock::time_point& time, std::unique_ptr<Task<T>> task)
{
	std::lock_guard<std::mutex> lock(mutex_);
	taskManager_->addTask(time, std::move(task));
	condition_.notify_all();
}

template <typename T>
std::unique_ptr<Task<T>> TaskManager<T>::popTaskReadyForExecution()
{
	std::unique_lock<std::mutex> lock(mutex_);
	while (true)
	{
		if (stopping_)
		{
			return nullptr;
		}

		if (taskManager_->getTasks().empty())
		{
			condition_.wait(lock, [this]() { return stopping_ || !taskManager_->getTasks().empty(); });
			if (stopping_) return nullptr;
			continue;
		}

		auto it = taskManager_->getTasks().begin();
		auto now = std::chrono::steady_clock::now();

		if (it->first <= now && it->second->getState() == TaskState::Pending)
		{
			std::unique_ptr<Task<T>> task = std::move(it->second);
			taskManager_->getTasks().erase(it);
			return task;
		}

		// Wait until the next task's scheduled time or until shutdown/new task
		condition_.wait_until(lock, it->first);
		if (stopping_) return nullptr;
	}
}

template<typename T>
void TaskManager<T>::markCompleted(const taskId id, TaskState state, std::unique_ptr<T> result)
{
	std::lock_guard<std::mutex> lock(mutex_);

	// If the result type is callable, schedule a new task
	if constexpr (std::is_invocable_v<T&>)
	{
		T callable = std::move(*result);
		taskManager_->getCompletedTasks().emplace(id, CompletedTask<T>{ TaskState::Completed, nullptr }); // I assume the return result of the current task should be nullptr since we are scheduling a new task

		// Schedule the callable as a new task with a random delay from now
		auto now = std::chrono::steady_clock::now();
		auto delay = getRandomDelay() + std::chrono::steady_clock::now();

		auto newTask = std::make_unique<Task<T>>(id, delay, [callable = std::move(callable)]() {
			std::invoke(callable);
			return T{}; });

		taskManager_->getTasks().emplace(delay, std::move(newTask));
		condition_.notify_all();
		return;
	}

	// If not - store the result
	taskManager_->getCompletedTasks().emplace(id, CompletedTask<T>{ state, std::move(result) });
	condition_.notify_all();
}

template<typename T>
TaskState TaskManager<T>::getTaskState(const taskId id) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto pendingTask = std::find_if(taskManager_->getTasks().begin(), taskManager_->getTasks().end(), 
		[id](const auto& p) { return p.second->getId() == id; });
	if (pendingTask != taskManager_->getTasks().end()) return pendingTask->second->getState();

	auto completedTask = taskManager_->getCompletedTasks().find(id);
	if (completedTask != taskManager_->getCompletedTasks().end()) return completedTask->second.taskState_;

	return TaskState::Invalid;
}

template<typename T>
std::unique_ptr<T> TaskManager<T>::getTaskResult(const taskId id)
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto completedTask = taskManager_->getCompletedTasks().find(id);
	if (completedTask != taskManager_->getCompletedTasks().end())
	{
		// I assume if the task result was returned upon request, 
		// there's no need to store the task further
		auto result = std::move(completedTask->second.result_);
		taskManager_->getCompletedTasks().erase(completedTask);
		return result;
	}
	return nullptr;
}

template<typename T>
bool TaskManager<T>::cancelTask(const taskId id)
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = std::find_if(taskManager_->getTasks().begin(), taskManager_->getTasks().end(), [id](const auto& p) { return p.second->getId() == id; });
	if (it != taskManager_->getTasks().end() && it->second->getState() == TaskState::Pending)
	{
		taskManager_->getCompletedTasks().emplace(
			it->second->getId(),
			CompletedTask<T>{ TaskState::Cancelled, nullptr });
		taskManager_->getTasks().erase(it);
		return true;
	}
	return false;
}

template<typename T>
void TaskManager<T>::shutdown()
{
	std::lock_guard<std::mutex> lock(mutex_);
	stopping_ = true;
	condition_.notify_all();
}
