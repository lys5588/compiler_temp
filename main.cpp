#include <iostream>
#include<fstream>
#include<string.h>
#include <stdlib.h>
#include<vector>
#include<direct.h>
#include<map>
#include <stack>
#include<regex>
#include<algorithm>

#define SYNTEX_EMPTY '`'
#define PRED_TABLE_EMPTY_SYNTEX "~"
static void ParseS();
static void ParseH();
static void ParseK();
static void ParseM();
static void ParseL();



using namespace std;



struct Time {
    int hour;
    int minute;
    int second;
    int msecond;
};

struct Subt {
    int sid;
    Time begin;
    Time end;
    string content;
    string time_str;
    vector<pair<char,string> > time_pair_arr;
};

vector<Subt> subt_array;

struct firstnode {
    char syntex;
    vector<char> ch_arr;
};
//first
vector<firstnode> FIRST;

struct follownode {
    char syntex;
    vector<char> ch_arr;
};
//follow
vector<follownode> FOLLOW;

struct grammernode {
    char syntex;
    vector<string> gene_str;
};
//文法数组
vector<grammernode> GRAMMER;

struct hashnode {
    char syntex;
    int first_index;
    int follow_index;
    int grammar_index;
    int pred_table_index;
};
//映射表
map<char, hashnode> syntex_hashmap;
map<char, int> pred_table_hashmap;

//预测表
vector<vector<string> > pred_table;

void cal_mathopoly(vector<pair<char,string>> pair_arr,string time_str){

}

void input_caption_byfile() {
    ifstream fin;
    string gra_str;
    string line;

    //    ofstream out("./2.txt");
    //    out << "hallo" << endl;

    //字幕文件读取
    fin.open("C:\\Users\\Administrator\\CLionProjects\\cpractise\\caption.txt");
    if (fin) // 有该文件
    {
        bool reach_end = false;
        int sid = -1;
        string time_str;
        string caption;

        while (!reach_end) {
            if (!getline(fin, line)) {
                reach_end=true;
                continue;
            }
            sid=stoi(line);
            getline(fin, time_str);
            caption="";
            while(true){
                getline(fin, line);
                if(line.size()==0){
                    break;
                }
                caption.append(line);
            }
            Subt subt_temp;
            subt_temp.sid=sid;
            subt_temp.time_str=time_str;
            subt_temp.content=caption;
            //处理得到词法数组
            subt_temp.time_pair_arr.push_back(make_pair('t',time_str.substr(0,2)));
            subt_temp.time_pair_arr.push_back(make_pair('c',time_str.substr(2,3)));
            subt_temp.time_pair_arr.push_back(make_pair('t',time_str.substr(3,5)));
            subt_temp.time_pair_arr.push_back(make_pair('c',time_str.substr(5,6)));
            subt_temp.time_pair_arr.push_back(make_pair('t',time_str.substr(6,8)));
            subt_temp.time_pair_arr.push_back(make_pair('d',time_str.substr(8,9)));
            subt_temp.time_pair_arr.push_back(make_pair('t',time_str.substr(9,12)));

            subt_temp.time_pair_arr.push_back(make_pair('p',time_str.substr(13,16)));

            subt_temp.time_pair_arr.push_back(make_pair('t',time_str.substr(17,19)));
            subt_temp.time_pair_arr.push_back(make_pair('c',time_str.substr(19,20)));
            subt_temp.time_pair_arr.push_back(make_pair('t',time_str.substr(20,22)));
            subt_temp.time_pair_arr.push_back(make_pair('c',time_str.substr(22,23)));
            subt_temp.time_pair_arr.push_back(make_pair('t',time_str.substr(23,25)));
            subt_temp.time_pair_arr.push_back(make_pair('d',time_str.substr(25,26)));
            subt_temp.time_pair_arr.push_back(make_pair('t',time_str.substr(26,29)));

            subt_array.push_back(subt_temp);
        }

    } else // 没有该文件
    {
        cout << "no such file" <<
             endl;
    }
};

