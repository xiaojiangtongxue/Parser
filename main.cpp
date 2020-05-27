#include <iostream>
#include <direct.h>
#include <vector>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <stdlib.h>

using namespace std;
#define MaxSize 100

//每一个单个产生式的数据结构
typedef struct {
    string element[MaxSize]; //每一个“|”分隔开的单个式子的集合
    int length;//或运算中所有式子的数量
} Productions;
typedef struct{
    char elem[MaxSize];
    int length;
    bool done;
}First,Follow;

typedef struct {
    char Non[MaxSize];//非终结符
    int NonNum;//非终结符数量
    int TerNum;//终结符数量
    char Ter[MaxSize];//终结符
    Productions Produ[MaxSize]; //产生式的有限集合 顺序与非终结符顺序一一对应
    int producNum;//产生式数量
    char start; //文法开始符号
    First first[MaxSize]; //二维数组 对应每个非终结符的first/follow
    Follow follow[MaxSize];
    string predAnal[MaxSize][MaxSize];
} CFG;

vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)){
        tokens.push_back(token);
    }
    return tokens;
}
//在char数组中判断特定元素是否存在
int isExist(char c,char chars[MaxSize],int length){
    for (int i = 0; i < length; ++i) {
        if(chars[i] == c)
            return i;
    }
    return -1;
}
int isExist(char c,string s){
    for (int i = 0; i < s.size(); ++i) {
        if(s[i] == c)
            return i;
    }
    return -1;
}
//存入已知起点的单个的产生式（或运算中的单个因子）
void saveNewProduction(CFG &cfg, string s,char start){
    //先将式子中新出现的字母存入终结符
    for (int k = 0; k < s.size(); ++k) {
        if(s[k] == '@')
            continue;
        if( isExist(s[k],cfg.Ter,cfg.TerNum)<0 && isExist(s[k],cfg.Non,cfg.NonNum)<0 ){
            cfg.Ter[cfg.TerNum++] = s[k];
        }
    }
    //对比该产生式的开始符号是否已在非终结符集合中存在，如果存在则合并入已存在的产生式中
    for (int i = 0; i < cfg.NonNum; ++i) {
        if(start == cfg.Non[i]){
            //存在还要保证该式子不重复，重复的话直接退出该式子，不重复则合并入
            for (int j = 0; j < cfg.Produ[i].length; ++j) {
                if(!s.compare(cfg.Produ[i].element[j]))
                    return;
            }
            cfg.Produ[i].element[cfg.Produ[i].length] += s;
            cfg.Produ[i].length++;
            return;
        }
    }
    //如果该产生式的开始符号不存在于非终结符集合 要存入cfg该式子中各项终结符，非终结符以及产生式
    //同时查找该开始符号是否在终结符集合中，如果是要拿出来
    if(isExist(start,cfg.Ter,cfg.TerNum) >= 0){
        for (int i = isExist(start,cfg.Ter,cfg.TerNum); i < cfg.TerNum-1; ++i) {
            cfg.Ter[i] = cfg.Ter[i+1];
        }
        cfg.TerNum--;
    }
    cfg.Non[cfg.NonNum++] = start;
    cfg.Produ[cfg.producNum++].element[0] += s;
    cfg.Produ[cfg.producNum-1].length = 1;

}

void initCFG(string s, CFG &cfg) {
    cfg.NonNum = 0;
    cfg.producNum = 0;
    cfg.TerNum = 0;

    vector<string> v = split(s,';');
    vector<string> m;
    char index;
    for(int i=0;i<=v.size()-1;i++){
        index = v[i][0];
        m = split(v[i].erase(0,2),'|');
        for (int j = 0; j < m.size(); ++j) {
            saveNewProduction(cfg,m[j],index);
        }
    }
}

void displayCFG(CFG cfg){
    cout<<"终结符数："<<cfg.TerNum<<endl;
    cout<<"终结符为：";
    for (int i = 0; i < cfg.TerNum; ++i) {
        cout<<cfg.Ter[i]<<" ";
    }
    cout<<endl;
    cout<<"非终结符数："<<cfg.NonNum<<endl;
    cout<<"非终结符为：";
    for (int i = 0; i < cfg.NonNum; ++i) {
        cout<<cfg.Non[i]<<" ";
    }
    cout<<endl;
    cout<<"产生式："<<endl;
    for (int j = 0; j < cfg.producNum; ++j) {
        cout<<cfg.Non[j]<<"->";
        for (int i = 0; i < cfg.Produ[j].length; ++i) {
            cout<<cfg.Produ[j].element[i];
            if(i < cfg.Produ[j].length-1){
                cout<<" | ";
            }
        }
        cout<<endl;
    }
    cout<<endl;
}

