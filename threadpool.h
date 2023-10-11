#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <thread>
#include <functional>
#include <memory>
#include <mutex>
#include <future>
#include <queue>

class ThreadPool
{
public:
    ThreadPool(int n){
        for (int i = 0; i < n; i++)
        {
           threads_.emplace_back(std::thread([this](){
                for(;;)
                {
                    std::function<void()> func;

                    {
                        std::unique_lock<std::mutex> ul(this->cv_mutex_);
                        cv_.wait(ul, [this](){ return (this->stop_ || !this->tasks_.empty());});
                    }
                        if(this->stop_ && this->tasks_.empty())
                            return;
                    {
                        std::lock_guard<std::mutex> lg(this->mutex_);
                        func = std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }

                    func();
                }
           }));
        }

    }

    ~ThreadPool(){
        {
            std::lock_guard<std::mutex> lg(mutex_);
            stop_ = true;
        }
        cv_.notify_all();

        for(auto& t : threads_){
            if(t.joinable())
                t.join();
        }
    }

    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        auto taskPtr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        {
            std::lock_guard<std::mutex> lg(mutex_);
            tasks_.emplace([taskPtr](){
                (*taskPtr)();
            });
        }

        cv_.notify_one();

        return taskPtr->get_future();
    }


    template <typename F, typename... Args>
    auto submit1(F&& f, Args&&... args) -> void
    {
        std::function<void()> fun = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::lock_guard<std::mutex> lg(mutex_);
            tasks_.emplace(fun);
        }
    }

private:
    bool stop_ = false;

    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;

    std::mutex cv_mutex_;
    std::condition_variable cv_;
};


#endif // !_THREADPOOL_H