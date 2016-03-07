//headers
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <functional>
#include <numeric>
#include <utility>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <thread>   
#include <mutex>

#define vi vector<int>
#define vpii vector< pair<int,int> >
#define pii pair<int,int>
#define mp(x,y) make_pair(x,y)
#define all(x) (x).begin(),(x).end()
#define sz(x) (int)(x).size()
#define FOR(i,n) for (int i = 0; i < n; i++)
#define ROF(i,n) for (int i = n-1; i >= 0; i--)
#define gmin(a,b) { if (b < a) a = b; }
#define gmax(a,b) { if (b > a) a = b; }
#define pb push_back
typedef long long ll;
using namespace std;

//globals
hash<string> str_hash;
const int MAXSTATES = 63973 * 53 * 7 * 7;  //166 137 881
vi crypt = vi();
vi decrypt = vi();
bool killit = false;

//settings
const double precision = 0.00001; //precision of GameState evaluating 
const double maxThreads = 4; //number of threads to count on. Beware it can sometimes use one more.  

//deterine if two doubles are too far from each other
//reducing precision speeds up the computation sligtly
bool isFar(double a, double b){
	return abs(a - b) > precision || a / b > 2.0 || b / a > 2.0;  //not sure if ratio is necessary here
}

//return current time in seconds, work only relatively
double getTime(){
	return (double)clock()/(double)CLOCKS_PER_SEC;
}

//transforms three integers into one
//represents state of one animal for both players
//x,y is numbers of remaining animals and z is who took them first
//z = -1, 0, 1; x, y>0
//if z == -1 then x, y > 1
int ints2int(int x, int y, int z){
	if(z == 0){
		if(x == 0 && y == 0) return 0;
		if(x == 0 && y == 1) return 1;
		if(x == 1 && y == 0) return 2;
		if(x == 1 && y == 1) return 3;
	}else{
		if(x == 0 && y == 0) return 4;
		if(x == 0 && y == 1) return 5;
		if(x == 1 && y == 0) return 6;
		if(x == 1 && y == 1) return 7;
	} 
	if(x == y) return (x + 1) * (x + 1) + 3;
	if(y > x){
		return y*y + 4 + x;
	}else{
		return x*x + 4 + x + y;
	}
}

//reverse function of ints2int
//m is maximal number of animals
void int2ints(int x, int &a, int &b, int &c, int m){
	if(x == 0){ a = m - 0; b = m - 0; c = 0; }
	else if(x == 1){ a = m - 0; b = m - 1; c = 0; }
	else if(x == 2){ a = m - 1; b = m - 0; c = 0; }
	else if(x == 3){ a = m - 1; b = m - 1; c = 0; }
	else if(x == 4){ a = m - 0; b = m - 0; c = 1; }
	else if(x == 5){ a = m - 0; b = m - 1; c = 1; }
	else if(x == 6){ a = m - 1; b = m - 0; c = 1; }
	else if(x == 7){ a = m - 1; b = m - 1; c = 1; }
	else{
		int s = 0;
		while(s*s <= x - 4) s++;
		s--;
		x -= 4 + s*s;
		if(x < s){
			a = m - x; 
			b = m - s; 
			c =- 1;
			if(x <= 1) c = 0;
			if(s <= 1) c = 1; 
			return;
		}else{
			a = m - s; 
			b = m - x + s; 
			c =- 1;
			if(s <= 1) c = 0;
			if(x - s <= 1) c = 1; 
			return;
		}
	}
}

//maps 4-tuples into one number strating from 0
//numbers can be permutated and holds a<13, b<20, c<29, d<40
//mapping is stored in two arrays crypt[] and decrypt[]
void setCrypt(){
	int p = 0;
	crypt = vi(13 * 20 * 29 * 40, -1);
	FOR(i, 13) FOR(j, 20) FOR(k, 29) FOR(l, 40){
		vi v;
		v.pb(i);
		v.pb(j);
		v.pb(k);
		v.pb(l);
		sort(all(v));
		int num = 0;
		num = num + v[0];
		num = num * 20 + v[1];
		num = num * 29 + v[2];
		num = num * 40 + v[3];
		if(crypt[num] == -1) crypt[num] = p++;
	}
	decrypt = vi(p, -1);
	FOR(i, 13 * 20 * 29 * 40){
		if(crypt[i] != -1) decrypt[crypt[i]] = i;
	}
}

//checks if two vectors of ints contains the same numbers ( < 6)
//used to reduce options when choosing trucks
bool eq(vi &v1, vi &v2){
	if(sz(v1) != sz(v2)) return false;
	vi v = vi(6, 0);
	FOR(i, sz(v1)) v[v1[i]]++;
	FOR(i, sz(v2)) v[v2[i]]--;
	FOR(i, 6) 
		if(v[i] != 0) return false;
	return true;
}

//*************** GameChange ***************
//represents change of GameStates
//determined by contents of two trucks and who took it first
//******************************************
class GameChange{
public:
	vi truck[2];
	bool startChange; //true if players should swap at the end of round

	GameChange(){
		truck[0] = vi();
		truck[1] = vi();
		startChange = false;
	}

	//convert into integer
	int toInt(){
		sort( all( truck[0] ) );
		sort( all( truck[1] ) );
		string s = "";
		FOR(i, sz(truck[0])) s += ('1' + truck[0][i]);
		if(startChange){
			s += "8";
		}else{
			s += "9";
		}
		FOR(i, sz(truck[1])) s += ('1' + truck[1][i]);
		return atoi( s.c_str() );
	}

