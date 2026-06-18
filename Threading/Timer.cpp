#include "Timer.h"

#include <iostream>
#include <tuple> 

std::chrono::steady_clock::time_point Timer::getNextTaskTime(
    const std::chrono::steady_clock::duration delay) const
{
	return std::chrono::steady_clock::now() + delay;
}

taskId Timer::generateTaskId()
{
    return nextId_.fetch_add(1);
}

Timer::Timer(const std::size_t threadCount) : taskData_(std::make_shared<TaskData>()), threadPool_(std::make_unique<ThreadPool>(taskData_, threadCount)), running_(true) {}

taskId Timer::addTask(std::function<void()> action, const std::chrono::steady_clock::duration delay)
{
    const auto id = generateTaskId();
    const auto time = getNextTaskTime(delay);
    auto newTaskPtr = std::make_unique<Task>(id, time, std::move(action), TaskState::Pending);
    taskData_->addTask(time, std::move(newTaskPtr));
	return id;
}

TaskState Timer::getTaskState(const taskId id) const
{
    return taskData_->getTaskState(id);
}

bool Timer::cancelTask(const taskId id)
{
    return taskData_->cancelTask(id);
}

bool Timer::shutdown()
{
    running_ = false;
    if (threadPool_) threadPool_->shutdown();
    return true;
}
