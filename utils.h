#ifndef UTILS_H
#define UTILS_H

#include <bits/stdc++.h>
using namespace std;

#define ORIGINAL_DB_FILE "db6k.dat"
#define KW_UPDATES_FILE "db6k_updates.csv"
#define MKW_UPDATES_FILE "meta_db6k_updates.csv"

vector<vector<string>> read_file(string filename);

template <typename T>
void write_file(string filename, const vector<vector<T>>& content);

template <typename T>
bool check_correctness_helper(vector<T> &ra, vector<T> &rs);

template <typename T>
bool check_correctness(vector<vector<T>> &ra, vector<vector<T>> &rs);

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
    int nBuckets, bucketSize;
    bool isOptimized;
    map<string, int> mp;
    vector<string> mkws;
    inline string construct(int bucket, int i, int j);
    vector<pair<int,int>> break_mkw_range(pair<int,int> range);
public:
    MKW_Converter(int nBuckets, int bucketSize, bool isOptimized=false);
    vector<int> find_kw(int kw);
    vector<int> find_mkw(int mkw);
    vector<int> convert_query(vector<int> query);
    vector<vector<int>> bucketize_query(vector<int> query);
};

extern MKW_Converter* mdb;

#endif

