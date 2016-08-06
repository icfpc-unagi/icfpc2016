#include<iostream>
#include<vector>
#include<map>
#include<cassert>

using namespace std;

int N;
const int N_MAX = 99;

typedef pair<int, int> Point;
typedef pair<Point, Point> Edge;

int random_coord(){
  return rand()%(N+1);
}

Point random_pt(){
  return make_pair(random_coord(), random_coord());
}

Point random_bound(){
  switch(rand()%4){
    case 0: return make_pair(rand()%N, 0);
    case 1: return make_pair(N,rand()%N);
    case 2: return make_pair(N-rand()%N, N);
    case 3: return make_pair(0,N-rand()%N);
  }
  assert(false);
  return make_pair(-1,-1);
}


int dx[8]={1,1,0,-1,-1,-1,0,1};
int dy[8]={0,1,1,1,0,-1,-1,-1};

vector<int> shape4;
void init_shapes(){
  for(int i=0; i<8; i++)
    for(int j=i+1; j<8; j++)
      for(int k=j+1; k<8; k++)
	for(int l=k+1; l<8; l++) {
	  if(j-i+l-k == 4){
	    shape4.push_back( (1<<i) | (1<<j) | (1<<k) | (1<<l) );
	  }
	}
}

int random_shape4(){
  return shape4[rand()%shape4.size()];
}

bool good_coord(int x){
  return 0<= x && x<=N;
}

bool good_edge(int x1, int y1, int x2, int y2){
  int x = x1+x2;
  int y = y1+y2;
  return (0 < x && x < 2*N && 0 < y && y < 2*N);
}

bool good_point(int x, int y){
  return good_coord(x) && good_coord(y);
}

bool is_boundary(int x, int y){
  return good_point(x,y) && (x==0 || x==N || y==0 || y==N);
}

bool good_shape(int s){
  int last=0;
  int sum=0;
  int sig=1;
  for(int i=0; i<8; i++){
    if(s&(1<<i)){
      sig*=-1;
      sum+=sig*i;
    }
  }
  return sig==1 && sum==4;
}

void print_vec(vector<int> v){
  cerr<<"<";
  for(int i=0; i<v.size(); i++){
    cerr<<v[i];
    cerr<<", ";
  }
  cerr<<">"<<endl;
}

void print_map(int e[N_MAX+1][N_MAX+1]){
  cerr<<"-----"<<endl;
  for(int y=N; y>=0; y--){
    for(int x=0; x<=N; x++){
      cerr << "." << (e[x][y]&(1<<0) ? "-" : " ");
    }
    cerr<<endl;
    for(int x=0; x<=N; x++){
      cerr << (e[x][y]&(1<<6) ? "|" : " ");
      bool f1, f2;
      f1 = e[x][y]&(1<<7);
      f2 = good_point(x+1,y) && (e[x+1][y]&(1<<5));
      cerr << (f1 ? (f2 ? "X" : "\\") : (f2 ? "/" : " "));
    }
    cerr<<endl;
  }
}

