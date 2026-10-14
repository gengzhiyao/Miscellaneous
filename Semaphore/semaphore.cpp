#define RESOURCE_ACQUIRE    // 控制资源的并发访问

/**
 * @brief 信号量的用途
 * 1. 针对二元信号的互斥
 * 2. 资源管理：控制对资源的并发访问
 * 3. 同步操作：生产者 - 消费者模型
 */

#ifdef _WIN32

#pragma region Windows
#ifndef RESOURCE_ACQUIRE
#include <stdio.h>
#include <iostream>
#include <atomic>
#include <Windows.h>

/** @brief Windows 中 CRITICAL_SECTION 和 Mutex 的区别
 * ! CRITICAL_SECTION
 * 用户态同步，无内核对象，只支持同一进程内线程互斥；性能极快：无系统调用
 * 创建：InitializeCriticalSection(&cs)  销毁：DeleteCriticalSection(&cs)；
 * 加锁：EnterCriticalSection，          解锁：LeaveCriticalSection；
 * ! Mutex
 * 内核态同步对象，跨进程、跨线程都能用；每次加解锁都要切换内核态
 * 创建：CreateMutex                           销毁：CloseHandle()；
 * 加锁：WaitForSingleObject(mutex, INFINITE)  解锁：ReleaseMutex(mutex)；
 */

#define N 5  // 生产者、消费者线程数量
#define M 10 // 环形缓冲区容量

std::atomic<bool> g_running(true);

int productin = 0;
int prochaseout = 0;
int buff[M] = {0};

HANDLE empty_sem;   // 空闲
HANDLE full_sem;    // 存放

// Windows原生临界区替代pthread_mutex
CRITICAL_SECTION cs_buf;

std::atomic<int> product_id(0);
std::atomic<int> prochase_id(0);

void PrintProduction()
{
    printf("当前缓冲区队列：");
    for (int i = 0; i < M; i++)
    {
        printf("%d  ", buff[i]);
    }
    printf("\n\n");
}

// Windows线程入口固定格式 DWORD WINAPI ThreadFunc(LPVOID)
DWORD WINAPI Product(LPVOID param)
{
    int id = ++product_id;
    delete static_cast<int *>(param);

    while (g_running.load())
    {
        Sleep(5000);

        WaitForSingleObject(empty_sem, INFINITE); // P 操作 拿一个空位 消耗空槽
        EnterCriticalSection(&cs_buf);

        int pos = productin % M;
        printf("生产者%d 将产品放入缓冲区位置 %d\n", id, pos + 1);
        buff[pos] = 1;
        PrintProduction();
        productin++;

        LeaveCriticalSection(&cs_buf);
        ReleaseSemaphore(full_sem, 1, nullptr); // V操作 新增一个产品 唤醒等待产品的消费者
    }
    printf("生产者%d 线程退出\n", id);
    return 0;
}

DWORD WINAPI Prochase(LPVOID param)
{
    int id = ++prochase_id;
    delete static_cast<int *>(param);

    while (g_running.load())
    {
        Sleep(7000);

        WaitForSingleObject(full_sem, INFINITE); // P 操作 拿一个产品 消耗
        EnterCriticalSection(&cs_buf);

        int pos = prochaseout % M;
        printf("消费者%d 从缓冲区位置 %d 取出产品\n", id, pos + 1);
        buff[pos] = 0;
        PrintProduction();
        prochaseout++;

        LeaveCriticalSection(&cs_buf);
        ReleaseSemaphore(empty_sem, 1, nullptr);    // 增加一个空槽
    }
    printf("消费者%d 线程退出\n", id);
    return 0;
}