	//create GameChange from integer, inverse function of Gamechange.toInt()
	GameChange(int gci){
		truck[0] = vi();
		truck[1] = vi();
		
		while(gci % 10 < 8){
			int k = gci % 10 - 1; 
			truck[1].pb(k); 
			gci /= 10;
		}
		int k = gci % 10; 
		startChange = (k == 8);
		gci /= 10;
		while(gci != 0){ 
			int k = gci % 10 - 1; 
			truck[0].pb(k); 
			gci /= 10;
		}
	}

};

//*************** GameState ***************
//represents game state at at the beggining of a round
//*****************************************
class GameState{   
public:
	int taken[2][6]; //number of taken animals by both players
	int finished[5]; //who finished animals first; 
					 //-1 noone, 0 current player, 1 other player

	GameState(){
		memset(taken, 0, sizeof(taken));
		memset(finished, -1, sizeof(finished));
	}

	//output GameState in human readable form
	void print(){
		FOR(i, 2) FOR(j, 6){
			FOR(k, j + 2 - (j == 5 ? 1 : 0)){
				if(k < taken[i][j]){
					cout << j + 1;
				}else{
					cout << " ";
				}
			}
			cout << "|";
		}
		FOR(i, 5) cout << (finished[i] == 0 ? "+" : (finished[i] == 1 ? "-" : "0"));
		cout << "|" << toInt();
		cout << endl;
	} 

	//convert GameState into integer (0 to MAXSTATES)
	int toInt(){
		int num = 0;
		vi v;
		v.pb( ints2int(2 - taken[0][0], 2 - taken[1][0], finished[0]) );
		v.pb( ints2int(3 - taken[0][1], 3 - taken[1][1], finished[1]) );
		v.pb( ints2int(4 - taken[0][2], 4 - taken[1][2], finished[2]) );
		v.pb( ints2int(5 - taken[0][3], 5 - taken[1][3], finished[3]) );
		sort( all( v ) );
		num = num + v[0];
		num = num * 20 + v[1];
		num = num * 29 + v[2];
		num = num * 40 + v[3];

		num = crypt[num];
		
		num = num * 53 + ints2int(6 - taken[0][4], 6 - taken[1][4], finished[4]);
		num = num * 7 + taken[0][5];
		num = num * 7 + taken[1][5];
		return num;    
	}

	//create GameState from integer, inverse function of GameState.toInt()
	GameState(int gsi){
		memset(taken, 0, sizeof(taken));
		memset(finished, -1, sizeof(finished));
		
		taken[1][5] = gsi % 7; gsi /= 7;
		taken[0][5] = gsi % 7; gsi /= 7;
		int x;

		x = gsi % 53; gsi /= 53;
		int2ints(x, taken[0][4], taken[1][4], finished[4], 6);

		gsi = decrypt[gsi];
		x = gsi % 40; gsi /= 40;
		int2ints(x, taken[0][3], taken[1][3], finished[3], 5);

		x = gsi % 29; gsi /= 29;
		int2ints(x, taken[0][2], taken[1][2], finished[2], 4);

		x = gsi % 20; gsi /= 20;
		int2ints(x, taken[0][1], taken[1][1], finished[1], 3);

		x = gsi % 13; gsi /= 13;
		int2ints(x, taken[0][0], taken[1][0], finished[0], 2);
	}

	//swap players, eg. change who starts in upcoming round
	GameState swap(){
		GameState gs = *this;
		std::swap(gs.taken[0], gs.taken[1]);
		FOR(i, 5){
			if(gs.finished[i] != -1) gs.finished[i] = 1 - gs.finished[i];
		}
		return gs;
	}

	//apply GameChange and return new GameState
	//GameState remain unmodified
	GameState add(GameChange gc){
		GameState gs2 = *this;

		FOR(i, 2){
			FOR(j, sz(gc.truck[i])){
				int dice = gc.truck[i][j];
				gs2.taken[i][dice]++;
				gmin(gs2.taken[i][dice], dice + 2 - (dice == 5 ? 1 : 0))
			}
		}

		if(gc.startChange == false){
			ROF(i, 2) FOR(j, 5){
				if(gs2.taken[i][j] >= j + 1 - (j == 5 ? 1 : 0) && gs2.finished[j] == -1) 
					gs2.finished[j] = i;
			}
		}else{
			FOR(i, 2) FOR(j, 5){
				if(gs2.taken[i][j] >= j + 1 - (j == 5 ? 1 : 0) && gs2.finished[j] == -1) 
					gs2.finished[j] = i;
			}
		}

		if(gc.startChange){
			return gs2.swap();
		}
		return gs2;
	}
	GameState add(int gci){
		return add(GameChange(gci));
	}

	//create GameState from a string
	//0123451030061-1---
	GameState(string gss){
		memset(taken, 0, sizeof(taken));
		memset(finished, -1, sizeof(finished));
		int k = 0;
		FOR(i, 2) FOR(j ,6){
			taken[i][j] = gss[k++] - '0';
		}
		FOR(i, 5){
			if(gss[k] != '0' && gss[k] != '1'){
				finished[i] = -1;
			}else{
				finished[i] = gss[k] - '0';
			}
			k++;
		}
	}

