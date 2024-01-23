#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
 public:
  explicit ThreadPool(size_t num_threads) {
    if (num_threads > std::thread::hardware_concurrency())
      std::runtime_error(
          "Error: Attempted to make a thread pool using more "
          "threads than hardware concurrency limit.");
    threads_.resize(num_threads);
    for (size_t i = 0; i < num_threads; ++i) threads_.at(i) = std::thread(&ThreadPool::ThreadLoop, this);
  }

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;
  ~ThreadPool() { Stop(); }

  // Adds a function to the execution queue.
  void QueueJob(const std::function<void()>& job) {
    if (job == nullptr) std::runtime_error("Error: Attempted to add a null job to the thread pool.");
    std::scoped_lock lock(queue_mutex_);
    jobs_.push(job);
    thread_status_.notify_one();
  }

  bool AreJobsEnqueued() {
    std::scoped_lock lock(queue_mutex_);
    return !jobs_.empty();
  }

 private:
  // Joins all current threads to ensure a clean shutdown.
  void Stop() {
    {
      std::scoped_lock lock(queue_mutex_);
      should_terminate_ = true;
      thread_status_.notify_all();
    }
    for (std::thread& active_thread : threads_) {
      active_thread.join();
    }
    threads_.clear();
  }

  // Runs a thread in an infinite loop, completing tasks from the job queue as they become available.
  void ThreadLoop() {
    while (true) {
      {
        // unique_lock must be used here as condition_variable is only compatible with it, not scoped_lock.
        std::unique_lock<std::mutex> lock(queue_mutex_);
        thread_status_.wait(lock, [this] { return !jobs_.empty() || should_terminate_.load(); });
        if (should_terminate_.load()) {
          return;
        }
      }

      std::function<void()> job_fn;
      {
        // Need to relock as wait() unlocks the mutex when it is complete.
        std::scoped_lock lock(queue_mutex_);
        job_fn = jobs_.front();
        jobs_.pop();
      }
      job_fn();
    }
  }

  std::atomic<bool> should_terminate_ = false;  // Tells threads to stop looking for jobs.
  std::mutex queue_mutex_;                      // Prevents data races to the job queue.
  std::condition_variable thread_status_;       // Allows threads to wait on new jobs or termination.
  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> jobs_;
};