void add_grammer_rule(string str) {
    char head = str[0];
    hashnode hashnode_temp = hashnode{head, -1, -1, -1, -1};

    cout << head << endl;
    smatch match;
    regex reg_body("\\|[a-zA-Z`]*\\|");
    vector<string> gene_str;
    //    for(sregex_iterator it(str.begin(),str.end(),reg_body), end;it!=end;++it)
    //    {
    //        string str_temp=it->str();
    //        gene_str.push_back(str_temp.substr(1,str_temp.size()-1));
    //        cout<<str_temp<<endl;
    //        cout<<str_temp.substr(0,str_temp.size()-2)<<endl;
    //    }
    string::const_iterator iter_begin = str.begin();
    string::const_iterator iter_end = str.end();
    while (regex_search(iter_begin, iter_end, match, reg_body)) {
        //       cout<<match[0]<<endl;
        iter_begin = match[0].second - 1;
        string temp_str = match[0];
        gene_str.push_back(temp_str.substr(1, temp_str.size() - 2));
        //       cout<<temp_str.substr(1,temp_str.size()-2)<<endl;
    }
    grammernode grammernnode_temp = grammernode{head, gene_str};
    hashnode_temp.grammar_index = GRAMMER.size();
    GRAMMER.push_back(grammernnode_temp);
    syntex_hashmap[head] = hashnode_temp;


}

void input_grammar_byfile() {
    ifstream fin;
    string gra_str;
    string line;

    //    ofstream out("./2.txt");
    //    out << "hallo" << endl;

    //文法文件读取
    fin.open("C:\\Users\\Administrator\\CLionProjects\\cpractise\\grammer_test.txt");
    if (fin) // 有该文件
    {
        while (getline(fin, line)) // line中不包括每行的换行符
        {
            add_grammer_rule(line);
        }
    } else // 没有该文件
    {
        cout << "no such file" << endl;
    }
};

vector<char> First(char syntex) {
    hashnode h_node = syntex_hashmap.find(syntex)->second;
    if (h_node.first_index != -1)
        return FIRST[syntex_hashmap.find(syntex)->second.first_index].ch_arr;
    syntex_hashmap.find(syntex)->second.first_index = FIRST.size();
    vector<char> vec_first;
    firstnode f_node = firstnode{syntex, vec_first};
    FIRST.push_back(f_node);

    vector<string> gene_arr = GRAMMER[syntex_hashmap.find(syntex)->second.grammar_index].gene_str;
    for (int i = 0; i < gene_arr.size(); i++) {
        int char_index = 0;
        bool search_finished = false;
        while (!search_finished && char_index < gene_arr[i].size()) {
            char start_char = gene_arr[i][char_index];
            if (start_char - 'a' < 26 and start_char - 'a' >= 0) {
                vec_first.push_back(start_char);
                search_finished = true;
                continue;
            } else if (start_char == SYNTEX_EMPTY) {
                vec_first.push_back(SYNTEX_EMPTY);
                search_finished = true;
                continue;
            } else {
                vector<char> vec_sub = First(start_char);
                for (int j = 0; j < vec_sub.size(); j++) {
                    vec_first.push_back(vec_sub[j]);
                }
                if (find(vec_sub.begin(), vec_sub.end(), SYNTEX_EMPTY) != vec_sub.end()) {
                    char_index++;
                    continue;
                } else {
                    search_finished = true;
                    continue;
                }
            }
        }


    }
    sort(vec_first.begin(), vec_first.end());
    vec_first.erase(unique(vec_first.begin(), vec_first.end()), vec_first.end());
    FIRST[syntex_hashmap.find(syntex)->second.first_index].ch_arr = vec_first;
    return vec_first;
}

void cal_first() {
    for (int i = 0; i < GRAMMER.size(); i++) {
        if (syntex_hashmap.find(GRAMMER[i].syntex)->second.first_index != -1) {
            continue;
        } else {
            First(GRAMMER[i].syntex);
        }
    }
}