	//get number of point of player p (0 or 1)
	int getPoints(int p){
		int points = 0;
		int barns = 0;
		FOR(j, 5){
			points += std::min(taken[p][j], j+1);
			if(taken[p][j] > j + 1) barns++;
		}

		if(taken[p][5] >= 6){
			barns -= 3;
		}else if(taken[p][5] >= 5){
			barns -= 2;
		}else if(taken[p][5] >= 3){
			barns -= 1;
		}
		
		if(barns > 0){
			points -= 2 * barns;
		}else{
			points -= barns;
		}            

		FOR(j, 5)
			if(finished[j] == p) points += (j == 4 ? 2 : 1);
		return points;
	}

	//determine who wins in this GameState
	//-1: noone, game not finished yet
	//0:  player 0
	//1:  player 1
	//2:  draw
	int winning(){
		int finished[] = {0, 0};  //number of finished animals by both players
		FOR(i, 2) FOR(j,5){
			if(taken[i][j] >= j+1) finished[i]++;
		}
		if(finished[0] < 4 && finished[1] < 4) return -1; //not finished yet

		int points[] = { getPoints(0), getPoints(1) };
		if(points[0] != points[1]) return (points[0] > points[1] ? 0 : 1);
		if(taken[0][5] != taken[1][5]) return (taken[0][5] > taken[1][5] ? 0 : 1);
		return 2; //draw
	}

	//determine if there is some GameChange that doesnt affect this GameState
	//this happens only if both players have at least one animal at full capacity
	bool hasSelf(){
		int full[] = {0, 0};
		FOR(i, 2) FOR(j, 6){
			if(taken[i][j] > j + 1 - (j == 5 ? 1 : 0)) full[i] = 1;
		}
		return full[0] == 1 && full[1] == 1;
	}

	//returns on number of all animals taken
	//determines order in which we will count GameStates
	int sum(){
		int num = 0;
		FOR(i, 2) FOR(j, 6){
			num += taken[i][j];
		}
		return num;
	}
};

//*************** RoundState ***************
//represents current round
//******************************************
class RoundState{
public:
	vector <vi> trucks;  //filled trucks
	int dices;  //remaining dices
	vector <vi> taken; //taken animals
	vi rolled;  //what two numbers have been rolled
	int tookFirst;  //who took truck first; -1: noone, 0: player 0, 1: player 1

	RoundState(){
		trucks = vector<vi>(3, vi());
		taken = vector<vi>(2, vi());
		rolled = vi();
		dices = 6;
		tookFirst = -1;
	}
	
	//create wen RoundState from string
	RoundState(string s){
		dices = s[0] - '0';
		rolled = vi();
		if(s[1] != '0') rolled.pb(s[1] - '1');
		if(s[2] != '0') rolled.pb(s[2] - '1');
		trucks = vector<vi>(3, vi());
		FOR(i, 3){
			FOR(j, 3){
				if(s[3 + i*3 + j] != '0'){
					trucks[i].pb(s[3 + i*3 + j] - '1');
				}
			}
		}
		tookFirst = -1;
		taken = vector<vi>(2, vi());
		FOR(i, 2){
			FOR(j, 3){
				if(s[12 + i*3 + j] != '0'){
					taken[i].pb(s[12 + i*3 + j] - '1');
					tookFirst = i;
				}
			}
		}
	}
};

//*************** DataBase ***************
//stores values of all GameStates
//it takes around 1,3GB of RAM
//since GameStates can be represented uniquely as integers
//  it only contains array of values
//-1.0 -> unknown else value between 0 and 1
//****************************************
class DataBase{
public:
	double* mem;  //dynamic array representing whole actual DB
	vector <int> added;  //all counted GameStates

	DataBase(){
		mem = new double[MAXSTATES];
		FOR(gsi, MAXSTATES) mem[gsi] = -1.0; 
	}

	//returns value of GameState
	double get(GameState gs){
		return get(gs.toInt());        
	}
	double get(int gsi){
		return mem[gsi];
	}

	//save value of GameState
	void put(GameState gs, double d){
		put(gs.toInt(), d);
	}
	void put(int gsi, double d){
		if(mem[gsi] != d) added.pb(gsi);
		mem[gsi] = d;
	}

	//saves whole BD into BD.dat file
	void save(){
		ofstream fout; 
		fout.open("DB.dat", ios::binary);
		FOR(gsi, MAXSTATES)
			fout.write(reinterpret_cast<char *>(&mem[gsi]), sizeof(mem[gsi]));
		fout.close();
		cout<<"DB saved."<<endl;
	}

	//saves only counted GameStates, one per line
	void saveDiff(){
		ofstream fout;
		stringstream filename;
		filename<<"computed.add";
		fout.open(filename.str().c_str(), ios::app);
		FOR(i, sz(added)){
			fout<<added[i]<<" "<<mem[added[i]]<<endl;
		}
		added.clear();
		fout.close();
		cout<<"DBDiff saved."<<endl;
	}

	//load DB from DB.dat
	void load(){
		cout<<"Loading DB... ";
		ifstream fin; 
		fin.open("DB.dat", ios::binary | ios::in);
		FOR(gsi, MAXSTATES)
			fin.read(reinterpret_cast<char *>(&mem[gsi]), sizeof(mem[gsi]));
		fin.close();
		cout<<"done."<<endl;
	}

	//load contents of .add file, save them into DB
	void loadDiff(string filename){
		cout<<"DBDiff loading... "<<endl;
		ifstream fin; 
		fin.open(filename, ios::in);
		int changed = 0;
		int same = 0;
		int gsi; double d;
		while(fin>>gsi && fin>>d){
			if(gsi >= MAXSTATES){
				cout<<"ERROR, GameSate integer is too large: "<<gsi<<endl;
				continue;
			}
			if(!isFar(mem[gsi], d)){
				same++;
			}else{
				changed++;
				if(mem[gsi] >= 0)
					cout<<"WARNING, gsi:"<<gsi<<", in BD: "<<mem[gsi]<<", in file: "<<d<<endl;
				put(gsi, d);
			}
		}
		fin.close();
		cout<<"DBDiff loaded. Changed: "<<changed<<", not changed: "<<same<<endl;
	}
	
