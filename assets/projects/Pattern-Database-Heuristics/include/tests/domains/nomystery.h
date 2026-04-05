#ifndef TESTS_DOMAINS_NOMYSTERY_H
#define TESTS_DOMAINS_NOMYSTERY_H

#include "tests/domains/classical_planning_domain.h"

#include "downward/task_proxy.h"

#include <compare>
#include <set>
#include <utility>

namespace tests {

struct RoadMapEdge {
    int origin;
    int destination;
    int drive_cost = 1;

    friend auto operator<=>(const RoadMapEdge&, const RoadMapEdge&) = default;
};

/**
 * @brief Models the logistics planning domain NoMystery.
 *
 * A NoMystery problem is a logistics task in which a truck must gather and
 * deliver packages to a specific locations, under the constraint that the load
 * of the truck may not exceed a specific capacity C (max. number of packages).
 * There are three kinds of actions:
 *
 * 1. The trucker may drive from one location l to another location l', if l is
 * connected to l'. In general, there may be one-way roads, i.e., l is connected
 * to l' but not vice versa.
 * 2. The trucker may load a package if both are at the same location and the
 * loading capacity of the truck has not been reached yet. This increases the
 * load by 1.
 * 3. The trucker may unload a package at his location. This decreases the
 * load by 1.
 *
 * This environment is modelled as an FDR task as follows:
 *
 * Variables:
 * + Truck-At - position of the truck, one of the locations.
 * + Load - number of packages currently loaded from 0 to C (inclusive).
 * + Package-At(p) - position of a package p, either a location or 'truck'.
 *
 * Operators:
 * + Drive(l, l') - The truck drives from location l to location l'. The
 *   operator only exists for some l and l' if l is connected to l'.
 *   - Precondition: Truck-At=l
 *   - Effect: Truck=l'
 * + Load-Package(l, p, c) - Loads the package p at location l when the truck
 * has a load of c < C.
 *   - Precondition: Truck-At=l, Package-At=l, Load=c
 *   - Effect: Package-At=truck, Load=c+1
 * + Unload-Package(l, p, c) - Unloads the package p at location l when the
 *   truck has a load of c > 0.
 *   - Precondition: Truck-At=l, Package-At(p)=truck, Load=c
 *   - Effect: Package-At=l, Load=c-1
 *
 * All operators have unit costs by default, but can be configured to have
 * different costs as well. See constructor arguments for details.
 *
 * @ingroup test_domains
 */
class NoMystery : public ClassicalPlanningDomain {
    int locations;
    int packages;
    int truck_capacity;
    std::set<RoadMapEdge> roadmap_graph;

public:
    // Construct a NoMystery problem instance with the specified number of
    // locations, packages and truck capacity. The connectedness of the
    // locations is given by the argument roadmap_graph, a set of edges such
    // that l is connected to l' iff <l, l'> is an edge in this set. Edges also
    // carry a cost, which gives the cost of the operator Drive(l, l') (1 by
    // default). The costs for loading and unloading a package can also be
    // specified by the constructor arguments load_cost and unload_cost (1 by
    // default).
    NoMystery(
        int locations,
        int packages,
        int truck_capacity,
        std::set<RoadMapEdge> roadmap_graph,
        int load_cost = 1,
        int unload_cost = 1);

    // Get the integer representation of a variable.
    int get_variable_truck_at() const;
    int get_variable_packages_loaded() const;
    int get_variable_package_at(int package) const;

    // Get the FactPair representation of a fact.
    FactPair get_fact_truck_at_location(int location) const;
    FactPair get_fact_packages_loaded(int num_loaded) const;
    FactPair get_fact_package_at_location(int package, int location) const;
    FactPair get_fact_package_in_truck(int package) const;

    // Get the OperatorID representation of an operator.
    OperatorID get_operator_load_package_id(
        int location,
        int package,
        int num_already_loaded) const;
    OperatorID get_operator_unload_package_id(
        int location,
        int package,
        int num_already_loaded) const;
    OperatorID get_operator_drive_id(int origin, int destination) const;

private:
    int get_operator_load_package_index(
        int location,
        int package,
        int num_already_loaded) const;
    int get_operator_unload_package_index(
        int location,
        int package,
        int num_already_loaded) const;
    int get_operator_drive_index(int origin, int destination) const;
};

} // namespace tests

#endif