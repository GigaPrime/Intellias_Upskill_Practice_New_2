#include "TaskData.h"

template<typename T>
std::multimap<std::chrono::steady_clock::time_point, std::unique_ptr<Task<T>>>& TaskData<T>::getTasks() const
{
	return tasks_;
}

template<typename T>
std::unordered_map<taskId, CompletedTask<T>>& TaskData<T>::getCompletedTasks() const
{
	return completedTasks_;
}