	//create new DB, save ended games
	void init(){
		cout<<"DB initing... ";
		FOR(gsi, MAXSTATES){
			GameState gs(gsi);
			int w = gs.winning();
			if(w == 2) mem[gsi] = 0.5;
			if(w == 0) mem[gsi] = 1.0;
			if(w == 1) mem[gsi] = 0.0;
			if(w == -1) mem[gsi] = -1.0;
		}
		cout<<"done."<<endl;
	}

	//print state of DB
	void print(){
		cout<<"DB dump: "<<endl;
		int start = 39827200;
		int comp = 0;
		int rem = 0;
		int win = 0;
		int lose = 0;
		FOR(gsi, MAXSTATES){
			if(mem[gsi] == 1) win++;
			if(mem[gsi] == 0) lose++;
			if(mem[gsi] != -1){
				comp++;
			}else{
				rem++;
			}
		}
		comp -= start;
		cout<<" - Computed: "<<comp<<endl;
		cout<<" - Remaining: "<<rem<<endl;
		cout<<" - Percetage computed: "<<((double)comp)/(comp+rem)*100<<"%"<<endl;
		cout<<" - Winning: "<<win<<endl;
		cout<<" - Loosing: "<<lose<<endl;
	}

	//return number of remaining GameStates to compute
	int getRemaining(){
		int res = 0;
		FOR(gsi, MAXSTATES){
			if(mem[gsi] == -1) res++;
		}
		return res;
	}
};

//*************** DataBase2  ***************
//lazy version of BD, used for Helper class
//works the same way like DataBase
//  but it stores data in hashtable since helper
//  doesnt need all GameStates values
//primary used to read from completly computed DB.dat file
//******************************************
class DataBase2{
public:
	unordered_map<int, double> mem;

	DataBase2(){
	}

	//returns value of GameState
	double get(GameState gs){
		return get(gs.toInt());        
	}
	double get(int gsi){
		if(mem.find(gsi) == mem.end())
			load(gsi);
		return mem[gsi];
	}

	//load one specific GameState from DB.dat
	void load(int gsi){
		ifstream fin; 
		fin.open("DB.dat", ios::binary | ios::in);
		double d;
		fin.seekg(gsi * sizeof(d));
		fin.read(reinterpret_cast<char *>(&d), sizeof(d));
		put(gsi, d);
	}

	//save value of GameState
	void put(int gsi, double d){
		mem[gsi] = d;
	}
};


//*************** Operator ***************
//represents one node of evaluating formula (tree)
//has 4 types:
// - constant: contains only GameChange
// - min: opponent turn and he tries to minimalize the value
// - max: current player turn, maxmimal value
// - avg: depends on dice roll, take avarange value
//****************************************
class Operator{
public:
	vector<Operator*> args; //vector of children Operators
	vector<string> choice;  //string representing move to get to children
							//length same as args
	double weight;			//weight of this Operator, used in avg nodes
	int type;   //0 const, 1 min, 2 max, 3 avg
	int value;  //value of absolute Operators, GameChange.toInt()
	ll hash;    //hashed Operator including all its children

	Operator(){
		args = vector<Operator*>();
		weight = 1.0;
		type = 0;
		hash = 0;
	}

	//size of subtree including this node
	int count(){
		int c = 1;
		FOR(i, sz(args)){
			c += args[i]->count();
		}
		return c;
	}

	//return value of absolute nodes (leaves of tree)
	int getValue(){
		return value;
	}

	//set value of absolute nodes (leaves of tree)
	void setValue(int value2){
		value = value2;
	}

};

//delete whole subtree recusively
void deleteAll(Operator* o){
	FOR(i, sz(o->args)){
		deleteAll(o->args[i]);
	}
	delete(o);
}

//set operator hashes for a subtree
string setHashOperator(Operator* o){
	std::stringstream ss;
	if(o->type != 0){
		ss<<o->type;;
		if(o->weight != 1.0){
			if(o->weight < 1.5/36){ //only two possible weights !=1: 1/36 and 2/36
				ss<<"1";
			}else{
				ss<<"2";
			}
		}
		ss<<"(";
		FOR(i, sz(o->args)){ 
			//recursive call
			ss<<setHashOperator(o->args[i]);
		}
		ss<<")";
	}else{ //absolute operator
		ss<<"("<<o->getValue()<<")";
	}
	string s = ss.str();
	o->hash = str_hash(s);
	return s;
}

