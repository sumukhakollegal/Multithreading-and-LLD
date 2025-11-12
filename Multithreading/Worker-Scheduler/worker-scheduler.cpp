#include<thread>
#include<iostream>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<vector>
#include<queue>
#include<memory>


using namespace std;

using Job = function<void()> ;

class Scheduler;

class Worker{
  mutex mtx;
  condition_variable cv;
  
  int id;
  bool has_job;
  bool should_stop;
  Job current_job;
  
  Scheduler* scheduler;
  
  thread worker_thread;
  public:
    Worker(int w_id, Scheduler* sched){
      id = w_id;
      should_stop = false;
      has_job = false;
      scheduler = sched;
    };
    
    void run();
    
    void init(){
      worker_thread = thread(&Worker::run, this);
    };
    
    
    void start_job(Job job){
      unique_lock<mutex> lock(mtx);
      has_job = true;
      current_job = job;
      
      lock.unlock();
      cv.notify_one();
      
    };
    
    void stop(){
      unique_lock<mutex> lock(mtx);
      should_stop = true;
      lock.unlock();
      cv.notify_one();
      
      if(worker_thread.joinable()){
        worker_thread.join();
      }
    }
    
  
};

class Scheduler{
  mutex mtx;
  vector<unique_ptr<Worker> > workers;
  queue<int> idle_workers;
  
  queue<Job> pending_jobs;
  
  public:
  Scheduler(int num_workers){
    for(int i=0; i<num_workers; i++){
      workers.emplace_back( make_unique<Worker>(i, this) );
    }
    
    for(int i=0; i<num_workers; i++){
      workers[i]->init();
      idle_workers.push(i);
    }
  };
  
  ~Scheduler(){
    for(auto &w: workers){
      w->stop();
    }
  }
  
  void submit_job(Job job){
    unique_lock<mutex> lock(mtx);
    if(!idle_workers.empty()){
      int worker_id = idle_workers.front();
      idle_workers.pop();
      
      lock.unlock();
      workers[worker_id]->start_job(job);
    }
    else{
      pending_jobs.push(job);
    }
  };
  
  void worker_done(int worker_id){
    unique_lock<mutex> lock(mtx);
    if(!pending_jobs.empty()){
      Job job = pending_jobs.front();
      pending_jobs.pop();
      lock.unlock();
      
      workers[worker_id]->start_job(job);
    }
    else{
      idle_workers.push(worker_id);
    }
  }
  
};

void Worker::run(){
  while(true){
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this](){
      return has_job || should_stop;
    });
    
    if(should_stop){
      break;
    }
    
    has_job = false;
    Job job = current_job;
    lock.unlock();
    
    job();
    
    scheduler->worker_done(id);
  }
  
  
}

int main() {
  Scheduler S(2);
  
  S.submit_job([](){
    cout<<"This is job 1 executed by thread "<<this_thread::get_id()<<endl;
    this_thread::sleep_for(chrono::milliseconds(300));
  });
  
  S.submit_job([](){
    cout<<"This is job 2 executed by thread "<<this_thread::get_id()<<endl;
        this_thread::sleep_for(chrono::milliseconds(300));

  });
  
  S.submit_job([](){
    cout<<"This is job 3 executed by thread "<<this_thread::get_id()<<endl;
        this_thread::sleep_for(chrono::milliseconds(300));

  });
  
  S.submit_job([](){
    cout<<"This is job 4 executed by thread "<<this_thread::get_id()<<endl;
        this_thread::sleep_for(chrono::milliseconds(300));

  });
  
  this_thread::sleep_for(chrono::seconds(2));
  
  
  

  return 0;
}