# SnapshotKV — Efficient Key-Value Store with Snapshots

## Problem Statement

Design an efficient **Key-Value Store** that supports **snapshots** with the following operations:

1. **`put(key, value)`** — Store/update a key-value pair
2. **`deleteKey(key)`** — Delete a key (marks it as deleted in current state)
3. **`get(key, snapshotID)`** — Retrieve value for a key from a specific snapshot
4. **`takeSnapshot()`** — Create a snapshot of current state and return snapshot ID
5. **`deleteSnapshot(snapshotID)`** — Delete a snapshot (mark as inactive)

### Key Challenges
- **Efficient snapshot creation** without copying entire state
- **Fast retrieval** from any historical snapshot
- **Memory optimization** to avoid storing redundant data
- **Snapshot lifecycle management** (creation, deletion, validation)

---

## Core Design: Versioned Data Structure

### Data Structure Overview
```cpp
unordered_map<string, vector<pair<int, optional<string>>>> db;
vector<bool> snapshot_alive;
int curr_id;
```

- **`db`**: Each key maps to a vector of `(snapshot_id, value)` pairs
- **`snapshot_alive`**: Tracks which snapshots are still active
- **`curr_id`**: Global counter for snapshot IDs (monotonically increasing)

### Why This Design?
- **Copy-on-write semantics**: Only store deltas (changes) per snapshot
- **Binary search optimization**: Fast lookup of values at specific snapshot
- **Memory efficient**: No data duplication across snapshots

---

## Core Operations Implementation

### 1. **`put(key, value)`** — Store/Update Key-Value Pair

```cpp
void put(string key, optional<string> value){
  try{
    if(key.empty())
      throw invalid_argument("Key given is empty!");
    add_to_db(key, value);
  }
  catch(...){
    throw;
  }
};
```

**Key Logic in `add_to_db()`:**
```cpp
void add_to_db(string key, optional<string> value){
  if(db.find(key)!=db.end() && curr_id == db[key].back().first){
    // Update existing entry for current snapshot
    auto &arr=db[key];
    arr.back() = {curr_id, value};
  }
  else{
    // Add new entry for current snapshot
    db[key].push_back({curr_id, value});
  }
};
```

**⚠️ Pitfalls to Avoid:**
- **Empty key validation**: Always check for empty keys to prevent undefined behavior
- **Snapshot ID consistency**: Ensure `curr_id` is incremented only after `takeSnapshot()`
- **Memory leaks**: Use `optional<string>` for deleted values instead of removing entries

---

### 2. **`deleteKey(key)`** — Mark Key as Deleted

```cpp
void deleteKey(string key){
  try{
    if(key.empty())
      throw invalid_argument("Key given is empty!");
    if(db.find(key)==db.end())
      throw invalid_argument("Key given doesn't exist in our KV Store!");
    add_to_db(key, nullopt);  // Mark as deleted
  }
  catch(...){
    throw;
  }
}
```

**⚠️ Pitfalls to Avoid:**
- **Logical deletion**: Use `nullopt` instead of removing entries (preserves history)
- **Key existence check**: Verify key exists before attempting deletion
- **Snapshot consistency**: Deletion affects current snapshot, not historical ones

---

### 3. **`get(key, snapshotID)`** — Retrieve Value from Snapshot

```cpp
string get(string key, int snapshotID){
  try{
    if(key.empty())
      throw invalid_argument("Key given is empty!");
    if(!valid_snapshotID(snapshotID))
      throw invalid_argument("Given snapshot ID is not valid!");
    if(!snapshot_alive[snapshotID])
      throw runtime_error("Snapshot with Given ID was deleted!");
    if(db.find(key)==db.end()){
      throw invalid_argument("Key given is not in our KV Store!");
    }

    auto &arr=db[key];
    int index = find(arr, snapshotID);  // Binary search
    
    if(index == -1){
      throw runtime_error("Snapshot ID doesn't exist");
    }
    
    auto &opt = arr[index].second;
    if(!opt.has_value()){
      throw runtime_error("Key was deleted in given Snapshot ID!");
    }
    
    return opt.value();
  }
  catch(...){
    throw;
  }
}
```

**⚠️ Pitfalls to Avoid:**
- **Multiple validation layers**: Check key, snapshot validity, and snapshot alive status
- **Binary search edge cases**: Handle empty arrays and out-of-range IDs
- **Deleted key handling**: Check `has_value()` before accessing value

