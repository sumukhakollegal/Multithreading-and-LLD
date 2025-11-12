#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

// Sample Tree:
// For n = 8 bits at leaf level
//         0
//    1          2
//  3   4     5     6
// 7 8 9 10 11 12 13 14 -> 8 leaf nodes -> Total 15 nodes
// 0 1 2 3  4   5  6 7 -> Corresponding to 8 bits     

class BinaryTree{
    vector<int> Tree;
    int num_leaves;
    int num_nodes;

    int start_leaf_index(){
        return num_leaves-1;
    }

    int offset_to_index(int offset){
        return start_leaf_index()+offset;
    }

    int find_parent(int index){
        if(index == 0)
            return -1;
        return (index-1)/2;
    }

    void update_node(int index){
        int left_child = 2*index+1;
        int right_child = 2*index+2;

        Tree[index] = Tree[left_child] & Tree[right_child];

    }

    void propagate_up(int left_leaf, int right_leaf){
        set<int> parent_set;
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
    }

    public:
    BinaryTree(int n): num_leaves(n), num_nodes(2*n-1){
        Tree.resize(num_nodes);
    }

    void set_bits(int offset, int length){
        // 3,4 -> in leaf level corresponds to 10,13 index in vector
        int left_leaf = offset_to_index(offset);
        int right_leaf = offset_to_index(offset+length-1);

        for(int i=left_leaf; i<=right_leaf; i++){
            Tree[i]=1;
        }

        propagate_up(left_leaf, right_leaf);

    }

    void clear_bits(int offset, int length){
        int left_leaf = offset_to_index(offset);
        int right_leaf = offset_to_index(offset+length-1);

        for(int i=left_leaf; i<=right_leaf; i++){
            Tree[i]=0;
        }

        propagate_up(left_leaf, right_leaf);

    }

    void display(){
        int level = 0;
        int nodes_in_level =1;
        int index =0;

        while(index<num_nodes){
            cout<<"Level "<<level<<endl;
            for(int i=0; i<nodes_in_level && index < num_nodes; i++, index++){
                cout<< Tree[index] <<" ";
            }
            cout<<endl;
            level++;
            nodes_in_level *=2;
        }
        cout<<endl<<endl;
    }



};

int main(){
    BinaryTree BT(8);

    BT.set_bits(3,5);
    BT.display();

    BT.clear_bits(3,2);
    BT.display();

}