//vector<char> Follow(char syntex) {
//    hashnode h_node=syntex_hashmap.find(syntex)->second;
//    if (h_node.first_index != -1)
//        return FIRST[syntex_hashmap.find(syntex)->second.first_index].ch_arr;
//    syntex_hashmap.find(syntex)->second.first_index = FIRST.size();
//    vector<char> vec_first;
//    if(syntex==GRAMMER[0].syntex) vec_first.push_back();
//    firstnode f_node = firstnode{syntex, vec_first};
//    FIRST.push_back(f_node);
//
//    vector<string> gene_arr = GRAMMER[syntex_hashmap.find(syntex)->second.grammar_index].gene_str;
//    for (int i = 0; i < gene_arr.size(); i++) {
//        int char_index = 0;
//        bool search_finished = false;
//        while (!search_finished && char_index<gene_arr[i].size()) {
//            char start_char = gene_arr[i][char_index];
//            if (start_char - 'a' < 26 and start_char - 'a' >= 0) {
//                vec_first.push_back(start_char);
//                search_finished = true;
//                continue;
//            } else if (start_char == SYNTEX_EMPTY) {
//                vec_first.push_back(SYNTEX_EMPTY);
//                search_finished = true;
//                continue;
//            } else {
//                vector<char> vec_sub = First(start_char);
//                for (int j = 0; j < vec_sub.size(); j++) {
//                    vec_first.push_back(vec_sub[j]);
//                }
//                if (find(vec_sub.begin(), vec_sub.end(), SYNTEX_EMPTY) != vec_sub.end()) {
//                    char_index++;
//                    continue;
//                } else {
//                    search_finished = true;
//                    continue;
//                }
//            }
//        }
//
//
//    }
//    sort(vec_first.begin(), vec_first.end());
//    vec_first.erase(unique(vec_first.begin(), vec_first.end()), vec_first.end());
//    FIRST[syntex_hashmap.find(syntex)->second.first_index].ch_arr = vec_first;
//    return vec_first;
//}
//

vector<char> sub_follow(char c) {
    int follow_index = syntex_hashmap.find(c)->second.follow_index;
    for (int i = 0; i < FOLLOW[follow_index].ch_arr.size(); i++) {
        if (FOLLOW[follow_index].ch_arr[i] - 'A' >= 0 && FOLLOW[follow_index].ch_arr[i] - 'A' < 26) {
            vector<char> follow_arr = sub_follow(FOLLOW[follow_index].ch_arr[i]);
            for (int k = 0; k < follow_arr.size(); k++) {
                FOLLOW[follow_index].ch_arr.push_back(follow_arr[k]);
            }
            FOLLOW[follow_index].ch_arr.erase(FOLLOW[follow_index].ch_arr.begin() + i);
            i--;
        }
    }
    return FOLLOW[follow_index].ch_arr;
}

