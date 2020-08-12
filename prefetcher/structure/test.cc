#include <iostream>
#include <string>
#include <random>
#include <ctime>
#include <algorithm>
#include <set>

//#define GRAPH_DEBUG
#include "graph.hh"
#include "format.hh"

class GTL_UNDIRECTED_GRAPH_TEST {
private:
    GTL::Graph<
        GTL::NodeTraits<GTL::Name<std::string>, std::string>,
        GTL::EdgeTraits<GTL::Name<std::string>, std::string>,
        GTL::Undirected> graph_;
    // Random device
    std::random_device rd_;
    std::mt19937 mersenne_;
public:

    GTL_UNDIRECTED_GRAPH_TEST() : rd_(), mersenne_(rd_()) { mersenne_.seed(time(0)); }

    void TEST_insert_node(std::size_t num) {
        std::cout << "\n[GTL_UNDIRECTED_GRAPH_TEST] Graph::insert_node\n";
        for (int i = 0; i < num; i++) {
            graph_.insert_node(
                {format::sprintf_static("%d", i), format::sprintf_static("node%d-data", i)});
            std::cout << format::sprintf_static("    Node %d is inserted.\n", i);
        }
    }
    void TEST_insert_edge(std::size_t num) {
        std::cout << "\n[GTL_UNDIRECTED_GRAPH_TEST] Graph::insert_edge\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        for (int i = 0; i < num; i++) {
            auto u = nodes[dist(mersenne_)];
            auto v = nodes[dist(mersenne_)];
            try {
                graph_.insert_edge(
                    {format::sprintf_static("%s-%s", u.name().c_str(), v.name().c_str()),
                     format::sprintf_static("edge%d-%d-data", u.name().c_str(), v.name().c_str())}, u, v);
                std::cout << format::sprintf_static("    Edge %s-%s is inserted.\n", u.name().c_str(), v.name().c_str());
            } catch (std::runtime_error& err) {
                std::cout << "   Node name duplicated.\n";
                i--; continue;
            }
        }
    }
    void TEST_remove_edge(std::size_t num) {
        std::cout << "\n[GTL_UNDIRECTED_GRAPH_TEST] Graph::remove_edge\n";
        auto edges = graph_.edges();
        std::uniform_int_distribution<int> dist(0, edges.size()-1);
        std::set<int> indices;
        while (indices.size() != num) {
            indices.insert(dist(mersenne_));
        }
        for (const auto& idx : indices) {
            auto name = edges[idx].name();
            graph_.remove_edge(edges[idx]);
            std::cout << format::sprintf_static("    Edge %s is removed.\n", name.c_str());
        }
    }
    void TEST_remove_node(std::size_t num) {
        std::cout << "\n[GTL_UNDIRECTED_GRAPH_TEST] Graph::remove_node\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        std::set<int> indices;
        while (indices.size() != ((num > nodes.size()) ? nodes.size() : num)) {
            indices.insert(dist(mersenne_));
        }
        for (const auto& idx : indices) {
            auto name = nodes[idx].name();
            graph_.remove_node(nodes[idx]);
            std::cout << format::sprintf_static("    Node %s is removed.\n", name.c_str());
        }
    }
    void TEST_incident_edges(std::size_t num) {
        std::cout << "\n[GTL_UNDIRECTED_GRAPH_TEST] NodeDescriptor::incident_edges\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        std::set<int> indices;
        while (indices.size() != ((num > nodes.size()) ? nodes.size() : num)) {
            indices.insert(dist(mersenne_));
        }
        for (const auto& idx : indices) {
            auto name = nodes[idx].name();
            auto incidents = nodes[idx].incident_edges();
            std::cout << format::sprintf_static("    Incident edges on node %s is:\n", name.c_str());
            for (const auto& e : incidents) {
                std::cout << format::sprintf_static("        Edge %s\n", e.name().c_str());
            }
        }
    }
    void TEST_is_adjacent_to(std::size_t num) {
        std::cout << "\n[GTL_UNDIRECTED_GRAPH_TEST] NodeDescriptor::is_adjacent_to\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        for (int i = 0; i < num; i++) {
            auto u = nodes[dist(mersenne_)];
            auto v = nodes[dist(mersenne_)];
            if (u.is_adjacent_to(v))
                std::cout << format::sprintf_static("    Node %s is adjacent to node %s\n", 
                    u.name().c_str(), v.name().c_str());
            else
                std::cout << format::sprintf_static("    Node %s is \"not\" adjacent to node %s\n",
                    u.name().c_str(), v.name().c_str());
        }
    }
    void TEST_opposite(std::size_t num) {
        std::cout << "\n[GTL_UNDIRECTED_GRAPH_TEST] EdgeDescriptor::opposite\n";
        auto edges = graph_.edges();
        std::uniform_int_distribution<int> dist(0, edges.size()-1);
        std::set<int> indices;
        while (indices.size() != ((num > edges.size()) ? edges.size() : num)) {
            indices.insert(dist(mersenne_));
        }
        for (const auto& idx : indices) {
            auto e = edges[idx];
            auto u = e.end_nodes().first;
            std::cout << format::sprintf_static("    The opposite node of %s along the edge %s is %s.\n",
                u.name().c_str(), e.name().c_str(), e.opposite(u).name().c_str());
        }
    }
    void TEST_is_incident_on(std::size_t num) {
        std::cout << "\n[GTL_UNDIRECTED_GRAPH_TEST] EdgeDescriptor::is_incident_on\n";
        auto edges = graph_.edges();
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, edges.size()-1);
        for (int i = 0; i < num; i++) {
            auto e = edges[dist(mersenne_)];
            auto u = nodes[dist(mersenne_)];
            if (e.is_incident_on(u))
                std::cout << format::sprintf_static("    Edge %s is incident on node %s\n",
                    e.name().c_str(), u.name().c_str());
            else
                std::cout << format::sprintf_static("    Edge %s is \"not\" incident on node %s\n",
                    e.name().c_str(), u.name().c_str());
        }
    }
};