int main()
{
    std::cout << "生产者、消费者各5个，缓冲区容量10\n生产者5s生产一个，消费者7s消费一个\n关闭窗口结束程序\n\n"
              << std::endl;

    HANDLE product_threads[N];
    HANDLE consume_threads[N];

    /**
     * HANDLE CreateSemaphoreW(
                                LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,    // 安全属性结构体 填 null
                                LONG                  lInitialCount,            // 信号量初始可用资源计数
                                LONG                  lMaximumCount,            // 信号量最大资源上限 计数不能超过该值
                                LPCWSTR               lpName);                  // 信号量共享名称 用于跨进程共享同一个信号量 null 代表仅本进程可用
     */
    empty_sem = CreateSemaphoreW(nullptr, M, M, nullptr);   
    full_sem = CreateSemaphoreW(nullptr, 0, M, nullptr);
    if (empty_sem == nullptr || full_sem == nullptr)
    {
        printf("信号量创建失败，错误码：%d\n", GetLastError());
        return -1;
    }

    // 初始化临界区
    InitializeCriticalSection(&cs_buf);

    // 创建生产者
    for (int i = 0; i < N; i++)
    {
        int *arg = new int(i);
        product_threads[i] = CreateThread(
            nullptr,
            0,
            Product,
            arg,
            0,
            nullptr);
        if (product_threads[i] == nullptr)
        {
            printf("生产者%d线程创建失败\n", i);
            delete arg;
            goto clean_cs;
        }
    }

    // 创建消费者
    for (int i = 0; i < N; i++)
    {
        int *arg = new int(i);
        consume_threads[i] = CreateThread(
            nullptr,
            0,
            Prochase,
            arg,
            0,
            nullptr);
        if (consume_threads[i] == nullptr)
        {
            printf("消费者%d线程创建失败\n", i);
            delete arg;
            goto clean_cs;
        }
    }

    // 等待所有线程结束
    WaitForMultipleObjects(N, product_threads, TRUE, INFINITE);
    WaitForMultipleObjects(N, consume_threads, TRUE, INFINITE);

    // 关闭线程句柄
    for (int i = 0; i < N; i++)
    {
        CloseHandle(product_threads[i]);
        CloseHandle(consume_threads[i]);
    }

clean_cs:
    DeleteCriticalSection(&cs_buf);
clean_sem:
    if (empty_sem != nullptr)
        CloseHandle(empty_sem);
    if (full_sem != nullptr)
        CloseHandle(full_sem);
    printf("所有资源释放完毕，程序退出\n");
    return 0;
}
#endif
#pragma endregion

#ifdef RESOURCE_ACQUIRE
#include <Windows.h>
#include <stdio.h>
#include <atomic>

// 最多允许 3 个线程同时访问共享资源
#define MAX_CONCURRENT 3
// 总工作线程数量
#define THREAD_COUNT 8

// 计数信号量：用来控制并发上限
HANDLE g_limit_sem;
// 仅用来打印不乱序，不参与并发限流
CRITICAL_SECTION g_print_cs;
// 线程自增ID
std::atomic<int> g_tid(0);
std::atomic<int> cur_concurrent = 0;

// 模拟耗时共享资源操作
void AccessSharedResource(int tid)
{
    // 模拟业务耗时 2 秒
    Sleep(2000);
    EnterCriticalSection(&g_print_cs);
    printf("线程%d 完成资源访问\n", tid);
    LeaveCriticalSection(&g_print_cs);
}

// 线程入口
DWORD WINAPI WorkThread(LPVOID param)
{
    int tid = ++g_tid;
    (void)param;

    EnterCriticalSection(&g_print_cs);
    printf("线程%d 准备进入资源区，等待通行权...\n", tid);
    LeaveCriticalSection(&g_print_cs);

    // P 操作：申请一个并发名额，超过 MAX_CONCURRENT 则阻塞
    WaitForSingleObject(g_limit_sem, INFINITE); // 内核将信号量计数 -1 若减后计数 = -1（原计数为 0）：线程阻塞，等待 ReleaseSemaphore

    int now = ++cur_concurrent;

    // ========= 最多同时 MAX_CONCURRENT 个线程走到这里 =========
    EnterCriticalSection(&g_print_cs);

    printf(">>>> 线程%d 进入共享资源区，当前并发不超过%d\n", tid, MAX_CONCURRENT);
    printf("线程%d 进入，当前并发=%d\n", tid, now);

    LeaveCriticalSection(&g_print_cs);

    AccessSharedResource(tid);

    --cur_concurrent;
    // V 操作：释放名额，唤醒排队线程
    ReleaseSemaphore(g_limit_sem, 1, nullptr);

    EnterCriticalSection(&g_print_cs);
    printf("线程%d 离开资源区，归还并发名额 <<<<\n", tid);
    LeaveCriticalSection(&g_print_cs);

    return 0;
}

int main()
{
    InitializeCriticalSection(&g_print_cs);

    // 创建计数信号量：初始可用名额 MAX_CONCURRENT，最大上限 MAX_CONCURRENT
    g_limit_sem = CreateSemaphoreW(
        nullptr,
        MAX_CONCURRENT,
        MAX_CONCURRENT,
        nullptr);
    if (g_limit_sem == nullptr)
    {
        printf("信号量创建失败 err=%d\n", GetLastError());
        DeleteCriticalSection(&g_print_cs);
        return -1;
    }

    HANDLE threads[THREAD_COUNT];
    // 批量创建8个工作线程
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        threads[i] = CreateThread(nullptr, 0, WorkThread, nullptr, 0, nullptr);
    }

    // 等待全部线程执行完毕
    WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, INFINITE);

    // 释放线程句柄
    for (int i = 0; i < THREAD_COUNT; i++)
        CloseHandle(threads[i]);

    // 清理资源
    CloseHandle(g_limit_sem);
    DeleteCriticalSection(&g_print_cs);
    printf("\n所有线程执行完毕\n");
    return 0;
}
#endif
#else
#pragma region Linux
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <iostream>
#include <atomic>
#include <time.h>
#include <thread>