int newletter = 0;//从A开始

//消除左递归时候申请新字母的函数
int askNewLetter(CFG cfg){
    char letter = 'A' + newletter;
    if(isExist(letter,cfg.Non,cfg.NonNum)>=0 || isExist(letter,cfg.Ter,cfg.TerNum)>=0){
        newletter++;
        askNewLetter(cfg);
    } else{
        newletter++;
        return letter;
    }
}


//消除某一个非终结符的左递归
void elminOneLeftRecur(CFG &cfg,int i){
    int index = 0;//记录这是遇到的第几个含左递归的因子
    int at;
    for (int j = 0; j < cfg.Produ[i].length; ++j) {
        if(cfg.Produ[i].element[j][0] == cfg.Non[i]){
            cout<<cfg.Non[i]<<"->"<<cfg.Produ[i].element[j]<<"含左递归"<<endl;
            if(index == 0){
                at = askNewLetter(cfg);
                if(!at){
                    cfg.Non[cfg.NonNum] = 'A' +(newletter-1);
                } else{
                    cfg.Non[cfg.NonNum] = at;
                }
//                cfg.Non[cfg.NonNum] = askNewLetter(cfg);
                cfg.NonNum++;
                cfg.Produ[cfg.producNum++].length = 0;
            }
            cfg.Produ[cfg.producNum-1].element[cfg.Produ[cfg.producNum-1].length++]
                    = cfg.Produ[i].element[j].erase(0,1) + cfg.Non[cfg.NonNum-1];
            for (int k = j; k < cfg.Produ[i].length-1; ++k) {
                cfg.Produ[i].element[k] = cfg.Produ[i].element[k+1];
            }
            cfg.Produ[i].length--;
            j--;
            index++;
        } else if (index!=0){
            cout<<cfg.Non[i]<<"->"<<cfg.Produ[i].element[j]<<"不含左递归"<<endl;
            if(cfg.Produ[i].element[j][0] == '@')
                cfg.Produ[i].element[j] = cfg.Non[cfg.NonNum-1];
            else
                cfg.Produ[i].element[j] += cfg.Non[cfg.NonNum-1];
        } else {
            cout << cfg.Non[i] << "->" << cfg.Produ[i].element[j] << "不含左递归" << endl;
        }
        if (j == cfg.Produ[i].length-1 && index!=0){
            cfg.Produ[cfg.producNum-1].element[cfg.Produ[cfg.producNum-1].length++] = '@';
        }
    }
}

