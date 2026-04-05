create_library(
    NAME mdp
    HELP "Core source files for supporting MDPs"
    SOURCES
        # Tasks
        probfd/probabilistic_task
        probfd/task_proxy

        probfd/task_utils/task_properties

        # Basic types
        probfd/value_type
    DEPENDS
        core_sources
        core_tasks
        plugins
        utils
        parser
)