//simplify a subtree
//expects all operators to have set hash
void checkOperator(Operator* o){
	FOR(i, sz(o->args)){
		if(o->args[i]->type == 1 && o->type == 1){ //both min
			o->args.insert(o->args.end(), all(o->args[i]->args));
			delete(o->args[i]);
			o->args.erase(o->args.begin() + i);
			i = -1;
			continue;
		}
		if(o->args[i]->type == 2 && o->type == 2){ //both max
			o->args.insert(o->args.end(), all(o->args[i]->args));
			delete(o->args[i]);
			o->args.erase(o->args.begin() + i);
			i = -1;
			continue;
		}
	}

	//remove indentical branches
	if(o->type == 1 || o->type == 2){ 
		set<ll> ss;
		FOR(i, sz(o->args)){
			if(o->args[i]->type == 0){
				ll hash = o->args[i]->hash;
				if(ss.find(hash) != ss.end()){
					deleteAll(o->args[i]); 
					o->args.erase(o->args.begin() + i); 
					i--; 
				}
				ss.insert(hash);
			}
		}
	}

	//only one children
	if(o->type == 1 || o->type == 2){
		if(o->args.size() == 1){
			Operator* toDelete = o->args[0];
			o->setValue(o->args[0]->getValue());
			o->weight = o->args[0]->weight;
			o->type = o->args[0]->type;
			o->hash = o->args[0]->hash;
			o->args = o->args[0]->args;
			delete(toDelete);
		}
	}

	//recursively call for all children
	FOR(i, sz(o->args)){
		checkOperator(o->args[i]);
	}
}

//create Operator (tree) that represents
//  choices for given RoundState
//if info is set then fill also choice with strings
//  indicating moves that should be taken to get into the children
//  used only for Helper, Evaluator doesnt need it
Operator* makeOperator(RoundState rs, bool info){ 
	Operator* o = new Operator();
	if(!rs.taken[0].empty() && !rs.taken[1].empty()){ //absolute operator
		GameChange gc = GameChange();
		gc.truck[0] = rs.taken[0];
		gc.truck[1] = rs.taken[1];
		gc.startChange = (rs.tookFirst == 0);
		o->type = 0;
		o->setValue(gc.toInt());
		return o;
	}else if(!rs.rolled.empty()){  //some dices are rolled
		if(!rs.taken[1].empty() || (rs.taken[0].empty() && (6 - rs.dices - rs.rolled.size()) % 4 == 0)){
			o->type = 2; //current player turn, max operrator
		}else{
			o->type = 1; //opponents turn, min operator
		}
		bool same = (rs.rolled[0] == rs.rolled[1]);
		vi used = vi(9, 0);

		//create all possible moves to insert dices into trucks
		FOR(i, 3){
			//truck i is full
			if(rs.trucks[i].size() >= 3) continue; //truck i is full
			
			//remove symetries
			if(i == 1 && eq(rs.trucks[0], rs.trucks[1])) continue; 
			if(i == 2 && (eq(rs.trucks[0], rs.trucks[2]) || eq(rs.trucks[1], rs.trucks[2]))) continue;
			
			//insert first dice into i-th truck
			RoundState rs2 = rs;
			rs2.trucks[i].pb(rs2.rolled[0]);

			FOR(j, 3){
				//removes symetries if both dices are the same
				if(same && used[i*3 + j] == 1) continue;
				if(same) used[j*3 + i]=1;

				//truck j is full
				if(rs2.trucks[j].size() >= 3) continue;
				
				//remove symetries
				if(j == 1 && eq(rs2.trucks[0], rs2.trucks[1])) continue;
				if(j == 2 && (eq(rs2.trucks[0], rs2.trucks[2]) || eq(rs2.trucks[1], rs2.trucks[2]))) continue;

				//insert second dice into j-th truck
				RoundState rs3 = rs2;
				rs3.trucks[j].pb(rs3.rolled[1]);
				rs3.rolled = vi();
				
				//create info about choices
				if(info){
					stringstream ss;
					ss<<(i+1)<<(j+1);
					o->choice.pb(ss.str());
				}

				//recursively insert into operator o as a child
				o->args.pb(makeOperator(rs3, info));
			}
		}
		return o;
	}else{ //dices are not rolled
		//take some truck
		if(!rs.taken[1].empty() || (rs.taken[0].empty() && (6 - rs.dices - rs.rolled.size()) % 4 == 0)){
			//current player turn, max operrator
			o->type = 2;
			FOR(i, 3){
				//i-th truck is empty
				if(rs.trucks[i].empty()) continue;

				//remove symetries
				if(i == 1 && eq(rs.trucks[0], rs.trucks[1])) continue;
				if(i == 2 && (eq(rs.trucks[0], rs.trucks[2]) || eq(rs.trucks[1], rs.trucks[2]))) continue;

				//take i-th truck
				RoundState rs2 = rs;
				rs2.taken[0] = rs2.trucks[i];
				rs2.trucks[i] = vi();
				if(rs2.tookFirst == -1) rs2.tookFirst = 0;
				
				//create info about choices
				if(info){
					stringstream ss;
					ss<<(i+1);
					o->choice.pb(ss.str());
				}

				//recursively insert into operator o as a child
				o->args.pb(makeOperator(rs2, info));   
			}
		}else{
			//opponents turn, min operator
			o->type = 1;
			FOR(i, 3){
				//i-th truck is empty
				if(rs.trucks[i].empty()) continue;

				//remove symetries
				if(i == 1 && eq(rs.trucks[0], rs.trucks[1])) continue;
				if(i == 2 && (eq(rs.trucks[0], rs.trucks[2]) || eq(rs.trucks[1], rs.trucks[2]))) continue;
				
				//take i-th truck
				RoundState rs2 = rs;
				rs2.taken[1] = rs2.trucks[i];
				rs2.trucks[i] = vi();
				if(rs2.tookFirst == -1) rs2.tookFirst = 1;
				
				//create info about choices
				if(info){
					stringstream ss;
					ss<<(i+1);
					o->choice.pb(ss.str());
				}

				//recursively insert into operator o as a child
				o->args.pb(makeOperator(rs2, info));   
			}
		}
		//roll dices, avg operator
		if(rs.dices != 0){
			Operator* o2 = new Operator();
			o2->type = 3;
			FOR(i, 6) FOR(j, 6){
				if(i > j) continue;
				
				//rolled i and j
				RoundState rs2 = rs;
				rs2.rolled.pb(i);
				rs2.rolled.pb(j);
				rs2.dices -= 2;

				//call recursively
				Operator* o3 = makeOperator(rs2, info);
				
				//set correct wieght to children
				o3->weight = 1.0 / 36;
				if(i != j) o3->weight *= 2;
				
				//create info about choices				
				if(info){
					stringstream ss;
					ss<<"r"<<(i+1)<<(j+1);
					o2->choice.pb(ss.str());
				}
				o2->args.pb(o3);
			} 

			//create info about choices
			if(info) o->choice.pb("r");

			//insert into operator o as a child
			o->args.pb(o2); 
		}
		return o;
	}
}

