
#ifndef GTL_GRAPH_H
#define GTL_GRAPH_H

#include <functional>
#include <memory>
#include <map>
#include <vector>
#include <list>
#include <stdexcept>

//#define GRAPH_NDEBUG
#ifndef GRAPH_NDEBUG
    #include <iostream>
    #define GRAPH_DEBUG_PRINT(expr) expr
#else
    #define GRAPH_DEBUG_PRINT(expr)
#endif

namespace gtl {

    template <typename NodeTraits, typename EdgeTraits>
    class NodeBase;
    template <typename NodeTraits, typename EdgeTraits>
    class EdgeBase;
    
    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    class Node;
    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    class Edge;
    
    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    class __NodeDescriptor;
    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    class __EdgeDescriptor;

    // Tag dispatchers
    struct Undirected {};
    struct Directed {};

    /**
     * Each of node and edge instances is divided into three parts;
     * connnector, storage, and name.
     * 
     * The connector is a component that interconnects between edges
     * and nodes and between a map of edges and a map of nodes. 
     * 
     * The storage is a component that holds a user-defined storage
     * structure, and the storage type can be anything; for example
     * <\code>
     *     class SomeStorage {
     *     private:
     *         std::int64_t weight_;     // weight of the edge
     *         std::int64_t capacity_;   // capacity of the edge
     *         ...
     *     };
     * <\endcode>
     * 
     * The name is an identifier such as "key" in the stl map
     * structure, and all the names of each of nodes and edges are
     * maintained in the stl map. 
     * Hence, if you want to use some type as name of edge or node
     * that is not supported by default in the stl map, you must provide
     * some comparison function object type.
     */

    template <typename Key, typename Compare = std::less<Key>>
    struct Name {
        typedef Key        KeyType;
        typedef Compare    KeyCompare;
    };

    template <typename Name, typename Storage>
    struct NodeTraits {
        typedef typename Name::KeyType      KeyType;
        typedef typename Name::KeyCompare   KeyCompare;
        typedef Storage                     StorageType;
    };
    template <typename Name, typename Storage>
    struct EdgeTraits {
        typedef typename Name::KeyType      KeyType;
        typedef typename Name::KeyCompare   KeyCompare;
        typedef Storage                     StorageType;
    };

//namespace internal {

    template <typename NodeTraits, typename EdgeTraits>
    class NodeBase {
        template <typename, typename, typename> friend class Graph;
    public:
        // Basic typedefs
        using NodeBaseType = NodeBase<NodeTraits, EdgeTraits>;
        using EdgeBaseType = EdgeBase<NodeTraits, EdgeTraits>;
        // Extract inner types
        using NameType     = typename NodeTraits::KeyType;
        using NameCompare  = typename NodeTraits::KeyCompare;
        using StorageType  = typename NodeTraits::StorageType;
    public:
        using LinkToMap
            = typename std::map<
                NameType,
                std::shared_ptr<NodeBaseType>,
                NameCompare>::iterator;
        using IncidentEdges
            = std::map<
                NameType,
                std::shared_ptr<EdgeBaseType>,
                NameCompare>;
    protected:
        // This hooks the position of this node in stl map, which is
        // later used to remove the node from the map in constant time.
        LinkToMap     this_in_node_map_;
    protected:
        NameType      name_;
        StorageType   storage_;
    public:
        NodeBase(const NameType& name, const StorageType& storage) : name_(name), storage_(storage) {}
        ~NodeBase() { 
            GRAPH_DEBUG_PRINT(
                std::cout << "Node " << name_ << " base instance is destructed.\n";
            )
        }
    };

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    class Node
        : public NodeBase<NodeTraits, EdgeTraits>
    {};