void export_map(int edges0[N_MAX+1][N_MAX+1]){
  vector<pair<int,int> > vertices;
  vector<vector<int> > polygons;

  int edges[2*N_MAX+1][2*N_MAX+1]={};
  for(int x=0; x<=N; x++)
    for(int y=0; y<=N; y++){
      int s = edges0[x][y];
      if(s == 0x11 || s == 0x22 || s == 0x44 || s == 0x88) { s = 0; }  // straight line
      edges[2*x][2*y]= s;
    }
  for(int x=0; x<N; x++)
    for(int y=0; y<N; y++){
      if((edges0[x][y]&(1<<1)) && (edges0[x+1][y]&(1<<3))){
	edges[2*x+1][2*y+1] = (1<<1)|(1<<3)|(1<<5)|(1<<7);
      }
    }

  bool used[2*N_MAX+1][2*N_MAX+1][8]={};
  map<pair<int,int>, int> vertex_id1;
  int vertex_cnt = 0;
  for(int x0=0; x0<=2*N; x0++)
    for(int y0=0; y0<=2*N; y0++) {
      for(int d0=0; d0<8; d0++) {
	if(used[x0][y0][d0]) continue;
	if((edges[x0][y0]&(1<<d0)) == 0) continue;
	int x=x0; int y=y0; int d=d0;
	vector<int> polygon;
	do{
	  used[x][y][d]=true;
	  int &v1 = vertex_id1[make_pair(x,y)];
	  if(v1==0){
	    v1 = vertex_cnt + 1;
	    vertex_cnt++;
	    /*
	    cout<<"vertex #"<<v1-1<<": "<< x << " " << y<< endl;
	    */
	    vertices.push_back(make_pair(x,y));
	  }
	  polygon.push_back(v1-1);
	  do{  // susumu
	    x+=dx[d];
	    y+=dy[d];
	  }while(edges[x][y]==0);
	  d = (d+4)%8;  // mawaru
	  do{
	    d = (d+7)%8;
	  }while((edges[x][y]&(1<<d))==0);
	}while(x!=x0 || y!=y0 || d!=d0);
	/*
	cout<<"polygon: ";
	cout<<polygon.size();
	for(int i=0; i<polygon.size(); i++){
	  cout<<" "<<polygon[i];
	}
	cout<<endl;
	*/
	if(!(x0 == 0 && y0 == 0 && d0 == 2)) {  // not sotogawa
	  polygons.push_back(polygon);
	}
      }
    }
  
  cout << vertices.size() << endl;
  for(int k=0; k<vertices.size(); k++) {
    pair<int, int> vertex = vertices[k];
    cout << vertex.first << "/" << 2*N << "," << vertex.second << "/" << 2*N << endl;
  }

  cout << polygons.size() << endl;
  for(int k=0; k<polygons.size(); k++) {
    vector<int> polygon = polygons[k];
    cout<<polygon.size();
    for(int i=0; i<polygon.size(); i++){
      cout<<" "<<polygon[i];
    }
    cout<<endl;
  }
  return;
}

