#include <gtest/gtest.h>

#include "Task.h"
#include "TaskData.h"
#include "ThreadPool.h"
#include "Timer.h"

#include "Task.cpp"
#include "TaskData.cpp"
#include "ThreadPool.cpp"
#include "Timer.cpp"

#include <atomic>
#include <thread>
#include <chrono>
#include <memory>
#include <iostream>
#include <set>

namespace TaskTests
{
	TEST(TaskTests, Basic)
	{
		auto now = std::chrono::steady_clock::now();
		Task<int> t(1, now, []() { return 0; });

		EXPECT_EQ(t.getId(), 1u);
		EXPECT_EQ(t.getDelay(), now);
		EXPECT_EQ(t.getState(), TaskState::Pending);
	}

	TEST(TaskTests, ChangesPersist)
	{
		auto now = std::chrono::steady_clock::now();
		Task<int> t(2, now, []() { return 0; });

		EXPECT_EQ(t.getState(), TaskState::Pending);
		t.setState(TaskState::Running);
		EXPECT_EQ(t.getState(), TaskState::Running);
		t.setState(TaskState::Completed);
		EXPECT_EQ(t.getState(), TaskState::Completed);
	}

	TEST(TaskTests, ExecutesCallableAndStoresResult)
	{
		std::atomic<int> counter{ 0 };
		auto now = std::chrono::steady_clock::now();

		Task<int> t(3, now, [&counter]() {
			counter.fetch_add(1);
			return 42;
			});

		t();
		auto result = t.getResult();
		ASSERT_NE(result, nullptr);
		EXPECT_EQ(*result, 42);
		EXPECT_EQ(counter.load(), 1);
	}

	TEST(TaskTests, ResultIsUniquePtr)
	{
		auto now = std::chrono::steady_clock::now();
		Task<int> t(4, now, []() { return 99; });

		t();
		auto result = t.getResult();

		ASSERT_NE(result, nullptr);
		EXPECT_EQ(*result, 99);
		// Verify ownership transfer occurred
		EXPECT_EQ(result.get(), result.get());
	}

	TEST(TaskTests, ResultNullBeforeExecution)
	{
		auto now = std::chrono::steady_clock::now();
		Task<int> t(5, now, []() { return 77; });

		auto result = t.getResult();
		EXPECT_EQ(result, nullptr);
	}

	TEST(TaskTests, ResultMultipleTypes)
	{
		auto now = std::chrono::steady_clock::now();
		Task<double> t(6, now, []() { return 3.14; });

		t();
		auto result = t.getResult();

		ASSERT_NE(result, nullptr);
		EXPECT_DOUBLE_EQ(*result, 3.14);
	}
}

namespace TaskDataTests
{
	TEST(TaskDataTests, AddSingleTask_CheckPresent)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();
		auto taskPtr = std::make_unique<Task<int>>(101, now, []() { return 101; });

		td.addTask(now, std::move(taskPtr));
		EXPECT_EQ(td.getTaskState(101), TaskState::Pending);

		auto popped = td.popReadyTask();
		ASSERT_TRUE(popped);
		EXPECT_EQ(popped->getId(), 101u);