#define N 5  // 生产者、消费者线程数量
#define M 10 // 环形缓冲区容量

// 全局控制标记，用于信号触发退出
std::atomic<bool> g_running(true);

int productin = 0;   // 生产者写入下标
int prochaseout = 0; // 消费者取出下标
int buff[M] = {0};   // 缓冲区 0=空 1=有产品

sem_t empty_sem;       // 空槽信号量，初始M
sem_t full_sem;        // 产品信号量，初始0
pthread_mutex_t mutex; // 缓冲区互斥锁

std::atomic<int> product_id(0);  // 原子生产者ID，避免多线程竞争
std::atomic<int> prochase_id(0); // 原子消费者ID

// Ctrl+C 信号回调，设置退出标记
void sigint_handler(int sig)
{
    (void)sig;
    printf("\n捕获退出信号，准备停止所有线程...\n");
    g_running.store(false);
}

// 打印缓冲区全部状态
void PrintProduction()
{
    printf("当前缓冲区队列：");
    for (int i = 0; i < M; i++)
    {
        printf("%d  ", buff[i]);
    }
    printf("\n\n");
}

// 生产者线程函数
void *Product(void *param)
{
    int id = ++product_id;
    delete static_cast<int *>(param); // 释放堆分配的参数

    while (g_running.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 5秒生产间隔

        sem_wait(&empty_sem);       // 等待空位置
        pthread_mutex_lock(&mutex); // 加锁访问缓冲区

        int pos = productin % M;
        printf("生产者%d 将产品放入缓冲区位置 %d\n", id, pos + 1);
        buff[pos] = 1;
        PrintProduction();
        productin++;

        pthread_mutex_unlock(&mutex);
        sem_post(&full_sem); // 通知有新产品
    }
    printf("生产者%d 线程退出\n", id);
    return nullptr;
}

// 消费者线程函数
void *Prochase(void *param)
{
    int id = ++prochase_id;
    delete static_cast<int *>(param);

    while (g_running.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(7)); // 7秒消费间隔

        sem_wait(&full_sem);
        pthread_mutex_lock(&mutex);

        int pos = prochaseout % M;
        printf("消费者%d 从缓冲区位置 %d 取出产品\n", id, pos + 1);
        buff[pos] = 0;
        PrintProduction();
        prochaseout++;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty_sem); // 释放空槽
    }
    printf("消费者%d 线程退出\n", id);
    return nullptr;
}

int main()
{
    // 注册Ctrl+C信号处理
    signal(SIGINT, sigint_handler);

    std::cout << "生产者、消费者各5个，缓冲区容量10\n生产者5s生产一个，消费者7s消费一个\n按下 Ctrl+C 安全退出\n\n"
              << std::endl;

    pthread_t product_threads[N];
    pthread_t consume_threads[N];
    int ret;

    // 1. 初始化信号量
    int sem_empty = sem_init(&empty_sem, 0, M);
    int sem_full = sem_init(&full_sem, 0, 0);
    if (sem_empty != 0 || sem_full != 0)
    {
        perror("sem_init 初始化失败");
        return -1;
    }

    // 2. 初始化互斥锁
    ret = pthread_mutex_init(&mutex, nullptr);
    if (ret != 0)
    {
        perror("pthread_mutex_init 初始化失败");
        goto clean_sem;
    }

    // 3. 创建生产者线程（堆分配参数，解决传参竞态）
    for (int i = 0; i < N; i++)
    {
        int *arg = new int(i);
        ret = pthread_create(&product_threads[i], nullptr, Product, arg);
        if (ret != 0)
        {
            perror("生产者线程创建失败");
            delete arg;
            goto clean_mutex;
        }
    }

    // 4. 创建消费者线程
    for (int i = 0; i < N; i++)
    {
        int *arg = new int(i);
        ret = pthread_create(&consume_threads[i], nullptr, Prochase, arg);
        if (ret != 0)
        {
            perror("消费者线程创建失败");
            delete arg;
            goto clean_mutex;
        }
    }

    // 等待所有线程执行完毕
    for (int i = 0; i < N; i++)
    {
        pthread_join(product_threads[i], nullptr);
    }
    for (int i = 0; i < N; i++)
    {
        pthread_join(consume_threads[i], nullptr);
    }

clean_mutex:
    pthread_mutex_destroy(&mutex);
clean_sem:
    sem_destroy(&empty_sem);
    sem_destroy(&full_sem);
    printf("所有资源释放完毕，程序退出\n");
    return 0;
}
#pragma endregion
#endif

