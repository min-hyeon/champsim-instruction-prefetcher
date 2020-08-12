
#ifndef GTL_GRAPH_H
#define GTL_GRAPH_H

#include <functional>
#include <memory>
#include <map>
#include <vector>
#include <list>
#include <stdexcept>
#include <algorithm>

#include "format.hh"

//#define GRAPH_DEBUG
#ifdef GRAPH_DEBUG
    #include <iostream>
    #define GRAPH_DEBUG_PRINT(expr) expr
#else
    #define GRAPH_DEBUG_PRINT(expr)
#endif

namespace GTL {

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
     * 1. Type qualifiers
     * 
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

    // [Type qualifiers] <\begin>
    // 

    // Wrapper class that acts as a type-holder of a node's name infos.
    // <\code>
    //     Name<name-type-of-node/edge, comparator-for-name-type>;
    // <\endcode>
    // Notice that "comparator-for-name-type" is std less functional
    // unit by default, so you must specify the comparator type if you
    // use some other "name-type-of-node/edge" that is not supported
    // by std less unit.
    template <typename Key, typename Compare = std::less<Key>>
    struct Name {
        typedef Key        KeyType;
        typedef Compare    KeyCompare;
    };

    // Wrapper class that acts as a type-holder of a node's inner
    // types; key(name), key comparator, and user-defined storage type.
    // <\code>
    //     NodeTraits<
    //         Name<name-type-of-node, comparator-for-name-type>,
    //         user-defined-storage-type>;
    // <\endcode>
    template <typename Name, typename Storage>
    struct NodeTraits {
        typedef typename Name::KeyType      KeyType;
        typedef typename Name::KeyCompare   KeyCompare;
        typedef Storage                     StorageType;
    };

    // Wrapper class that acts as a type-holder of a edge's inner
    // types; key(name), key comparator, and user-defined storage type.
    // <\code>
    //     EdgeTraits<
    //         Name<name-type-of-edge, comparator-for-name-type>,
    //         user-defined-storage-type>;
    // <\endcode>
    template <typename Name, typename Storage>
    struct EdgeTraits {
        typedef typename Name::KeyType      KeyType;
        typedef typename Name::KeyCompare   KeyCompare;
        typedef Storage                     StorageType;
    };

    //
    // [Type qualifiers] <\end>

    /**
     * 2. Class hierarchy of node classes. (Inheritance)
     * 
     * Node<..., Undirected> and Node<..., Directed> each inherits 
     * NodeBase<...> class, which holds "name" and "storage" instances
     * and useful typedefs for "connectors". The connectors are
     * implemented in the derived class, because they act differently
     * based on whether the graph is directed or not.
     * 
     * - LinkToMap: Because all the pointers to each node objects are
     *     maintained in the stl map structure with the "name" as key,
     *     we need to "iterator" to the position of its node in the map
     *     for constant-time elimination.
     *     Notice that stl map provides some API for removing an entry;
     *     "std::map::erase(position)" where position is of type iterator.
     * 
     * - IncidentEdges: A node object "v" holds a collection I(v),
     *     called the incidence collection of v, whose elements store
     *     pointers to the edges incident on "v".
     * 
     * Note that all pointer types in NodeBase<...> and its derived class
     * are of type shared_ptr<"NodeBase"> or shared_ptr<"EdgeBase">,
     * which means that it is needed to down-cast this pointer for
     * further use and access.
     * 
     * Reference)
     * For more detailed information on the whole graph structure, see:
     * 
     * Michael T. Goodrich, Roberto Tamassia, and David M. Mount. (2009).
     * "Data Structures and Algorithms in C++." Wiley Publishing.
     */

    // [Internal node types] <\begin>
    // 

    // Commonly used components
    template <typename NodeTraits, typename EdgeTraits>
    class NodeBase {
        template <typename, typename>
        friend class __NodeDescriptorBase;
        template <typename, typename, typename>
        friend class Graph;
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

