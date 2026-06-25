#include "Task.h"

template<typename T>
void Task<T>::operator()()
{
	result_ = std::make_unique<T>(action_());
}

template<typename T>
std::size_t Task<T>::getId() const
{
	return id_;
}

template<typename T>
std::chrono::steady_clock::time_point Task<T>::getDelay() const
{
	return delay_;
}

template<typename T>
void Task<T>::setState(const TaskState newState)
{
	state_ = newState;
}

template<typename T>
TaskState Task<T>::getState() const
{
	return state_;
}

template<typename T>
std::unique_ptr<T> Task<T>::getResult()
{
	return std::move(result_);
}
