#include <iostream>
#include <cstdio>

namespace Graph
{
constexpr unsigned MAX_ELEMENTS = 20;

using Vertex = char;
using Edge = int;

struct MatrixGraph
{
    Vertex vertex[MAX_ELEMENTS];
    Edge   arc[MAX_ELEMENTS][MAX_ELEMENTS];
    int    vertexNum;
    int    edgeNum;

    MatrixGraph( )
    {
        for ( auto element = std::begin( vertex ); element != std::end( vertex ); element++ )
        {
            *element = 0;
        }

        for ( size_t i = 0; i < MAX_ELEMENTS; i++ )
        {
            for ( size_t j = 0; j < MAX_ELEMENTS; j++ )
            {
                arc[i][j] = 0;
            }
        }
    }
};

int visited[MAX_ELEMENTS];

const MatrixGraph& CreateGraph( )
{
    static MatrixGraph graph;
    graph.vertexNum = 9;
    graph.edgeNum = 15;
    for ( int i = 0; i < MAX_ELEMENTS; i++ )
    {
        visited[i] = 0;
    }

    for ( size_t i = 0; i < 9; i++ )
    {
        static int temp = 'A';
        sprintf( &graph.vertex[i], "%c", temp++ );
    }

    graph.arc[0][1] = 1;
    graph.arc[0][5] = 1;
    graph.arc[1][2] = 1;
    graph.arc[1][6] = 1;
    graph.arc[1][8] = 1;
    graph.arc[2][3] = 1;
    graph.arc[2][8] = 1;
    graph.arc[3][4] = 1;
    graph.arc[3][6] = 1;
    graph.arc[3][8] = 1;
    graph.arc[4][5] = 1;
    graph.arc[4][7] = 1;
    graph.arc[5][6] = 1;
    graph.arc[6][7] = 1;

    for ( size_t i = 0; i < graph.edgeNum; i++ )
    {
        for ( size_t j = 0; j < graph.edgeNum; j++ )
        {
            graph.arc[j][i] = graph.arc[i][j];
        }
    }
    return graph;
}

void DFS( const MatrixGraph& graph, int vertex )    // 邻接矩阵 - 深度优先搜索
{
    visited[vertex] = 1;

    std::cout << graph.vertex[vertex] << '\n';

    for ( size_t col = 0; col < graph.vertexNum; ++col )
    {
        // 没有边 或者 邻接点已经被访问，则跳过
        if ( graph.arc[vertex][col] == 0 || visited[col] ) continue;   // 与当前顶点有连线并且没有被访问过的顶点

        DFS( graph, col );
    }
}
}    // namespace Graph

int main( )
{
    const Graph::MatrixGraph graph = Graph::CreateGraph( );
    Graph::DFS( graph, 0 );
    return 0;
}