    // Commonly used components + "Undirected" connectors
    template <typename NodeTraits, typename EdgeTraits>
    class Node<NodeTraits, EdgeTraits, Undirected>
        : public NodeBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename>
        friend class __NodeDescriptor;
        template <typename, typename, typename>
        friend class Graph;
    public:
        using Base = NodeBase<NodeTraits, EdgeTraits>;
    private:
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

    // Commonly used components + "Directed" connectors
    template <typename NodeTraits, typename EdgeTraits>
    class Node<NodeTraits, EdgeTraits, Directed>
        : public NodeBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename>
        friend class __NodeDescriptor;
        template <typename, typename, typename>
        friend class Graph;
    public:
        using Base = NodeBase<NodeTraits, EdgeTraits>;
    private:
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

    //
    // [Internal node types] <\end>

    /**
     * 3. Class hierarchy of edge classes. (Inheritance)
     * 
     * Edge<..., Undirected> and Edge<..., Directed> each inherits 
     * EdgeBase<...> class, which holds "name" and "storage" instances
     * and useful typedefs for "connectors". The connectors are
     * implemented in the derived class, because they act differently
     * based on whether the graph is directed or not.
     * 
     * - LinkToNode: All edges, either directed or not, have two end 
     *     points. Therefore, each edge instance maintains two pointers
     *     to its two end nodes.
     * 
     * - LinkToMap: Because all the pointers to each edge objects are
     *     maintained in the stl map structure with the "name" as key,
     *     we need to "iterator" to the position of its node in the map
     *     for constant-time elimination.
     *     Notice that stl map provides some API for removing an entry;
     *     "std::map::erase(position)" where position is of type iterator.
     * 
     * - IncidentEdgesPtr: Recall that a node object "v" holds a
     *     collection I(v), called the incidence collection of v, whose
     *     elements store pointers to the edges incident on "v".
     *     Similar to LinkToMap, if edge "e" has two end nodes "u1", "u2",
     *     then the instance of "e" maintains its position (iterator)
     *     in the I(u1) and I(u2), respectively, for constant-time
     *     elimination.
     * 
     * Note that all pointer types in EdgeBase<...> and its derived class
     * are of type shared_ptr<"EdgeBase"> or shared_ptr<"EdgeBase">,
     * which means that it is needed to down-cast this pointer for
     * further use and access.
     * 
     * Reference)
     * For more detailed information on the whole graph structure, see:
     * 
     * Michael T. Goodrich, Roberto Tamassia, and David M. Mount. (2009).
     * "Data Structures and Algorithms in C++." Wiley Publishing.
     */

    // [Internal edge types] <\begin>
    //

    // Commonly used components
    template <typename NodeTraits, typename EdgeTraits>
    class EdgeBase {
        template <typename, typename>
        friend class __EdgeDescriptorBase;
        template <typename, typename, typename>
        friend class Graph;
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

    // Commonly used components + "Undirected" connectors
    template <typename NodeTraits, typename EdgeTraits>
    class Edge<NodeTraits, EdgeTraits, Undirected>
        : public EdgeBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class __EdgeDescriptor;
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

    // Commonly used components + "Directed" connectors
    template <typename NodeTraits, typename EdgeTraits>
    class Edge<NodeTraits, EdgeTraits, Directed>
        : public EdgeBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class __EdgeDescriptor;
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

    //
    // [Internal node types] <\end>

//}   // namespace internal

    namespace type_traits {

        template <template <typename ...> class Container>
        struct is_stl_seq {
            static constexpr bool value = false;
        };
        
        template <> struct is_stl_seq<std::vector> { static constexpr bool value = true; };
        template <> struct is_stl_seq<std::list>   { static constexpr bool value = true; };

    }   // namespace type_traits

    /**
     * 4. Node descriptors.
     * 
     * Node descriptors provide access to element and information
     * regarding incident edges and adjacent nodes. That is, node
     * descriptors behave like stl iterators, except that they don't
     * provide any iteration support.
     * 
     * - operator*(): Return the copy of the storage associated
     *     with the node pointed by "p_node_".
     * 
     * - name(): Return the name of the corresponding node.
     * 
     * - storage(): Return the referecne of the storage.
     * - storage_copy(): Return the copy of the storage, behave like
     *     operator*(). 
     */