//检查有无隐式左递归，要是有化成显式左递归后进行消除左递归
void elminAllLeftRecur(CFG &cfg){
    string tail;
    for (int i = 0; i < cfg.producNum; ++i) {
        for (int j = 0; j < i; ++j) {
            for (int k = 0; k < cfg.Produ[i].length; ++k) {
                if(cfg.Produ[i].element[k][0] == cfg.Non[j]){
                    tail.append(cfg.Produ[i].element[k],1,cfg.Produ[i].element[k].size()-1);
                    if(tail.empty())
                        continue;
                    cfg.Produ[i].element[k] = cfg.Produ[j].element[0] + tail;
                    for (int l = 1; l < cfg.Produ[j].length; ++l) {
                        if(cfg.Produ[j].element[l][0] == '@')
                            cfg.Produ[j].element[l] = tail;
                        else
                            cfg.Produ[i].element[cfg.Produ[i].length] = cfg.Produ[j].element[l] + tail;
                        cfg.Produ[i].length++;
                    }
                }
            }
        }
        elminOneLeftRecur(cfg,i);

    }
}
//who.compare(start,to,withwho)
//已知两个字符串得出他们两个的最长前缀
string getLongestPremix(string str1,string str2){
    if(str1.size()*str2.size() == 0)
        return "";
//    cout<<"error1"<<endl;
    string prestr;
    for (int i = 0; i < str1.size(); ++i) {
       if(str1[i] == str2[i]){
//           cout<<"match"<<i<<endl;
           prestr += str1[i];
       }else{
//           cout<<"not match"<<i<<endl;
           return prestr;
       }
    }
    return prestr;
}
//求第i个产生式的最长前缀
string getLeftFactor(CFG &cfg,int i){
    string longest,index;
    for (int j = 0; j < cfg.Produ[i].length; ++j) {
        for (int k = j+1; k < cfg.Produ[i].length; ++k) {
            index = getLongestPremix(cfg.Produ[i].element[j],cfg.Produ[i].element[k]);
            if(longest.size() < index.size())
                longest = index;
        }
    }
    return longest;
}
//处理所有产生式的左因子
void elimnAllLeftFactor(CFG &cfg){
    string lf,index;
    char at;
    for (int i = 0; i < cfg.producNum; ++i) {
        lf = getLeftFactor(cfg,i);
        if(lf.empty())
            continue;
        at = askNewLetter(cfg);
        if(!at){
            cfg.Non[cfg.NonNum] = 'A' +(newletter-1);
        } else{
            cfg.Non[cfg.NonNum] = at;
        }
        cfg.NonNum++;
        cfg.Produ[cfg.producNum++].length = 0;
        for (int j = 0; j < cfg.Produ[i].length; ++j) {
            if(lf == index.assign(cfg.Produ[i].element[j],0,lf.size())){
                //这个if else用来处理新生成的产生式
                if(cfg.Produ[i].element[j].size()-lf.size() == 0)
                    cfg.Produ[cfg.producNum-1].element[cfg.Produ[cfg.producNum-1].length] = "@";
                else{
                    cfg.Produ[cfg.producNum-1].element[cfg.Produ[cfg.producNum-1].length].assign(cfg.Produ[i].element[j],lf.size(),cfg.Produ[i].element[j].size()-lf.size());
                }
                    cfg.Produ[cfg.producNum-1].length++;
                //这个用来处理原来的产生式
                if(j == cfg.Produ[i].length-1){
                    cfg.Produ[i].element[j] = lf + cfg.Non[cfg.NonNum-1];
                    return;
//                    cfg.Produ[i].element[j].erase();
//                    cfg.Produ[i].length--;
                }else{
                    for (int k = j; k < cfg.Produ[i].length-1; ++k) {
                        cfg.Produ[i].element[k].swap(cfg.Produ[i].element[k+1]);
                    }
                    cfg.Produ[i].length--;
                    j--;
//                    cfg.Produ[i].element[j] = cfg.Produ[i].element[cfg.Produ[i].length-1];
//                    j--;
                }

            }
            index = "";
            if(j == cfg.Produ[i].length-1){
                cfg.Produ[i].element[j+1] = lf + cfg.Non[cfg.NonNum-1];
                cfg.Produ[i].length++;
                return;
//                    cfg.Produ[i].element[j].erase();
//                    cfg.Produ[i].length--;
            }
        }
    }
}
//判断字符是否已存在，如果不存在将字符插入字符集中
void insertChar(char ch,char (&chars)[MaxSize],int &length){
    if(isExist(ch,chars,length)>=0)
        return;
    chars[length++] = ch;
}
//获取非终结符集合中下标为i的非终结符的first集
void getOneFirstSet(CFG &cfg,int i){
    if(i >= cfg.NonNum)
        return;
    cfg.first[i].done = false;
    if(!cfg.first[i].elem[0])
        cfg.first[i].length = 0;
    char temp;
    int temp2,NonNum = 0,temp3;
    bool isContinue;
    //根据其各个产生式的首字母是终结符还是非终结符：
    for (int j = 0; j < cfg.Produ[i].length; ++j) {
        temp = cfg.Produ[i].element[j][0];
        //如果是终结符或空，则直接存入first集合
        if(isExist(temp,cfg.Ter,cfg.TerNum) >=0 ||temp == '@'){
//            cout<<"插入"<<temp<<endl;
            insertChar(temp,cfg.first[i].elem,cfg.first[i].length);
        }else {
            temp2 = isExist(temp,cfg.Non,cfg.NonNum);
            //如果是非终结符 分两种情况 分别是一个非终结符+终结符 另一种情况是多个终结符+非终结符
            if(temp2>=0){
                for (int k = 0; k < cfg.Produ[i].element[j].size(); ++k) {
                    if(isExist(cfg.Produ[i].element[j][k],cfg.Non,cfg.NonNum)<0)
                        break;
                    NonNum++;
                }
                for (int k = 0; k < NonNum; ++k) {
                    temp3 = isExist(cfg.Produ[i].element[j][k],cfg.Non,cfg.NonNum);
                    isContinue = false;
                    getOneFirstSet(cfg,temp3);
                    if(cfg.first[temp3].done){
//                        cout<<"temp:"<<temp3<<endl;
                        for (int l = 0; l < cfg.first[temp3].length; ++l) {
                            if(cfg.first[temp3].elem[l] == '@'){
                                isContinue = true;
                                continue;
                            }
//                            cout<<"正在插入："<<cfg.first[temp3].elem[l]<<endl;
                            insertChar(cfg.first[temp3].elem[l],cfg.first[i].elem,cfg.first[i].length);
                        }
                    }else{
                        cout<<"error: "<<cfg.Non[temp3]<<"的求first集过程发生错误"<<endl;
                        return;
                    }
                    if(!isContinue) {
                        break;
                    }
                    if(k == NonNum-1){
                        if(cfg.Produ[i].element[j].size() > NonNum){
                            insertChar(cfg.Produ[i].element[j][NonNum],cfg.first[i].elem,cfg.first[i].length);
                        } else if (cfg.Produ[i].element[j].size() == NonNum){
                            insertChar('@',cfg.first[i].elem,cfg.first[i].length);
                        }else{
                            cout<<"error: 连续非终结符数量 > 整个产生式长度"<<endl;
                        }
                    }
                }


            }

        }
    }
    cfg.first[i].done = true;
}
void getAllFirstSet(CFG &cfg){
    for (int i = 0; i < cfg.NonNum; ++i) {
        cfg.first[i].done = false;
    }
    for (int j = 0; j < cfg.NonNum; ++j) {
        if(!cfg.first[j].done){
            getOneFirstSet(cfg,j);
        }
    }
//    getOneFirstSet(cfg,2);
}
void displayFirst(CFG cfg){
    cout<<" First      Follow"<<endl;
    for (int i = 0; i < cfg.NonNum; ++i) {
        cout<<cfg.Non[i]<<":";
        for (int j = 0; j < cfg.first[i].length; ++j) {
            cout<<cfg.first[i].elem[j];
            if(j < cfg.first[i].length-1) cout<<" ";
        }
        cout<<"       ";
        for (int j = 0; j < cfg.follow[i].length; ++j) {
            cout<<cfg.follow[i].elem[j];
            if(j < cfg.follow[i].length-1) cout<<" ";
        }
        cout<<endl;
    }
}
void displayFollow(CFG cfg){
    cout<<"     Follow集如下"<<endl;
    for (int i = 0; i < cfg.NonNum; ++i) {
        cout<<cfg.Non[i]<<":";
        for (int j = 0; j < cfg.follow[i].length; ++j) {
            cout<<cfg.follow[i].elem[j];
            if(j < cfg.follow[i].length-1) cout<<" ";
        }
        cout<<endl;
    }
}
//构造一个非终结符的Follow集
void getOneFollow(CFG &cfg,int i){
    cfg.follow[i].done = true;
//    cout<<"正在进行"<<cfg.Non[i]<<"的follow"<<endl;
    char temp;
    int temp2,temp3;
    for (int j = 0; j < cfg.producNum; ++j) {
        for (int k = 0; k < cfg.Produ[j].length; ++k) {
            temp2 = isExist(cfg.Non[i],cfg.Produ[j].element[k]);
            if(temp2 < 0) {
//                cout<<cfg.Produ[j].element[k]<<"中不含"<<cfg.Non[i]<<endl;
                continue;
            }
            else{
                //不是最后一个
                if(temp2 < cfg.Produ[j].element[k].size()-1){
//                    cout<<cfg.Produ[j].element[k]<<"1中含有"<<cfg.Non[i]<<endl;
                    temp = cfg.Produ[j].element[k][temp2+1];
                    temp3 = isExist(temp,cfg.Non,cfg.NonNum);
                    //判断如果跟在非终结符后边是非终结符
                    if(temp3 >= 0){
                        for (int l = 0; l < cfg.first[temp3].length; ++l) {
                            if(cfg.first[temp3].elem[l] == '@') continue;
                            insertChar(cfg.first[temp3].elem[l],cfg.follow[i].elem,cfg.follow[i].length);
                        }
                        if(isExist('@',cfg.first[temp3].elem,cfg.first[temp3].length) >= 0){
                            if(!cfg.follow[j].done){
                                if(cfg.Non[j] == cfg.Non[i]){
                                    continue;
                                }
                                getOneFollow(cfg,j);
                            }
                            if(!cfg.follow[j].done){
                                return;
                            }
                            for (int l = 0; l < cfg.follow[j].length; ++l) {
                                insertChar(cfg.follow[j].elem[l],cfg.follow[i].elem,cfg.follow[i].length);
                            }
                        }

                    }else{//如果跟在非终结符后边是终结符
                        insertChar(temp,cfg.follow[i].elem,cfg.follow[i].length);
                    }
                }else if(temp2 == cfg.Produ[j].element[k].size()-1){
                    if(!cfg.follow[j].done){
                        if(cfg.Non[j] == cfg.Non[i]){
                            continue;
                        }
                        getOneFollow(cfg,j);
                    }
                    if(!cfg.follow[j].done){
                        return;
                    }
                    for (int l = 0; l < cfg.follow[j].length; ++l) {
                        insertChar(cfg.follow[j].elem[l],cfg.follow[i].elem,cfg.follow[i].length);
                    }
                }
            }
        }
    }

}
//构造FOLLOW集
void getAllFollow(CFG &cfg){
    //加入#到FOLLOW(S)，其中，S是开始符号，#是输入结束标记；
    cfg.follow[0].elem[0] = '#';
    cfg.follow[0].length++;
    for (int i = 0; i < cfg.NonNum; ++i) {
        cfg.follow[i].done = false;
    }
    for (int i = 0; i < cfg.NonNum; ++i) {
        getOneFollow(cfg,i);
    }
}
void displayPreAna(CFG &cfg){
    cout<<"预测分析表如下："<<endl;
    cout<<"     ";
    for (int j = 0; j < cfg.TerNum; ++j) {
        cout<<cfg.Ter[j]<<"      ";
    }
    cout<<"#";
    for (int i = 0; i < cfg.NonNum; ++i) {
        cout<<endl;
        cout<<cfg.Non[i]<<":  " ;
        for (int j = 0; j < cfg.TerNum+1; ++j) {
            cout<<cfg.predAnal[i][j]<<"   ";
//            if(cfg.predAnal[i][j]!="error"){
//                cout<<cfg.predAnal[i][j]<<"   ";
//            }else{
//                cout<<"    ";
//            }
        }
        cout<<endl;
    }
}
//构造预测分析表
void predictAnalysis(CFG &cfg){
    for (int i = 0; i < cfg.NonNum; ++i) {
        for (int j = 0; j < cfg.TerNum+1; ++j) {
            cfg.predAnal[i][j] = "error";
        }
    }
    int nonIndex;
    //先处理First集
    for (int k = 0; k < cfg.NonNum; ++k) {
        for (int i = 0; i < cfg.first[k].length; ++i) {
            if(cfg.first[k].elem[i] == '@')
                continue;
            nonIndex = isExist(cfg.first[k].elem[i],cfg.Ter);
            //分两种情况：
            //1.某产生式只有一个非空分支且为非终结符开头，那么将该分支填入每个first对应的格子里
            //有一个非空分支有两种情况：只有一个非空分支，或者有一个非空分支和一个空分支
            //2.其他：每个产生式都至少有一个终结符开头的分支
            if((cfg.Produ[k].length == 1&&isExist(cfg.Produ[k].element[0][0],cfg.Non)>=0)
            ||(cfg.Produ[k].length == 2&&isExist(cfg.Produ[k].element[0][0],cfg.Non)>=0&&cfg.Produ[k].element[1][0]=='@')){
                //如果格子没被填进过 那么内容是error，就正常填入
                if(cfg.predAnal[k][nonIndex] == "error")
                    cfg.predAnal[k][nonIndex] = cfg.Produ[k].element[0];
                else{//如果内容不是error 那么说明这个格子里要填不止一个元素，直接报错退出
                    displayPreAna(cfg);
                    cout<<"填入"<<cfg.Produ[k].element[0]<<"和"<<cfg.predAnal[k][nonIndex]<<"在二维数组下标("<<k<<","<<nonIndex<<")撞车，直接退出"<<endl;
                    cout<<"预测分析表预测显示该文法不是LL(1)文法"<<endl;
                    return;
                }
            }else{
                for (int j = 0; j < cfg.Produ[k].length; ++j) {
                    if(cfg.first[k].elem[i] == cfg.Produ[k].element[j][0]){
                        //如果格子没被填进过 那么内容是error，就正常填入
                        if(cfg.predAnal[k][nonIndex] == "error")
                            cfg.predAnal[k][nonIndex] = cfg.Produ[k].element[j];
                        else{//如果内容不是error 那么说明这个格子里要填不止一个元素，直接报错退出
                            displayPreAna(cfg);
                            cout<<"填入"<<cfg.Produ[k].element[j]<<"和"<<cfg.predAnal[k][nonIndex]<<"在二维数组下标("<<k<<","<<nonIndex<<")撞车，直接退出"<<endl;
                            cout<<"预测分析表预测显示该文法不是LL(1)文法"<<endl;
                            return;
                        }
                    }
                }
            }
        }
    }
    //处理FOLLOW集
//    cout<<"处理FOLLOW集"<<endl;
    bool haveNull = false;
    for (int l = 0; l < cfg.NonNum; ++l) {
        for (int i = 0; i < cfg.first[l].length; ++i) {
            if(cfg.first[l].elem[i] == '@')
                haveNull = true;
        }
        if(!haveNull) continue;
        for (int i = 0; i < cfg.follow[l].length; ++i) {
            if(cfg.follow[l].elem[i] == '#'){
                nonIndex = cfg.TerNum;
            } else{
                nonIndex = isExist(cfg.follow[l].elem[i],cfg.Ter);
//                cout<<cfg.follow[l].elem[i]<<"在"<<nonIndex<<endl;
            }
            if(cfg.predAnal[l][nonIndex] == "error")
                cfg.predAnal[l][nonIndex] = '@';
            else{//如果内容不是error 那么说明这个格子里要填不止一个元素，直接报错退出
                displayPreAna(cfg);
                cout<<"填入"<<"@"<<"和"<<cfg.predAnal[l][nonIndex]<<"在二维数组下标("<<l<<","<<nonIndex<<")撞车，直接退出"<<endl;
                cout<<"预测分析表预测显示该文法不是LL(1)文法"<<endl;
                return;
            }
        }
        haveNull = false;
//        displayPreAna(cfg);
    }
    cout<<"预测分析表预测显示该文法是LL(1)文法"<<endl;
    displayPreAna(cfg);
}
//从文件读入到string里
string readFileIntoString(char * filename)
{
    ifstream ifile(filename);
//将文件读入到ostringstream对象buf中
    ostringstream buf;
    char ch;
    while(buf&&ifile.get(ch))
        buf.put(ch);
//返回与流对象buf关联的字符串
    return buf.str();
}


