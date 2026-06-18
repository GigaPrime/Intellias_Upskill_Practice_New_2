#include "Task.h"

std::size_t Task::getId() const
{
	return id_;
}

std::chrono::steady_clock::time_point Task::getDelay() const
{
	return delay_;
}

void Task::setState(const TaskState newState)
{
	state_ = newState;
}

TaskState Task::getState() const
{
	return state_;
}

std::function<void()> Task::getAction() const
{
	return std::move(action_);
}
