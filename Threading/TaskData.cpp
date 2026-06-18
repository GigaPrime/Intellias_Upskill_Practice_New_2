#include "TaskData.h"

#include <algorithm>
#include <iostream>

void TaskData::addTask(const std::chrono::steady_clock::time_point& time, std::unique_ptr<Task> task)
{
	std::lock_guard<std::mutex> lock(mutex_);
	tasks_.emplace(time, std::move(task));
	condition_.notify_all();
}

std::unique_ptr<Task> TaskData::popReadyTask()
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

TaskState TaskData::getTaskState(const taskId id)
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const auto& p) { return p.second->getId() == id; });
	if (it != tasks_.end()) return it->second->getState();
	auto ct = completedTasks_.find(id);
	if (ct != completedTasks_.end()) return ct->second;
	return TaskState::Invalid;
}

bool TaskData::cancelTask(const taskId id)
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

// Not really...
void TaskData::markCompleted(const taskId id, TaskState state)
{
	std::lock_guard<std::mutex> lock(mutex_);
	completedTasks_[id] = state;
}

void TaskData::shutdown()
{
	std::lock_guard<std::mutex> lock(mutex_);
	stopping_ = true;
	condition_.notify_all();
}
