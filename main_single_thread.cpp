#define _GNU_SOURCE

#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include <set>
#include <algorithm>
#include <functional>

#include <unordered_set>


#include "odxt_main_single_thread.h"
#include "aes.h"

using namespace std;

#define time_t std::chrono::_V2::system_clock::time_point
#define TIME_MARKER() chrono::high_resolution_clock::now()
#define TIME_ELAPSED(start, stop) chrono::duration_cast<chrono::microseconds>(stop - start).count()

// #define TIME_START() start_marker = chrono::high_resolution_clock::now()
// #define TIME_STOP() stop_marker = chrono::high_resolution_clock::now()
// #define TIME_ELAPSED() chrono::duration_cast<chrono::microseconds>(stop_marker - start_marker).count()
// std::chrono::_V2::system_clock::time_point start_marker, stop_marker;

string widxdb_file = "widrxdb.csv";//Raw enron database
string subdir_name;

mpz_class Prime{"7237005577332262213973186563042994240857116359379907606001950938285454250989",10};//Curve25519 curve order
mpz_class InvExp{"7237005577332262213973186563042994240857116359379907606001950938285454250987",10};

unsigned char ecc_basep[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09};

unsigned char KS[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
unsigned char KX[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3D};
unsigned char KY[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3E};
unsigned char KZ[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3F};
unsigned char KT[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};

std::map<std::string,unsigned int> update_count;

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
    return ans;
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

int ODXT_SetUp_Top()
{
    cout << "Executing ODXT setup..." << endl;
    string update_count_file = "update_count.csv";
    
    // check for existing setup (check whether the updatecnt file exists)
    if(filesystem::exists(update_count_file)){
        cout << "Loading existing update count..." << endl;
        update_count.clear();
        auto content = read_file(update_count_file);
        for(auto& row:content){
            update_count[row[0]] = stoi(row[1]);
        }
    }
    else{
        cout << "Creating new update count..." << endl;
        ifstream widxdb_file_handle;
        widxdb_file_handle.open(widxdb_file,ios_base::in|ios_base::binary);

        stringstream ss;

        string widxdb_row;
        vector<string> widxdb_data;
        string widxdb_row_current;
        string s;

        unsigned int kw_count = 0;

        std::string w;
        std::string id;

        widxdb_row.clear();
        while(getline(widxdb_file_handle,widxdb_row)){
            widxdb_data.push_back(widxdb_row);
            widxdb_row.clear();
        }
        cout << "Number of Keywords: " << widxdb_data.size() << endl;

        widxdb_file_handle.close();

        Sys_Init();
        
        ODXT_Setup();

        cout << "Executing ODXT Update..." << endl;

        for(auto v:widxdb_data){
            widxdb_row_current = widxdb_data.at(kw_count);

            ss.str(std::string());
            ss << widxdb_row_current;
            std::getline(ss,s,',');//Get the keyword

            w.clear();
            w = s.substr(0,8);

            while(std::getline(ss,s,',') && !ss.eof()) {
                if(!s.empty()){
                    id.clear();
                    id = s.substr(0,8);
                    ODXT_Update(w,id,0x01);//0x01 == ADD, 0x00 == DEL
                    // cout << "Update: " << w << " " << id << endl;
                }
            }

            ss.clear();
            ss.seekg(0);

            kw_count++;
        }
        vector<vector<string>> update_count_vec;
        for(auto& kv:update_count){
            update_count_vec.push_back({kv.first, to_string(kv.second)});
        }
        write_file(update_count_file, update_count_vec);

        Sys_Clear();
    }

    cout << "Update complete!" << endl;

    return 0;
}

int OXT_Search_Single()
{   
    std::vector<std::string> query;

    query.push_back("00003e7b");
    query.push_back("00003b7e");

    std::unordered_set<std::string> IdList;

    std::cout << "Executing ODXT Search..." << std::endl;

    ODXT_Search(&IdList,query);

    for(auto id_r:IdList){
        std::cout << id_r << std::endl;
    }

    std::cout << "Search complete!" << std::endl;

    return 0;
}



void ODXT_Search(){
    string tv_file = "./test_vectors/" + subdir_name + "/testvector.csv";
    string bv_file = "./test_vectors/" + subdir_name + "/binvector.csv";
    string act_res_file = "./test_vectors/" + subdir_name + "/actual_result.csv";
    string odxt_res_file = "./results/" + subdir_name + "/odxt_result.csv";
    string precision_file = "./results/" + subdir_name + "/practical_precision.csv";
    string query_time_file = "./results/" + subdir_name + "/query_time.csv";

    auto tv = read_file(tv_file);
    auto bv = read_file(bv_file);

    vector<vector<string>> res;
    vector<vector<string>> query_time;
    for(int i=0; i<bv.size(); i++){
        auto tv_line = tv[i];
        auto bv_line = bv[i];
        set<string> res_line;
        auto start = TIME_MARKER();
        for(int start=0, end=0; end<bv_line.size(); start=end){
            vector<string> query;
            while(end<bv_line.size() && bv_line[end]==bv_line[start]){
                query.push_back(tv_line[end]);
                end++;
            }
            unordered_set<string> temp;
            // cout << "Querying: {";
            // for(auto x:query){
            //     cout << x << " ";
            // }
            // cout << "}\n";
            ODXT_Search(&temp, query);
            
            // for(auto x:temp){
            //     cout << "\t" << x << "\n";
            // }
            res_line.insert(temp.begin(), temp.end());
        }
        auto stop = TIME_MARKER();
        query_time.push_back({to_string(TIME_ELAPSED(start, stop))});
        res.push_back(vector<string>(res_line.begin(), res_line.end()));
    }

    vector<vector<string>> act_res = read_file(act_res_file);
    if(check_correctness(act_res, res)){
        cout << "Correct\n";
    }
    else{
        cout << "Incorrect\n";
    }
    vector<vector<long double>> precision;
    for(int i=0; i<act_res.size(); i++){
        precision.push_back({(long double) act_res[i].size()/res[i].size()});
    }

    write_file(odxt_res_file, res);    
    write_file(precision_file, precision);
    write_file(query_time_file, query_time);
}

long double average(string filename){
    auto content = read_file(filename);
    long double sum = 0;
    for(auto& row:content){
        sum += stold(row[0]);
    }
    return sum/content.size();
}

int main(int argc, char *argv[])
{
    if(argc != 2){
        cout << "Usage: " << argv[0] << " <test_subdir_name>\n";
        return 1;
    }
    subdir_name = argv[1];
    widxdb_file = "./test_vectors/"+ subdir_name + "/meta_db6k.dat";
    filesystem::create_directories("./results/" + subdir_name);
    
    std::cout << "Starting..." << std::endl;

    time_t start, stop;
    start = TIME_MARKER();
        ODXT_SetUp_Top();
    stop = TIME_MARKER();

    auto setup_time = TIME_ELAPSED(start, stop);
    cout << "Setup time: " << setup_time << " microseconds\n";

    start = TIME_MARKER();
        ODXT_Search();
    stop = TIME_MARKER();

    auto search_time = TIME_ELAPSED(start, stop);
    cout << "Search time: " << search_time << " microseconds\n";

    string query_time_file = "./results/" + subdir_name + "/query_time.csv";
    string precision_file = "./results/" + subdir_name + "/practical_precision.csv";
    string stats_file = "./results/" + subdir_name + "/res_stats.csv";
    
    vector<vector<string>> stats_content;
    stats_content.push_back({to_string(setup_time)});
    stats_content.push_back({to_string(search_time)});
    stats_content.push_back({to_string(average(query_time_file))});
    stats_content.push_back({to_string(average(precision_file))});
    write_file(stats_file, stats_content);

    std::cout << "Complete!" << std::endl;

    return 0;
}