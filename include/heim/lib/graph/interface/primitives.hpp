#ifndef HEIM_LIB_GRAPH_INTERFACE_PRIMITIVES_HPP
#define HEIM_LIB_GRAPH_INTERFACE_PRIMITIVES_HPP

namespace heim::graphs
{
struct undirected_graph_tag                         { };
struct directed_graph_tag                           { };
struct bidirectional_graph_tag : directed_graph_tag { };

} // namespace heim::graphs

#endif // HEIM_LIB_GRAPH_INTERFACE_PRIMITIVES_HPP
