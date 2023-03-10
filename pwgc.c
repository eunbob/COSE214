#include <stdio.h>
#include <stdlib.h>

#define PEASANT 0x08
#define WOLF	0x04
#define GOAT	0x02
#define CABBAGE	0x01

// 주어진 상태 state의 이름(마지막 4비트)을 화면에 출력
// 예) state가 7(0111)일 때, "<0111>"을 출력
static void print_statename( FILE *fp, int state);

// 주어진 상태 state에서 농부, 늑대, 염소, 양배추의 상태를 각각 추출하여 p, w, g, c에 저장
// 예) state가 7(0111)일 때, p = 0, w = 1, g = 1, c = 1
static void get_pwgc( int state, int *p, int *w, int *g, int *c);

// 허용되지 않는 상태인지 검사
// 예) 농부없이 늑대와 염소가 같이 있는 경우 / 농부없이 염소와 양배추가 같이 있는 경우
// return value: 1 허용되지 않는 상태인 경우, 0 허용되는 상태인 경우
static int is_dead_end( int state);

// state1 상태에서 state2 상태로의 전이 가능성 점검
// 농부 또는 농부와 다른 하나의 아이템이 강 반대편으로 이동할 수 있는 상태만 허용
// 허용되지 않는 상태(dead-end)로의 전이인지 검사
// return value: 1 전이 가능한 경우, 0 전이 불이가능한 경우 
static int is_possible_transition( int state1,	int state2);

// 상태 변경: 농부 이동
// return value : 새로운 상태
static int changeP( int state);

// 상태 변경: 농부, 늑대 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePW( int state);
// 상태 변경: 농부, 염소 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePG( int state);

// 상태 변경: 농부, 양배추 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1 
static int changePC( int state);

// 주어진 state가 이미 방문한 상태인지 검사
// return value : 1 visited, 0 not visited
static int is_visited( int visited[], int depth, int state);

// 방문한 경로(상태들)을 차례로 화면에 출력
static void print_path( int visited[], int depth);

// recursive function
static void dfs_main( int initial_state, int goal_state, int depth, int visited[]);

////////////////////////////////////////////////////////////////////////////////
// 상태들의 인접 행렬을 구하여 graph에 저장
// 상태간 전이 가능성 점검
// 허용되지 않는 상태인지 점검 
void make_adjacency_matrix( int graph[][16]);

// 인접행렬로 표현된 graph를 화면에 출력
void print_graph( int graph[][16], int num);

// 주어진 그래프(graph)를 .net 파일로 저장
// pgwc.net 참조
void save_graph( char *filename, int graph[][16], int num);

////////////////////////////////////////////////////////////////////////////////
// 깊이 우선 탐색 (초기 상태 -> 목적 상태)
void depth_first_search( int initial_state, int goal_state);

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char **argv)
{
	int graph[16][16] = {0,};
	
	// 인접 행렬 만들기
	make_adjacency_matrix( graph);

	// 인접 행렬 출력 (only for debugging)
	//print_graph( graph, 16);
	
	// .net 파일 만들기
	save_graph( "pwgc.net", graph, 16);

	// 깊이 우선 탐색
	depth_first_search( 0, 15); // initial state, goal state
	
	return 0;
}















// 주어진 상태 state의 이름(마지막 4비트)을 화면에 출력
// 예) state가 7(0111)일 때, "<0111>"을 출력
static void print_statename( FILE *fp, int state)
{
	int p, w, g, c;
	get_pwgc(state, &p, &w, &g, &c);
	
	fprintf(fp, "\"<%d%d%d%d>\"\n", p, w, g, c);
}

// 주어진 상태 state에서 농부, 늑대, 염소, 양배추의 상태를 각각 추출하여 p, w, g, c에 저장
// 예) state가 7(0111)일 때, p = 0, w = 1, g = 1, c = 1
static void get_pwgc( int state, int *p, int *w, int *g, int *c){
	*c = state % 2;
	state /= 2;
	
	*g = state % 2;
	state /= 2;
	
	*w = state %2;
	state /= 2;
	
	*p = state;
}

// 허용되지 않는 상태인지 검사
// 예) 농부없이 늑대와 염소가 같이 있는 경우 / 농부없이 염소와 양배추가 같이 있는 경우
// return value: 1 허용되지 않는 상태인 경우, 0 허용되는 상태인 경우
static int is_dead_end( int state){
	int p, w, g, c;
	get_pwgc(state, &p, &w, &g, &c);
	
	if(w == g && w != p){
		return 1;
	}
	
	if(g == c && g != p){
		return 1;
	}
	
	return 0;	
}

