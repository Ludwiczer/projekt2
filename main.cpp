#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <sstream>
#include <string>
#include <limits>

using Graph = std::vector<std::vector<int>>;
std::mutex cout_mutex;

void BFS(const Graph& graph, int start_vertex, int graph_id) {
    if (graph.empty()) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Graf " << graph_id << " jest pusty!" << std::endl;
        return;
    }

    size_t n = graph.size();
    std::vector<bool> visited(n, false);
    std::queue<int> queue;

    visited[start_vertex] = true;
    queue.push(start_vertex);

    std::vector<int> result;
    
    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();
        
        result.push_back(current);
        
        for (int neighbor : graph[current]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                queue.push(neighbor);
            }
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Graf #" << graph_id << " - BFS od wierzchołka " << start_vertex << ": ";
        for (int v : result) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }
}

void readGraph(Graph& graph, int graph_id) {
    int n;
    std::cout << "\nPodaj liczbę wierzchołków dla grafu #" << graph_id << ": ";
    while (!(std::cin >> n) || n <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Niepoprawna liczba. Podaj dodatnią liczbę wierzchołków: ";
    }

    graph.resize(n);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ważne!

    std::cout << "Dla każdego wierzchołka podaj sąsiadów (oddzielonych spacjami), zakończ linię enterem:\n";
    for (int i = 0; i < n; ++i) {
        std::cout << "Wierzchołek " << i << ": ";
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        int neighbor;

        while (iss >> neighbor) {
            if (neighbor < 0 || neighbor >= n) {
                std::cout << "Błędne dane! Wierzchołek " << neighbor << " jest poza zakresem 0 do " << (n - 1) << ". Zostanie pominięty." << std::endl;
                continue;
            }
            graph[i].push_back(neighbor);
        }
    }
}

int main() {
    std::cout << "Program do przeszukiwania grafu metodą BFS w wielu wątkach\n" << std::endl;

    int graph_count;
    std::cout << "Ile grafów chcesz wczytać? (minimum 2): ";
    while (!(std::cin >> graph_count) || graph_count < 2) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Niepoprawna liczba. Podaj liczbę >= 2: ";
    }

    std::vector<Graph> graphs(graph_count);
    std::vector<int> start_vertices(graph_count);

    for (int i = 0; i < graph_count; ++i) {
        readGraph(graphs[i], i + 1);

        if (!graphs[i].empty()) {
            std::cout << "Podaj wierzchołek startowy dla grafu #" << (i + 1) << " (0-" << (graphs[i].size() - 1) << "): ";
            while (!(std::cin >> start_vertices[i]) || start_vertices[i] < 0 || start_vertices[i] >= graphs[i].size()) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Niepoprawna liczba. Podaj wierzchołek od 0 do " << (graphs[i].size() - 1) << ": ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            start_vertices[i] = 0;
        }
    }

    std::vector<std::thread> threads;

    std::cout << "\nRozpoczynanie przeszukiwania BFS w osobnych wątkach..." << std::endl;

    for (int i = 0; i < graph_count; ++i) {
        threads.emplace_back(BFS, std::ref(graphs[i]), start_vertices[i], i + 1);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nWszystkie wątki zakończyły działanie." << std::endl;

    std::cout << "Naciśnij ENTER, aby zakończyć...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    return 0;
}