    // [Node descriptors] <\begin>
    //

    template <typename NodeTraits, typename EdgeTraits>
    class __NodeDescriptorBase {
    public:
        using NodeDescriptorBase
            = __NodeDescriptorBase<NodeTraits, EdgeTraits>;
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

        bool operator!=(const NodeDescriptorBase& v) const { return p_node_ != v.p_node_; }
        bool operator==(const NodeDescriptorBase& v) const { return p_node_ == v.p_node_; }

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

    // Undirected node descriptor
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

        std::size_t degree() const { return this->p_node_derived_->incidents_.size(); }

        /**
         * Return a list of undirected edge descriptors incident on
         * the node pointed by "p_node_derived_".
         */
        template <template <typename ...> class Container = std::vector,
            typename std::enable_if<
                type_traits::is_stl_seq<Container>::value, void>::type* = nullptr >
        auto incident_edges() const -> Container<EdgeDescriptor>
        {
            Container<EdgeDescriptor> seq;
            const auto& incidents = this->p_node_derived_->incidents_;
            for (auto it = incidents.begin(); it != incidents.end(); it++) {
                seq.push_back(EdgeDescriptor(it->second.get()));
            }
            return seq;
        }

        // Test whether the node "v" and and this node are adjacent.
        auto is_adjacent_to(const NodeDescriptor& v) const -> bool
        {
            if (v.degree() < this->degree()) {
                auto incidents = v.incident_edges<std::vector>();
                return std::find_if(
                    incidents.begin(), incidents.end(),
                    [this, v](const EdgeDescriptor& e) -> bool {
                        return *this == e.opposite(v);
                        }) != incidents.end();
            } else {
                auto incidents = this->incident_edges<std::vector>();
                return std::find_if(
                    incidents.begin(), incidents.end(),
                    [this, v](const EdgeDescriptor& e) -> bool {
                        return v == e.opposite(*this);
                        }) != incidents.end();
            }
        }
    };

    // Directed node descriptor
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
        using NodeDerivedType = Node<NodeTraits, EdgeTraits, Directed>;
    private:
        NodeDerivedType* p_node_derived_;
    public:

        __NodeDescriptor(typename Base::NodeBaseType* p_node)
            : Base(p_node), p_node_derived_(static_cast<NodeDerivedType*>(this->p_node_)) {}

        std::size_t indegree()  const { return this->p_node_derived_->incomings_.size(); }
        std::size_t outdegree() const { return this->p_node_derived_->outgoings_.size(); }

        /** 
         * Return a list of directed edge descriptors outgoing from
         * the node pointed by "p_node_derived_".
         */
        template <template <typename ...> class Container = std::vector,
            typename std::enable_if<
                type_traits::is_stl_seq<Container>::value, void>::type* = nullptr >
        auto outgoing_edges() const -> Container<EdgeDescriptor>
        {
            Container<EdgeDescriptor> seq;
            const auto& outgoings = this->p_node_derived_->outgoings_;
            for (auto it = outgoings.begin(); it != outgoings.end(); it++) {
                seq.push_back(EdgeDescriptor(it->second.get()));
            }
            return seq;
        }

        /** 
         * Return a list of directed edge descriptors incoming into
         * the node pointed by "p_node_derived_".
         */
        template <template <typename ...> class Container = std::vector,
            typename std::enable_if<
                type_traits::is_stl_seq<Container>::value, void>::type* = nullptr >
        auto incoming_edges() const -> Container<EdgeDescriptor>
        {
            Container<EdgeDescriptor> seq;
            const auto& incomings = this->p_node_derived_->incomings_;
            for (auto it = incomings.begin(); it != incomings.end(); it++) {
                seq.push_back(EdgeDescriptor(it->second.get()));
            }
            return seq;
        }