void cal_follow() {
    for (int i = 0; i < GRAMMER.size(); i++) {
        syntex_hashmap.find(GRAMMER[i].syntex)->second.follow_index = FOLLOW.size();
        vector<char> vec_follow;
        follownode f_node = follownode{GRAMMER[i].syntex, vec_follow};
        FOLLOW.push_back(f_node);
    }
    int start_index = syntex_hashmap.find(GRAMMER[0].syntex)->second.follow_index;
    FOLLOW[start_index].ch_arr.push_back('$');
    //搜索并添加follow项
    for (int i = 0; i < GRAMMER.size(); i++) {
        char syntex = GRAMMER[i].syntex;
        char syntex_follow_index = syntex_hashmap.find(syntex)->second.follow_index;
        for (int j = 0; j < GRAMMER.size(); j++) {

            for (int k = 0; k < GRAMMER[j].gene_str.size(); k++) {
                if (GRAMMER[j].gene_str[k][0] != SYNTEX_EMPTY &&
                    find(GRAMMER[j].gene_str[k].begin(), GRAMMER[j].gene_str[k].end(), syntex) !=
                    GRAMMER[j].gene_str[k].end()) {
                    char gene_head = GRAMMER[j].syntex;
                    string gene_body = GRAMMER[j].gene_str[k];
                    bool reached_end = false;
                    int index = 0;
                    while (!reached_end) {
                        while (index < gene_body.size() && gene_body[index] != syntex) {
                            index++;
                        }
                        if (index == gene_body.size()) {
                            reached_end = true;
                            continue;
                        }
                        //内部搜索

                        int index_sub = index + 1;
                        bool sub_searched_fin = false;
                        while (!sub_searched_fin) {
                            if (index_sub == gene_body.size()) {
                                int follow_index = syntex_hashmap.find(gene_head)->second.follow_index;
                                if (FOLLOW[follow_index].ch_arr.size() == 0) {
                                    FOLLOW[syntex_follow_index].ch_arr.push_back(FOLLOW[follow_index].syntex);
                                } else {
                                    if (gene_head != syntex) {
                                        for (int ptr = 0; ptr < FOLLOW[follow_index].ch_arr.size(); ptr++) {
                                            FOLLOW[syntex_follow_index].ch_arr.push_back(
                                                    FOLLOW[follow_index].ch_arr[ptr]);
                                        }
                                    }
                                }
                                sub_searched_fin = true;
                            }
                            if (gene_body[index_sub] - 'a' >= 0 && gene_body[index_sub] - 'a' < 26) {
                                FOLLOW[syntex_follow_index].ch_arr.push_back(gene_body[index_sub]);
                                sub_searched_fin = true;
                            }
                            if (gene_body[index_sub] - 'A' >= 0 && gene_body[index_sub] - 'A' < 26) {
                                int first_index = syntex_hashmap.find(gene_body[index_sub])->second.first_index;
                                bool has_empty_syntex = false;
                                for (int ptr = 0; ptr < FIRST[first_index].ch_arr.size(); ptr++) {
                                    if (FIRST[first_index].ch_arr[ptr] != SYNTEX_EMPTY) {
                                        FOLLOW[syntex_follow_index].ch_arr.push_back(FIRST[first_index].ch_arr[ptr]);
                                    } else {
                                        has_empty_syntex = true;
                                    }
                                }
                                if (!has_empty_syntex) {
                                    sub_searched_fin = true;
                                } else {
                                    index_sub++;
                                    continue;
                                }
                            }
                            index++;
                        }
                    }
                }
            }
        }

    }


    for (int i = 0; i < FOLLOW.size(); i++) {
        sort(FOLLOW[i].ch_arr.begin(), FOLLOW[i].ch_arr.end());
        FOLLOW[i].ch_arr.erase(unique(FOLLOW[i].ch_arr.begin(), FOLLOW[i].ch_arr.end()), FOLLOW[i].ch_arr.end());
    }

    for (int i = 0; i < FOLLOW.size(); i++) {
        sub_follow(FOLLOW[i].syntex);
    }

    for (int i = 0; i < FOLLOW.size(); i++) {
        sort(FOLLOW[i].ch_arr.begin(), FOLLOW[i].ch_arr.end());
        FOLLOW[i].ch_arr.erase(unique(FOLLOW[i].ch_arr.begin(), FOLLOW[i].ch_arr.end()), FOLLOW[i].ch_arr.end());
    }

}

//计算预测表
void cal_pred_table() {
    for (int i = 0; i < GRAMMER.size(); i++) {
        for (int j = 0; j < GRAMMER[i].gene_str.size(); j++) {
            string gene = GRAMMER[i].gene_str[j];
            for (int k = 0; k < GRAMMER[i].gene_str[j].size(); k++) {
                if (gene[k] != SYNTEX_EMPTY && gene[k] - 'a' >= 0 && gene[k] - 'a' < 26) {
                    if (pred_table_hashmap.find(gene[k]) == pred_table_hashmap.end()) {
                        pred_table_hashmap[gene[k]] = pred_table_hashmap.size() + 1;
                    }
                }
            }
        }
    }
    pred_table_hashmap['$'] = pred_table_hashmap.size() + 1;

    for (int i = 0; i < GRAMMER.size(); i++) {
        vector<string> pred_table_block;
        for (int j = 0; j < pred_table_hashmap.size() + 1; j++) {
            pred_table_block.push_back("~");
        }
        pred_table_block[0] = GRAMMER[i].syntex;
        syntex_hashmap.find(GRAMMER[i].syntex)->second.pred_table_index = pred_table.size();
        pred_table.push_back(pred_table_block);
        for (int j = 0; j < GRAMMER[i].gene_str.size(); j++) {
            bool has_syntex_empty = false;
            bool add_head_follow = false;
            char ch_present = GRAMMER[i].gene_str[j][0];
            if (ch_present == SYNTEX_EMPTY) {
                add_head_follow = true;
            } else {
                if (ch_present - 'A' >= 0 && ch_present - 'A' < 26) {
                    int first_index = syntex_hashmap.find(ch_present)->second.first_index;

                    for (int ptr = 0; ptr < FIRST[first_index].ch_arr.size(); ptr++) {
                        if (FIRST[first_index].ch_arr[ptr] == SYNTEX_EMPTY) {
                            has_syntex_empty = true;
                        } else {
                            pred_table[i][pred_table_hashmap.find(
                                    FIRST[first_index].ch_arr[ptr])->second] = GRAMMER[i].gene_str[j];
                        }
                    }
                } else {
                    pred_table[i][pred_table_hashmap.find(ch_present)->second] = GRAMMER[i].gene_str[j];
                }
            }
            if (has_syntex_empty) {
                int follow_index = syntex_hashmap.find(ch_present)->second.follow_index;
                for (int k = 0; k < FOLLOW[follow_index].ch_arr.size(); k++) {
                    pred_table[i][pred_table_hashmap.find(
                            FOLLOW[follow_index].ch_arr[k])->second] = GRAMMER[i].gene_str[j];
                }
            }
            if (add_head_follow) {
                int follow_index = syntex_hashmap.find(GRAMMER[i].syntex)->second.follow_index;
                for (int k = 0; k < FOLLOW[follow_index].ch_arr.size(); k++) {
                    pred_table[i][pred_table_hashmap.find(
                            FOLLOW[follow_index].ch_arr[k])->second] = GRAMMER[i].gene_str[j];
                }
            }
        }
    }
}

