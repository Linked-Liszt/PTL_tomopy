//
// MIT License
// Copyright (c) 2019 Jonathan R. Madsen
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// ---------------------------------------------------------------
// Tasking class header file
//
// Class Description:
//
// This file creates an abstract base class for the grouping the thread-pool
// tasking system into independently joinable units
//
// ---------------------------------------------------------------
// Author: Jonathan Madsen (Feb 13th 2018)
// ---------------------------------------------------------------

#pragma once

#include "PTL/AutoLock.hh"
#include "PTL/Threading.hh"
#include "PTL/VTask.hh"

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <deque>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

class ThreadPool;

#define _MOVE_MEMBER(_member) _member = std::move(rhs._member)

class VTaskGroup
{
public:
    template <typename _Tp>
    using container_type = std::list<_Tp>;
    template <typename _Tp>
    using list_type = std::list<_Tp>;

    typedef VTaskGroup                   this_type;
    typedef std::thread::id              tid_type;
    typedef VTask                        task_type;
    typedef uintmax_t                    size_type;
    typedef Mutex                        lock_t;
    typedef std::atomic_intmax_t         atomic_int;
    typedef std::atomic_uintmax_t        atomic_uint;
    typedef Condition                    condition_t;
    typedef std::shared_ptr<task_type>   task_pointer;
    typedef container_type<task_pointer> vtask_list_type;

public:
    // Constructor and Destructors
    explicit VTaskGroup(ThreadPool* tp = nullptr);
    // Virtual destructors are required by abstract classes
    // so add it by default, just in case
    virtual ~VTaskGroup();

    VTaskGroup(const this_type&) = delete;
    VTaskGroup(this_type&& rhs)  = default;

    this_type& operator=(const this_type&) = delete;
    this_type& operator=(this_type&& rhs) = default;

public:
    //------------------------------------------------------------------------//
    // wait to finish
    virtual void wait();

    //------------------------------------------------------------------------//
    // increment (prefix)
    intmax_t operator++() { return ++m_tot_task_count; }
    intmax_t operator++(int) { return m_tot_task_count++; }
    //------------------------------------------------------------------------//
    // increment (prefix)
    intmax_t operator--() { return --m_tot_task_count; }
    intmax_t operator--(int) { return m_tot_task_count--; }
    //------------------------------------------------------------------------//
    // size
    intmax_t size() const { return m_tot_task_count.load(); }

    // get the locks/conditions
    lock_t&            task_lock() { return m_task_lock; }
    condition_t&       task_cond() { return m_task_cond; }
    const lock_t&      task_lock() const { return m_task_lock; }
    const condition_t& task_cond() const { return m_task_cond; }

    // identifier
    const uintmax_t& id() const { return m_id; }

    // thread pool
    void         set_pool(ThreadPool* tp) { m_pool = tp; }
    ThreadPool*& pool() { return m_pool; }
    ThreadPool*  pool() const { return m_pool; }

    virtual void clear() { vtask_list.clear(); }
    virtual bool is_native_task_group() const { return true; }
    virtual bool is_master() const { return this_tid() == m_main_tid; }

    //------------------------------------------------------------------------//
    // check if any tasks are still pending
    virtual intmax_t pending() { return m_tot_task_count.load(); }

protected:
    //------------------------------------------------------------------------//
    // get the thread id
    static tid_type this_tid() { return std::this_thread::get_id(); }

    //------------------------------------------------------------------------//
    // get the task count
    atomic_int&       task_count() { return m_tot_task_count; }
    const atomic_int& task_count() const { return m_tot_task_count; }

    //------------------------------------------------------------------------//
    // process tasks in personal bin
    void execute_this_threads_tasks();

protected:
    // Private variables
    atomic_int      m_tot_task_count;
    uintmax_t       m_id;
    ThreadPool*     m_pool;
    condition_t     m_task_cond;
    lock_t          m_task_lock;
    tid_type        m_main_tid;
    vtask_list_type vtask_list;

protected:
    enum class state : int
    {
        STARTED = 0,
        STOPPED = 1,
        NONINIT = 2
    };
};

//--------------------------------------------------------------------------------------//

// don't pollute
#undef _MOVE_MEMBER
