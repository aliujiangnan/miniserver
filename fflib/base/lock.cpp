#include "base/lock.h"
#include <sys/time.h>
using namespace ff;

Mutex::Mutex()
{
    pthread_mutex_init(&m_mutex, NULL);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

bool Mutex::lock()
{
    if (pthread_mutex_lock(&m_mutex))
    {
        return false;
    }
    return true;
}

bool Mutex::unlock()
{
    if (pthread_mutex_unlock(&m_mutex))
    {
        return false;
    }
    return true;
}

bool Mutex::try_lock()
{
    if (pthread_mutex_trylock(&m_mutex))
    {
        return false;
    }
    return true;
}


RWMutex::RWMutex()
{
    pthread_rwlock_init(&m_rwlock, NULL);
}

RWMutex::~RWMutex()
{
    pthread_rwlock_destroy(&m_rwlock);
}

bool RWMutex::rd_lock()
{
    return 0 == pthread_rwlock_rdlock(&m_rwlock);
}

bool RWMutex::wr_lock()
{
    return 0 == pthread_rwlock_wrlock(&m_rwlock);
}

bool RWMutex::unlock()
{
    return 0 == pthread_rwlock_unlock(&m_rwlock);
}

ConditionVar::ConditionVar(Mutex& mutex_):
    m_mutex(mutex_)
{
    pthread_cond_init(&m_cond, NULL);
}

ConditionVar::~ConditionVar()
{
    pthread_cond_destroy(&m_cond);
}

bool ConditionVar::wait()
{
    return 0 == pthread_cond_wait(&m_cond, &m_mutex.get_mutex());
}

bool ConditionVar::signal()
{
    return 0 == pthread_cond_signal(&m_cond);
}

bool ConditionVar::broadcast()
{
    return 0 == pthread_cond_broadcast(&m_cond);
}

SpinLock::SpinLock()
{
    pthread_spin_init(&spinlock, 0);
}
SpinLock::~SpinLock()
{
    pthread_spin_destroy(&spinlock);
}
void SpinLock::lock()
{
    pthread_spin_lock(&spinlock);
}
void SpinLock::unlock()
{
    pthread_spin_unlock(&spinlock);
}
bool SpinLock::try_lock()
{
    if (pthread_spin_trylock(&spinlock))
    {
        return false;
    }
    return true;
}

spin_LockGuard::spin_LockGuard(SpinLock& lock_):
    lock(lock_)
{
    lock.lock();
}
spin_LockGuard::~spin_LockGuard()
{
    lock.unlock();
}