//*************** Evaluator ***************
//handles all evaluating of GameStates
//nedd reference to DataBase and evaluating tree of operators (reference to the root)
//*****************************************
class Evaluator{
public:
	DataBase* db; //reference to a DataBase
	Operator* formula; //reference to the root of evaluating tree of operators
	Operator* allOperators; //array of all unique operators
	mutex mThreads, mFinished, mComputing; //mutex locks for multithread computing
	int counted; //number of counted GameStates
	int remaining; //number of remaining GameStates, inits once 
	set <int> computing; //set of GameStates that are currently computing
	int numThreads; //number of running threads
	double startTime; //start time of beggining of evaluation

	Evaluator(){
		allOperators = new Operator[400000];
		counted = 0;
		numThreads = 0;
		startTime = 0;
	}

	//assigns DataBase object to Evaluator, as a reference
	void assignDB(DataBase *db2){
		db = db2;
		remaining = db->getRemaining();
	}

	//merges all operators with the same hash
	//after that all substrees are different
	//save them to array allOperators
	void reduceFormula(){
		map <ll, Operator*> mapOperators; //map of all operators
		mapOperators[formula->hash] = formula;
		ll formulaHash = formula->hash;

		//call recurively on all subtree
		reduceOperator(formula, mapOperators);

		cout<<"Number of unique operators: "<<sz(mapOperators)<<endl;

		//rehash operators, so it numbers starting from 0
		//store them in array allOperators
		formula->hash = 0;
		Operator o = *formula;
		allOperators[0] = o;

		int i = 1;
		for(map<ll, Operator*>::iterator it = mapOperators.begin(); it!=mapOperators.end(); ++it){
			if(formulaHash == it->first) continue; //skip the root, already saved into allOperators[0]
			it->second->hash = i;
			allOperators[i] = *it->second;
			i++;
		}

		//change pointers of operators children into the array
		FOR(i, sz(mapOperators)){
			FOR(j, allOperators[i].args.size()){
				allOperators[i].args[j] = &allOperators[allOperators[i].args[j]->hash];
			}
		}

		//delete all former operators
		for(map<ll, Operator*>::iterator it = mapOperators.begin(); it!=mapOperators.end(); ++it){
			delete(it->second);
		}
		
		//set pointer of the root to the array
		formula = &allOperators[0];
	}

	//recursive call of reduceFormula()
	void reduceOperator(Operator* o, map<ll, Operator*> &mapOperators){
		FOR(i, sz(o->args)){
			ll hash = o->args[i]->hash;
			if(mapOperators.find(hash) != mapOperators.end()){ //this subtree already exists
				deleteAll(o->args[i]);
				o->args[i] = mapOperators[hash];
			}else{ //save into mapOperators and call recursively
				mapOperators[hash] = o->args[i];
				reduceOperator(o->args[i], mapOperators);
			}
		}
	}

	//clear cached memory of operators values
	void clearMem( vector <pair<double, bool> >& mem){
		FOR(i, sz(mem)){
			if(mem[i].second == true){mem[i] = mp(-1, false);}
		}
	}

	//evaluate all GameStates of given size
	//total is just number of all GameStates with this size
	void evalAll(int size){
		int skipped = 0;
		counted = 0;
		vector <thread> threads;
		if(!startTime) startTime = getTime();

		FOR(gsi, MAXSTATES){
			if(killit) break;

			//wait until there are some free threads
			while(numThreads >= maxThreads){
				this_thread::sleep_for (std::chrono::milliseconds(10));
			}

			//erase old threads
			if(sz(threads) > 100){
				threads[0].join();
				threads.erase(threads.begin());
				continue;
			}

			GameState gs(gsi);
			if(gs.sum() != size) continue; //only correct size
			if(gs.swap().toInt() < gsi) continue; 
			if(gs.winning() != -1) continue; //only not finished GameStates
			if(db->get(gsi) >= 0 && db->get(gs.swap()) >= 0){ skipped+=2; continue; } //skip already counted


			bool hasSelf = gs.hasSelf();

			//create computing thread
			mComputing.lock();
			computing.insert(gsi);
			if(hasSelf) computing.insert(gs.swap().toInt());
			mComputing.unlock();
			mThreads.lock();
			numThreads++;
			mThreads.unlock();
			threads.pb(thread (&Evaluator::compute, this, gsi, hasSelf));

			//start another thread with swapped GameState
			if(!hasSelf && gs.swap().toInt() != gsi){ 
				mThreads.lock();
				numThreads++;
				mThreads.unlock();
				threads.pb(thread (&Evaluator::compute, this, gs.swap().toInt(), hasSelf));
			}
		}

		//wait to all threads to finish
		while(sz(threads) > 0){
			threads[0].join();
			threads.erase(threads.begin());
		}
	}  

