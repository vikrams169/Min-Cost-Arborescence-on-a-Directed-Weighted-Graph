//Importing all the necessary libraries
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <utility>
#include <set>
using namespace std ;

//'edge' is used to implement the adjacency list. adjacency_list[start_vertex] points to {end_vertex,weight}
//'edge_uvw' is used to store edges in sets. It is of the form {{start_vertex,end_vertex},weight}
typedef pair<int,int> edge ;
typedef pair<pair<int,int>,int> edge_uvw ;


//Reading the input from the specified format and storing all the edges in a set
set<edge_uvw> collect_set_of_edges(int m, int n, int *invalid){

	set<edge_uvw> edges ;
	for(int i=1 ; i<=m ; i++){
		int u, v, w ;
		cin >> u >> v >> w ;
		if((u>n || v>n) || (u==v || w<0)){
			*invalid=1 ;
		}
		else{
			edges.insert(make_pair(make_pair(u,v),w)) ;
		}
	}
	return edges ;

}


//Used to delete an adjacency list when no longer required
void delete_weighted_graph(vector<edge> *graph, int n){

	for(int i=0 ; i<=2*n; i++){
		for(int j=0 ; j<graph[i].size() ; j++){
			graph[i].erase(graph[i].begin()) ;
		}
	}

}


//Used to modify the adjacency lists to prevent storing unnecessary edges of unreachable verticies
//Further creates a new adjacency list with adjacency list[end_vertex] points to {start_vertex,weight}, useful in edge contraction
void modify_adj_lists(int reachable[], vector<edge> *graph1, vector<edge> *graph2, int n){

	for(int i=1 ; i<=n ; i++){
		if(reachable[i]==0){
			for(int j=0 ; j<graph1[i].size() ; j++){
				graph1[i].erase(graph1[i].begin()) ;
			}
		}
	}
	for(int i=1 ; i<=n ; i++){
		for(auto j=graph1[i].begin() ; j!=graph1[i].end() ; ++j){
			int v = j->first, w = j->second ;
			graph2[v].push_back(make_pair(i,w)) ;
		}
	}

}


//Makes an adjacency list from a set of edges
void make_graph_from_edges(vector<edge> *graph, set<edge_uvw> edges){

	for(auto i=edges.begin() ; i!=edges.end() ; ++i){
		int u = (i->first).first, v = (i->first).second, w = i->second ; 
		graph[u].push_back(make_pair(v,w)) ;
	}

}


//Breadth First Search to identify all unreachable verticies
void bfs(int reachable[], vector<edge> *graph, int n, int s){

	queue<int> bfs_queue ;
	bfs_queue.push(s) ;
	reachable[s] = 1 ;
	while(!bfs_queue.empty()){
		int node = bfs_queue.front() ;
		bfs_queue.pop() ;
		for(auto i=graph[node].begin() ; i!=graph[node].end() ; ++i){
			int vertex = i->first ;
			if(reachable[vertex]==0){
				bfs_queue.push(vertex) ;
				reachable[vertex]=1 ;
			}
		}
	}

}


//Contracting all the edges, the first step of Edmond's Branching Algorithm
//For each edge, (new weight) = (original weight) - (minimum weight of edge entering the end_vertex)
void contract_edges(set<edge_uvw> &edges, vector<edge> *graph, int n, int s){

	for(int i=1 ; i<=2*n ; i++){
		int min=999999, u=0 ;
		for(auto j=graph[i].begin() ; j!=graph[i].end() ; ++j){
			int w = j->second ;
			if(w<=min){
				if(u==0){
					min = w ;
					u=j->first ;
				}
				else{
					if(w<min){
						min=w ;
						u=j->first ;
					}
					else if(u < j->first){
						u=j->first ;
					}
				}
			}
		}
		for(auto j=graph[i].begin() ; j!=graph[i].end() ; ++j){
			int contracted_weight = j->second - min, source = j->first ;
			if(contracted_weight==0 && source==(j->first)){
				if(i!=s){
					edges.insert(make_pair(make_pair(source,i),contracted_weight)) ;
				}
			}
			j->second = contracted_weight ;
		}
	}

}