        // Test whether this node points to the node "v".
        auto point_to(const NodeDescriptor& v) const -> bool
        {
            if (this->outdegree() < v.indegree()) {
                auto outgoings = this->outgoing_edges<std::vector>();
                return std::find_if(
                    outgoings.begin(), outgoings.end(),
                    [this, v](const EdgeDescriptor& e) -> bool {
                        return v == e.dst();
                        }) != outgoings.end();
            } else {
                auto incomings = v.incoming_edges<std::vector>();
                return std::find_if(
                    incomings.begin(), incomings.end(),
                    [this, v](const EdgeDescriptor& e) -> bool {
                        return *this == e.src();
                        }) != incomings.end();
            }
        }

        // Test whether this node is pointed by the node "v".
        auto is_pointed_by(const NodeDescriptor& v) const -> bool
        {
            if (this->indegree() < v.outdegree()) {
                auto incomings = this->incoming_edges<std::vector>();
                return std::find_if(
                    incomings.begin(), incomings.end(),
                    [this, v](const EdgeDescriptor& e) -> bool {
                        return v == e.src();
                        }) != incomings.end();
            } else {
                auto outgoings = v.outgoing_edges<std::vector>();
                return std::find_if(
                    outgoings.begin(), outgoings.end(),
                    [this, v](const EdgeDescriptor& e) -> bool {
                        return *this == e.dst();
                        }) != outgoings.end();
            }
        }
    };

    //
    // [Node descriptors] <\end>

    /**
     * 5. Edge descriptors.
     * 
     * Edge descriptors provide access to element and information
     * regarding the edge's incidence relationships. That is, edge
     * descriptors behave like stl iterators, except that they don't
     * provide any iteration support.
     * 
     * - operator*(): Return the copy of the storage associated
     *     with the edge pointed by "p_edge_".
     * 
     * - name(): Return the name of the corresponding edge.
     * 
     * - storage(): Return the referecne of the storage.
     * - storage_copy(): Return the copy of the storage, behave like
     *     operator*(). 
     */

    // [Edge descriptors] <\begin>
    //

    template <typename NodeTraits, typename EdgeTraits>
    class __EdgeDescriptorBase {
    public:
        using EdgeDescriptorBase
            = __EdgeDescriptorBase<NodeTraits, EdgeTraits>;
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

        bool operator!=(const EdgeDescriptorBase& e) const { return p_edge_ != e.p_edge_; }
        bool operator==(const EdgeDescriptorBase& e) const { return p_edge_ == e.p_edge_; }

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
    {};

    // Undirected edge descriptor
    template <typename NodeTraits, typename EdgeTraits>
    class __EdgeDescriptor<NodeTraits, EdgeTraits, Undirected>
        : public __EdgeDescriptorBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class Graph;
    public:
        using Base           = __EdgeDescriptorBase<NodeTraits, EdgeTraits>;
    public:
        using NodeDescriptor = __NodeDescriptor<NodeTraits, EdgeTraits, Undirected>;
        using EdgeDescriptor = __EdgeDescriptor<NodeTraits, EdgeTraits, Undirected>;
    public:
        using EdgeDerivedType = Edge<NodeTraits, EdgeTraits, Undirected>;
    private:
        EdgeDerivedType* p_edge_derived_;
    public:

        __EdgeDescriptor(typename Base::EdgeBaseType* p_edge)
            : Base(p_edge), p_edge_derived_(static_cast<EdgeDerivedType*>(this->p_edge_)) {}

        /**
         * Return the end node of this edge distinct from the node
         * "v"; an error occurs if e is not incident on v.
         */
        auto opposite(const NodeDescriptor& v) const -> NodeDescriptor
        {
            if (NodeDescriptor(p_edge_derived_->p_node1_) == v)
                return NodeDescriptor(p_edge_derived_->p_node2_);
            else if (NodeDescriptor(p_edge_derived_->p_node2_) == v)
                return NodeDescriptor(p_edge_derived_->p_node1_);
            else
                throw std::runtime_error(
                    format::sprintf(
                        "[Error] Eedge {} is not incident on the node {}", "{}",
                        this->name(), v.name()));
        }

