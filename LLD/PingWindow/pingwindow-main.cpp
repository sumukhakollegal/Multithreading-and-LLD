// PingWindow

#include <exception>
#include <cstdlib>
#include <iostream>
#include <bits/stdc++.h>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <thread>
#include <unordered_map>
#include <exception>
#include <vector>
// #include <condition_variable>
using namespace std;

class PingWindow{
  int lastTime;
  int K;
  int n;
  vector<vector<int>> buckets;
  unordered_map<int, int> count_map;
  set<int> stale;

  void expireUpto(int curr_time){
    if(curr_time > lastTime+K){

      // Clear buckets
      for(int i=0; i<=K; i++)
        buckets[i].clear();
      // Everything is stale
      for(int i=0; i<n; i++){
        count_map[i]=0;
        stale.insert(i);
      }
      // Important: Update lastTime
      lastTime=curr_time;
      return;
    }

    // Walk forward from the moment after the last processed time up to the current time.
    // For each intermediate tick x, the time that falls out of the K-sized window is (x - K).
    // If that expireTime is valid (>= 0), evict everything that was recorded at that time.
    for(int x=lastTime+1; x<=curr_time; x++){
      int expireTime = x-K;
      
      if(expireTime>=0){
        // Map the expireTime into a circular bucket index within [0, K]
        int index=expireTime%(K+1);
        // Every node stored in this bucket was seen exactly at expireTime and is now out of window.
        // Decrement its in-window count; if it drops to zero, mark the node as stale.
        for(int node: buckets[index]){
          count_map[node]--;
          if(count_map[node]==0)
            stale.insert(node);
        }
        // Clear this bucket so it can be reused for future times that map to the same index.
        buckets[index].clear();
      }
    }

    lastTime=curr_time;

  };

  public:
  PingWindow(int num_nodes, int k){
    lastTime=-1;
    K=k;
    n=num_nodes;
    buckets.resize(K+1);

    // n nodes -> 0 to n-1
    for(int i=0; i<n; i++){
      count_map[i]=0;
      stale.insert(i);
    }
  };

  void ingest(int time, int node){
    // Ensure time is non-decreasing relative to the last processed time
    if(time < lastTime)
      time=lastTime;
    // Bring the window up to 'time' before recording this event
    expireUpto(time);

    cout<<"Time="<<time<<" | Node="<<node<<endl;

    // Map this time to its circular bucket and record the node
    int index=time%(K+1);
    buckets[index].push_back(node);

    // Node is no longer stale if this is its first in-window occurrence
    if(count_map[node]==0)
      stale.erase(node);
    // Track how many times this node appears within the active window
    count_map[node]++;
  }

  void query(int time){
    // First expire out-of-window data up to 'time'
    expireUpto(time);
    // Report all nodes that currently have zero in-window occurrences
    cout<<"Time="<<time<<" | Stale Nodes: ";
    for(int node: stale)
      cout<<node<<" ";
    cout<<endl;
  }
  
};


// To execute C++, please define "int main()"


int main() {
  PingWindow pingwindow(8, 4);
  cout<<"Nodes: 0 to 7 (8 nodes), Window size = 4"<<endl;

  for(int i=1; i<=20; i++)
    pingwindow.ingest(i, rand()%8);

  pingwindow.query(21);
  pingwindow.query(22);
  pingwindow.query(23);
  pingwindow.query(24);
  pingwindow.query(25);
  
}