int main(int argc, char **argv){
  assert (argc > 4);
  int M;
  int P;
  sscanf(argv[1], "%d", &N);
  assert (1 < N && N <= N_MAX);
  sscanf(argv[2], "%d", &M);
  sscanf(argv[3], "%d", &P);
  assert(0<=P && P<=100);
  {
    int r;
    sscanf(argv[4], "%d", &r);
    srand(r);
  }

  int edges[N_MAX+1][N_MAX+1]={};
  init_shapes();
//  for(int i=0; i<shape4.size(); i++){
//    for(int b=0; b<8; b++){
//      if(shape4[i]&(1<<b)) cout<<b;
//    }
//    cout<<endl;
//  }
  /*
  for(int i=0; i<256; i++){
    if(good_shape(i)){
      for(int j=0; j<8; j++){
	cout<<(i&(1<<j) ? ".":" ");
      }
      cout<<endl;
    }
  }
  */

  for(int loop=0; loop<M; loop++) {
    int edges_bkup[N_MAX+1][N_MAX+1];
    for(int x=0; x<=N; x++)
      for(int y=0; y<=N; y++) {
	edges_bkup[x][y]=edges[x][y];
      }
    int x0, y0, s_new;
    if(loop >= 1 && random()%100 < P){
      // print_map(edges);
      for(int orz=0; orz<10; orz++){
	// try to select a point (x0,y0) on an existing edge near the boundary.
	int s_old;
	do {
	  pair<int,int> xy=random_bound();
	  x0=xy.first;
	  y0=xy.second;
	  s_old = edges[x0][y0];
	} while (s_old == 0 || (s_old & (s_old-1)) != 0);
	int d1 = 0;
// cerr<<x0 << " " << y0 <<" "<< s_old << "!"<<endl;
	while(s_old != (1<<d1)) d1++;
// cerr<<x0 << " " << y0 <<" "<< d1 << "!"<<endl;
	int len = 0;
	while(true){
	  len++;
	  int s=edges[x0+len*dx[d1]][y0+len*dy[d1]];
	  s &= (s-1);
	  s &= (s-1);
	  if(is_boundary(x0+len*dx[d1], y0+len*dy[d1]) || s!=0) break;
	}
// cerr<<len<<endl;
	if(len<=1){
	  continue;
	}
	len--;
	// int l=1+rand()%len;
	int l=1+(rand()%(len*len))/len;
	while(l--){
	  edges[x0][y0] &= ~(1<<d1);
	  x0 += dx[d1];
	  y0 += dy[d1];
	  edges[x0][y0] &= ~(1<<((d1+4)%8));
	}
	while(true){
	  s_new = random_shape4();
	  // must preserve existing edge
	  if((s_new & (1<<d1)) ==0) continue;
	  s_new &= ~(1<<d1);
	  // should not reconstruct deleted edge
	  if((s_new & (1<<((d1+4)%8))) == 0) break;
	}
      // print_map(edges);
	goto X_AND_Y_CHOSEN;
      }
      cerr << "orz" << endl;
      goto REVERT;
    }else{
      do{
	x0=random_coord();
	y0=random_coord();
      }while(edges[x0][y0] != 0);
      s_new = random_shape4();
    }
X_AND_Y_CHOSEN:
//    cout << x0 << " " << y0 << endl;
    for(int d0=0; d0<8; d0++){
      if((s_new&(1<<d0)) == 0) continue;
// cout << "[" << d0 << "]" << endl;
      int x=x0; int y=y0; int d=d0;
      while(true){
// print_map(edges);
// cout << x << " " << y << " " << d <<  endl;
	if(!good_edge(x, y, x+dx[d], y+dy[d])) break;

	edges[x][y] |= 1<<d;
	x += dx[d];
	y += dy[d];
	if(x==x0 && y==y0) goto REVERT;
	int &s = edges[x][y];
	d = (d+4)%8;
	if(s&(1<<d)){
	  cout << "ERROR!!!" <<endl;
	  return -1;
	}
	s |= 1<<d;

	if(is_boundary(x,y)) break;
	vector<int> cand;
// cout<<s<<endl;
	for(int dnew=0; dnew<8; dnew++){
	  if(dnew==d) continue;
	  if(good_shape(s|(1<<dnew))) cand.push_back(dnew);
	}
// print_vec(cand);
	d = cand[rand()%cand.size()];
// cout<<"("<<d<<")"<<endl;
      }
    }
    continue;
REVERT:
    loop--;
    for(int x=0; x<=N; x++)
      for(int y=0; y<=N; y++) {
	edges[x][y]=edges_bkup[x][y];
      }
  }
  print_map(edges);
  for(int x=0; x<N; x++){
    edges[x][0] |= (1<<0);
    edges[x][N] |= (1<<0);
  }
  for(int x=N; x>0; x--){
    edges[x][0] |= (1<<4);
    edges[x][N] |= (1<<4);
  }
  for(int y=0; y<N; y++){
    edges[0][y] |= (1<<2);
    edges[N][y] |= (1<<2);
  }
  for(int y=N; y>0; y--){
    edges[0][y] |= (1<<6);
    edges[N][y] |= (1<<6);
  }
  // print_map(edges);
  export_map(edges);
  return 0;
}

int main_orz (){
  vector<Point> ps;
  vector<Edge> es;
  for(int loop=0; loop<20; loop++){
    Point p = random_pt();
    bool bad = false;
    for(int i=0; i<ps.size(); i++){
      if(p == ps[i]) bad = true;
    }
    if(bad) continue;
  }
  return 0;
}
