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

string widxdb_file = "widrxdb.csv";//Raw enron database

mpz_class Prime{"7237005577332262213973186563042994240857116359379907606001950938285454250989",10};//Curve25519 curve order
mpz_class InvExp{"7237005577332262213973186563042994240857116359379907606001950938285454250987",10};

unsigned char ecc_basep[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09};

unsigned char KS[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
unsigned char KX[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3D};
unsigned char KY[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3E};
unsigned char KZ[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3F};
unsigned char KT[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};

std::map<std::string,unsigned int> update_count;

int ODXT_SetUp_Top()
{
    cout << "Executing ODXT setup..." << endl;

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

    Sys_Clear();

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

void ODXT_Search(string subdir_name){
    string tv_file = "./test_vectors/" + subdir_name + "/testvector.csv";
    string bv_file = "./test_vectors/" + subdir_name + "/binvector.csv";
    string act_res_file = "./test_vectors/" + subdir_name + "/actual_result.csv";
    string odxt_res_file = "./test_vectors/" + subdir_name + "/odxt_result.csv";

    auto tv = read_file(tv_file);
    auto bv = read_file(bv_file);

    vector<vector<string>> res;
    for(int i=0; i<bv.size(); i++){
        auto tv_line = tv[i];
        auto bv_line = bv[i];
        set<string> res_line;
        for(int start=0, end=0; end<bv_line.size(); start=end){
            vector<string> query;
            while(end<bv_line.size() && bv_line[end]==bv_line[start]){
                query.push_back(tv_line[end]);
                end++;
            }
            unordered_set<string> temp;
            ODXT_Search(&temp, query);
            res_line.insert(temp.begin(), temp.end());
        }
        res.push_back(vector<string>(res_line.begin(), res_line.end()));
    }

    vector<vector<string>> act_res = read_file(act_res_file);
    if(check_correctness(act_res, res)){
        cout << "Correct\n";
    }
    else{
        cout << "Incorrect\n";
    }

    write_file(odxt_res_file, res);    
}

int main()
{
    std::cout << "Starting..." << std::endl;
    widxdb_file = "./test_vectors/5_2/meta_db6k.dat";
    ODXT_SetUp_Top();
    // OXT_Search_Single();
    ODXT_Search("5_2");
    std::cout << "Complete!" << std::endl;

    return 0;
}