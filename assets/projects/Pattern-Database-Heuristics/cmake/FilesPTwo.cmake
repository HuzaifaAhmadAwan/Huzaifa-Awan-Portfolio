create_library(
    NAME extra_tasks
    HELP "Non-core task transformations"
    SOURCES
        downward/tasks/syntactic_projection
    DEPENDS
        task_properties
    TARGET downward
)

create_library(
    NAME pdbs
    HELP "Plugin containing the code for PDBs"
    SOURCES
        downward/pdbs/hstar
        downward/pdbs/state_enumerator
        downward/pdbs/subcategory
        downward/pdbs/types
    DEPENDS
        causal_graph
        extra_tasks
        max_cliques
        priority_queues
        successor_generator
        task_properties
        variable_order_finder
    TARGET
        downward
)

create_library(
    NAME pdb_heuristic
    HELP "The PDB heuristic for a single pattern"
    SOURCES
        downward/heuristics/pdb_heuristic
    DEPENDS
        pdbs
    TARGET downward
)

create_library(
    NAME cliques_heuristic
    HELP "The cliques heuristic"
    SOURCES
        downward/heuristics/cliques_heuristic
    DEPENDS
        pdbs
    TARGET downward
)
