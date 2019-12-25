/*
 * Copyright 2019-present Julián Bermúdez Ortega.
 *
 * This file is part of vaneins::utils.
 *
 * vaneins::utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vaneins::utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with vaneins::utils.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VANEINS_UTIL_SHAREDMUTEX_SHAREDMUTEX_HPP_
#define VANEINS_UTIL_SHAREDMUTEX_SHAREDMUTEX_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>

namespace vaneins {
namespace util {
    
class SharedMutex
{
public:
    friend class SharedLock;
    friend class ExclusiveLock;

    SharedMutex()
        : mtx_{}
        , cv_{}
        , shared_counter_{0}
        , exclusively_locked_{false}
    {}

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    size_t shared_counter_;
    bool exclusively_locked_;
    std::thread::id exclusive_tread_id_;
};

class SharedLock 
{
public:
    SharedLock(
            SharedMutex& shared_mtx)
        : shared_mtx_{shared_mtx}
        , locked_{false}
    {
        lock();
    }

    ~SharedLock()
    {
        unlock();
    }

    void lock()
    {
        if (!locked_)
        {
            std::thread::id thread_id = std::this_thread::get_id();
            std::unique_lock<std::mutex> lock{shared_mtx_.mtx_};
            shared_mtx_.cv_.wait(
                lock,
                [&]{ return !shared_mtx_.exclusively_locked_ || 
                        thread_id == shared_mtx_.exclusive_tread_id_; });
            ++shared_mtx_.shared_counter_;
            locked_ = true;
        }
    }

    void unlock()
    {
        if (locked_)
        {
            std::unique_lock<std::mutex> lock{shared_mtx_.mtx_};
            --shared_mtx_.shared_counter_;
            if (0 == shared_mtx_.shared_counter_)
            {
                shared_mtx_.cv_.notify_one();
            }
            locked_ = false;
        }
    }

private:
    SharedMutex& shared_mtx_;
    bool locked_;
};

class ExclusiveLock 
{
public:
    ExclusiveLock(
            SharedMutex& shared_mtx)
        : shared_mtx_{shared_mtx}
        , locked_{false}
    {
        lock();
    }

    ~ExclusiveLock()
    {
        unlock();
    }

    void lock()
    {
        if (!locked_)
        {
            std::unique_lock<std::mutex> lock{shared_mtx_.mtx_};
            shared_mtx_.cv_.wait(
                lock,
                [&]{ return (0 == shared_mtx_.shared_counter_) &&  !shared_mtx_.exclusively_locked_; });
            shared_mtx_.exclusively_locked_ = true;
            shared_mtx_.exclusive_tread_id_ = std::this_thread::get_id();
            locked_ = true;
        }
    }

    void unlock()
    {
        if (locked_)
        {
            std::unique_lock<std::mutex> lock{shared_mtx_.mtx_};
            shared_mtx_.exclusively_locked_ = false;
            shared_mtx_.cv_.notify_all();
            locked_ = false;
        }
    }

private:
    SharedMutex& shared_mtx_;
    bool locked_;
};


} // namespace util
} // namespace vaneins

#endif // VANEINS_UTIL_SHAREDMUTEX_SHAREDMUTEX_HPP_
