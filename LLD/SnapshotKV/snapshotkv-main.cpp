#include <exception>
#include <iostream>
#include <bits/stdc++.h>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <exception>
using namespace std;

// 1.put(key, value)
// 2.delete(key)
// 3.deleteSnapshot(snapshotID)
// 4.get(key, snapshotID) ->return value
// 5.takesnapshot() ->return snapshot ID

//                  take snapshot here -> move counter forward
// a -> apple, art, articulate,        
// b -> ball, bat

class KVSnapshots{
  
  unordered_map <string, vector< pair<int, optional<string>>>> db;
  int curr_id;

  vector<bool> snapshot_alive;

  
  void add_to_db(string key, optional<string> value){
    if(db.find(key)!=db.end() && curr_id == db[key].back().first){
          auto &arr=db[key];
          arr.back() = {curr_id, value};
        }
        else{
          db[key].push_back({curr_id, value});
        }
  };

  bool valid_snapshotID(int snapshotID){
    if(snapshotID<0 || snapshotID>=curr_id || snapshotID>=snapshot_alive.size())
      return false;
    return true;
  }

  int find(vector< pair<int, optional<string>>> arr, int id){
    int low=0;
    int high=arr.size()-1;
    int mid, ans=-1;

    // 1 2         5, find snapshot id 3
    while(low<=high){
      mid=low+(high-low)/2;
      if(arr[mid].first <= id){
        // candidate snapshot
        ans=mid;
        low=mid+1;
      }
      else {
        high=mid-1;
      }
    }

    return ans;
  }

  public:
    KVSnapshots(){curr_id=0;};

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

    void deleteKey(string key){
      try{
        if(key.empty())
          throw invalid_argument("Key given is empty!");

        if(db.find(key)==db.end())
          throw invalid_argument("Key given is doesn't exist in our KV Store!");

        add_to_db(key, nullopt);
        
      }
      catch(...){
        throw;
      }
    }

    void deleteSnapshot(int snapshotID){
      try{
        if(!valid_snapshotID(snapshotID))
          throw invalid_argument("Given snapshot ID is not valid (Doesn't exist)!");

        if(!snapshot_alive[snapshotID])
          throw runtime_error("Snapshot with Given ID was deleted!");

        snapshot_alive[snapshotID]=false;
      }
      catch(...){
        throw;
      }
    }

    string get(string key, int snapshotID){
      try{
        if(key.empty())
          throw invalid_argument("Key given is empty!");

        if(!valid_snapshotID(snapshotID))
          throw invalid_argument("Given snapshot ID is not valid (Doesn't exist)!");

        if(!snapshot_alive[snapshotID])
          throw runtime_error("Snapshot with Given ID was deleted!");

        if(db.find(key)==db.end()){
          throw invalid_argument("Key given is not in our KV Store!");
        }

        auto &arr=db[key];

        int index = find(arr, snapshotID);

        if(index == -1){
          throw runtime_error("Snapshot ID doesn't exist");
        }

        auto &opt = arr[index].second;

        if(!opt.has_value()){
          throw runtime_error("Key was deleted in given Snapshot ID!");
        }

        return opt.value();

        // 1 2 5
        // Find with snapshotID=3 | 2 was valid for 2,3,4

        
      }
      catch(...){
        throw;
      }
    }

    int takeSnapshot(){
      snapshot_alive.resize(curr_id+1);
      snapshot_alive[curr_id]=true;

      int snapshot_id = curr_id;
      curr_id++;

      return snapshot_id;
    }




};