        // Test whether this edge is incident on "v".
        auto is_incident_on(const NodeDescriptor& v) const -> bool
        {
            return (
                v == NodeDescriptor(p_edge_derived_->p_node1_) ||
                v == NodeDescriptor(p_edge_derived_->p_node2_)) ? true : false;
        }

        auto end_nodes() const -> std::pair<NodeDescriptor, NodeDescriptor>
        {
            return std::make_pair(
                NodeDescriptor(p_edge_derived_->p_node1_),
                NodeDescriptor(p_edge_derived_->p_node2_));
        } 
    };

    // Directed edge descriptor
    template <typename NodeTraits, typename EdgeTraits>
    class __EdgeDescriptor<NodeTraits, EdgeTraits, Directed>
        : public __EdgeDescriptorBase<NodeTraits, EdgeTraits>
    {
        template <typename, typename, typename> friend class Graph;
    public:
        using Base           = __EdgeDescriptorBase<NodeTraits, EdgeTraits>;
    public:
        using NodeDescriptor = __NodeDescriptor<NodeTraits, EdgeTraits, Directed>;
        using EdgeDescriptor = __EdgeDescriptor<NodeTraits, EdgeTraits, Directed>;
    public:
        using EdgeDerivedType = Edge<NodeTraits, EdgeTraits, Directed>;
    private:
        EdgeDerivedType* p_edge_derived_;
    public:

        __EdgeDescriptor(typename Base::EdgeBaseType* p_edge)
            : Base(p_edge), p_edge_derived_(static_cast<EdgeDerivedType*>(this->p_edge_)) {}

        /**
         * Return the end node of this edge distinct from the node
         * "v"; an error occurs if e is not incident on v.
         */
        auto opposite(const NodeDescriptor& v) const -> NodeDescriptor
        {
            if (NodeDescriptor(p_edge_derived_->p_node_from_) == v)
                return NodeDescriptor(p_edge_derived_->p_node_to_);
            else if (NodeDescriptor(p_edge_derived_->p_node_to_) == v)
                return NodeDescriptor(p_edge_derived_->p_node_from_);
            else
                throw std::runtime_error(
                    format::sprintf(
                        "[Error] Eedge {} is not incident on the node {}", "{}",
                        this->name(), v.name()));
        }

        auto src() const -> NodeDescriptor { return NodeDescriptor(p_edge_derived_->p_node_from_); }
        auto dst() const -> NodeDescriptor { return NodeDescriptor(p_edge_derived_->p_node_to_); }

    };

    //
    // [Edge descriptors] <\end>

    /**
     * 6. Graphs with adjacency list structure
     * 
     * All the nodes and edges in the graph are stored in the stl
     * map; "node_map_" and "edge_map_", respectively.
     * 
     * Undirected)
     * 
     * Each node "u" holds a collection of pointers to its incident
     * edges; I(u), and each edge stores two pointers to two end nodes.
     * 
     * When node "u" and "v" are connected by an edge "e", a pointer
     * to the instance of "e" is inserted into I(u) and I(v), and
     * the two inserted positions (two iterators) are recorded in the
     * edge "e".
     * 
     * Directed)
     * 
     * Each node "u" holds a collection of pointers to its outgoing
     * edges and incoming edges; Out(u), In(u), and each edge stores
     * two pointers to two end nodes.
     * 
     * When node "u" and "v" are connected by an edge "e" (u->v), a
     * pointer to the instance of "e" is inserted into Out(u) and In(v),
     * and the two inserted positions (two iterators) are recorded in the
     * edge "e".
     */

    // [Graphs] <\begin>
    //

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected = Undirected>
    class Graph {
    private:
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
        using NodeDescriptor  = __NodeDescriptor<NodeTraits, EdgeTraits, IsDirected>;
        using EdgeDescriptor  = __EdgeDescriptor<NodeTraits, EdgeTraits, IsDirected>;
    private:
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

