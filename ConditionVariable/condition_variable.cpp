#define CONDITION_VARIABLE  // 条件变量示例
// 否则为 条件变量 决定的线程顺序执行

#pragma region CondVar

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>

// 全局任务队列相关同步组件
std::queue<int> task_queue;
std::mutex mtx;
std::condition_variable cv;
// 全局退出标记，用于优雅停止所有消费者
std::atomic<bool> exit_flag{false};

// 生产者：持续往队列投放任务
void producer(int id, int task_count)
{
    for (int i = 0; i < task_count; ++i)
    {
        // 独占锁，离开作用域自动释放
        std::unique_lock<std::mutex> lock(mtx);
        int task = id * 100 + i;
        task_queue.push(task);
        std::cout << "生产者" << id << " 生产任务: " << task << std::endl;
        lock.unlock(); // 提前解锁，减少锁持有时间

        // 唤醒一个等待中的消费者
        cv.notify_one(); // 随机唤醒一个当前正在该条件变量上阻塞等待的线程，只唤醒 1 个，不是指定某一个
        // 每个 condition_variable 独立等待队列，cv1.notify_one 只会唤醒 cv1 上等待的线程，不会影响 cv2

        // 模拟生产耗时
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

// 消费者：阻塞等待任务、消费任务
void consumer(int id)
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);

        // 核心：while循环，双重判断
        // 1. 队列为空 且 没有退出信号 → 阻塞等待
        cv.wait(lock, [&]()
                { return !task_queue.empty() || exit_flag.load(); }); // 条件变量强制要求传入已上锁的 unique_lock
        // 执行到条件变量 wait 时，首先使用 unlock 解锁，线程进入阻塞队列休眠，此时其他线程可以正常获取锁
        // 收到 notify 被唤醒之后，线程从休眠中恢复，内部自动获取锁，获取到之后，退出 wait

        // 收到退出信号，且队列无剩余任务，直接退出线程
        if (exit_flag && task_queue.empty())
        {
            std::cout << "消费者" << id << " 收到退出指令，线程结束\n";
            break;
        }

        // 取出任务
        int task = task_queue.front();
        task_queue.pop();
        lock.unlock();

        // 模拟消费耗时
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        std::cout << "消费者" << id << " 完成任务: " << task << std::endl;
    }
}

int main()
{

    return 0;
}

#pragma endregion

#pragma region SeqExec

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
// 当前允许执行的线程编号
int current_step = 1;

void ThreadFunc(int id)
{
    std::unique_lock<std::mutex> lk(mtx);

    // 没轮到自己就阻塞等待
    cv.wait(lk, [&]()
            { return current_step == id; });

    // ================= 本线程业务逻辑 =================
    std::cout << "线程" << id << " 执行任务\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // ==================================================

    // 修改序号，唤醒下一个线程
    current_step++;
    cv.notify_all();
}

#pragma endregion

int main()
{
#ifdef CONDITION_VARIABLE

    const int producer_num = 2; // 生产者线程数量
    const int consumer_num = 3; // 消费者线程数量
    const int per_produce_task = 4;

    std::thread producers[producer_num];
    std::thread consumers[consumer_num];

    // 启动生产者线程
    for (int i = 0; i < producer_num; ++i)
    {
        producers[i] = std::thread(producer, i + 1, per_produce_task);
    }

    // 启动消费者线程
    for (int i = 0; i < consumer_num; ++i)
    {
        consumers[i] = std::thread(consumer, i + 1);
    }

    // 等待所有生产者生产完毕
    for (auto &t : producers)
    {
        t.join();
    }
    std::cout << "\n所有生产者完成生产，准备通知消费者退出\n\n";

    // 设置退出标记，唤醒全部消费者
    exit_flag.store(true);
    cv.notify_all();

    // 等待所有消费者处理完剩余任务并退出
    for (auto &t : consumers)
    {
        t.join();
    }

    std::cout << "\n全部任务处理完成，程序退出\n";

#else

    std::thread t1(ThreadFunc, 1);
    std::thread t2(ThreadFunc, 2);
    std::thread t3(ThreadFunc, 3);

    t1.join();
    t2.join();
    t3.join();
    std::cout << "全部线程按顺序执行完毕\n";
#endif
    return 0;
}