// To execute C++, please define "int main()"
int main() {
  KVSnapshots kvstore;

  kvstore.put("a", "apple");
  int snapshot0 = kvstore.takeSnapshot();

  kvstore.put("a", "ant");
  kvstore.put("b", "ball");
  kvstore.put("d", "doll");
  int snapshot1 = kvstore.takeSnapshot();

  cout<<kvstore.get("a", 1)<<endl; 
  // Op = ant

  kvstore.put("a", "artistic");
  kvstore.put("b", "ballistic");
  int snapshot2 = kvstore.takeSnapshot();

  cout<<kvstore.get("a", 2)<<endl;
  // Op -> artistic
  cout<<kvstore.get("b", 2)<<endl;
  // Op -> ballistic

  kvstore.deleteKey("a");
  kvstore.deleteKey("b");
  int snapshot3 = kvstore.takeSnapshot();

  try{
    cout<<kvstore.get("a", snapshot3);
  }catch(exception &e){
    cout<<e.what();
  }

  kvstore.deleteSnapshot(snapshot2);
  try{
    cout<<kvstore.get("a", snapshot2);
  }catch(exception &e){
    cout<<e.what();
  }



  return 0;
}

// Snapshot KV

// #include <iostream>
// #include <optional>
// #include <bits/stdc++.h>
// #include <stdexcept>
// using namespace std;

// class SnapshotKV{
//   unordered_map<string, vector< pair<int, optional<string>>>>  db;
//   int currID = 1;
//   vector<bool> alive;

//   void updateDB(string key, optional<string> value){
//     auto &arr=db[key];

//     if(!arr.empty() && arr.back().first == currID){
//         arr.back().second=value;
//     }
//     else{
//       arr.push_back({currID, value});
//     }
//   }

//   bool isIDValid(int id){
//     return id>=1 && id<currID && id<alive.size();
//   }

//   // bool isvalidKey(string key){
//   //   if(db.find(key)==db.end())
//   //     return false;
//   //   return true;
//   // }

//   int find(vector<pair<int, optional<string>>> &arr, int id){
//     int low=0;
//     int high=arr.size()-1;
//     int mid, ans=-1;

//     while(low<=high){
//       mid=low+(high-low)/2;
//       if(arr[mid].first<=id){
//         ans=mid;
//         low=mid+1;
//       }
//       else {
//         high=mid-1;
//       }
//     }

//     return ans;
//   }

//   public:

//   void put(string key, string value){
//     try{
//       if(key.empty())
//         throw invalid_argument("Key is empty");
//       updateDB(key, value);
//     }
//     catch(...){
//       throw;
//     }
//   };

//   int takeSnapShot(){
//     alive.resize(currID+1);
//     alive[currID]=true;

//     int snapshotID = currID;
//     currID++;

//     return snapshotID;
//   }

//   void deleteKey(string key){
//     updateDB(key, nullopt);
//   }

//   string get(string key, int snapshotID){
//     try{
//       if(!isIDValid(snapshotID))
//         throw invalid_argument("snapshotID given is wrong");

//       // if(!isvalidKey(key))
//       //   throw invalid_argument("There is no given key in snapshotID provided");

//       if(!alive[snapshotID])
//         throw runtime_error("snapshot ID given has been deleted!");

//       auto it=db.find(key);
//       if(it==db.end())
//         throw runtime_error("Key not present in given snapshot id");

//       auto &version=it->second;
//       int index=find(version, snapshotID);

//       if(index==-1)
//         throw runtime_error("Key not present in given snapshot id");

//       auto &opt=version[index].second;

//       if(!opt.has_value())
//         throw runtime_error("Key in the given snapshotID was deleted!");

//       return opt.value();

//     }
//     catch(...){
//       throw;
//     }
//   };

//   void deleteSnapshot(int snapshotID){
//     try{
//       if(!isIDValid(snapshotID))
//         throw invalid_argument("snapshotID given is wrong");
//       if(!alive[snapshotID])
//         throw invalid_argument("snapshotID already deleted!");

//       alive[snapshotID]=false;
//   }
//   catch(...){
//     throw;
//   }
// };

// // To execute C++, please define "int main()"
// int main() {
//   auto words = { "Hello, ", "World!", "\n" };
//   for (const char* const& word : words) {
//     cout << word;
//   }
//   return 0;
// }

// —————————————————————————————————