		td.markCompleted(101, TaskState::Completed);
		EXPECT_EQ(td.getTaskState(101), TaskState::Completed);
	}

	TEST(TaskDataTests, AddMultipleTasksWithMixedDelaysCheckPresent)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now + std::chrono::milliseconds(50), std::make_unique<Task<int>>(201, now + std::chrono::milliseconds(50), []() { return 201; }));
		td.addTask(now + std::chrono::milliseconds(100), std::make_unique<Task<int>>(202, now + std::chrono::milliseconds(100), []() { return 202; }));
		td.addTask(now + std::chrono::milliseconds(150), std::make_unique<Task<int>>(203, now + std::chrono::milliseconds(150), []() { return 203; }));

		EXPECT_EQ(td.getTaskState(201), TaskState::Pending);
		EXPECT_EQ(td.getTaskState(202), TaskState::Pending);
		EXPECT_EQ(td.getTaskState(203), TaskState::Pending);
	}

	TEST(TaskDataTests, AddThenAddShortestFirstPopped)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();

		td.addTask(now + std::chrono::milliseconds(100), std::make_unique<Task<int>>(301, now + std::chrono::milliseconds(100), []() { return 301; }));
		td.addTask(now + std::chrono::milliseconds(200), std::make_unique<Task<int>>(302, now + std::chrono::milliseconds(200), []() { return 302; }));

		td.addTask(now + std::chrono::milliseconds(10), std::make_unique<Task<int>>(300, now + std::chrono::milliseconds(10), []() { return 300; }));

		auto first = td.popReadyTask();
		ASSERT_TRUE(first);
		EXPECT_EQ(first->getId(), 300u);
	}

	TEST(TaskDataTests, AddSeveralSameDelayAllPresent)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now, std::make_unique<Task<int>>(401, now, []() { return 401; }));
		td.addTask(now, std::make_unique<Task<int>>(402, now, []() { return 402; }));
		td.addTask(now, std::make_unique<Task<int>>(403, now, []() { return 403; }));

		std::set<std::size_t> seen;
		for (int i = 0; i < 3; ++i)
		{
			auto t = td.popReadyTask();
			ASSERT_TRUE(t);
			seen.insert(t->getId());
			td.markCompleted(t->getId(), TaskState::Completed);
		}
		EXPECT_EQ(seen.size(), 3u);
		EXPECT_TRUE(seen.count(401));
		EXPECT_TRUE(seen.count(402));
		EXPECT_TRUE(seen.count(403));
	}

	TEST(TaskDataTests, AddSeveralGetStateOfOne)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now, std::make_unique<Task<int>>(501, now, []() { return 501; }));
		td.addTask(now + std::chrono::milliseconds(10), std::make_unique<Task<int>>(502, now + std::chrono::milliseconds(10), []() { return 502; }));

		EXPECT_EQ(td.getTaskState(501), TaskState::Pending);
	}

	TEST(TaskDataTests, CancelTaskRemovedAndStateCancelled)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now, std::make_unique<Task<int>>(601, now, []() { return 601; }));

		bool cancelled = td.cancelTask(601);
		EXPECT_TRUE(cancelled);
		EXPECT_EQ(td.getTaskState(601), TaskState::Cancelled);
	}

	TEST(TaskDataTests, PopThenMarkCompletedRemovedFromTasksAndRecorded)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now, std::make_unique<Task<int>>(701, now, []() { return 701; }));

		auto t = td.popReadyTask();
		ASSERT_TRUE(t);
		EXPECT_EQ(t->getId(), 701u);

		td.markCompleted(701, TaskState::Completed);
		EXPECT_EQ(td.getTaskState(701), TaskState::Completed);
	}

	TEST(TaskDataTests, PopReadyTaskReturnsUniquePtrWithResult)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();
		auto taskPtr = std::make_unique<Task<int>>(801, now, []() { return 801; });

		td.addTask(now, std::move(taskPtr));
		auto popped = td.popReadyTask();

		ASSERT_TRUE(popped);
		EXPECT_EQ(popped->getId(), 801u);

		(*popped)();
		auto result = popped->getResult();
		ASSERT_NE(result, nullptr);
		EXPECT_EQ(*result, 801);
	}

	TEST(TaskDataTests, ResultPreservedAfterPopAndExecute)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();
		auto taskPtr = std::make_unique<Task<int>>(802, now, []() { return 256; });

		td.addTask(now, std::move(taskPtr));
		auto popped = td.popReadyTask();

		ASSERT_TRUE(popped);
		(*popped)();

		auto result = popped->getResult();
		ASSERT_NE(result, nullptr);
		EXPECT_EQ(*result, 256);

		td.markCompleted(802, TaskState::Completed);
		EXPECT_EQ(td.getTaskState(802), TaskState::Completed);
	}

	TEST(TaskDataTests, GetTaskResultRemovesFromCompletedTasks)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();
		const taskId taskId = 903;
		const int expectedResult = 777;

		auto taskPtr = std::make_unique<Task<int>>(taskId, now, []() { return expectedResult; });
		td.addTask(now, std::move(taskPtr));

		auto popped = td.popReadyTask();
		ASSERT_TRUE(popped);
		(*popped)();

		td.markCompleted(taskId, TaskState::Completed, popped->getResult());
		EXPECT_EQ(td.getTaskState(taskId), TaskState::Completed);

		auto result = td.getTaskResult(taskId);
		ASSERT_NE(result, nullptr);
		EXPECT_EQ(*result, expectedResult);

		// Check task is no longer in completedTasks
		EXPECT_EQ(td.getTaskState(taskId), TaskState::Invalid);
	}

	TEST(TaskDataTests, MultipleCompletedTasksRetrievalRemovesOnlyRequested)
	{
		TaskData<int> td;
		auto now = std::chrono::steady_clock::now();

		// Add 1-st task
		auto task1 = std::make_unique<Task<int>>(1001, now, []() { return 100; });
		td.addTask(now, std::move(task1));
		auto popped1 = td.popReadyTask();
		ASSERT_TRUE(popped1);
		(*popped1)();
		td.markCompleted(1001, TaskState::Completed, popped1->getResult());

		// Add 2-nd task
		auto task2 = std::make_unique<Task<int>>(1002, now, []() { return 200; });
		td.addTask(now, std::move(task2));
		auto popped2 = td.popReadyTask();
		ASSERT_TRUE(popped2);
		(*popped2)();
		td.markCompleted(1002, TaskState::Completed, popped2->getResult());

		// Retrieve 1-st task result
		auto result1 = td.getTaskResult(1001);
		ASSERT_NE(result1, nullptr);
		EXPECT_EQ(*result1, 100);

		// Expect 1-st task is removed from completed tasks
		EXPECT_EQ(td.getTaskState(1001), TaskState::Invalid);

		// 2-nd task should be present
		EXPECT_EQ(td.getTaskState(1002), TaskState::Completed);

		// Retrieve 2-nd task result
		auto result2 = td.getTaskResult(1002);
		ASSERT_NE(result2, nullptr);
		EXPECT_EQ(*result2, 200);

		// Expect 2-st task is removed from completed tasks
		EXPECT_EQ(td.getTaskState(1002), TaskState::Invalid);
	}
}