	//compute GameState
	void compute(int gsi, bool hasSelf){
		GameState gs(gsi);
		eval(gs, hasSelf);
		mThreads.lock();
		numThreads--;
		mThreads.unlock();
	}


	//finished computing of GameState gs with value of value
	//called from eval()
	void finished(GameState &gs, double value){
		mFinished.lock();
		int gsi = gs.toInt();
		if(db->get(gsi) < 0) counted++;
		db->put(gsi, value);
		gs.print();
		mComputing.lock();
		if(computing.find(gsi) != computing.end()) computing.erase(computing.find(gsi));
		mComputing.unlock();

		//every 500 evals call db.saveDiff()
		if(counted % 500 == 0){
			db->saveDiff();
		}

		//every 20 print stats
		if(counted % 20 == 0){
			double timeToOne = (getTime() - startTime) / counted;
			int hours  = (int)(timeToOne * (remaining - counted) / 3600);
			cout << "counted:" << counted << ", remaining: " << remaining - counted << 
				", hours remaining: " << hours << endl;
		}

		mFinished.unlock();
	}

	//evaluate formula with starting GameState gs
	//if hasSelf, then it need to evaluate also gs.swap() GameState 
	//at the end call finished(gs, d), where d is the result
	void eval(GameState gs, bool hasSelf){
		if(!hasSelf){ //easy, straight forward version
			vector <pair<double,bool> > mem = vector <pair<double,bool> >(400000, mp(-1, false));
			double d = eval(formula, gs, 0, 0, mem).first;
			finished(gs, d);
		}else{ 
			//in formula there is at least once value of this GameState
			//(when there is GameChange that doesnt change size of GameState)
			//we have to guess values of this GS and its swapped GS
			//until the differnce is close enough, max 20 iterations
			double guess1, guess2;
			double d1 = -1; double d2 = -1;
			double d12 = -1; double d22 = -1;
			int iteration = 1;
			vector <pair<double,bool> > mem1 = vector <pair<double,bool> >(400000, mp(-1, false));
			vector <pair<double,bool> > mem2 = vector <pair<double,bool> >(400000, mp(-1, false));
			GameState gs2 = gs.swap();
			do{
				//first two iteration are (1, 0) and (0, 1)
				if(iteration == 1){
					guess1 = 1; 
					guess2 = 0;
				}
				if(iteration == 2){
					guess1 = 0; 
					guess2 = 1;
				}
				//a bit of heuristics for 3rd iteration
				if(iteration == 3){
					if(abs(d1 - 0.5) > abs(d12 - 0.5)){
						guess1 = d1; 
					}else{ 
						guess1 = d12; 
					}
					
					if(abs(d2 - 0.5) > abs(d22 - 0.5)){
						guess2 = d2; 
					}else{ 
						guess2 = d22; 
					}
				}
				//else choose values as previous results
				if(iteration > 3){
					guess1 = d1;
					guess2 = d2;
				}

				//eval both GameStates
				clearMem(mem1);
				d1 = eval(formula, gs, guess1, guess2, mem1).first;
				clearMem(mem2);
				d2 = eval(formula, gs2, guess2, guess1, mem2).first;
				if(d12 == -1 && d22 == -1){
					d12 = d1; 
					d22 = d2;
				}
			}while((isFar(guess1, d1) || isFar(guess2, d2)) && (iteration++) <= 20);
			finished(gs, d1);
			finished(gs2, d2);
		}
	}

	//evaluate subtree of operator o
	//guess1 is expected value of gs and guess2 of gs.swap()
	//return pair<value, hasSelf>, where hasSelf is true if this
	//  subtree contains value of gs or gs.swap()
	pair<double, bool> eval(
			Operator* o, GameState& gs, 
			double guess1, 
			double guess2, 
			vector <pair<double, bool> >& mem
	){
		//return cached results
		if(mem[o->hash].first != -1) return mem[o->hash];

		if(o->type == 0){ //absolute operator
			GameState gs2 = gs.add(o->getValue());
			bool hasSelf = false;
			double d = -1;
			if(gs.toInt() == gs2.toInt()){ //gs == gs2
				d = guess1;
				hasSelf = true;
			}else if(gs.swap().toInt() == gs2.toInt()){ //gs.swap() == gs2
				d = guess2;
				hasSelf = true;
			}else{ //take value from DataBase
				d = db->get(gs2);
			}

			if(d < 0.0){ 
				cout<<"FATAL ERROR: When counting "<<gs.toInt()<<", value of "<<gs2.toInt()<<" is unknow."<<endl;
				killit = true;
				return mp(-1, false);
			}

			if(GameChange(o->getValue()).startChange == true){d = 1 - d;}
			mem[o->hash] = mp(d, hasSelf); //save to cache
			return mp(d, hasSelf);
		}

		if(o->type == 1){  //min operator
			double m = -1;
			bool hasSelf = false;
			FOR(j, sz(o->args)){
				//recursive call
				pair<double, bool> res = eval(o->args[j], gs, guess1, guess2, mem);
				double d = res.first;
				if(res.second) hasSelf = true;
				if(m == -1 || d < m) m = d; //get min
			}
			mem[o->hash] = mp(m, hasSelf); //save to cache
			return mp(m, hasSelf);
		}

		if(o->type == 2){ //max operator
			double m = -1;
			bool hasSelf = false;
			FOR(j, sz(o->args)){
				//recursive call
				pair<double, bool> res = eval(o->args[j], gs, guess1, guess2, mem);
				double d = res.first;
				if(res.second) hasSelf = true;
				if(m == -1 || d > m) m = d; //get max
			}
			mem[o->hash] = mp(m, hasSelf);  //save to cache
			return mp(m, hasSelf);
		}

		if(o->type == 3){ //avg operator
			double sum = 0;
			bool hasSelf = false;
			FOR(j, sz(o->args)){
				//recursive call
				pair<double, bool> res = eval(o->args[j], gs, guess1, guess2, mem);
				double d = res.first;
				if(res.second) hasSelf = true;
				sum += d * o->args[j]->weight; //add to sum value*weight
			}
			mem[o->hash] = mp(sum, hasSelf); //save to cache
			return mp(sum, hasSelf);
		}
	}
};

