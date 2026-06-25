#include "Timer.h"

#include <iostream>
#include <tuple> 

template<typename T>
std::chrono::steady_clock::time_point Timer<T>::getNextTaskTime(
    const std::chrono::steady_clock::duration delay) const
{
	return std::chrono::steady_clock::now() + delay;
}

template<typename T>
taskId Timer<T>::generateTaskId() const
{
    return nextId_.fetch_add(1);
}

template<typename T>
Timer<T>::Timer(const std::size_t threadCount) 
    : taskData_(std::make_shared<TaskData>())
    , threadPool_(std::make_unique<ThreadPool<T>>(taskData_, threadCount))
    , running_(true) 
{}

template<typename T>
taskId Timer<T>::addTask(std::function<void()> action, const std::chrono::steady_clock::duration delay)
{
    const auto id = generateTaskId();
    const auto time = getNextTaskTime(delay);
    auto newTaskPtr = std::make_unique<Task>(id, time, std::move(action), TaskState::Pending);
    taskData_->addTask(time, std::move(newTaskPtr));
	return id;
}

template<typename T>
TaskState Timer<T>::getTaskState(const taskId id) const
{
    return taskData_->getTaskState(id);
}

template<typename T>
T* Timer<T>::getResult(const taskId id) const
{
    return taskData_->getTaskResult().get();
}

template<typename T>
bool Timer<T>::cancelTask(const taskId id)
{
    return taskData_->cancelTask(id);
}

template<typename T>
bool Timer<T>::shutdown()
{
    running_ = false;
    if (threadPool_) threadPool_->shutdown();
    return true;
}