//Uses DFS to detect a cycle from a set of edges (through an adjacency list)
//Uses the concept that a back edge detected during DFS on a directed graph indicates a cycle
void dfs_cycle_detect(int reachable[],set<int> &supernode_verticies, vector<edge> *graph, int prev_nodes[], int n, int s, int *cycle){

	stack<int> dfs_stack ;
	int black_nodes[2*n+1] ;
	int out_of_stack[2*n+1] ;
	for(int i=0 ; i<2*n+1 ; i++){
		black_nodes[i] = 0 ;
		out_of_stack[i] = 0 ;
	}
	int cycle_node1 = 0 ;
	int cycle_node2 = 0 ;
	for(int k=1 ; k<=2*n ; k++){
		if(black_nodes[k]==0){
			dfs_stack.push(k) ;
			prev_nodes[k] = 0 ;
			while(!dfs_stack.empty()){
				int node = dfs_stack.top() ;
				dfs_stack.pop() ;
				if(black_nodes[node]==0){
					black_nodes[node]=1 ;
					dfs_stack.push(node) ;
					for(auto i=graph[node].rbegin() ; i!=graph[node].rend() ; i++){
						int vertex = i->first ;
						if(black_nodes[vertex]==0){
							dfs_stack.push(vertex) ;
							prev_nodes[vertex]=node ;
						}
						else{
							if(out_of_stack[vertex]==0){
								*cycle=1 ;
								cycle_node1 = node ;
								cycle_node2 = vertex ;
								break ;
							}
						}
					}
				}
				else{
					out_of_stack[node] = 1 ;
				}
				if((*cycle)==1){
					break ;
				}
			}
			if((*cycle)==1){
				supernode_verticies.insert(cycle_node2) ;
				prev_nodes[cycle_node2]=cycle_node1 ;
				while(cycle_node1!=cycle_node2){
					supernode_verticies.insert(cycle_node1) ;
					cycle_node1 = prev_nodes[cycle_node1] ;
				}
				return ;
			}
		}
	}

}


//Determines whether a proposed arboroscence is an arboroscence or not. A cycle indicates that it is not an arboroscence
int is_arboroscence(int reachable[], set<int> &supernode_verticies, set<edge_uvw> edges, int prev_nodes[], int n, int s){

	int cycle = 0 ;
	vector<edge> *graph = new vector<edge>[2*n+1] ;
	make_graph_from_edges(graph,edges) ;
	dfs_cycle_detect(reachable,supernode_verticies,graph,prev_nodes,n,s,&cycle) ;
	delete_weighted_graph(graph,n) ;
	if(cycle==0){
		return 1 ;
	}
	else{
		return 0 ;
	}

}


//Contracts a cycle into a single supernode having a vertex number n+running_supernode (thus supernodes are numbered n+1, n+2, n+3, etc)
set<edge_uvw> contract_supernode(int arr[], set<int> supernode_verticies, int prev_nodes[], vector<edge> *graph, vector<edge> *graph_new, int n, int running_supernode){

	set<edge_uvw> cycle_edges ;
	for(auto i=supernode_verticies.begin() ; i!=supernode_verticies.end() ; ++i){
		arr[*i]=1 ;
		cycle_edges.insert(make_pair(make_pair(prev_nodes[*i],*i),0)) ;
	}
	for(int i=1 ; i<n+running_supernode ; i++){
		if(arr[i]==1){
			for(auto j=graph[i].begin() ; j!=graph[i].end() ; ++j){
				int u = j->first, w=j->second ;
				if(arr[u]==0){
					graph_new[n+running_supernode].push_back(make_pair(u,w)) ;
				}
			}
		}
		else{
			for(auto j=graph[i].begin() ; j!=graph[i].end() ; ++j){
				int u = j->first, w = j->second ;
				if(arr[u]==0){
					graph_new[i].push_back(make_pair(u,w)) ;
				}
				else{
					graph_new[i].push_back(make_pair(n+running_supernode,w)) ;
				}
			}
		}
	}
	return cycle_edges ;

}


//Expands a supernode back into its original cycle, and further modifies it to fit into the arboroscence as per Edmond's Branching Algorithm
set<edge_uvw> expand_supernode(int arr[], set<int> supernode_verticies, set<edge_uvw> edges, set<edge_uvw> &cycle_edges, vector<edge> *graph, int n, int running_supernode){

	set<edge_uvw> new_edges ;
	int u, v, w, x, y, z, node=0, min=999999 ;
	for(auto i=edges.begin() ; i!=edges.end() ; ++i){
		u=(i->first).first, v=(i->first).second, w=i->second ;
		if(v==(n+running_supernode)){
			break ;
		}
	}
	edges.erase({{u,v},w}) ;
	for(auto i=supernode_verticies.begin() ; i!=supernode_verticies.end() ; ++i){
		for(auto j=graph[*i].begin() ; j!=graph[*i].end() ; ++j){
			if((j->first)==u && (j->second)<min){
				node = *i ;
				min = j->second ;
			}
		}
	}
	for(auto i=cycle_edges.begin() ; i!=cycle_edges.end() ; ++i){
		x=(i->first).first, y=(i->first).second, z=i->second ;
		if(y==node){
			continue ;
		}
		else{
			new_edges.insert(make_pair(make_pair(x,y),z)) ;
		}
	}
	cycle_edges.clear() ;
	new_edges.insert(make_pair(make_pair(u,node),0)) ;
	for(auto i=edges.begin() ; i!=edges.end() ; ++i){
		x=(i->first).first, y=(i->first).second, z=i->second ;
		if(x==(n+running_supernode)){
			for(auto j=graph[y].begin() ; j!=graph[y].end() ; ++j){
				int a=j->first, b=j->second ;
				if(arr[a]==1 && b==z){
					new_edges.insert(make_pair(make_pair(a,y),z)) ;
				}
			}
		}
		else{
			new_edges.insert(make_pair(make_pair(x,y),z)) ;
		}
	}
	return new_edges ;

}