class GTL_DIRECTED_GRAPH_TEST {
private:
    GTL::Graph<
        GTL::NodeTraits<GTL::Name<std::string>, std::string>,
        GTL::EdgeTraits<GTL::Name<std::string>, std::string>,
        GTL::Directed> graph_;
    // Random device
    std::random_device rd_;
    std::mt19937 mersenne_;
public:

    GTL_DIRECTED_GRAPH_TEST() : rd_(), mersenne_(rd_()) { mersenne_.seed(time(0)); }

    void TEST_insert_node(std::size_t num) {
        std::cout << "\n[GTL_DIRECTED_GRAPH_TEST] Graph::insert_node\n";
        for (int i = 0; i < num; i++) {
            graph_.insert_node(
                {format::sprintf_static("%d", i), format::sprintf_static("node%d-data", i)});
            std::cout << format::sprintf_static("    Node %d is inserted.\n", i);
        }
    }
    void TEST_insert_edge(std::size_t num) {
        std::cout << "\n[GTL_DIRECTED_GRAPH_TEST] Graph::insert_edge\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        for (int i = 0; i < num; i++) {
            auto u = nodes[dist(mersenne_)];
            auto v = nodes[dist(mersenne_)];
            try {
                graph_.insert_edge(
                    {format::sprintf_static("%s->%s", u.name().c_str(), v.name().c_str()),
                     format::sprintf_static("edge%d->%d-data", u.name().c_str(), v.name().c_str())}, u, v);
                std::cout << format::sprintf_static("    Edge %s->%s is inserted.\n", u.name().c_str(), v.name().c_str());
            } catch (std::runtime_error& err) {
                std::cout << "   Node name duplicated.\n";
                i--; continue;
            }
        }
    }
    void TEST_remove_edge(std::size_t num) {
        std::cout << "\n[GTL_DIRECTED_GRAPH_TEST] Graph::remove_edge\n";
        auto edges = graph_.edges();
        std::uniform_int_distribution<int> dist(0, edges.size()-1);
        std::set<int> indices;
        while (indices.size() != num) {
            indices.insert(dist(mersenne_));
        }
        for (const auto& idx : indices) {
            auto name = edges[idx].name();
            graph_.remove_edge(edges[idx]);
            std::cout << format::sprintf_static("    Edge %s is removed.\n", name.c_str());
        }
    }
    void TEST_remove_node(std::size_t num) {
        std::cout << "\n[GTL_DIRECTED_GRAPH_TEST] Graph::remove_node\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        std::set<int> indices;
        while (indices.size() != ((num > nodes.size()) ? nodes.size() : num)) {
            indices.insert(dist(mersenne_));
        }
        for (const auto& idx : indices) {
            auto name = nodes[idx].name();
            graph_.remove_node(nodes[idx]);
            std::cout << format::sprintf_static("    Node %s is removed.\n", name.c_str());
        }
    }
    void TEST_outgoing_edges(std::size_t num) {
        std::cout << "\n[GTL_DIRECTED_GRAPH_TEST] NodeDescriptor::outgoing_edges\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        std::set<int> indices;
        while (indices.size() != ((num > nodes.size()) ? nodes.size() : num)) {
            indices.insert(dist(mersenne_));
        }
        for (const auto& idx : indices) {
            auto name = nodes[idx].name();
            auto outgoings = nodes[idx].outgoing_edges();
            std::cout << format::sprintf_static("    Outgoing edges on node %s is:\n", name.c_str());
            for (const auto& e : outgoings) {
                std::cout << format::sprintf_static("        Edge %s\n", e.name().c_str());
            }
        }
    }
    void TEST_incoming_edges(std::size_t num) {
        std::cout << "\n[GTL_DIRECTED_GRAPH_TEST] NodeDescriptor::incoming_edges\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        std::set<int> indices;
        while (indices.size() != ((num > nodes.size()) ? nodes.size() : num)) {
            indices.insert(dist(mersenne_));
        }
        for (const auto& idx : indices) {
            auto name = nodes[idx].name();
            auto incomings = nodes[idx].incoming_edges();
            std::cout << format::sprintf_static("    Incoming edges on node %s is:\n", name.c_str());
            for (const auto& e : incomings) {
                std::cout << format::sprintf_static("        Edge %s\n", e.name().c_str());
            }
        }
    }
    void TEST_point_to(std::size_t num) {
        std::cout << "\n[GTL_DIRECTED_GRAPH_TEST] NodeDescriptor::point_to\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        for (int i = 0; i < num; i++) {
            auto u = nodes[dist(mersenne_)];
            auto v = nodes[dist(mersenne_)];
            if (u.point_to(v))
                std::cout << format::sprintf_static("    Node %s points to node %s\n", 
                    u.name().c_str(), v.name().c_str());
            else
                std::cout << format::sprintf_static("    Node %s dosen't point to node %s\n",
                    u.name().c_str(), v.name().c_str());
        }
    }
    void TEST_is_pointed_by(std::size_t num) {
        std::cout << "\n[GTL_DIRECTED_GRAPH_TEST] NodeDescriptor::is_pointed_by\n";
        auto nodes = graph_.nodes();
        std::uniform_int_distribution<int> dist(0, nodes.size()-1);
        for (int i = 0; i < num; i++) {
            auto u = nodes[dist(mersenne_)];
            auto v = nodes[dist(mersenne_)];
            if (u.is_pointed_by(v))
                std::cout << format::sprintf_static("    Node %s is pointed by node %s\n", 
                    u.name().c_str(), v.name().c_str());
            else
                std::cout << format::sprintf_static("    Node %s isn't pointed by node %s\n",
                    u.name().c_str(), v.name().c_str());
        }
    }
    void TEST_opposite(std::size_t num) {
        std::cout << "\n[GTL_UNDIRECTED_GRAPH_TEST] EdgeDescriptor::opposite\n";
        auto edges = graph_.edges();
        std::uniform_int_distribution<int> dist(0, edges.size()-1);
        std::set<int> indices;
        while (indices.size() != ((num > edges.size()) ? edges.size() : num)) {
            indices.insert(dist(mersenne_));
        }
        for (const auto& idx : indices) {
            auto e = edges[idx];
            auto u = e.src();
            std::cout << format::sprintf_static("    The opposite node of %s along the edge %s is %s.\n",
                u.name().c_str(), e.name().c_str(), e.opposite(u).name().c_str());
        }
    }
};