// state1 상태에서 state2 상태로의 전이 가능성 점검
// 농부 또는 농부와 다른 하나의 아이템이 강 반대편으로 이동할 수 있는 상태만 허용
// 허용되지 않는 상태(dead-end)로의 전이인지 검사
// return value: 1 전이 가능한 경우, 0 전이 불이가능한 경우 
static int is_possible_transition( int state1,	int state2){
	if(is_dead_end(state2) || is_dead_end(state1))
		return 0;
	
	if(state2 == changeP(state1))
		return 1;
	
	if(state2 == changePW(state1))
		return 1;
	
	if(state2 == changePG(state1))
		return 1;
	
	if(state2 == changePC(state1))
		return 1;
	
	return 0;	
}

// 상태 변경: 농부 이동
// return value : 새로운 상태
static int changeP( int state){
	int p, w, g, c, nstate;
	get_pwgc(state, &p, &w, &g, &c);
	
	if(p == 1) p = 0;
	else p = 1;
	
	nstate = p * 8 + w * 4 + g * 2 + c * 1;
	return nstate;
}

// 상태 변경: 농부, 늑대 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePW( int state){
	int p, w, g, c, nstate;
	get_pwgc(state, &p, &w, &g, &c);
	
	if(p != w) return -1;
	
	if(p == 1) p = w = 0;
	else p = w = 1;
	
	nstate = p * 8 + w * 4 + g * 2 + c * 1;
	return nstate;
}

// 상태 변경: 농부, 염소 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePG( int state){
	int p, w, g, c, nstate;
	get_pwgc(state, &p, &w, &g, &c);

	if(p != g) return -1;
	
	if(p == 1) p = g = 0;
	else p = g = 1;
	
	nstate = p * 8 + w * 4 + g * 2 + c * 1;
	return nstate;
}

// 상태 변경: 농부, 양배추 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1 
static int changePC( int state){
	int p, w, g, c, nstate;
	get_pwgc(state, &p, &w, &g, &c);
	
	if(p != c) return -1;
	
	if(p == 1) p = c = 0;
	else p = c =1;
	
	nstate = p * 8 + w * 4 + g * 2 + c * 1;
	return nstate;
}

// 주어진 state가 이미 방문한 상태인지 검사
// return value : 1 visited, 0 not visited
static int is_visited( int visited[], int depth, int state){
	for(int i = 0; i < depth; i++){
		if(visited[i] == state)
			return 1;		
	} 
	return 0;
}

// 방문한 경로(상태들)을 차례로 화면에 출력
static void print_path( int visited[], int depth){
	int p, w, g, c;
	for(int i = 0; i < depth; i++){
		get_pwgc(visited[i], &p, &w, &g, &c);
		printf("<%d%d%d%d>\n", p, w, g, c);
	}
	printf("\n");
}