        // Return the sequential container of all nodes in the graph
        template <template <typename ...> class Container = std::vector,
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

        // Return the sequential container of all edges in the graph
        template <template <typename ...> class Container = std::vector,
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

        /**
         * Insert a node with the given name-storage pair, and return
         * a new node descriptor associated with the newly inserted node.
         */
        auto insert_node(const std::pair<NodeNameType, NodeStorageType>& pair) -> NodeDescriptor
        {
            std::shared_ptr<NodeBaseType> p{ new NodeDerivedType{pair.first, pair.second} };            
            auto it = node_map_.insert(std::make_pair(pair.first, p));

            if (it.second == false)
                throw std::runtime_error(
                    format::sprintf(
                        "[Error] Node name is duplicated. {} == {}.", "{}",
                        it.first->first, pair.first));
            p->this_in_node_map_ = it.first;

            return NodeDescriptor(p.get());
        }

        /**
         * Insert an edge with the given name-storage pair and the two
         * node descriptors, and return a new edge descriptor associated
         * with the newly inserted edge.
         * 
         * In the graph is undirected, the order of "node1" and "node2"
         * is meaningless, but if the graph is directed, the ordermeans
         * the direction; "node1 --> node2".
         */
        auto insert_edge(
            const std::pair<EdgeNameType, EdgeStorageType>& pair,
            const NodeDescriptor& node1,
            const NodeDescriptor& node2) -> EdgeDescriptor
        {
            std::shared_ptr<EdgeBaseType> p{ new EdgeDerivedType{pair.first, pair.second} };
            auto it = edge_map_.insert(std::make_pair(pair.first, p));

            if (it.second == false)
                throw std::runtime_error(
                    format::sprintf(
                        "[Error] Edge name is duplicated. {} == {}.", "{}",
                        it.first->first, pair.first));
            p->this_in_edge_map_ = it.first;
            
            this->_M_impl_hook_nodes(pair, p, node1, node2, IsDirected{});
            
            return EdgeDescriptor(it.first->second.get());
        }

        // Remove node instance associated with the edge descriptor "v"
        // and all its connected edge instances.
        void remove_node(const NodeDescriptor& v) {
            this->_M_impl_disconnect_node(v, IsDirected{});
            this->node_map_.erase(v.p_node_derived_->this_in_node_map_);
        }

        // Remove edge instance associated with the edge descriptor "e".
        void remove_edge(const EdgeDescriptor& e)
        {
            auto p = e.p_edge_derived_;
            this->_M_impl_unhook_nodes(p, IsDirected{});
            this->edge_map_.erase(p->this_in_edge_map_);
        }

    private:

        /**
         * Some inernal helper functions.
         * 
         * These auxiliaries are required because the scheme for inter-
         * connecting or dis-connecting nodes and edges are quite different
         * based on the type of graph; directed or not.
         */

        // Helper for insert_edge
        void _M_impl_hook_nodes(
            const std::pair<EdgeNameType, EdgeStorageType>& pair,
            std::shared_ptr<EdgeBaseType>& p,
            const NodeDescriptor& node1,
            const NodeDescriptor& node2,
            Undirected);
        void _M_impl_hook_nodes(
            const std::pair<EdgeNameType, EdgeStorageType>& pair,
            std::shared_ptr<EdgeBaseType>& p,
            const NodeDescriptor& node1,
            const NodeDescriptor& node2,
            Directed);

        // Helper for remove_edge
        void _M_impl_unhook_nodes(EdgeDerivedType* p, Undirected);
        void _M_impl_unhook_nodes(EdgeDerivedType* p, Directed);

        // Helper for remove_node
        void _M_impl_disconnect_node(const NodeDescriptor& v, Undirected);
        void _M_impl_disconnect_node(const NodeDescriptor& v, Directed);

    };

    //
    // [Graphs] <\end>