    template <typename NodeTraits, typename EdgeTraits>
    class Node<NodeTraits, EdgeTraits, Undirected>
        : public NodeBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class Graph;
    public:
        using Base = NodeBase<NodeTraits, EdgeTraits>;
    private:
        // A node object v holds a reference to a collection I(v),
        // called the incidence collection of v, whose elements store
        // references to the edges incident on v.
        typename Base::IncidentEdges incidents_;
    public:
        Node(const typename Base::NameType& name, const typename Base::StorageType& storage)
            : Base(name, storage), incidents_() {}
        ~Node() { 
            GRAPH_DEBUG_PRINT(
                std::cout << "Node " << this->name_ << " derived (undirected) instance is destructed.\n";
            )
        }
    };

    template <typename NodeTraits, typename EdgeTraits>
    class Node<NodeTraits, EdgeTraits, Directed>
        : public NodeBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class Graph;
    public:
        using Base = NodeBase<NodeTraits, EdgeTraits>;
    private:
        // A node object v holds a reference to a collection I(v),
        // called the incidence collection of v, whose elements store
        // references to the edges incident on v.
        typename Base::IncidentEdges outgoings_;
        typename Base::IncidentEdges incomings_;
    public:
        Node(const typename Base::NameType& name, const typename Base::StorageType& storage)
            : Base(name, storage), outgoings_(), incomings_() {}
        ~Node() {
            GRAPH_DEBUG_PRINT(
                std::cout << "Node " << this->name_ << " derived (directed) instance is destructed.\n";
            )
        }
    };

    template <typename NodeTraits, typename EdgeTraits>
    class EdgeBase {
        template <typename, typename, typename> friend class Graph;
    public:
        // Basic typedefs
        using NodeBaseType = NodeBase<NodeTraits, EdgeTraits>;
        using EdgeBaseType = EdgeBase<NodeTraits, EdgeTraits>;
        // Extract inner types
        using NameType     = typename EdgeTraits::KeyType;
        using NameCompare  = typename EdgeTraits::KeyCompare;
        using StorageType  = typename EdgeTraits::StorageType;
    public:
        using LinkToNode
            = std::shared_ptr<NodeBaseType>;
        using LinkToMap
            = typename std::map<
                NameType,
                std::shared_ptr<EdgeBaseType>,
                NameCompare>::iterator;
        using IncidentEdgesPtr
            = typename std::map<
                NameType,
                std::shared_ptr<EdgeBaseType>,
                NameCompare>::iterator;
    protected:
        // This hooks the position of this edge in stl map, which is
        // later used to remove the edge from the map in constant time.
        LinkToMap     this_in_edge_map_;
    protected:
        NameType      name_;
        StorageType   storage_;
    public:
        EdgeBase(const NameType& name, const StorageType& storage) : name_(name), storage_(storage) {}
        ~EdgeBase() {
            GRAPH_DEBUG_PRINT(
                std::cout << "Edge " << this->name_ << " base instance is destructed.\n";
            )
        }
    };

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    class Edge
        : public EdgeBase<NodeTraits, EdgeTraits>
    {};

    template <typename NodeTraits, typename EdgeTraits>
    class Edge<NodeTraits, EdgeTraits, Undirected>
        : public EdgeBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class Graph;
    public:
        using Base = EdgeBase<NodeTraits, EdgeTraits>;
    private:
        typename Base::NodeBaseType
            *p_node1_{},
            *p_node2_{};
        typename Base::IncidentEdgesPtr
            this1_in_incidents_{},
            this2_in_incidents_{};
    public:
        Edge(const typename Base::NameType& name, const typename Base::StorageType& storage)
            : Base(name, storage) {}
        ~Edge() {
            GRAPH_DEBUG_PRINT(
                std::cout << "Edge " << this->name_ << " derived (undirected) instance is destructed.\n";
            )
        }
    };

    template <typename NodeTraits, typename EdgeTraits>
    class Edge<NodeTraits, EdgeTraits, Directed>
        : public EdgeBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class Graph;
    public:
        using Base = EdgeBase<NodeTraits, EdgeTraits>;
    private:
        typename Base::NodeBaseType
            *p_node_from_{},
            *p_node_to_{};
        typename Base::IncidentEdgesPtr
            this_in_outgoings_{},
            this_in_incomings_{};
    public:
        Edge(const typename Base::NameType& name, const typename Base::StorageType& storage)
            : Base(name, storage) {}
        ~Edge() {
            GRAPH_DEBUG_PRINT(
                std::cout << "Edge " << this->name_ << " derived (directed) instance is destructed.\n";
            )
        }
    };

