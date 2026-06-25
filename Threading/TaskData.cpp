#include "TaskData.h"

#include <algorithm>
#include <iostream>

template<typename T>
void TaskData<T>::addTask(const std::chrono::steady_clock::time_point& time, std::unique_ptr<Task<T>> task)
{
	std::lock_guard<std::mutex> lock(mutex_);
	tasks_.emplace(time, std::move(task));
	condition_.notify_all();
}

template <typename T>
std::unique_ptr<Task<T>> TaskData<T>::popReadyTask()
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
		if (it->first <= now)
		{
			std::unique_ptr<Task> t = std::move(it->second);
			tasks_.erase(it);
			return t;
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
std::unique_ptr<T> TaskData<T>::getTaskResult(const taskId id) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto ct = completedTasks_.find(id);
	if (ct != completedTasks_.end())
	{
		// I assume if the task result was returned upon request, 
		// there's no need to store the task further
		completedTasks_.erase(id);
		return std::move(ct->second.result_);
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
		completedTasks_.insert({ it->second->getId(), TaskState::Cancelled });
		tasks_.erase(it);
		return true;
	}
	return false;
}

template <typename T>
void TaskData<T>::markCompleted(const taskId id, TaskState state, const std::unique_ptr<T> result)
{
	std::lock_guard<std::mutex> lock(mutex_);
	CompletedTask task;
	task.taskState_ = state;
	task.result_ = std::move(result);

	completedTasks_.insert({ id, task });
}

template <typename T>
void TaskData<T>::shutdown()
{
	std::lock_guard<std::mutex> lock(mutex_);
	stopping_ = true;
	condition_.notify_all();
}