//输入文本分析
void evaluate_code() {
    string pred_empty_str = PRED_TABLE_EMPTY_SYNTEX;
    string code;
    bool fin = false;
    while (!fin) {
        stack<char> stack_code;
        stack_code.push('$');
        stack_code.push(GRAMMER[0].syntex);
        int index = 0;
        cin >> code;
        if (code[0] == '~') {
            fin = true;
            continue;
        }
        while (!stack_code.empty() && !fin) {
            if (stack_code.top() == '$') {
                cout << "finished" << endl;
                fin = true;
                continue;
            }
            if (index == code.size()) {
                cout << "error" << endl;
                fin = true;
                continue;
            }
            if (code[index] == stack_code.top()) {
                cout << "matched : " << stack_code.top() << endl;
                stack_code.pop();
                index++;
                continue;
            } else if (stack_code.top() - 'a' >= 0 && stack_code.top() - 'a' < 26) {
                cout << "error" << endl;
                fin = true;
                continue;
            } else if (stack_code.top() - 'A' >= 0 && stack_code.top() - 'A' < 26) {
                string ptr_str = pred_table[syntex_hashmap.find(
                        stack_code.top())->second.pred_table_index][pred_table_hashmap.find(code[index])->second];
                if (!equal(ptr_str.begin(), ptr_str.end(), pred_empty_str.begin(), pred_empty_str.end())) {
                    cout << stack_code.top() << " -> " << ptr_str << endl;
                    stack_code.pop();
                    for (int j = ptr_str.size() - 1; j >= 0; j--) {
                        if (ptr_str[j] != SYNTEX_EMPTY) {
                            stack_code.push(ptr_str[j]);
                        }
                    }
                }

            } else {
                cout << "error" << endl;
                fin = true;
                continue;
            }
        }
        fin = false;
    }

}

//测试程序
void syntex_process() {
    input_grammar_byfile();
    cout << "input fin" << endl;
    for (int i = 0; i < GRAMMER.size(); i++) {
        cout << GRAMMER[i].syntex << endl;
        for (int j = 0; j < GRAMMER[i].gene_str.size(); j++) {
            cout << (GRAMMER[i].gene_str)[j] << " ";
        }
        cout << endl;
    }
    cal_first();
    cout << endl << "cal_first fin" << endl;
    for (int i = 0; i < FIRST.size(); i++) {
        cout << FIRST[i].syntex << endl;
        for (int j = 0; j < FIRST[i].ch_arr.size(); j++) {
            cout << (FIRST[i].ch_arr)[j] << " ";
        }
        cout << endl;
    }
    cal_follow();
    cout << endl << "cal_follow fin" << endl;
    for (int i = 0; i < FOLLOW.size(); i++) {
        cout << FOLLOW[i].syntex << endl;
        for (int j = 0; j < FOLLOW[i].ch_arr.size(); j++) {
            cout << (FOLLOW[i].ch_arr)[j] << " ";
        }
        cout << endl;
    }

    cal_pred_table();

    cout << endl << "cal_pred_table fin" << endl;
    //    for (int j = 0; j < pred_table_hashmap.size(); j++) {
    //        cout << "      "<<pred_table_hashmap[j+1] << " ";
    //    }
    for (int i = 0; i < pred_table.size(); i++) {
        cout << pred_table[i][0] << endl;
        for (int j = 0; j < pred_table[i].size(); j++) {
            cout << pred_table[i][j] << " ";
        }
        cout << endl;
    }
}

