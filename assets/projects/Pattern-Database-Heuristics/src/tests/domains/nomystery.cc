#include "tests/domains/nomystery.h"

#include <algorithm>
#include <exception>
#include <ranges>

namespace tests {

static std::pair<int, int> as_pair(const RoadMapEdge& e)
{
    return std::make_pair(e.origin, e.destination);
};

NoMystery::NoMystery(
    int locations,
    int packages,
    int truck_capacity,
    std::set<RoadMapEdge> roadmap_graph,
    int load_cost,
    int unload_cost)
    : ClassicalPlanningDomain(
          2 + packages,
          2 * truck_capacity * locations * packages + roadmap_graph.size())
    , locations(locations)
    , packages(packages)
    , truck_capacity(truck_capacity)
    , roadmap_graph(std::move(roadmap_graph))
{
    if (truck_capacity < 0) {
        throw std::range_error("Truck capacity must be at least one!");
    }

    // Verify roadmap
    for (const auto& [origin, destination, drive_cost] : this->roadmap_graph) {
        if (origin < 0 || origin >= locations || destination < 0 ||
            origin > locations) {
            throw std::range_error("Encountered a location ID out of range "
                                   "while reading the roadmap graph!");
        }

        if (drive_cost < 0) {
            throw std::range_error("Encountered a negative cost "
                                   "while reading the roadmap graph!");
        }
    }

    if (std::ranges::adjacent_find(
            roadmap_graph,
            std::ranges::equal_to(),
            as_pair) != roadmap_graph.end()) {
        throw std::range_error("Encountered a duplicated edge "
                               "while reading the roadmap graph!");
    }

    // Variables
    {
        auto& var_info = variable_infos[get_variable_truck_at()];
        var_info.name = "truck-at";
        var_info.domain_size = locations;

        for (int l = 0; l != locations; ++l) {
            var_info.fact_names.emplace_back("truck-at L" + std::to_string(l));
        }
    }

    {
        auto& var_info = variable_infos[get_variable_packages_loaded()];
        var_info.name = "packages-loaded";
        var_info.domain_size = truck_capacity + 1;

        for (int c = 0; c <= truck_capacity; ++c) {
            var_info.fact_names.emplace_back(
                "packages-loaded " + std::to_string(c));
        }
    }

    for (int p = 0; p != packages; ++p) {
        auto& var_info = variable_infos[get_variable_package_at(p)];

        var_info.name = "package-P" + std::to_string(p) + "-at";
        var_info.domain_size = locations + 1;

        for (int l = 0; l != locations; ++l) {
            var_info.fact_names.emplace_back(
                "package-P" + std::to_string(p) + "-at L" + std::to_string(l));
        }

        var_info.fact_names.emplace_back(
            "package-P" + std::to_string(p) + "-at truck");
    }

    // Operators
    for (int n = 0; n != truck_capacity; ++n) {
        for (int l = 0; l != locations; ++l) {
            for (int p = 0; p != packages; ++p) {
                {
                    auto& op_info_load =
                        operators[get_operator_load_package_index(l, p, n)];

                    op_info_load.name = "load-package-C" + std::to_string(n) +
                                        "-L" + std::to_string(l) + "-P" +
                                        std::to_string(p);

                    op_info_load.cost = load_cost;

                    op_info_load.precondition.emplace_back(
                        get_fact_truck_at_location(l));
                    op_info_load.precondition.emplace_back(
                        get_fact_packages_loaded(n));
                    op_info_load.precondition.emplace_back(
                        get_fact_package_at_location(p, l));

                    op_info_load.effect.emplace_back(
                        get_fact_packages_loaded(n + 1));
                    op_info_load.effect.emplace_back(
                        get_fact_package_in_truck(p));
                }
                {
                    auto& op_info_unload = operators
                        [get_operator_unload_package_index(l, p, n + 1)];

                    op_info_unload.name =
                        "unload-package-C" + std::to_string(n + 1) + "-L" +
                        std::to_string(l) + "-P" + std::to_string(p);

                    op_info_unload.cost = unload_cost;

                    op_info_unload.precondition.emplace_back(
                        get_fact_truck_at_location(l));
                    op_info_unload.precondition.emplace_back(
                        get_fact_packages_loaded(n + 1));
                    op_info_unload.precondition.emplace_back(
                        get_fact_package_in_truck(p));

                    op_info_unload.effect.emplace_back(
                        get_fact_packages_loaded(n));
                    op_info_unload.effect.emplace_back(
                        get_fact_package_at_location(p, l));
                }
            }
        }
    }

    for (const auto& [origin, destination, drive_cost] : this->roadmap_graph) {
        auto& op_info_move =
            operators[get_operator_drive_index(origin, destination)];

        op_info_move.name = "drive-L" + std::to_string(origin) + "-L" +
                            std::to_string(destination);

        op_info_move.cost = drive_cost;

        op_info_move.precondition.emplace_back(
            get_fact_truck_at_location(origin));
        op_info_move.effect.emplace_back(
            get_fact_truck_at_location(destination));
    }
}

int NoMystery::get_variable_truck_at() const
{
    return 0;
}

int NoMystery::get_variable_packages_loaded() const
{
    return 1;
}

int NoMystery::get_variable_package_at(int package) const
{
    return 2 + package;
}

FactPair NoMystery::get_fact_truck_at_location(int location) const
{
    return {get_variable_truck_at(), location};
}

FactPair NoMystery::get_fact_packages_loaded(int num_loaded) const
{
    return {get_variable_packages_loaded(), num_loaded};
}

FactPair
NoMystery::get_fact_package_at_location(int package, int location) const
{
    return {get_variable_package_at(package), location};
}

FactPair NoMystery::get_fact_package_in_truck(int package) const
{
    return {get_variable_package_at(package), locations};
}

OperatorID NoMystery::get_operator_load_package_id(
    int location,
    int package,
    int num_already_loaded) const
{
    return OperatorID(
        get_operator_load_package_index(location, package, num_already_loaded));
}

OperatorID NoMystery::get_operator_unload_package_id(
    int location,
    int package,
    int num_already_loaded) const
{
    return OperatorID(get_operator_unload_package_index(
        location,
        package,
        num_already_loaded));
}

OperatorID NoMystery::get_operator_drive_id(int origin, int destination) const
{
    return OperatorID(get_operator_drive_index(origin, destination));
}

int NoMystery::get_operator_load_package_index(
    int location,
    int package,
    int num_already_loaded) const
{
    return num_already_loaded + truck_capacity * package +
           truck_capacity * packages * location;
}

int NoMystery::get_operator_unload_package_index(
    int location,
    int package,
    int num_already_loaded) const
{
    return truck_capacity * locations * packages + (num_already_loaded - 1) +
           truck_capacity * package + truck_capacity * packages * location;
}

int NoMystery::get_operator_drive_index(int origin, int destination)
    const
{
    auto it = std::ranges::find(
        roadmap_graph,
        std::make_pair(origin, destination),
        as_pair);

    if (it == roadmap_graph.end()) {
        throw std::invalid_argument(
            "Cannot drive between these two locations!");
    }

    return 2 * truck_capacity * locations * packages +
           std::distance(roadmap_graph.begin(), it);
}

} // namespace tests