//*************** Helper ***************
//counts all moves and their values of GameState and RoundState
//need to have reference to DataBase2 with precomputed values of this GameState
//**************************************
class Helper{
public:
	DataBase2* db; 

	Helper(DataBase2* db2){ db = db2;}

	//returns all choices of GS and RS
	//empty if GameState is not precomputed
	vector<pair<string, double> > getChoices(GameState gs, RoundState rs){
		vector<pair<string, double> > res;
		if(db->get(gs) == -1){
			return res;
		}
		Operator* o = makeOperator(rs, true);
		FOR(j, sz(o->args)){
			res.pb(mp(o->choice[j], eval(o->args[j], gs)));
		}
		deleteAll(o);
		return res;
	}

	//evaluate GameState with formula o
	double eval(Operator* o, GameState& gs){
		if(o->type == 0){ //abs operator
			double d = db->get(gs.add(o->getValue()));
			if(d < 0){
				cout<<"FATAL ERROR: Some precomputed values are missing."<<endl;
				killit = true;
			}
			if(GameChange(o->getValue()).startChange){d = 1 - d;}
			return d;
		}
		if(o->type == 1){ //min operator
			double m = -1;
			FOR(j, sz(o->args)){
				double d = eval(o->args[j], gs);
				if(m == -1 || d < m) m = d;
			}
			return m;
		}
		if(o->type == 2){ //max operator
			double m = -1;
			FOR(j, sz(o->args)){
				double d = eval(o->args[j], gs);
				if(m == -1 || d > m) m = d;
			}
			return m;
		}
		if(o->type == 3){ //avg operator
			double m = 0;
			FOR(j, sz(o->args)){
				double d = eval(o->args[j], gs) * o->args[j]->weight;
				m += d;
			}
			return m;
		}
	}

};

//input GameState and RoundState as strings
//prints JSON for all moves with corresponding values
void help(int argc, char *argv[]){
	DataBase2 db2;

	Helper h(&db2);

	GameState gs;
	RoundState rs;

	if(argc < 3){
		cout << "Set GameState: " << endl;
		string s;
		cin >> s;
		gs = GameState(s);
	}else{
		gs = GameState(argv[2]);
	}

	if(argc < 4){
		cout << "Set RoundState: " << endl;
		string s;
		cin >> s;
		rs = RoundState(s);
	}else{
		rs = RoundState(argv[3]);
	}

	auto res = h.getChoices(gs, rs);

	//format as JSON
	stringstream ss;
	ss << "[";
	if(res.size()){
		FOR(i, sz(res)){
			if(i != 0) ss<<",";
			ss << "[\"" << res[i].first << "\",\"" << res[i].second << "\"]";
		}
	}
	ss << "]";
	cout << ss.str() << endl;
}

//merge .add file into DB and save it
void merge(string fileName){
	DataBase db;
	db.load();
	db.print();
	db.loadDiff(fileName);
	db.print();
	db.save();
}

//create empty database of all GameStates and compute ending ones
void init(){
	DataBase db;
	db.init();
	db.print();
	db.save();
}

//procompute all not counted GameStates
void precompute(){
	cout << "Preparing for precompute, this may take a while." << endl;
	Evaluator e;
	cout << "Creating formula... ";
	e.formula = makeOperator(RoundState(), false);
	cout << "done." << endl;
	cout << "Simplifying formula... ";
	setHashOperator(e.formula);
	checkOperator(e.formula);
	setHashOperator(e.formula);
	cout << "done." << endl;
	e.reduceFormula();
	
	DataBase db;
	db.init();
	db.load();
	db.print();

	e.assignDB(&db);

	//compute biggest unsolved size
	int size  = -1;
	FOR(gsi, MAXSTATES){
		if(db.get(gsi) == -1)
			gmax(size, GameState(gsi).sum());
	}
	
	while(size >= 0){
		int total = 0;
		cout << "Precomputing size " << size << endl;
		e.evalAll(size);
		db.saveDiff();
		size--;
	}
	cout << "All GameStates computed" << endl;
}

//main function
int main(int argc, char *argv[])
{
	if (sizeof(void*) < 8){
		cerr << "Sorry, you need 64bit system to run this program." << endl;
		return -1;
	}
	setCrypt();

	string mode = "";
	if(argc > 1){
		mode = argv[1];
	}else{
		cout << "Select mode (pre, help, init, merge):" << endl;
		cin >> mode;
	}

	if(mode == "pre")
		precompute();
	if(mode == "help")
		help(argc, argv);
	if(mode == "init")
		init();
	if(mode == "merge")
		merge(argv[2]);
	
	return EXIT_SUCCESS;
}
