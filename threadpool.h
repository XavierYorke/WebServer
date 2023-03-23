#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<pthread.h>
#include<list>
#include"locker.h"
#include<exception>
#include<cstdio>

// 线程池类，定义成模板类是为了复用，模板参数T是任务类
template<typename T>
class threadpool {

private:
    // 线程的数量
    int m_thread_number;

    // 线程池数组，大小为m_thread_number
    pthread_t* m_threads;

    // 请求队列中最多允许的，等待处理的请求数量
    int m_max_requests;

    // 请求队列
    std::list<T*> m_workqueue;

    // 互斥锁
    locker m_queuelocker;

    // 信号量用来判断是否有任务需要处理
    sem m_queuestat;

    // 是否结束线程
    bool m_stop;

    // 静态函数不能访问非静态成员
    static void* worker(void* arg);
    void run();

public:
    threadpool(int thread_number=8, int max_requests=10000);
    ~threadpool();

    // 添加任务
    bool append(T* request);

};

template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests):
    m_thread_number(thread_number), m_max_requests(max_requests),
    m_stop(false), m_threads(NULL) {
    if ((thread_number <= 0) || (max_requests <= 0)) {
        throw std::exception();
    }

    // new 的析构要销毁
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads) throw std::exception();

    // 创建thread_number个线程，并将它们设置为线程脱离
    for (int i = 0; i < thread_number; ++i) {
        printf("creating the %dth thread\n", i);

        // 执行函数woker必须是静态函数，将this传递给静态函数
        if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
            delete [] m_threads;
            throw std::exception();
        }

        // 成功返回0，失败返回error number
        if (pthread_detach(m_threads[i]) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template<typename T>
threadpool<T>::~threadpool() {
    delete[] m_threads;
    m_stop = true;
}

// 添加任务
template<typename T>
bool threadpool<T>::append(T* request) {
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests) {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    // 信号量增加
    m_queuestat.post();
    return true;
}

template<typename T>
void* threadpool<T>::worker(void* arg) {
    // 获取该线程池
    threadpool* pool = (threadpool*) arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run() {
    while(!m_stop) {
        m_queuestat.wait();
        m_queuelocker.lock();

        if (m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        
        if (!request) {
            continue;
        }
        
        request->process();
    }
}

#endif