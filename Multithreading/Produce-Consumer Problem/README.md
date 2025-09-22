# Producer-Consumer Problem 🚀

A classic multithreading synchronization problem implemented using a **Bounded Blocking Queue** with modern C++ synchronization primitives.

## 📋 Problem Description

The Producer-Consumer problem is one of the fundamental challenges in concurrent programming where:

- **Producers** generate data and place it into a shared buffer
- **Consumers** remove data from the buffer and process it
- The buffer has a **limited capacity** (bounded)
- **Synchronization** is required to prevent race conditions and ensure thread safety

### Key Challenges:
- ✅ **Buffer Overflow**: Producers shouldn't add to a full buffer
- ✅ **Buffer Underflow**: Consumers shouldn't remove from an empty buffer
- ✅ **Race Conditions**: Multiple threads accessing shared data simultaneously
- ✅ **Deadlock Prevention**: Proper ordering of synchronization primitives

---

## 🏗️ Architecture Overview

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Producer 1    │    │                  │    │   Consumer 1    │
│   Producer 2    │───▶│ Bounded Blocking │───▶│   Consumer 2    │
│   Producer N    │    │      Queue       │    │   Consumer N    │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

---

## 🔧 Implementation Details

### Core Components

#### 1. **Bounded Blocking Queue Class**
```cpp
class Bounded_Blocking_Queue {
    queue<int> q;                           // Shared buffer
    int size;                               // Buffer capacity
    counting_semaphore<INT_MAX> spaces;     // Available spaces
    counting_semaphore<INT_MAX> items;      // Available items
    mutex mtx;                              // Mutual exclusion
};
```

#### 2. **Synchronization Mechanisms**

| Component | Purpose | Type |
|-----------|---------|------|
| `spaces` | Tracks available slots in buffer | `counting_semaphore` |
| `items` | Tracks items ready for consumption | `counting_semaphore` |
| `mtx` | Protects critical sections | `mutex` |

---

## 🎯 Key Code Sections Explained

### **Constructor & Initialization**
```cpp
Bounded_Blocking_Queue(int s): size(s), spaces(s), items(0){};
```
- **`spaces(s)`**: Initialize with buffer capacity (10 in our case)
- **`items(0)`**: Start with zero items in buffer

### **Producer Thread Logic**
```cpp
void producer(int threadID){
    for(int i=0; i<20; i++){
        spaces.acquire();           // 🔒 Wait for available space
        int value = rand()%10;      // 📦 Generate data
        
        unique_lock<mutex> lock(mtx); // 🔐 Lock critical section
        q.push(value);              // ➕ Add to buffer
        lock.unlock();              // 🔓 Release lock
        
        items.release();            // ✅ Signal item available
    }
}
```

**🔄 Producer Flow:**
1. **Acquire Space**: Wait if buffer is full
2. **Generate Data**: Create item to produce
3. **Critical Section**: Atomically add to buffer
4. **Signal**: Notify consumers of new item

### **Consumer Thread Logic**
```cpp
void consumer(int threadID){
    for(int i=0; i<20; i++){
        items.acquire();            // 🔒 Wait for available item
        
        unique_lock<mutex> lock(mtx); // 🔐 Lock critical section
        int value = q.front();      // 👀 Get item
        q.pop();                    // ➖ Remove from buffer
        lock.unlock();              // 🔓 Release lock
        
        spaces.release();           // ✅ Signal space available
    }
}
```

**🔄 Consumer Flow:**
1. **Acquire Item**: Wait if buffer is empty
2. **Critical Section**: Atomically remove from buffer
3. **Process**: Handle the consumed item
4. **Signal**: Notify producers of available space

---

## 🚦 Synchronization Strategy

### **Semaphore Ordering** (Critical!)
```cpp
// Producer: spaces → mutex → items
spaces.acquire();    // 1️⃣ First
{mutex operations}   // 2️⃣ Second  
items.release();     // 3️⃣ Third

// Consumer: items → mutex → spaces  
items.acquire();     // 1️⃣ First
{mutex operations}   // 2️⃣ Second
spaces.release();    // 3️⃣ Third
```

**⚠️ Why This Order Matters:**
- Prevents **deadlock** scenarios
- Ensures **progress** is always possible
- Maintains **liveness** properties

---

## 🎮 Usage & Execution

### **Thread Creation**
```cpp
vector<thread> threads;
for(int i=0; i<4; i++){
    threads.emplace_back([i, &bbq]{
        if(i%2){
            bbq.producer(i);    // Odd threads = Producers
        }
        else{
            bbq.consumer(i);    // Even threads = Consumers
        }
    });
}
```

### **Expected Output Pattern**
```
Thread 1 produced 3
Thread 0 consumed 3
Thread 3 produced 7
Thread 2 consumed 7
...
```

---

## 🧪 Testing & Validation

### **Current Test Metrics:**
- **Buffer Size**: 10 items
- **Threads**: 4 (2 producers + 2 consumers)
- **Items per Thread**: 20
- **Total Operations**: 80 (40 produce + 40 consume)

### **TODO: Validation Checks**
```cpp
// TODO: Add test cases and error handling 
// ✅ No. of items from producer should be == no. of items from consumer
// ✅ Sum(producer) should be == sum(consumer)
```

---

## 🔍 Key Concepts Demonstrated

| Concept | Implementation | Benefit |
|---------|---------------|---------|
| **Bounded Buffer** | Fixed-size queue | Prevents memory overflow |
| **Blocking Operations** | Semaphore-based waiting | Automatic flow control |
| **Mutual Exclusion** | `mutex` protection | Thread-safe data access |
| **Conditional Synchronization** | Semaphore signaling | Efficient thread coordination |

---

## 🛠️ Compilation & Running

```bash
# Compile with C++20 support (for counting_semaphore)
g++ -std=c++20 -pthread producer-consumer-main.cpp -o producer_consumer

# Run the program
./producer_consumer
```

---

## 🎯 Learning Outcomes

After studying this implementation, you'll understand:

- ✅ **Semaphore-based synchronization**
- ✅ **Critical section protection**
- ✅ **Producer-consumer coordination**
- ✅ **Deadlock prevention strategies**
- ✅ **Modern C++ threading primitives**

---

## 🔗 Related Concepts

- **Monitor Pattern**
- **Readers-Writers Problem**
- **Dining Philosophers Problem**
- **Condition Variables**
- **Atomic Operations**

---

*This implementation showcases fundamental concurrent programming patterns using modern C++ synchronization primitives. Perfect for understanding thread coordination and shared resource management!* 🎉