int main() {
    string s = "";
    CFG cfg;
//    cin>>s;
    s += readFileIntoString("C:\\Users\\Houble\\CLionProjects\\Parser\\test.txt");
//    s = "S:(T)|a+S|a;T:T,S|S";//"E:E+T|T;T:T*F|F;F:(E)|i"; //S:(T)|a+S|a;T:T,S|S
//    s = "S:Aa|b;A:Ac|Sd|@";
//    s = "A:aABj|a;B:Bb|d ";
//    s = "M:MaH|H;H:b(M)|(M)|b";
//    s = "S:AdD|@;A:aAd|@;D:DdA|b|@";
// S:ABCD;A:a|@;B:b|@;C:c|@;D:d
//S:(L)|aS|a;L:L,S|S;
    for(int k = 0; k < s.size(); ++k) {
        if((int)s[k] == 32){
            cout<<"语句中有空格，请删除空格后再进行测试。"<<endl;
            return 0;
        }else if((int)s[k] < 32||(int)s[k] > 126){
            cout<<"语句中有非法字符，请替换为ASCII码在33-126区间内符号后再进行测试。"<<endl;
            return 0;
        }
    }
    initCFG(s,cfg);
    cout<<"初始化为CFG："<<endl;
    displayCFG(cfg);
    cout<<"正在处理左递归："<<endl;
    elminAllLeftRecur(cfg);
    cout<<endl;
    cout<<"消除左递归后为："<<endl;
    displayCFG(cfg);
    cout<<"提取左因子后为："<<endl;
    elimnAllLeftFactor(cfg);
    displayCFG(cfg);
//    cout<<getLeftFactor(cfg,0);
    getAllFirstSet(cfg);
//    displayFirst(cfg);
    getAllFollow(cfg);
    displayFirst(cfg);
    predictAnalysis(cfg);
    system("pause");
    return 0;
}
