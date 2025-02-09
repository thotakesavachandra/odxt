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

    query.push_back("00000006");
    query.push_back("0000001a");

    std::unordered_set<std::string> IdList;

    std::cout << "Executing ODXT Search..." << std::endl;

    ODXT_Search(&IdList,query);

    for(auto id_r:IdList){
        std::cout << id_r << std::endl;
    }

    std::cout << "Search complete!" << std::endl;

    return 0;
}

int main()
{
    std::cout << "Starting..." << std::endl;
    ODXT_SetUp_Top();
    OXT_Search_Single();
    std::cout << "Complete!" << std::endl;

    return 0;
}