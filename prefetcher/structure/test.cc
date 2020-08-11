#include <iostream>
#include <string>
#define GRAPH_DEBUG
#include "graph.hh"

int main() {

    // undirected graph
    {
        GTL::Graph<
                GTL::NodeTraits<
                        GTL::Name<std::string>, std::string>,
                GTL::EdgeTraits<
                        GTL::Name<std::string>, std::string>,
                GTL::Undirected> graph;

        std::cout << "[Test.Graph] insert_node (undirected)" << std::endl;

        auto node0 = graph.insert_node({"0", "node0-data"});
        auto node1 = graph.insert_node({"1", "node1-data"});
        auto node2 = graph.insert_node({"2", "node2-data"});
        auto node3 = graph.insert_node({"3", "node3-data"});
        auto node4 = graph.insert_node({"4", "node4-data"});
        auto node5 = graph.insert_node({"5", "node5-data"});
        auto node6 = graph.insert_node({"6", "node6-data"});
        auto node7 = graph.insert_node({"7", "node7-data"});
        auto node8 = graph.insert_node({"8", "node8-data"});
        auto node9 = graph.insert_node({"9", "node9-data"});

        try {
            auto node = graph.insert_node({"5", "?"});
        } catch (std::runtime_error &err) {
            std::cout << "Node name duplication." << std::endl;
        }

        std::cout << "[Test.Graph] insert_edge (undirected)" << std::endl;

        auto edge01 = graph.insert_edge({"01", "edge01-data"}, node0, node1);
        auto edge02 = graph.insert_edge({"02", "edge02-data"}, node0, node2);
        auto edge03 = graph.insert_edge({"03", "edge03-data"}, node0, node3);
        auto edge24 = graph.insert_edge({"24", "edge24-data"}, node2, node4);
        auto edge56 = graph.insert_edge({"56", "edge56-data"}, node5, node6);
        auto edge59 = graph.insert_edge({"59", "edge59-data"}, node5, node9);
        auto edge78 = graph.insert_edge({"78", "edge78-data"}, node7, node8);

        try {
            auto edge = graph.insert_edge({"01", "?"}, node8, node9);
        } catch (std::runtime_error &err) {
            std::cout << "Edge name duplication." << std::endl;
        }

        std::cout << "[Test.Graph] nodes (undirected)" << std::endl;

        auto nodes = graph.nodes<std::vector>();
        for (const auto &node : nodes) {
            std::cout << node.name() << std::endl;
        }

        std::cout << "[Test.Graph] edges (undirected)" << std::endl;

        auto edges = graph.edges<std::vector>();
        for (const auto &edge : edges) {
            std::cout << edge.name() << std::endl;
        }

        std::cout << "[Test.NodeDescriptor] incident_edges (undirected)" << std::endl;

        auto incidents = node0.incident_edges<std::vector>();
        for (const auto &edge : incidents) {
            std::cout << edge.name() << std::endl;
        }

        std::cout << "[Test.NodeDescriptor] is_adjacent_to (undirected)" << std::endl;

        std::cout << node0.is_adjacent_to(node1) << std::endl;
        std::cout << node0.is_adjacent_to(node2) << std::endl;
        std::cout << node0.is_adjacent_to(node3) << std::endl;
        std::cout << node0.is_adjacent_to(node4) << std::endl;

        std::cout << "[Test.EdgeDescriptor] opposite (undirected)" << std::endl;

        auto v = edge01.opposite(node0);
        std::cout << v.name() << std::endl;
        auto u = edge01.opposite(node1);
        std::cout << u.name() << std::endl;
        try {
            auto w = edge01.opposite(node2);
            std::cout << w.name() << std::endl;
        } catch (std::runtime_error &err) {
            std::cout << "Edge is not incident on the given node" << std::endl;
        }

        std::cout << "[Test.EdgeDescriptor] is_incident_on (undirected)" << std::endl;

        std::cout << edge01.is_incident_on(node0) << std::endl;
        std::cout << edge01.is_incident_on(node1) << std::endl;
        std::cout << edge01.is_incident_on(node2) << std::endl;

        std::cout << "[Test.EdgeDescriptor] end_nodes (undirected)" << std::endl;

        auto end_points = edge59.end_nodes();
        std::cout << end_points.first.name() << std::endl;
        std::cout << end_points.second.name() << std::endl;
    }

    // Directed graph
    {
        GTL::Graph<
                GTL::NodeTraits<
                        GTL::Name<std::string>, std::string>,
                GTL::EdgeTraits<
                        GTL::Name<std::string>, std::string>,
                GTL::Directed> graph;

        std::cout << "[Test.Graph] insert_node (directed)" << std::endl;

        auto node0 = graph.insert_node({"0", "node0-data"});
        auto node1 = graph.insert_node({"1", "node1-data"});
        auto node2 = graph.insert_node({"2", "node2-data"});
        auto node3 = graph.insert_node({"3", "node3-data"});
        auto node4 = graph.insert_node({"4", "node4-data"});
        auto node5 = graph.insert_node({"5", "node5-data"});
        auto node6 = graph.insert_node({"6", "node6-data"});
        auto node7 = graph.insert_node({"7", "node7-data"});
        auto node8 = graph.insert_node({"8", "node8-data"});
        auto node9 = graph.insert_node({"9", "node9-data"});

        try {
            auto node = graph.insert_node({"5", "?"});
        } catch (std::runtime_error &err) {
            std::cout << "Node name duplication." << std::endl;
        }

        std::cout << "[Test.Graph] insert_edge (directed)" << std::endl;

        auto edge01 = graph.insert_edge({"01", "edge01-data"}, node0, node1);
        auto edge02 = graph.insert_edge({"02", "edge02-data"}, node0, node2);
        auto edge03 = graph.insert_edge({"03", "edge03-data"}, node0, node3);
        auto edge24 = graph.insert_edge({"24", "edge24-data"}, node2, node4);
        auto edge52 = graph.insert_edge({"52", "edge52-data"}, node5, node2);
        auto edge59 = graph.insert_edge({"59", "edge59-data"}, node5, node9);
        auto edge78 = graph.insert_edge({"78", "edge78-data"}, node7, node8);

        try {
            auto edge = graph.insert_edge({"01", "?"}, node8, node9);
        } catch (std::runtime_error &err) {
            std::cout << "Edge name duplication." << std::endl;
        }

        std::cout << "[Test.Graph] nodes (directed)" << std::endl;

        auto nodes = graph.nodes<std::vector>();
        for (const auto &node : nodes) {
            std::cout << node.name() << std::endl;
        }

        std::cout << "[Test.Graph] edges (directed)" << std::endl;

        auto edges = graph.edges<std::vector>();
        for (const auto &edge : edges) {
            std::cout << edge.name() << std::endl;
        }

        std::cout << "[Test.NodeDescriptor] outgoing_edges (directed)" << std::endl;

        auto outgoings = node0.outgoing_edges<std::vector>();
        for (const auto &edge : outgoings) {
            std::cout << edge.name() << std::endl;
        }

        std::cout << "[Test.NodeDescriptor] incoming_edges (directed)" << std::endl;

        auto incomings = node2.incoming_edges<std::vector>();
        for (const auto &edge : incomings) {
            std::cout << edge.name() << std::endl;
        }

        std::cout << "[Test.NodeDescriptor] point_to (directed)" << std::endl;

        std::cout << node0.point_to(node1) << std::endl;
        std::cout << node0.point_to(node2) << std::endl;
        std::cout << node0.point_to(node3) << std::endl;
        std::cout << node0.point_to(node4) << std::endl;

        std::cout << "[Test.NodeDescriptor] is_pointed_by (directed)" << std::endl;

        std::cout << node2.is_pointed_by(node0) << std::endl;
        std::cout << node2.is_pointed_by(node5) << std::endl;
        std::cout << node2.is_pointed_by(node7) << std::endl;

        std::cout << "[Test.EdgeDescriptor] opposite (directed)" << std::endl;

        auto v = edge01.opposite(node0);
        std::cout << v.name() << std::endl;
        auto u = edge01.opposite(node1);
        std::cout << u.name() << std::endl;
        try {
            auto w = edge01.opposite(node2);
            std::cout << w.name() << std::endl;
        } catch (std::runtime_error &err) {
            std::cout << "Edge is not incident on the given node" << std::endl;
        }

        std::cout << "[Test.EdgeDescriptor] src (directed)" << std::endl;

        auto src = edge59.src();
        std::cout << src.name() << std::endl;

        std::cout << "[Test.EdgeDescriptor] dst (directed)" << std::endl;

        auto dst = edge59.dst();
        std::cout << dst.name() << std::endl;
    }

    return 0;
}