    /**
     * Auxiliaries
     */

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    void Graph<NodeTraits, EdgeTraits, IsDirected>::_M_impl_hook_nodes(
            const std::pair<EdgeNameType, EdgeStorageType>& pair,
            std::shared_ptr<EdgeBaseType>& p,
            const NodeDescriptor& node1,
            const NodeDescriptor& node2,
            Undirected)
    {
        auto it1 = node1.p_node_derived_->incidents_.insert(std::make_pair(pair.first, p));
        auto it2 = node2.p_node_derived_->incidents_.insert(std::make_pair(pair.first, p));

        static_cast<EdgeDerivedType*>(p.get())->this1_in_incidents_ = it1.first;
        static_cast<EdgeDerivedType*>(p.get())->this2_in_incidents_ = it2.first;

        static_cast<EdgeDerivedType*>(p.get())->p_node1_ = node1.p_node_derived_->this_in_node_map_->second.get();
        static_cast<EdgeDerivedType*>(p.get())->p_node2_ = node2.p_node_derived_->this_in_node_map_->second.get();
    }

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    void Graph<NodeTraits, EdgeTraits, IsDirected>::_M_impl_hook_nodes(
            const std::pair<EdgeNameType, EdgeStorageType>& pair,
            std::shared_ptr<EdgeBaseType>& p,
            const NodeDescriptor& node1,
            const NodeDescriptor& node2,
            Directed)
    {
        auto it1 = node1.p_node_derived_->outgoings_.insert(std::make_pair(pair.first, p));
        auto it2 = node2.p_node_derived_->incomings_.insert(std::make_pair(pair.first, p));

        static_cast<EdgeDerivedType*>(p.get())->this_in_outgoings_ = it1.first;
        static_cast<EdgeDerivedType*>(p.get())->this_in_incomings_ = it2.first;

        static_cast<EdgeDerivedType*>(p.get())->p_node_from_ = node1.p_node_derived_->this_in_node_map_->second.get();
        static_cast<EdgeDerivedType*>(p.get())->p_node_to_   = node2.p_node_derived_->this_in_node_map_->second.get();
    }

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    void Graph<NodeTraits, EdgeTraits, IsDirected>::_M_impl_unhook_nodes(EdgeDerivedType* p, Undirected)
    {
        static_cast<NodeDerivedType*>(p->p_node1_)->incidents_.erase(p->this1_in_incidents_);
        if (p->p_node1_ == p->p_node2_)
            return;
        static_cast<NodeDerivedType*>(p->p_node2_)->incidents_.erase(p->this2_in_incidents_);
    }

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    void Graph<NodeTraits, EdgeTraits, IsDirected>::_M_impl_unhook_nodes(EdgeDerivedType* p, Directed)
    {
        static_cast<NodeDerivedType*>(p->p_node_from_)->outgoings_.erase(p->this_in_outgoings_);
        static_cast<NodeDerivedType*>(p->p_node_to_)->incomings_.erase(p->this_in_incomings_);
    }

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    void Graph<NodeTraits, EdgeTraits, IsDirected>::_M_impl_disconnect_node(const NodeDescriptor& v, Undirected)
    {
        auto incidents = v.incident_edges();
        std::for_each(
            incidents.begin(), incidents.end(),
            [this](const EdgeDescriptor& e) -> void {
                this->remove_edge(e);
            }
        );
    }

    template <typename NodeTraits, typename EdgeTraits, typename IsDirected>
    void Graph<NodeTraits, EdgeTraits, IsDirected>::_M_impl_disconnect_node(const NodeDescriptor& v, Directed)
    {
        auto incomings = v.incoming_edges();
        std::for_each(
            incomings.begin(), incomings.end(),
            [this](const EdgeDescriptor& e) -> void {
                this->remove_edge(e);
            }
        );
        auto outgoings = v.outgoing_edges();
        std::for_each(
            outgoings.begin(), outgoings.end(),
            [this](const EdgeDescriptor& e) -> void {
                this->remove_edge(e);
            }
        );
    }

}   // namespace GTL

#endif   // GTL_GRAPH_H