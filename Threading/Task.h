#pragma once
#include <cstddef>
#include <functional>
#include <chrono>

// I assume the task should be absolutely unique, 
// no copy constructors or assignment operators are allowed
// I also assume the delay is a second-size value

enum class TaskState
{
	Invalid, // No task with this ID exists
	Pending,
	Running,
	Completed,
	Cancelled,
	Failed
};

class Task 
{
private:
	std::size_t id_;
	std::chrono::steady_clock::time_point delay_;
	std::function<void()> action_;
	TaskState state_;

public: 
	Task(const std::size_t id, const std::chrono::steady_clock::time_point& delay, std::function<void()> action, TaskState state = TaskState::Pending)
		: id_(id), delay_(delay), action_(std::move(action)), state_(state) {}

	Task(const Task&) = delete;
	Task& operator=(const Task&) = delete;

	std::size_t getId() const;
	std::chrono::steady_clock::time_point getDelay() const;
	void setState(const TaskState newState);
	TaskState getState() const;
	std::function<void()> getAction() const;
};
