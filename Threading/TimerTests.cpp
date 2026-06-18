#include <gtest/gtest.h>

#include "Task.h"
#include "TaskData.h"
#include "ThreadPool.h"
#include "Timer.h"

#include <atomic>
#include <thread>
#include <chrono>
#include <memory>

namespace TaskTests
{
	TEST(TaskTests, Basic)
	{
		auto now = std::chrono::steady_clock::now();
		Task t(1, now, std::function<void()>([]() {}));

		EXPECT_EQ(t.getId(), 1u);
		EXPECT_EQ(t.getDelay(), now);
		EXPECT_EQ(t.getState(), TaskState::Pending);
	}

	TEST(TaskTests, ChangesPersist)
	{
		auto now = std::chrono::steady_clock::now();
		Task t(2, now, std::function<void()>([]() {}));

		EXPECT_EQ(t.getState(), TaskState::Pending);
		t.setState(TaskState::Running);
		EXPECT_EQ(t.getState(), TaskState::Running);
		t.setState(TaskState::Completed);
		EXPECT_EQ(t.getState(), TaskState::Completed);
	}

	TEST(TaskTests, GetActionInvokesCallable)
	{
		std::atomic<int> counter{0};
		auto action = [&counter]() { counter.fetch_add(1); };
		auto now = std::chrono::steady_clock::now();

		Task t(3, now, std::function<void()>(action));

		auto fn = t.getAction();
		ASSERT_TRUE(static_cast<bool>(fn));
		fn();
		EXPECT_EQ(counter.load(), 1);
	}
}

namespace TaskDataTests
{
	TEST(TaskDataTests, AddSingleTask_CheckPresent)
	{
		TaskData td;
		auto now = std::chrono::steady_clock::now();
		auto taskPtr = std::make_unique<Task>(101, now, std::function<void()>([]() {}));

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
		TaskData td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now + std::chrono::milliseconds(50), std::make_unique<Task>(201, now + std::chrono::milliseconds(50), std::function<void()>([]() {})));
		td.addTask(now + std::chrono::milliseconds(100), std::make_unique<Task>(202, now + std::chrono::milliseconds(100), std::function<void()>([]() {})));
		td.addTask(now + std::chrono::milliseconds(150), std::make_unique<Task>(203, now + std::chrono::milliseconds(150), std::function<void()>([]() {})));

		EXPECT_EQ(td.getTaskState(201), TaskState::Pending);
		EXPECT_EQ(td.getTaskState(202), TaskState::Pending);
		EXPECT_EQ(td.getTaskState(203), TaskState::Pending);
	}

	TEST(TaskDataTests, AddThenAddShortestFirstPopped)
	{
		TaskData td;
		auto now = std::chrono::steady_clock::now();

		td.addTask(now + std::chrono::milliseconds(100), std::make_unique<Task>(301, now + std::chrono::milliseconds(100), std::function<void()>([]() {})));
		td.addTask(now + std::chrono::milliseconds(200), std::make_unique<Task>(302, now + std::chrono::milliseconds(200), std::function<void()>([]() {})));

		td.addTask(now + std::chrono::milliseconds(10), std::make_unique<Task>(300, now + std::chrono::milliseconds(10), std::function<void()>([]() {})));

		auto first = td.popReadyTask();
		ASSERT_TRUE(first);
		EXPECT_EQ(first->getId(), 300u);
	}

	TEST(TaskDataTests, AddSeveralSameDelayAllPresent)
	{
		TaskData td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now, std::make_unique<Task>(401, now, std::function<void()>([]() {})));
		td.addTask(now, std::make_unique<Task>(402, now, std::function<void()>([]() {})));
		td.addTask(now, std::make_unique<Task>(403, now, std::function<void()>([]() {})));

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
		TaskData td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now, std::make_unique<Task>(501, now, std::function<void()>([]() {})));
		td.addTask(now + std::chrono::milliseconds(10), std::make_unique<Task>(502, now + std::chrono::milliseconds(10), std::function<void()>([]() {})));

		EXPECT_EQ(td.getTaskState(501), TaskState::Pending);
	}

	TEST(TaskDataTests, CancelTaskRemovedAndStateCancelled)
	{
		TaskData td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now, std::make_unique<Task>(601, now, std::function<void()>([]() {})));

		bool cancelled = td.cancelTask(601);
		EXPECT_TRUE(cancelled);
		EXPECT_EQ(td.getTaskState(601), TaskState::Cancelled);
	}

	TEST(TaskDataTests, PopThenMarkCompletedRemovedFromTasksAndRecorded)
	{
		TaskData td;
		auto now = std::chrono::steady_clock::now();
		td.addTask(now, std::make_unique<Task>(701, now, std::function<void()>([]() {})));

		auto t = td.popReadyTask();
		ASSERT_TRUE(t);
		EXPECT_EQ(t->getId(), 701u);

		td.markCompleted(701, TaskState::Completed);
		EXPECT_EQ(td.getTaskState(701), TaskState::Completed);
	}
}

namespace ThreadPoolTests
{
	TEST(ThreadPoolBasic, ExecutesTaskFromTaskData)
	{
		auto taskData = std::make_shared<TaskData>();
		std::atomic<bool> executed{false};

		ThreadPool pool(taskData, 1);

		auto now = std::chrono::steady_clock::now();
		auto taskPtr = std::make_unique<Task>(7, now, std::function<void()>([&executed]() { executed.store(true); }));
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
}

namespace TimerTests
{
	TEST(TimerBasic, AddTaskRunsViaThreadPool)
	{
		Timer timer(1);
		std::atomic<bool> executed{false};
		timer.addTask([&executed]() { executed.store(true); }, std::chrono::milliseconds(0));
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
		Timer timer(1);

		auto pendingId = timer.addTask([]() {}, std::chrono::milliseconds(200));
		EXPECT_EQ(timer.getTaskState(pendingId), TaskState::Pending);

		std::atomic<bool> shouldNotRun{false};
		auto cancelId = timer.addTask([&shouldNotRun]() { shouldNotRun.store(true); }, std::chrono::milliseconds(200));
		bool cancelled = timer.cancelTask(cancelId);
		EXPECT_TRUE(cancelled);
		EXPECT_EQ(timer.getTaskState(cancelId), TaskState::Cancelled);

		std::atomic<bool> executed{false};
		auto completedId = timer.addTask([&executed]() { executed.store(true); }, std::chrono::milliseconds(0));
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
		Timer timer(1);
		std::atomic<bool> executed{false};

		auto id = timer.addTask([&executed]() { executed.store(true); }, std::chrono::milliseconds(100));
		bool cancelled = timer.cancelTask(id);
		EXPECT_TRUE(cancelled);

		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		EXPECT_FALSE(executed.load());
		EXPECT_EQ(timer.getTaskState(id), TaskState::Cancelled);

		timer.shutdown();
	}

	TEST(TimerTests, ShutdownWaitsForRunningTask)
	{
		Timer timer(1);

		std::atomic<bool> started{false};
		std::atomic<bool> finished{false};

		auto id = timer.addTask([&started, &finished]() {
			started.store(true);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			finished.store(true);
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
}
