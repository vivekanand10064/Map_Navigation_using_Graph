#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <limits>
#include <algorithm>
#include <sstream>
#include <set>
using namespace std;
class Location {
public:
    string name;
    string description;

    Location(string n, string d = "") : name(n), description(d) {}
};
class Road {
public:
    int destination;
    double distance; 
    Road(int dest, double dist) : destination(dest), distance(dist) {}
};
class MapGraph {
private:
    vector<Location> locations;
    unordered_map<int, vector<Road>> adjList;

public:
    int addLocation(const string& name, const string& desc = "") {
        locations.emplace_back(name, desc);
        return locations.size() - 1;
    }
    void addRoad(int src, int dest, double distance) {
        adjList[src].emplace_back(dest, distance);
        adjList[dest].emplace_back(src, distance); 
    }
    int getLocationCount() const {
        return locations.size();
    }

    const vector<Road>& getRoadsFrom(int loc) const {
        static const vector<Road> empty;
        auto it = adjList.find(loc);
        return it != adjList.end() ? it->second : empty;
    }

    const Location& getLocation(int id) const {
        return locations[id];
    }
    //dijkstra algorithm
    vector<int> shortestPathByDistance(int start, int end) {
        int n = locations.size();
        vector<double> dist(n, numeric_limits<double>::infinity());
        vector<int> prev(n, -1);
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;

        dist[start] = 0;
        pq.emplace(0, start);

        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (u == end) break;

            for (const Road& road : adjList[u]) {
                int v = road.destination;
                double alt = d + road.distance;
                if (alt < dist[v]) {
                    dist[v] = alt;
                    prev[v] = u;
                    pq.emplace(alt, v);
                }
            }
        }

        vector<int> path;
        for (int at = end; at != -1; at = prev[at])
            path.push_back(at);

        reverse(path.begin(), path.end());
        return path;
    }
};
class MapVisualizer {
private:
    sf::RenderWindow window;
    MapGraph& map;
    unordered_map<int, sf::Vector2f> nodePositions;
    vector<int> currentPath;
    double totalDistance = 0; 
    double totalTime = 0;    

public:
    MapVisualizer(MapGraph& m) : map(m), window(sf::VideoMode(800, 600), "CU Map Navigation") {
        nodePositions = {
            {0, sf::Vector2f(100, 100)},  // Block A
            {1, sf::Vector2f(300, 100)},  // Block B
            {2, sf::Vector2f(500, 200)},  // Block C
            {3, sf::Vector2f(300, 300)},  // Block D
            {4, sf::Vector2f(100, 400)}   // Hostel
        };
    }

    void setPathInfo(const vector<int>& path, MapGraph& map) {
        currentPath = path;
        totalDistance = 0;
        totalTime = 0;

        for (size_t i = 1; i < path.size(); ++i) {
            for (const auto& road : map.getRoadsFrom(path[i-1])) {
                if (road.destination == path[i]) {
                    totalDistance += road.distance;
                    totalTime += road.distance * 2.0; 
                    break;
                }
            }
        }
    }

    void drawMap() {
        window.clear(sf::Color::White);

        sf::Font font;
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cerr << "Error: Cannot load font\n";
            return;
        }
        sf::VertexArray allRoads(sf::Lines);
        set<pair<int, int>> drawnEdges;

        for (int i = 0; i < map.getLocationCount(); ++i) {
            for (const Road& road : map.getRoadsFrom(i)) {
                int u = i;
                int v = road.destination;
                if (drawnEdges.count({u, v}) || drawnEdges.count({v, u}))
                    continue;

                drawnEdges.insert({u, v});
                allRoads.append(sf::Vertex(nodePositions[u], sf::Color::Blue));
                allRoads.append(sf::Vertex(nodePositions[v], sf::Color::Blue));
            }
        }
        window.draw(allRoads);
        if (!currentPath.empty() && currentPath.size() > 1) {
            sf::VertexArray shortestPath(sf::LinesStrip, currentPath.size());
            for (size_t i = 0; i < currentPath.size(); ++i) {
                shortestPath[i].position = nodePositions[currentPath[i]];
                shortestPath[i].color = sf::Color::Green;
            }
            sf::RenderStates states;
            sf::VertexArray thickLine(sf::TrianglesStrip);
            const float thickness = 5.0f;
            
            for (size_t i = 1; i < currentPath.size(); ++i) {
                sf::Vector2f p1 = nodePositions[currentPath[i-1]];
                sf::Vector2f p2 = nodePositions[currentPath[i]];
                
                sf::Vector2f direction = p2 - p1;
                sf::Vector2f unitDirection = direction / sqrt(direction.x*direction.x + direction.y*direction.y);
                sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);
                
                sf::Vector2f offset = (thickness / 2.0f) * unitPerpendicular;
                
                thickLine.append(sf::Vertex(p1 + offset, sf::Color::Green));
                thickLine.append(sf::Vertex(p1 - offset, sf::Color::Green));
                thickLine.append(sf::Vertex(p2 + offset, sf::Color::Green));
                thickLine.append(sf::Vertex(p2 - offset, sf::Color::Green));
            }
            
            window.draw(thickLine);
        }
        for (const auto& [id, pos] : nodePositions) {
            sf::CircleShape node(20);
            node.setPosition(pos.x - 20, pos.y - 20);
            node.setFillColor(sf::Color::Red);
            window.draw(node);

            sf::Text label(map.getLocation(id).name, font, 14);
            label.setFillColor(sf::Color::Green);
            label.setPosition(pos.x - 30, pos.y - 40);
            window.draw(label);
        }
        sf::Text info;
        info.setFont(font);
        info.setCharacterSize(16);
        info.setFillColor(sf::Color::Black);
        info.setPosition(10, 550);

        ostringstream ss;
        ss.precision(2);
        ss << std::fixed;
        ss << "Shortest Path: " << totalDistance << " km | Estimated Time: " << totalTime << " mins";
        info.setString(ss.str());
        window.draw(info);

        window.display();
    }

    void run() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed ||
                    (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                    window.close();
                }
            }

            drawMap();
        }
    }
};
int main() {
    MapGraph cuMap;

    // Add locations with realistic distances in km
    int blockA = cuMap.addLocation("Block A", "Admin Block");
    int blockB = cuMap.addLocation("Block B", "Engineering Block");
    int blockC = cuMap.addLocation("Block C", "Management Block");
    int blockD = cuMap.addLocation("Block D", "Law Department");
    int hostel = cuMap.addLocation("Hostel", "Boys/Girls Hostel");

    // Add roads with distances 
    cuMap.addRoad(blockA, blockB, 0.5);   
    cuMap.addRoad(blockB, blockC, 0.8);   
    cuMap.addRoad(blockC, blockD, 1.0);   
    cuMap.addRoad(blockD, hostel, 1.2); 
    cuMap.addRoad(blockA, hostel, 1.5);  
    cuMap.addRoad(blockB, blockD, 0.7);   
    cuMap.addRoad(blockC, hostel, 0.9); 
    cout << "Available Locations:\n";
    for (int i = 0; i < cuMap.getLocationCount(); ++i) {
        std::cout << i << ". " << cuMap.getLocation(i).name << " - " << cuMap.getLocation(i).description << "\n";
    }

    int start, end;
    cout << "Enter start location number: ";
    cin >> start;
    cout << "Enter destination location number: ";
    cin >> end;

    vector<int> path = cuMap.shortestPathByDistance(start, end);

    MapVisualizer visualizer(cuMap);
    visualizer.setPathInfo(path, cuMap);
    visualizer.run();

    return 0;
}