//}   // namespace internal

    namespace type_traits {

        template <template <typename ...> class Container>
        struct is_stl_seq {
            static constexpr bool value = false;
        };
        
        template <> struct is_stl_seq<std::vector> { static constexpr bool value = true; };
        template <> struct is_stl_seq<std::list>   { static constexpr bool value = true; };

    }   // namespace type_traits

    template <typename NodeTraits, typename EdgeTraits>
    class __NodeDescriptorBase {
    public:
        // Basic typedefs
        using NodeBaseType   = NodeBase<NodeTraits, EdgeTraits>;
        // Extract inner types
        using NameType       = typename NodeTraits::KeyType;
        using NameCompare    = typename NodeTraits::KeyCompare;
        using StorageType    = typename NodeTraits::StorageType;
    protected:
        NodeBaseType* p_node_;
    public:

        __NodeDescriptorBase(NodeBaseType* p_node) : p_node_(p_node) {}

        // Get the name of this node
        NameType name() const { return p_node_->name_; }
        // Get reference of the storage of this node
        const StorageType& storage() const { return p_node_->storage_; }
              StorageType& storage()       { return p_node_->storage_; }
        // Get copy of the storage of this node
        StorageType storage_copy() const { return p_node_->storage_; }
    };

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    class __NodeDescriptor
        : public __NodeDescriptorBase<NodeTraits, EdgeTraits>
    {};

    template <typename NodeTraits, typename EdgeTraits>
    class __NodeDescriptor<NodeTraits, EdgeTraits, Undirected>
        : public __NodeDescriptorBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class Graph;
    public:
        using Base            = __NodeDescriptorBase<NodeTraits, EdgeTraits>;
    public:
        using NodeDescriptor  = __NodeDescriptor<NodeTraits, EdgeTraits, Undirected>;
        using EdgeDescriptor  = __EdgeDescriptor<NodeTraits, EdgeTraits, Undirected>;
    public:
        using NodeDerivedType = Node<NodeTraits, EdgeTraits, Undirected>;
    private:
        NodeDerivedType* p_node_derived_;
    public:

        __NodeDescriptor(typename Base::NodeBaseType* p_node)
            : Base(p_node), p_node_derived_(static_cast<NodeDerivedType*>(this->p_node_)) {}

        template <template <typename ...> class Container,
            typename std::enable_if<
                type_traits::is_stl_seq<Container>::value, void>::type* = nullptr >
        auto incident_edges() const -> Container<EdgeDescriptor>
        {
            Container<EdgeDescriptor> seq;
            const auto& incidents = *(this->p_node_derived_->incidents_);
            for (auto it = incidents.begin(); it != incidents.end(); it++) {
                seq.push_back(EdgeDescriptor(it->second.get()));
            }
            return seq;
        }
    };

    template <typename NodeTraits, typename EdgeTraits>
    class __NodeDescriptor<NodeTraits, EdgeTraits, Directed>
        : public __NodeDescriptorBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class Graph;
    public:
        using Base           = __NodeDescriptorBase<NodeTraits, EdgeTraits>;
    public:
        using NodeDescriptor = __NodeDescriptor<NodeTraits, EdgeTraits, Directed>;
        using EdgeDescriptor = __EdgeDescriptor<NodeTraits, EdgeTraits, Directed>;
    public:
        using NodeDerivedType = Node<NodeTraits, EdgeTraits, Undirected>;
    private:
        NodeDerivedType* p_node_derived_;
    public:

        __NodeDescriptor(typename Base::NodeBaseType* p_node)
            : Base(p_node), p_node_derived_(static_cast<NodeDerivedType*>(this->p_node_)) {}

        template <template <typename ...> class Container,
            typename std::enable_if<
                type_traits::is_stl_seq<Container>::value, void>::type* = nullptr >
        auto outgoing_edges() const -> Container<EdgeDescriptor>
        {
            Container<EdgeDescriptor> seq;
            const auto& outgoings = *(this->p_node_derived_->outgoings_);
            for (auto it = outgoings.begin(); it != outgoings.end(); it++) {
                seq.push_back(EdgeDescriptor(it->second.get()));
            }
            return seq;
        }
        template <template <typename ...> class Container,
            typename std::enable_if<
                type_traits::is_stl_seq<Container>::value, void>::type* = nullptr >
        auto incoming_edges() const -> Container<EdgeDescriptor>
        {
            Container<EdgeDescriptor> seq;
            const auto& incomings = *(this->p_node_derived_->incomings_);
            for (auto it = incomings.begin(); it != incomings.end(); it++) {
                seq.push_back(EdgeDescriptor(it->second.get()));
            }
            return seq;
        }
    };

    template <typename NodeTraits, typename EdgeTraits>
    class __EdgeDescriptorBase {
    public:
        // Basic typedefs
        using EdgeBaseType   = EdgeBase<NodeTraits, EdgeTraits>;
        // Extract inner types
        using NameType       = typename EdgeTraits::KeyType;
        using NameCompare    = typename EdgeTraits::KeyCompare;
        using StorageType    = typename EdgeTraits::StorageType;
    protected:
        EdgeBaseType* p_edge_;
    public:

        __EdgeDescriptorBase(EdgeBaseType* p_edge) : p_edge_(p_edge) {}

        // Get the name of this edge
        NameType name() const { return p_edge_->name_; }
        // Get reference of the storage of this edge
        const StorageType& storage() const { return p_edge_->storage_; }
              StorageType& storage()       { return p_edge_->storage_; }
        // Get copy of the storage of this edge
        StorageType storage_copy() const { return p_edge_->storage_; }
    };

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    class __EdgeDescriptor
        : public __EdgeDescriptorBase<NodeTraits, EdgeTraits>
    {
    public:
        using Base           = __EdgeDescriptorBase<NodeTraits, EdgeTraits>;
    public:
        __EdgeDescriptor(typename Base::EdgeBaseType* p_edge) : Base(this->p_edge_) {}
    };

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected = Undirected>
    class Graph {
    public:
        // Basic typedefs
        using NodeBaseType    = NodeBase<NodeTraits, EdgeTraits>;
        using EdgeBaseType    = EdgeBase<NodeTraits, EdgeTraits>;
        using NodeDerivedType = Node<NodeTraits, EdgeTraits, IsDirected>;
        using EdgeDerivedType = Edge<NodeTraits, EdgeTraits, IsDirected>;
    public:
        // Extract inner types for node
        using NodeNameType    = typename NodeTraits::KeyType;
        using NodeNameCompare = typename NodeTraits::KeyCompare;
        using NodeStorageType = typename NodeTraits::StorageType;
        // Extract inner types for edge
        using EdgeNameType    = typename EdgeTraits::KeyType;
        using EdgeNameCompare = typename EdgeTraits::KeyCompare;
        using EdgeStorageType = typename EdgeTraits::StorageType;
    public:
        using NodeDescriptor  = __NodeDescriptor<NodeTraits, EdgeTraits, Directed>;
        using EdgeDescriptor  = __EdgeDescriptor<NodeTraits, EdgeTraits, Directed>;
    public:
        // Inner container types
        using NodeMapType
            = std::map<
                NodeNameType,
                std::shared_ptr<NodeBaseType>,
                NodeNameCompare>;
        using EdgeMapType
            = std::map<
                EdgeNameType,
                std::shared_ptr<EdgeBaseType>,
                EdgeNameCompare>;
    private:
        NodeMapType node_map_;
        EdgeMapType edge_map_;
    public:

        template <template <typename ...> class Container,
            typename std::enable_if<
                type_traits::is_stl_seq<Container>::value, void>::type* = nullptr >
        auto nodes() const -> Container<NodeDescriptor>
        {
            Container<NodeDescriptor> seq;
            for (auto it = node_map_.begin(); it != node_map_.end(); it++) {
                seq.push_back(NodeDescriptor(it->second.get()));
            }
            return seq;
        }
        template <template <typename ...> class Container,
            typename std::enable_if<
                type_traits::is_stl_seq<Container>::value, void>::type* = nullptr >
        auto edges() const -> Container<EdgeDescriptor>
        {
            Container<EdgeDescriptor> seq;
            for (auto it = edge_map_.begin(); it != edge_map_.end(); it++) {
                seq.push_back(EdgeDescriptor(it->second.get()));
            }
            return seq;
        }

        NodeDescriptor insert_node(const std::pair<NodeNameType, NodeStorageType>& pair) {
            std::shared_ptr<NodeBaseType> p{ new NodeDerivedType{pair.first, pair.second} };            
            auto it = node_map_.insert(std::make_pair(pair.first, p));
            if (it.second == false)
                throw std::runtime_error(
                    "[Error] Node name is duplicated.");
            p->this_in_node_map_ = it.first;
            return NodeDescriptor(p.get());
        }

        template <typename std::enable_if<
            std::is_same<
                IsDirected, Undirected>::value, void>::type* = nullptr >
        EdgeDescriptor insert_edge(
            const std::pair<EdgeNameType, EdgeStorageType>& pair,
            const NodeDescriptor& node1,
            const NodeDescriptor& node2)
        {
            std::shared_ptr<EdgeBaseType> p{ new EdgeDerivedType{pair.first, pair.second} };
            auto it = edge_map_.insert(std::make_pair(pair.first, p));
            if (it.second == false)
                throw std::runtime_error(
                    "[Error] Edge name is duplicated.");
            p->this_in_edge_map_ = it.first;

            auto it1pair = node1.p_node_derived_->incidents_.insert(std::make_pair(pair.first, p));
            auto it2pair = node2.p_node_derived_->incidents_.insert(std::make_pair(pair.first, p));

            static_cast<EdgeDerivedType*>(p.get())->this1_in_incidents_ = it1pair.first;
            static_cast<EdgeDerivedType*>(p.get())->this2_in_incidents_ = it2pair.first;

            static_cast<EdgeDerivedType*>(p.get())->p_node1_ = node1.p_node_derived_->this_in_node_map_->second.get();
            static_cast<EdgeDerivedType*>(p.get())->p_node2_ = node2.p_node_derived_->this_in_node_map_->second.get();

            return EdgeDescriptor(it.first->second.get());
        }
    };

}   // namespace gtl

#endif   // GTL_GRAPH_H