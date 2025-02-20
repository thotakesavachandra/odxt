#ifndef UTILS_H
#define UTILS_H

#include <bits/stdc++.h>
using namespace std;

#define ORIGINAL_DB_FILE "db6k.dat"
#define KW_UPDATES_FILE "db6k_updates.csv"
#define MKW_UPDATES_FILE "meta_db6k_updates.csv"

vector<vector<string>> read_file(string filename);

template <typename T>
void write_file(string filename, const vector<vector<T>>& content){
    ofstream file(filename);
    for(auto& row:content){
        for(auto& s:row){
            file << s << ",";
        }
        file << "\n";
    }
    file.close();
}

template <typename T>
bool check_correctness_helper(vector<T> &ra, vector<T> &rs){
	if(rs.size() < ra.size()){
		return false;
	}
	
	set<T> st(rs.begin(), rs.end());
	for(auto x:ra){
		if(st.find(x) == st.end()){
            for(auto x:rs){
                cout << x << " ";
            }
            cout << "\n";
			return false;
		}
	}
	return true;
}

template <typename T>
bool check_correctness(vector<vector<T>> &ra, vector<vector<T>> &rs){
    if(ra.size() != rs.size()){
        cout << "Failed " << ra.size() << " " << rs.size() << "\n";
        return false;
    }
    for(int i=0; i<ra.size(); i++){
        if(!check_correctness_helper(ra[i], rs[i])){
            cout << "Failed " << i << "\n";
            return false;
        }
    }
    return true;
}

long double average(string filename);

string intToStr(int num);

int strToInt(string s);

bool isPowerOfTwo(int n);

class FMap{
    map<string, int> mp;
public:
    void insert(const string& s);
    void insert(const vector<string>& vec);
    void erase(const string& s);
    void erase(const vector<string>& vec);
    vector<string> get(int limit=1);
};

class MKW_Converter{
    int bucketSize;
    bool isOptimized;
    vector<vector<int>> kws;
    vector<vector<int>> mkws;
    map<pair<int,int>, int> mp;
    vector<pair<int,int>> break_mkw_range(pair<int,int> range);
public:
    MKW_Converter(int bucketSize, bool isOptimized);
    vector<int> find_mkw(int kw);
    vector<int> find_kw(int mkw);
    vector<int> convert_query(vector<int> query);
    vector<vector<int>> bucketize_query(vector<int> query);
};

extern MKW_Converter* mdb;

#endif

