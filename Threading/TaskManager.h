#pragma once
#include "TaskData.h"

template <typename T>
class TaskManager 
{
private:
	mutable std::mutex mutex_;
	std::condition_variable condition_;

	std::unique_ptr<TaskData<T>> taskManager_;

	bool stopping_ = false;

	TaskManager();

	std::chrono::milliseconds getRandomDelay() const;

public:
	TaskManager(const TaskManager&) = delete;
	TaskManager& operator=(const TaskManager&) = delete;

	static TaskManager<T>& getInstance()
	{
		static TaskManager<T> instance;
		return instance;
	}

	void addTask(const std::chrono::steady_clock::time_point& time, std::unique_ptr<Task<T>> task);
	std::unique_ptr<Task<T>> popTaskReadyForExecution();
	void markCompleted(const taskId id, TaskState state, std::unique_ptr<T> result = nullptr);
	TaskState getTaskState(const taskId id) const;
	std::unique_ptr<T> getTaskResult(const taskId id);
	bool cancelTask(const taskId id);

	void shutdown();
};