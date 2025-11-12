It will be helpful to visualize this with the help of an example.

> **Problem framing:** You are tasked with implementing a data structure to efficiently manage a binary state system where parent nodes automatically reflect the state of their children. The system is modeled as a complete binary tree with `n` leaf nodes (representing bit positions `0` to `n-1`). Each node stores either `0` or `1`, and we maintain the invariant that an internal node is `1` if and only if **all** nodes in its subtree are `1`.

---

## Overview

- `bit-buddy-tree.cpp` builds a complete binary tree with `2n - 1` nodes in a flat `vector<int> Tree`.
- Leaf nodes correspond one-to-one with the memory layout in a bit buddy allocator:  
  - `1` means the block is **occupied**.  
  - `0` means the block is **free**.
- Internal nodes aggregate the state of their children, allowing fast checks for whether an entire region is fully occupied.

---

## Tree Layout

For `n = 8` leaf bits, the tree is shaped like this (array indices shown):

```
                 [0]
          /               \
       [1]                 [2]
     /     \            /       \
   [3]     [4]        [5]       [6]
  /  \    /  \       /  \      /  \
[7][8] [9][10]    [11][12]  [13][14]
```

- Leaves `[7]` through `[14]` correspond directly to the bit positions `0` through `7`.
- Because the tree is stored in an array:
  - `left_child(idx) = 2 * idx + 1`
  - `right_child(idx) = 2 * idx + 2`
  - `parent(idx) = floor((idx - 1) / 2)`

---

## Memory-State Diagram

When you set `BT.set_bits(3, 5)`, we mark bit positions `3` through `7` as occupied. The leaf layer looks like this:

```
Leaf index:   0 1 2 3 4 5 6 7
Tree index:   7 8 9 10 11 12 13 14
State:        0 0 0 1 1 1 1 1
```

The visualization of the entire tree after propagation:

```
Level 0:                        0
                              /   \
Level 1:                    0       1
                           / \     /  \
Level 2:                 0     1  1     1
                        / \   / \ / \  / \
Level 3 (leaves):      0   0 0  1 1 1 1   1  
```

Only clusters where both children are `1` bubble up as `1`, so internal nodes on the left stay `0`.

---

## Understanding `propagate_up`

```51:86:LLD/Bit-Buddy Allocator Tree/bit-buddy-tree.cpp
        for(int i=left_leaf; i<=right_leaf; i++){
            int curr = i;
            while(curr>0){
                int parent = find_parent(curr);
                parent_set.insert(parent);
                curr= parent;
            }
        }

        vector<int> sorted_parents(parent_set.begin(), parent_set.end());
        sort(sorted_parents.rbegin(), sorted_parents.rend());

        for(auto &node: sorted_parents){
            update_node(node);
        }
```

- We walk up from every modified leaf to its ancestors, collecting unique parent indices in a `set`.
- The parents are then sorted in **descending** order (highest indices first). In an array-backed binary tree, higher indices correspond to nodes lower in the tree (closer to the leaves). Updating lower-level parents before higher-level ones ensures that when we compute a parent's value, both of its children are already up-to-date.
- Finally, `update_node` recomputes the AND of each node’s children, fulfilling the invariant.

---

## Key Operations

- **`set_bits(offset, length)`**: Marks a contiguous range of leaves as occupied (`1`) and propagates the change upward.
- **`clear_bits(offset, length)`**: Marks the same range as free (`0`) and recomputes affected parents.
- **`display()`**: Prints each tree level to help visualize state transitions during debugging or demos.

---

## Sample Usage

```cpp
int main(){
    BinaryTree BT(8);

    BT.set_bits(3, 5);
    BT.display();

    BT.clear_bits(3, 2);
    BT.display();
}
```

Running this example illustrates the tree adapting as allocations (set) and deallocations (clear) happen, keeping parent nodes consistent with the memory-state encoded in the leaves.

---

## Takeaways

- Leaf nodes map directly to bit buddy memory blocks; their state is the single source of truth.
- Internal nodes act as cached summaries so you can instantly tell whether a region is fully allocated.
- Ordering parent updates from bottom to top guarantees correctness without redundant recomputation.


