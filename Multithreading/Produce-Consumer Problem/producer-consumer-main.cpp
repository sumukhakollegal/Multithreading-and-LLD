// Producer Consumer
#include <iostream>
#include <optional>
#include <bits/stdc++.h>
#include <stdexcept>
using namespace std;

class Bounded_Blocking_Queue{
  queue<int> q;
  int size;

  counting_semaphore<INT_MAX> spaces;
  counting_semaphore<INT_MAX> items;

  mutex mtx;

  public: 
  Bounded_Blocking_Queue(int s): size(s), spaces(s), items(0){};

  void producer(int threadID){
    for(int i=0; i<20; i++){
      spaces.acquire();
      int value=rand()%10;
      unique_lock<mutex> lock(mtx);
      q.push(value);
      lock.unlock();

      cout<<"Thread "<<threadID<<" produced "<<value<<endl;

      items.release();
    }
  };

  void consumer(int threadID){
    for(int i=0; i<20; i++){
      items.acquire();
      int value;
      
      unique_lock<mutex> lock(mtx);
      value=q.front();
      q.pop();
      lock.unlock();

      cout<<"Thread "<<threadID<<" consumed "<<value<<endl;

      spaces.release();
    }
  };

};


int main() {
  Bounded_Blocking_Queue bbq(10);

  vector<thread> threads;

  for(int i=0; i<4; i++){
    threads.emplace_back([i, &bbq]{
      if(i%2){
        bbq.producer(i);
      }
      else{
        bbq.consumer(i);
      }
    });
  }

  for(auto &t: threads)
    t.join();
  
}

// Todo:
// Add test cases and error handling 
// No. of items from producer should be == no. of items from consumer
// Sum(producer) should be == sum(consumer)