int main() {

    GTL_UNDIRECTED_GRAPH_TEST UNDIRECTED_GRAPH_TESTER;

    UNDIRECTED_GRAPH_TESTER.TEST_insert_node(50);
    UNDIRECTED_GRAPH_TESTER.TEST_insert_edge(70);
    UNDIRECTED_GRAPH_TESTER.TEST_remove_edge(10);
    UNDIRECTED_GRAPH_TESTER.TEST_remove_node(10);

    UNDIRECTED_GRAPH_TESTER.TEST_incident_edges(10);
    UNDIRECTED_GRAPH_TESTER.TEST_is_adjacent_to(50);

    UNDIRECTED_GRAPH_TESTER.TEST_opposite(10);
    UNDIRECTED_GRAPH_TESTER.TEST_is_incident_on(50);

    GTL_DIRECTED_GRAPH_TEST DIRECTED_GRAPH_TESTER;

    DIRECTED_GRAPH_TESTER.TEST_insert_node(50);
    DIRECTED_GRAPH_TESTER.TEST_insert_edge(70);
    DIRECTED_GRAPH_TESTER.TEST_remove_edge(10);
    DIRECTED_GRAPH_TESTER.TEST_remove_node(10);

    DIRECTED_GRAPH_TESTER.TEST_incoming_edges(10);
    DIRECTED_GRAPH_TESTER.TEST_outgoing_edges(10);
    DIRECTED_GRAPH_TESTER.TEST_point_to(50);
    DIRECTED_GRAPH_TESTER.TEST_is_pointed_by(50);

    DIRECTED_GRAPH_TESTER.TEST_opposite(10);

    return 0;
}