// recursive function
static void dfs_main( int initial_state, int goal_state, int depth, int visited[]){
	
	int p, w, g, c;
	int next_state, np, nw, ng, nc;
	
	get_pwgc(initial_state, &p, &w, &g, &c);
	
	if(is_dead_end(initial_state)){
		return;
	}
	
	if(is_visited(visited, depth, initial_state)){
		return;
	}
	
	else 
		visited[depth] = initial_state;
	
	printf("current state is <%d%d%d%d> (depth %d)\n", p, w, g, c, depth);
	
	if(initial_state == goal_state){
		printf("Goal-state found!\n");
		print_path(visited, depth + 1);
		return;
	}
	
	
	next_state = changeP(initial_state);
	for(int i = depth + 1; i < 16; i++){
		visited[i] = 0;
	}
	dfs_main(next_state, goal_state, depth + 1, visited);
	if(next_state == goal_state){
		printf("back to <%d%d%d%d> (depth %d)\n", p, w, g, c, depth);
	}
	else if(is_dead_end(next_state)){
		get_pwgc(next_state, &np, &nw, &ng, &nc);
		printf("\tnext state <%d%d%d%d> is dead-end\n", np, nw, ng, nc); 
	}
	
	for(int i = depth + 1; i < 16; i++){
		visited[i] = 0;
		}		
	if(is_visited(visited, depth, next_state)){
		get_pwgc(next_state, &np, &nw, &ng, &nc);
		printf("\tnext state <%d%d%d%d> has been visited\n", np, nw, ng, nc); 
	}
	
	
	if(p == w) {
		next_state = changePW(initial_state);
		for(int i = depth + 1; i < 16; i++){
		visited[i] = 0;
		}
		dfs_main(next_state, goal_state, depth + 1, visited);
		
		if(next_state == goal_state){
		printf("back to <%d%d%d%d> (depth %d)\n", p, w, g, c, depth);
		}
		else if(is_dead_end(next_state)){
			get_pwgc(next_state, &np, &nw, &ng, &nc);
			printf("\tnext state <%d%d%d%d> is dead-end\n", np, nw, ng, nc); 
		}
		
		for(int i = depth + 1; i < 16; i++){
		visited[i] = 0;
		}
		if(is_visited(visited, depth, next_state)){
			get_pwgc(next_state, &np, &nw, &ng, &nc);
			printf("\tnext state <%d%d%d%d> has been visited\n", np, nw, ng, nc); 
		}
	}
	
	
	if(p == g) {
		next_state = changePG(initial_state);
		for(int i = depth + 1; i < 16; i++){
		visited[i] = 0;
		}
		dfs_main(next_state, goal_state, depth + 1, visited);
		
		if(next_state == goal_state){
			printf("back to <%d%d%d%d> (depth %d)\n", p, w, g, c, depth);
		}
		if(is_dead_end(next_state)){
			get_pwgc(next_state, &np, &nw, &ng, &nc);
			printf("\tnext state <%d%d%d%d> is dead-end\n", np, nw, ng, nc); 
		}
		
		for(int i = depth + 1; i < 16; i++){
			visited[i] = 0;
		}
		if(is_visited(visited, depth, next_state)){
			get_pwgc(next_state, &np, &nw, &ng, &nc);
			printf("\tnext state <%d%d%d%d> has been visited\n", np, nw, ng, nc); 
		}
	}
	
	if(p == c) {
		next_state = changePC(initial_state);
		for(int i = depth + 1; i < 16; i++){
		visited[i] = 0;
		}
		dfs_main(next_state, goal_state, depth + 1, visited);
		
		if(next_state == goal_state){
			printf("back to <%d%d%d%d> (depth %d)\n", p, w, g, c, depth);
		}
		else if(is_dead_end(next_state)){
			get_pwgc(next_state, &np, &nw, &ng, &nc);
			printf("\tnext state <%d%d%d%d> is dead-end\n", np, nw, ng, nc); 
		}
		
		for(int i = depth + 1; i < 16; i++){
		visited[i] = 0;
		}
		if(is_visited(visited, depth, next_state)){
			get_pwgc(next_state, &np, &nw, &ng, &nc);
			printf("\tnext state <%d%d%d%d> has been visited\n", np, nw, ng, nc); 
		}
	}
	
	get_pwgc(visited[depth - 1], &np, &nw, &ng, &nc);
	if(depth - 1 >= 0){
		printf("back to <%d%d%d%d> (depth %d)\n", np, nw, ng, nc, depth - 1);
		for(int i = depth; i < 16; i++){
			visited[i] = 0;
		}
	}		
	
}

////////////////////////////////////////////////////////////////////////////////
// 상태들의 인접 행렬을 구하여 graph에 저장
// 상태간 전이 가능성 점검
// 허용되지 않는 상태인지 점검 
void make_adjacency_matrix( int graph[][16]){
	for(int i = 0; i < 15; i++){
		for(int j = i + 1; j < 16; j++){
			if(is_possible_transition(i, j)){
				graph[i][j] = 1;
				graph[j][i] = 1;
			}
		}  
	}
}
// 인접행렬로 표현된 graph를 화면에 출력
void print_graph( int graph[][16], int num){
	for(int i = 0; i < 16; i++){
		for(int j = 0; j< 15; j++){
			printf("%d\t", graph[i][j]);
		}
		printf("%d\n", graph[i][15]);
	}
}

// 주어진 그래프(graph)를 .net 파일로 저장
// pgwc.net 참조
void save_graph( char *filename, int graph[][16], int num)
{
	FILE *file = fopen(filename, "wb");
	
	if(file == NULL)
		printf("\n\n\n0\n\n\n\n");
	
	fprintf(file, "*Vertices 16\n");
	for(int i = 0; i < 16; i++)
	{	
		fprintf(file, "%d ", i + 1);
		print_statename(file, i);
	}
	
	fprintf(file, "*Edges\n");
	for(int i = 0; i < 15; i++)
	{
		for(int j = i + 1; j < 16; j++)
			if(graph[i][j])
				fprintf(file, "  %d  %d\n", i + 1, j + 1);
	}
	
	fclose(file);
}

////////////////////////////////////////////////////////////////////////////////
// 깊이 우선 탐색 (초기 상태 -> 목적 상태)
void depth_first_search( int initial_state, int goal_state)
{
	int depth = 0;
	int visited[16] = {0,}; // 방문한 정점을 저장
	
	dfs_main( initial_state, goal_state, depth, visited); 
}