---

### 4. **`takeSnapshot()`** — Create New Snapshot

```cpp
int takeSnapshot(){
  snapshot_alive.resize(curr_id+1);
  snapshot_alive[curr_id]=true;
  
  int snapshot_id = curr_id;
  curr_id++;
  
  return snapshot_id;
}
```

**⚠️ Pitfalls to Avoid:**
- **Atomic operation**: Increment `curr_id` only after marking snapshot as alive
- **Memory management**: Resize `snapshot_alive` vector as needed
- **ID uniqueness**: Never reuse snapshot IDs (use monotonically increasing counter)

---

### 5. **`deleteSnapshot(snapshotID)`** — Mark Snapshot as Deleted

```cpp
void deleteSnapshot(int snapshotID){
  try{
    if(!valid_snapshotID(snapshotID))
      throw invalid_argument("Given snapshot ID is not valid!");
    if(!snapshot_alive[snapshotID])
      throw runtime_error("Snapshot with Given ID was deleted!");
    
    snapshot_alive[snapshotID]=false;  // Logical deletion
  }
  catch(...){
    throw;
  }
}
```

**⚠️ Pitfalls to Avoid:**
- **Logical deletion**: Don't actually remove data, just mark as inactive
- **Double deletion**: Check if snapshot is already deleted
- **Data preservation**: Keep historical data for potential recovery

---

## Auxiliary Functions

### **`valid_snapshotID(int snapshotID)`** — Snapshot Validation

```cpp
bool valid_snapshotID(int snapshotID){
  if(snapshotID<0 || snapshotID>=curr_id || snapshotID>=snapshot_alive.size())
    return false;
  return true;
}
```

**Purpose**: Validates snapshot ID bounds and existence
**⚠️ Pitfalls**: 
- Check both `curr_id` and `snapshot_alive.size()` bounds
- Handle negative IDs gracefully

### **`find()` — Binary Search Implementation**

```cpp
int find(vector<pair<int, optional<string>>> arr, int id){
  int low=0;
  int high=arr.size()-1;
  int mid, ans=-1;

  while(low<=high){
    mid=low+(high-low)/2;
    if(arr[mid].first <= id){
      ans=mid;        // Candidate snapshot
      low=mid+1;
    }
    else {
      high=mid-1;
    }
  }
  return ans;
}
```

**Purpose**: Find the latest snapshot entry ≤ target snapshot ID
**Algorithm**: Binary search for largest snapshot ID ≤ target
**⚠️ Pitfalls to Avoid:**
- **Off-by-one errors**: Use `<=` comparison, not `<`
- **Empty array handling**: Check if array is empty before search
- **Integer overflow**: Use `low+(high-low)/2` instead of `(low+high)/2`
- **Edge cases**: Handle single-element arrays and boundary conditions

---

## Complexity Analysis

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| `put()` | O(1) amortized | O(1) per operation |
| `deleteKey()` | O(1) amortized | O(1) per operation |
| `get()` | O(log m) where m = versions per key | O(1) |
| `takeSnapshot()` | O(1) | O(1) |
| `deleteSnapshot()` | O(1) | O(1) |

**Where m** = number of versions stored for a specific key

---

## Example Usage

```cpp
int main() {
  KVSnapshots kvstore;

  kvstore.put("a", "apple");
  int snapshot0 = kvstore.takeSnapshot();

  kvstore.put("a", "ant");
  kvstore.put("b", "ball");
  int snapshot1 = kvstore.takeSnapshot();

  cout << kvstore.get("a", 1) << endl;  // Output: ant

  kvstore.deleteKey("a");
  int snapshot2 = kvstore.takeSnapshot();

  try{
    cout << kvstore.get("a", snapshot2);  // Throws: Key was deleted
  }catch(exception &e){
    cout << e.what();
  }
}
```

---

## Build & Run

Copy-paste in online editor 

Or to run locally:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -o snapshotkv snapshotkv-main.cpp && ./snapshotkv
```

---

## Key Design Decisions

1. **Versioned Storage**: Store only deltas per snapshot, not full copies
2. **Binary Search**: O(log m) lookup instead of linear scan
3. **Logical Deletion**: Use `optional<string>` for deleted values
4. **Snapshot Lifecycle**: Separate creation and deletion with validation
5. **Error Handling**: Comprehensive validation with descriptive error messages

This design provides efficient snapshot operations while maintaining data integrity and optimal memory usage.
