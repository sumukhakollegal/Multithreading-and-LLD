## PingWindow — Track Unpinged Nodes in the Last K Seconds

### Problem
Given `n` nodes and a time-based event stream of pings `(time, node)`, efficiently answer:

- **Query**: How many (and which) nodes have not been pinged in the last **K** seconds at a given time `t`?

Constraints to keep in mind:
- Time moves forward (monotonic non-decreasing in `ingest`).
- We need queries to be fast, even for large `n` and long runtimes.

### Core Idea (Sliding Time Window with Circular Buckets)
- Maintain a circular array `buckets` of size `K + 1`. Each bucket at index `i` stores nodes that were pinged at times congruent to `i (mod K+1)`.
- Keep a `count_map[node]` of how many pings of that node are currently inside the active window `[t-K, t]`.
- Maintain a live `stale` set of nodes whose `count_map[node] == 0` (not seen in the last `K` seconds).
- As time advances, expiring a tick `x` means evicting everything recorded at `expireTime = x - K` from the window by decrementing their counts and reusing its bucket.

This makes both ingestion and expiration amortized O(1) per event, and queries O(S) where S is the number of stale nodes returned (or O(1) to just get a count via `stale.size()`).

### Important Code Sections

Constructor — initialize window state and mark all nodes stale initially:
```69:81:LLD/PingWindow/pingwindow-main.cpp
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
```

Advance/expire window up to current time `curr_time` and evict out-of-window data using the circular index:
```47:63:LLD/PingWindow/pingwindow-main.cpp
for(int x=lastTime+1; x<=curr_time; x++){
  int expireTime = x-K;
  
  if(expireTime>=0){
    int index=expireTime%(K+1);
    for(int node: buckets[index]){
      count_map[node]--;
      if(count_map[node]==0)
        stale.insert(node);
    }
    buckets[index].clear();
  }
}
```

Ingest an event `(time, node)` — map time to bucket, update counts, and remove node from `stale` if needed:
```92:101:LLD/PingWindow/pingwindow-main.cpp
int index=time%(K+1);
buckets[index].push_back(node);

if(count_map[node]==0)
  stale.erase(node);
count_map[node]++;
```

Query at time `t` — expire first, then read `stale` directly:
```103:111:LLD/PingWindow/pingwindow-main.cpp
void query(int time){
  expireUpto(time);
  cout<<"Time="<<time<<" | Stale Nodes: ";
  for(int node: stale)
    cout<<node<<" ";
  cout<<endl;
}
```

### Complexity
- **Ingest**: Amortized O(1) per event (single bucket push + occasional expire work distributed per tick).
- **Expire**: O(#items expiring at that tick). Each event is inserted once and removed once.
- **Query**: O(S) to list all stale nodes; O(1) to get the count as `stale.size()`.
- **Space**: O(K + total distinct nodes + events in the last K seconds).

### Correctness Highlights
- **Monotonic time**: If an ingest arrives with `time < lastTime`, it’s clamped to `lastTime` to preserve monotonic progress.
- **Full reset**: If `curr_time > lastTime + K`, the window has completely advanced; we clear all buckets and mark all nodes stale.
- **No double-counting**: Each event lives in precisely one bucket mapped by `time % (K+1)` and is evicted exactly once.

### Build & Run
Using Clang or GCC on macOS/Linux:
```bash
g++ -std=c++17 -O2 -Wall -Wextra -o pingwindow pingwindow-main.cpp && ./pingwindow
```

### Customization
- **Number of nodes**: Change `PingWindow pingwindow(<num_nodes>, K)` in `main`.
- **Window size K**: Change the second parameter in the constructor and rebuild.
- **Input stream**: Replace the `for` loop and `rand()%n` with real event ingestion.

### Why Circular Buckets?
- Time modulo `K+1` lets us reuse memory predictably and evict whole time slices in O(1) per tick, avoiding per-event scans.
- This keeps the structure light, predictable, and cache-friendly under steady streams.

### Example Usage (from `main`)
```119:126:LLD/PingWindow/pingwindow-main.cpp
int main() {
  PingWindow pingwindow(8, 4);
  cout<<"Nodes: 0 to 7 (8 nodes), Window size = 4"<<endl;

  for(int i=1; i<=20; i++)
    pingwindow.ingest(i, rand()%8);
```

At the end, several `query(t)` calls demonstrate the live contents of `stale` at successive times.


