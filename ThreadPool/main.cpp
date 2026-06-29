#include <iostream>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <thread>
#include <string>
#include <chrono>

class ThreadPool
{
public:
    ThreadPool(int numThreads)
        : stop(false)
    {
        for (int i = 0; i < numThreads; i++)
        {
            threads.emplace_back([this]() { // 工作线程：有任务就执行，无任务就休眠
                for (;;)
                {
                    std::unique_lock<std::mutex> lock(mtx); // 相比 lock_guard 更加灵活，支持手动解锁，构造时自动加锁
                    condition.wait(lock, [this]() {         // wait 会先自动解锁 lock，然后让当前线程进行休眠状态，等待被唤醒；当其他线程使用 notify_one/notity_all 后，该线程被唤醒，自动加锁，执行第二个参数
                        return !tasks.empty() || stop;      // wait 的第二个参数为谓词：谓词返回true时，wait函数返回，执行后续代码；否则，重复解锁加休眠的过程
                    });
                    if (stop && tasks.empty())
                        return;

                    std::function<void()> task(std::move(tasks.front()));
                    tasks.pop();
                    lock.unlock();
                    task(); // 执行取出来的任务
                }

            });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        condition.notify_all(); // 唤醒*所有*等待在 condition 上的工作线程，让他们从 wait 中醒来，重新加锁后检查谓词，此时 stop 为 true，谓词返回 true，线程退出 wait，检查 if 判断后返回，线程结束

        for (auto &i : threads)
        {
            i.join(); // 等待所有线程结束
        }
    }

    template <class F, class... Args>
    void enqueue(F &&f, Args &&...args) // 任务入队函数模板
    {
        std::function<void()> task =
            std::bind(std::forward<F>(f), std::forward<Args>(args)...); // 核心：消除参数差异，统一任务类型为：std::function<void()>

        {
            std::unique_lock<std::mutex> lock(mtx);

            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace(std::move(task));
        }

        condition.notify_one(); // 放在锁外面，先解锁，再通知，唤醒一个等待在 condition 条件变量上的工作线程
                                // 无法指定哪一个工作线程被执行，被唤醒的线程是由操作系统的线程调度器随机选择的
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable condition; // 无任务时休眠，有任务时等待
    bool stop;
};

std::mutex printMutex;

int main()
{
    ThreadPool pool(std::thread::hardware_concurrency());
    std::cout << "该 CPU 最大支持的并发线程数量为：" << std::thread::hardware_concurrency() << std::endl;
    for (int i = 0; i < 12; i++)
    {
        pool.enqueue([i]() { // 向线程池中添加线程
            {
                std::unique_lock<std::mutex> guard(printMutex);
                std::cout << "tasks : " << i << "start!" << std::endl;
            }
            // 线程等待期间不要锁
            std::this_thread::sleep_for(std::chrono::seconds(3));

            {
                std::unique_lock<std::mutex> guard(printMutex);
                std::cout << "tasks : " << i << "finished..." << std::endl;
            }
        });
    }

    return 0;
}