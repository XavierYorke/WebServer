#ifndef LOCKER_H
#define LOCKER_H

#include<exception>
#include<pthread.h>
#include<semaphore.h>

// 线程同步机制封装类

// 互斥锁类
class locker {

private:
    pthread_mutex_t m_mutex;

public:
    // 构造函数
    locker() {
        if (pthread_mutex_init(&m_mutex, NULL) != 0) 
            throw std::exception();
    }

    // 析构函数
    ~locker() {
        pthread_mutex_destroy(&m_mutex);
    }

    // 上锁
    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    // 解锁
    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    // 获取互斥量
    pthread_mutex_t* get() {
        return &m_mutex;
    }
};

// 条件变量类 判断队列中是否有数据
class cond {

private:
    pthread_cond_t m_cond;

public:
    cond() {
        if (pthread_cond_init(&m_cond, NULL) != 0) 
            throw std::exception();
    }
    ~cond() {
        pthread_cond_destroy(&m_cond);
    }

    // 配合互斥锁使用
    bool wait(pthread_mutex_t* mutex) {
        return pthread_cond_wait(&m_cond, mutex) == 0;
    }

    bool timedwait(pthread_mutex_t* mutex, struct timespec t) {
        return pthread_cond_timedwait(&m_cond, mutex, &t) == 0;
    }

    // 唤醒一个线程
    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;
    }

    // 唤醒所有线程
    bool broadcast() {
        return pthread_cond_broadcast(&m_cond) == 0;
    }
};

// 信号量类
class sem {

private:
    sem_t m_sem;
    
public:
    sem() {
        if (sem_init(&m_sem, 0, 0) != 0)
            throw std::exception();
    }

    sem(int num) {
        if (sem_init(&m_sem, 0, num) != 0)
            throw std::exception();     
    }

    ~sem() {
        sem_destroy(&m_sem);
    }

    // 等待信号量
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }

    // 增加信号量
    bool post() {
        return sem_post(&m_sem) == 0;
    }
};


#endif