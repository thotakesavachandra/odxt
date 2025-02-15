#include "utils.h"


vector<vector<string>> read_file(string filename){
    vector<vector<string>> ans;
    ifstream file(filename);
    string line;
    while(getline(file, line, '\n')){
        ans.push_back({});
        stringstream ss(line);
        string s;
        while(getline(ss, s, ',')){
            ans.back().push_back(s);
        }
    }
    file.close();
    return move(ans);
}

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


long double average(string filename){
    auto content = read_file(filename);
    long double sum = 0;
    for(auto& row:content){
        sum += stold(row[0]);
    }
    return sum/content.size();
}


string intToStr(int num){
    stringstream ss;
    ss << hex;
    ss << setw(8) << setfill('0') << num;
    return ss.str();
}


int strToInt(string s){
    return stoi(s, nullptr, 16);
}

bool isPowerOfTwo(int n){
    return (n & (n-1)) == 0;
}

void FMap::insert(const string& s){
    mp[s]++;
}

void FMap::insert(const vector<string>& vec){
    for(auto& s:vec){
        insert(s);
    }
}

void FMap::erase(const string& s){
    if(mp.find(s) == mp.end()){
        return;
    }
    mp[s]--;
    if(mp[s] == 0){
        mp.erase(s);
    }
}

void FMap::erase(const vector<string>& vec){
    for(auto& s:vec){
        erase(s);
    }
}

vector<string> FMap::get(int limit){
    vector<string> ans;
    for(auto& p:mp){
        if(p.second >= limit){
            ans.push_back(p.first);
        }
    }
    return move(ans);
}




MKW_Converter::MKW_Converter(int nBuckets, int bucketSize, bool isOptimized):nBuckets(nBuckets), bucketSize(bucketSize), isOptimized(isOptimized){
    for(int bucket=0; bucket<nBuckets; bucket++){
        for(int i=0; i<bucketSize; i++){
            for(int j=i; j<bucketSize; j++){
                if(j-i+1 == bucketSize) continue;
                if(isOptimized && !isPowerOfTwo(j-i+1)) continue;
                auto mkw = construct(bucket, i, j);
                mp[mkw] = mkws.size();
                mkws.push_back(mkw);
            }
        }
    }
    shuffle(mkws.begin(), mkws.end(), default_random_engine(0));
    for(int i=0; i<mkws.size(); i++){
        mp[mkws[i]] = i;
    }
}

inline string MKW_Converter::construct(int bucket, int i, int j){
    return intToStr(bucket) + intToStr(i) + intToStr(j);
}


vector<pair<int,int>> MKW_Converter::break_mkw_range(pair<int,int> range){
    int i = range.first, j = range.second;
    if(!isOptimized || isPowerOfTwo(j-i+1)) return {range};
    vector<pair<int,int>> res;
    int len = j-i+1;

    // find largest power of 2 that is less than len
    int p = -1;
    while((1<<(p+1)) < len) p++;
    
    int largest_len = 1<<p;
    return {{i, i+largest_len-1}, {j-largest_len+1, j}};
}


/**
    @brief Returns keywords that are covered by the given meta-keyword
    @param mkw Chosen meta-keyword
    @return Vector of keywords
*/
vector<int> MKW_Converter::find_kw(int mkw){
    string s = mkws[mkw];
    int bucket = strToInt(s.substr(0, 8));
    int i = strToInt(s.substr(8, 8));
    int j = strToInt(s.substr(16, 8));
    vector<int> res;
    for(int idx=0; idx<bucketSize; idx++){
        if(i<=idx && idx<=j) continue;
        res.push_back(bucket*bucketSize + idx);
    }
    return std::move(res);
}


/**
    @brief Returns meta-keywords that cover the given keyword
    @param kw: Chosen keyword
    @return Vector of meta-keywords 
*/
vector<int> MKW_Converter::find_mkw(int kw){
    int bucket = kw / bucketSize;
    int idx = kw % bucketSize;
    vector<int> res;
    for(int i=0; i<bucketSize; i++){
        for(int j=i; j<bucketSize; j++){
            if(j-i+1 == bucketSize) continue;
            if(isOptimized && !isPowerOfTwo(j-i+1)) continue;
            if(i<=idx && idx<=j) continue;
            res.push_back(mp[construct(bucket, i, j)]);
        }
    }
    return std::move(res);
}


/**
    @brief Converts a given disjunctive keyword query into a conjunctive meta-keyword query
    @param query Vector of keywords
    @return Vector of meta-keywords

    Requires: All the keywords in the query are distinct but belong to the same bucket 
*/
vector<int> MKW_Converter::convert_query(vector<int> query){
    if(query.size()==0) return {};
    sort(query.begin(), query.end());
    int bucket = query[0]/bucketSize;
    for(auto& x:query){
        x = x % bucketSize;
    }
    vector<int> res;
    int prev = -1; // upto where you have already left
    query.push_back(bucketSize);
    for(auto x:query){
        // leave from prev+1 to curr-1
        int i = prev + 1;
        int j = x - 1;
        if(j-i+1 <= 0 || j-i+1 == bucketSize){}
        else{
            auto ranges = break_mkw_range({i, j});
            for(auto r:ranges){
                res.push_back(mp[construct(bucket, r.first, r.second)]);
            }
        }
        prev = x;
    }
    return std::move(res);
}


/**
    @brief Group together the keywords in the query that belong to the same bucket
    @param query Vector of keywords
    @return keywords grouped by bucket 
*/
vector<vector<int>> MKW_Converter::bucketize_query(vector<int> query){
    sort(query.begin(), query.end());
    vector<vector<int>> res;
    for(int start=0, end=0; end<query.size(); start=end){
        res.push_back({});
        while(end<query.size() && query[end]/bucketSize == query[start]/bucketSize){
            res.back().push_back(query[end]);
            end++;
        }
    }
    return std::move(res);
}