//Implementing Edmond's Branching Algorithm recursively 
set<edge_uvw> min_cost_arboroscence(int reachable[], vector<edge> *graph2, int n, int s, int running_supernode){

	set<edge_uvw> edges ;
	set<int> supernode_verticies ;
	int prev_nodes[2*n+1] ;
	for(int i=0 ; i<=2*n ; i++){
		prev_nodes[i]=-1 ;
	}
	contract_edges(edges,graph2,n,s) ;
	int done = is_arboroscence(reachable,supernode_verticies,edges,prev_nodes,n,s) ;
	if(done==1){
		return edges ;
	}
	else{
		edges.clear() ;
		int arr[2*n+1] ;
		for(int i=0 ; i<2*n+1 ; i++){
			arr[i]=0 ;
		}
		vector<edge> *graph_new = new vector<edge>[2*n+1] ;
		set<edge_uvw> cycle_edges = contract_supernode(arr,supernode_verticies,prev_nodes,graph2,graph_new,n,running_supernode) ;
		edges = min_cost_arboroscence(reachable,graph_new,n,s,running_supernode+1) ;
		edges = expand_supernode(arr,supernode_verticies,edges,cycle_edges,graph2,n,running_supernode) ;
		delete_weighted_graph(graph_new,n) ;
		return edges ;
	}

}


//Printing the minimum cost arboroscence in the specified output format
void print_output(vector<edge> *graph, set<edge_uvw> edges, int reachable[], int n, int s){

	int prev_node[n+1], weight[n+1], total_dist[n+1], arboroscence_sum=0 ;
	for(int i=1 ; i<=n ; i++){
		prev_node[i] = 0 ;
		weight[i] = 0 ;
		total_dist[i] = 0 ;
	}
	for(auto i=edges.begin() ; i!=edges.end() ; ++i){
		int u=(i->first).first, v=(i->first).second, w=0 ;
		prev_node[v]=u ;
		for(auto j=graph[u].begin() ; j!=graph[u].end() ; j++){
			if(j->first == v){
				w = j->second ;
				break ;
			}
		}
		weight[v]=w ;
		arboroscence_sum += w ;
	}
	for(int i=1 ; i<=n ; i++){
		if(reachable[i]==0){
			continue ;
		}
		else{
			int node=i ;
			while(prev_node[node]!=0){
				total_dist[i]+=weight[node] ;
				node=prev_node[node] ;
			}
		}
	}
	cout << arboroscence_sum ;
	for(int i=1 ; i<=n ; i++){
		if(reachable[i]==0){
			cout << " -1" ;
		}
		else{
			cout << " " << total_dist[i] ;
		}
	}
	cout << " #" ;
	for(int i=1 ; i<=n ; i++){
		if(reachable[i]==0){
			cout << " -1" ;
		}
		else{
			cout << " " << prev_node[i] ;
		}
	}


}


//To evaluate each test case
void perform_ops(){

	int invalid=0, running_supernode=1 ;
	int m, n, s ;
	cin >> n >> m >> s ;
	set<edge_uvw> initial_edges = collect_set_of_edges(m,n,&invalid) ;
	if(invalid==1){
		cout << "-1" ;
	}
	else{
		int reachable[n+1] ;
		for(int i=1 ; i<=n ; i++){
			reachable[i]=0 ;
		}
		vector<edge> *graph1 = new vector<edge>[2*n+1] ;
		vector<edge> *graph2 = new vector<edge>[2*n+1] ;
		make_graph_from_edges(graph1,initial_edges) ;
		bfs(reachable,graph1,n,s) ;
		modify_adj_lists(reachable,graph1,graph2,n) ;
		set<edge_uvw> edges = min_cost_arboroscence(reachable,graph2,n,s,running_supernode) ;
		print_output(graph1,edges,reachable,n,s) ;
		delete_weighted_graph(graph1,n) ;
	}

}


int main(){

	int t ;
	cin >> t ;

	while(t>0){

		perform_ops() ;

		if(t>1){
			cout << endl ;
		}

		t-- ;

	}

	return 0 ;

}