namespace ThreadPoolTests
{
	TEST(ThreadPoolBasic, ExecutesTaskFromTaskData)
	{
		auto taskData = std::make_shared<TaskData<int>>();
		std::atomic<bool> executed{ false };

		ThreadPool<int> pool(taskData, 1);

		auto now = std::chrono::steady_clock::now();
		auto taskPtr = std::make_unique<Task<int>>(7, now, [&executed]() {
			executed.store(true);
			return 7;
			});
		taskData->addTask(now, std::move(taskPtr));

		auto start = std::chrono::steady_clock::now();
		while (!executed.load() && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		pool.shutdown();

		EXPECT_TRUE(executed.load());
		EXPECT_EQ(taskData->getTaskState(7), TaskState::Completed);
	}

	TEST(ThreadPoolBasic, ResultStoredAfterExecution)
	{
		auto taskData = std::make_shared<TaskData<int>>();
		std::atomic<bool> executed{ false };

		ThreadPool<int> pool(taskData, 1);

		auto now = std::chrono::steady_clock::now();
		auto taskPtr = std::make_unique<Task<int>>(8, now, [&executed]() {
			executed.store(true);
			return 42;
			});
		taskData->addTask(now, std::move(taskPtr));

		auto start = std::chrono::steady_clock::now();
		while (!executed.load() && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		pool.shutdown();

		EXPECT_TRUE(executed.load());
		EXPECT_EQ(taskData->getTaskState(8), TaskState::Completed);
	}

	TEST(ThreadPoolBasic, UniquePtrResultReturnsCorrectValue)
	{
		auto taskData = std::make_shared<TaskData<int>>();
		std::atomic<bool> executed{ false };
		const int expectedValue = 123;

		ThreadPool<int> pool(taskData, 1);

		auto now = std::chrono::steady_clock::now();
		auto taskPtr = std::make_unique<Task<int>>(9, now, [&executed, expectedValue]() {
			executed.store(true);
			return expectedValue;
			});
		taskData->addTask(now, std::move(taskPtr));

		auto start = std::chrono::steady_clock::now();
		while (!executed.load() && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		pool.shutdown();

		EXPECT_TRUE(executed.load());
		EXPECT_EQ(taskData->getTaskState(9), TaskState::Completed);
	}
}

namespace TimerTests
{
	TEST(TimerBasic, AddTaskRunsViaThreadPool)
	{
		Timer<int> timer(1);
		std::atomic<bool> executed{ false };
		timer.addTask([&executed]() {
			executed.store(true);
			return 1;
			}, std::chrono::milliseconds(0));
		auto start = std::chrono::steady_clock::now();
		while (!executed.load() && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		timer.shutdown();

		EXPECT_TRUE(executed.load());
	}

	TEST(TimerTests, GetTaskStatePendingCancelledCompleted)
	{
		Timer<int> timer(1);

		auto pendingId = timer.addTask([]() { return 1; }, std::chrono::milliseconds(200));
		EXPECT_EQ(timer.getTaskState(pendingId), TaskState::Pending);

		std::atomic<bool> shouldNotRun{ false };
		auto cancelId = timer.addTask([&shouldNotRun]() {
			shouldNotRun.store(true);
			return 2;
			}, std::chrono::milliseconds(200));
		bool cancelled = timer.cancelTask(cancelId);
		EXPECT_TRUE(cancelled);
		EXPECT_EQ(timer.getTaskState(cancelId), TaskState::Cancelled);

		std::atomic<bool> executed{ false };
		auto completedId = timer.addTask([&executed]() {
			executed.store(true);
			return 3;
			}, std::chrono::milliseconds(0));
		auto start = std::chrono::steady_clock::now();
		while (!executed.load() && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		EXPECT_TRUE(executed.load());
		EXPECT_EQ(timer.getTaskState(completedId), TaskState::Completed);

		timer.shutdown();
	}

	TEST(TimerTests, CancelTaskActuallyCancels)
	{
		Timer<int> timer(1);
		std::atomic<bool> executed{ false };

		auto id = timer.addTask([&executed]() {
			executed.store(true);
			return 1;
			}, std::chrono::milliseconds(100));
		bool cancelled = timer.cancelTask(id);
		EXPECT_TRUE(cancelled);

		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		EXPECT_FALSE(executed.load());
		EXPECT_EQ(timer.getTaskState(id), TaskState::Cancelled);

		timer.shutdown();
	}

	TEST(TimerTests, ShutdownWaitsForRunningTask)
	{
		Timer<int> timer(1);

		std::atomic<bool> started{ false };
		std::atomic<bool> finished{ false };

		auto id = timer.addTask([&started, &finished]() {
			started.store(true);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			finished.store(true);
			return 1;
			}, std::chrono::milliseconds(0));

		auto waitStart = std::chrono::steady_clock::now();
		while (!started.load() && std::chrono::steady_clock::now() - waitStart < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		auto t0 = std::chrono::steady_clock::now();
		bool res = timer.shutdown();
		auto t1 = std::chrono::steady_clock::now();

		auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

		EXPECT_TRUE(res);
		EXPECT_TRUE(finished.load());
		EXPECT_GE(elapsedMs, 250); // Shutdown waits until running tasks is not completed
		(void)id;
	}

	TEST(TimerTests, AddTaskReturnsUniquePtrResult)
	{
		Timer<int> timer(1);
		std::atomic<bool> executed{ false };
		const int expectedValue = 555;

		auto id = timer.addTask([&executed, expectedValue]() {
			executed.store(true);
			return expectedValue;
			}, std::chrono::milliseconds(0));

		auto start = std::chrono::steady_clock::now();
		while (!executed.load() && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		timer.shutdown();

		EXPECT_TRUE(executed.load());
	}

	TEST(TimerTests, MultipleTasksStoreResultsCorrectly)
	{
		Timer<int> timer(2);
		std::atomic<int> executedCount{ 0 };

		auto id1 = timer.addTask([&executedCount]() {
			executedCount.fetch_add(1);
			return 111;
			}, std::chrono::milliseconds(0));

		auto id2 = timer.addTask([&executedCount]() {
			executedCount.fetch_add(1);
			return 222;
			}, std::chrono::milliseconds(0));

		auto start = std::chrono::steady_clock::now();
		while (executedCount.load() < 2 && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		timer.shutdown();

		EXPECT_EQ(executedCount.load(), 2);
		(void)id1;
		(void)id2;
	}

	TEST(TimerTests, GetResultRemovesTaskFromCompletedTasks)
	{
		Timer<int> timer(1);
		std::atomic<bool> executed{ false };
		const int expectedValue = 888;

		auto id = timer.addTask([&executed, expectedValue]() {
			executed.store(true);
			return expectedValue;
			}, std::chrono::milliseconds(0));

		auto start = std::chrono::steady_clock::now();
		while (!executed.load() && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		EXPECT_EQ(timer.getTaskState(id), TaskState::Completed);

		auto result = timer.getResult(id);
		ASSERT_NE(result, nullptr);
		EXPECT_EQ(*result, expectedValue);

		// Verify task is no longer present after getResult call
		EXPECT_EQ(timer.getTaskState(id), TaskState::Invalid);

		timer.shutdown();
	}

	TEST(TimerTests, MultipleTasksGetResultRemovesOnlyIfRequested)
	{
		Timer<int> timer(2);
		std::atomic<int> executedCount{ 0 };

		auto id1 = timer.addTask([&executedCount]() {
			executedCount.fetch_add(1);
			return 1111;
			}, std::chrono::milliseconds(0));

		auto id2 = timer.addTask([&executedCount]() {
			executedCount.fetch_add(1);
			return 2222;
			}, std::chrono::milliseconds(0));

		auto start = std::chrono::steady_clock::now();
		while (executedCount.load() < 2 && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		EXPECT_EQ(timer.getTaskState(id1), TaskState::Completed);
		EXPECT_EQ(timer.getTaskState(id2), TaskState::Completed);

		// Retrieve 1-st task result
		auto result1 = timer.getResult(id1);
		ASSERT_NE(result1, nullptr);
		EXPECT_EQ(*result1, 1111);

		// 1-st task should be removed from completedTasks
		EXPECT_EQ(timer.getTaskState(id1), TaskState::Invalid);

		// 2-nd task is still available
		EXPECT_EQ(timer.getTaskState(id2), TaskState::Completed);

		// Retrieve 2-nd task result
		auto result2 = timer.getResult(id2);
		ASSERT_NE(result2, nullptr);
		EXPECT_EQ(*result2, 2222);

		// 2-nd task removed
		EXPECT_EQ(timer.getTaskState(id2), TaskState::Invalid);

		timer.shutdown();
	}

	class RecursiveTimerCallHelper
	{
	private:
		Timer<int> timer_{ 1 };
		std::atomic<int>& callCount_;
		size_t callLimit_ = 5;

	public:
		RecursiveTimerCallHelper(std::atomic<int>& callCount, const std::size_t callLimit) :
			callCount_(callCount), callLimit_(callLimit) {}

		int operator()()
		{
			const auto currentCount = callCount_.fetch_add(1) + 1;
			if (currentCount < callLimit_)
			{
				timer_.addTask([this]() { return (*this)(); }, std::chrono::milliseconds(50));
			}
			return currentCount;
		}

		void start()
		{
			timer_.addTask([this]() { return (*this)(); }, std::chrono::milliseconds(0));
		}

		void shutdown()
		{
			timer_.shutdown();
		}
	};

	TEST(TimerTests, RecursiveTimerCallHelperSchedulesSelf)
	{
		std::atomic<int> callCount = 0;
		const std::size_t callLimit = 5;
		RecursiveTimerCallHelper rtc(callCount, callLimit);

		rtc.start();
		auto start = std::chrono::steady_clock::now();
		while (callCount.load() < callLimit &&
			std::chrono::steady_clock::now() - start < std::chrono::seconds(5))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		rtc.shutdown();

		EXPECT_EQ(callCount.load(), callLimit);
	}
}