//添加文法规

//递归下降法
char lookahead;
string syntex_str;
int length;
int index;
void MatchToken(char c){
    if(index==length){
        cout<<"syntax error!"<<endl;
        exit(0);
    }
    lookahead=syntex_str[index];
    if(lookahead==c){
        index++;
    }
    else{
        cout<<"syntax error!"<<endl;
        exit(0);
    }
}




static void ParseS(){
    switch(lookahead){
        case 'd':
            ParseM();
            ParseH();
            cout<<"finish"<<endl;
        case 'b':
            ParseM();
            ParseH();
            cout<<"finish"<<endl;
        case 'e':
            ParseM();
            ParseH();
            cout<<"finish"<<endl;
        case 'a':
            MatchToken('a');
            cout<<"finish"<<endl;
    }
}

static void ParseH(){
    switch(lookahead){
        case 'e':
            ParseL();
            ParseS();
            MatchToken('o');
    }
}

static void ParseK(){
    switch(lookahead){
        case 'd':
            MatchToken('d');
            ParseM();
            ParseL();
    }
}

static void ParseM(){
    switch(lookahead){
        case 'd':
            ParseK();
        case 'b':
            MatchToken('b');
            ParseL();
            ParseM();
    }
}
static void ParseL(){
    switch(lookahead){
        case 'e':
            MatchToken('e');
            ParseH();
            MatchToken('f');
    }
}



int main() {
//    cin>>syntex_str;
//    index=0;
//    length=syntex_str.size();
//    ParseS();


//    S:==TtT
//    T:==hcscsdm
    syntex_process();

    evaluate_code();

}


//    for(int i=0;i<GRAMMER.size();i++){
//        for(int j=0;j<GRAMMER[i].gene_str.size();j++){
//            string str=GRAMMER[i].gene_str[j];
//            for(int k=1;k<str.size();k++){
//                if(str[k-1]-'A'>=0 &&str[k-1]-'A'<26){
//                    int follow_index=syntex_hashmap.find(str[k-1])->second.follow_index;
//                    if(str[k]-'a'>=0&&str[k]-'a'<26){
//                        FOLLOW[follow_index].ch_arr.push_back(str[k]);
//                    }
//                    else if(str[k]-'A'>=0 &&str[k]-'A'<26){
//                        int first_index=syntex_hashmap.find(str[k])->second.first_index;
//                        for(int ptr=0;ptr<FIRST[first_index].ch_arr.size();ptr++){
//                            if(FIRST[first_index].ch_arr[ptr]!=SYNTEX_EMPTY){
//                                FOLLOW[follow_index].ch_arr.push_back(FIRST[first_index].ch_arr[ptr]);
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//    for(int i=0;i<FOLLOW.size();i++){
//        sort(FOLLOW[i].ch_arr.begin(), FOLLOW[i].ch_arr.end());
//        FOLLOW[i].ch_arr.erase(unique(FOLLOW[i].ch_arr.begin(), FOLLOW[i].ch_arr.end()), FOLLOW[i].ch_arr.end());
//    }
//
//    for(int i=0;i<GRAMMER.size();i++){
//        for(int j=0;j<GRAMMER[i].gene_str.size();j++){
//            string str=GRAMMER[i].gene_str[j];
//            //1.
//            for(int k=1;k<str.size();k++){
//                int first_index=syntex_hashmap.find(str[i-1])->second.first_index;
//                if(find())
//            }
//        }
//    }