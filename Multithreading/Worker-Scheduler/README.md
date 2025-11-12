# Worker Scheduler Overview

This example implements a small job scheduler that dispatches queued `Job` lambdas onto a fixed set of worker threads. A `Scheduler` owns the workers, feeds them jobs, tracks which workers are idle, and fills a pending queue when everything is busy. Each `Worker` runs a private loop, waits on a condition variable, executes work, and then lets the scheduler know it is ready for more.

---

## Why `unique_ptr<Worker>`?

`Scheduler` stores its workers inside `vector<unique_ptr<Worker>>` because each `Worker` object owns a `std::thread`. Threads are not copyable and only movable once; moving a `Worker` after its thread has been started would require moving the thread as well, which is error-prone and easy to misuse inside a `vector`. By wrapping each worker in a `unique_ptr`, the scheduler:

- Keeps `vector` from trying to copy or move `Worker` instances if it reallocates.
- Makes ownership explicit: the scheduler solely owns each worker instance and its thread.
- Avoids undefined behavior that could come from accidentally moving a live thread when the container grows or shrinks.

```76:84:Multithreading/Worker-Scheduler/worker-scheduler.cpp
Scheduler(int num_workers){
  for(int i=0; i<num_workers; i++){
    workers.emplace_back( make_unique<Worker>(i, this) );
  }
  ...
}
```

`unique_ptr` gives us stable addresses for every `Worker`, which is especially important because each one continuously runs its `std::thread` and registers itself back with the scheduler.

---

## Where and why `mutex` is used

There are two kinds of `mutex` in this program:

- **Scheduler mutex** (`Scheduler::mtx`): protects shared scheduling state that is touched by multiple threads (`idle_workers`, `pending_jobs`, and the `workers` vector when indexing into it).
- **Worker mutex** (`Worker::mtx`): guards per-worker state (`has_job`, `should_stop`, `current_job`) and coordinates the worker loop with its condition variable.

```93:118:Multithreading/Worker-Scheduler/worker-scheduler.cpp
void Scheduler::submit_job(Job job){
  unique_lock<mutex> lock(mtx);
  if(!idle_workers.empty()){
    int worker_id = idle_workers.front();
    idle_workers.pop();
    lock.unlock();
    workers[worker_id]->start_job(job);
  } else {
    pending_jobs.push(job);
  }
}
```

The scheduler takes the lock before checking or modifying the queues; without it, multiple workers finishing simultaneously could interleave pushes/pops and corrupt the queues or leak jobs.

```123:141:Multithreading/Worker-Scheduler/worker-scheduler.cpp
void Worker::run(){
  while(true){
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this](){ return has_job || should_stop; });
    if(should_stop){ break; }
    has_job = false;
    Job job = current_job;
    lock.unlock();
    job();
    scheduler->worker_done(id);
  }
}
```

Each worker locks its own state while it waits for work and when picking up `current_job`. That ensures no other thread (only the scheduler!) can write these members concurrently.

---

## Potential race conditions & how the locks prevent them

Below are the primary shared regions of code and why they are synchronized. The snippets show where races *would* occur if we removed the locking.

- **Idle worker queue (`idle_workers`)**: Multiple worker threads can call `Scheduler::worker_done` concurrently. Without the scheduler mutex, two threads could both pop the same idle worker or interleave pushes, corrupting the queue.

```107:118:Multithreading/Worker-Scheduler/worker-scheduler.cpp
void Scheduler::worker_done(int worker_id){
  unique_lock<mutex> lock(mtx);
  if(!pending_jobs.empty()){
    Job job = pending_jobs.front();
    pending_jobs.pop();
    lock.unlock();
    workers[worker_id]->start_job(job);
  } else {
    idle_workers.push(worker_id);
  }
}
```

- **Current job hand-off (`current_job` / `has_job`)**: The scheduler thread and the worker thread touch these fields from different contexts. If `Worker::start_job` set `current_job` without the worker mutex, the worker loop could read a half-written function object or miss the `has_job` flag update.

```44:52:Multithreading/Worker-Scheduler/worker-scheduler.cpp
void Worker::start_job(Job job){
  unique_lock<mutex> lock(mtx);
  has_job = true;
  current_job = job;
  lock.unlock();
  cv.notify_one();
}
```

- **Shutdown (`should_stop`)**: `Scheduler` sets `should_stop` from a different thread when destroying workers. Guarding it with the worker mutex guarantees the worker receives the updated value before or during the wait and exits cleanly.

```54:63:Multithreading/Worker-Scheduler/worker-scheduler.cpp
void Worker::stop(){
  unique_lock<mutex> lock(mtx);
  should_stop = true;
  lock.unlock();
  cv.notify_one();
  if(worker_thread.joinable()){
    worker_thread.join();
  }
}
```

---

## Per-thread mutex and condition variable

Each `Worker` owns its own `mutex` and `condition_variable`. This fine-grained locking means:

- A worker only waits on its own condition variable, so it does not contend with other workers waiting for unrelated jobs.
- The scheduler can wake exactly one worker by calling `start_job` on the target worker, avoiding broadcasts and unnecessary wakeups.
- Worker-specific state (`has_job`, `should_stop`, `current_job`) never needs to be shared across workers, keeping the scope of the lock small and preventing cross-thread interference.

The design contrasts with a shared condition variable for all workers, which would require more bookkeeping and risk wakeups going to the wrong thread.

---

## Why `notify_one()` in `start_job()` and `stop()`

`notify_one()` wakes a single waiting thread. In this scheduler, exactly one worker should wake up when a job is assigned or when we request that worker to exit:

- **`start_job()`**: only the worker receiving the job should resume. `notify_one()` targets whichever thread is waiting on that worker's condition variable. Broadcasting (`notify_all()`) would wake unrelated workers that do not have new jobs, causing needless wakeups and immediate re-waits.

- **`stop()`**: only the worker that is being shut down needs to leave the wait. Again, `notify_one()` is sufficient and avoids triggering other workers.

Using `notify_one()` is both more efficient and semantically correct because each worker has its own condition variable; there is exactly one waiting thread per condition.

---

## Summary

- `unique_ptr<Worker>` ensures workers (and therefore threads) stay at stable addresses, preventing accidental moves when the vector resizes.
- A scheduler-level mutex protects shared queues; worker-level mutexes protect each thread’s state. Together they eliminate data races on shared job queues and hand-off variables.
- Potential race points include the idle queue, job hand-off, and shutdown flag—each snippet above shows where the mutex removes the hazard.
- Every worker owns its synchronization primitives, letting the scheduler wake precise threads with `notify_one()` and avoid unnecessary contention.

