#include "probfd/probabilistic_task.h"

#include "downward/plugins/plugin.h"

namespace probfd {

static class ProbabilisticTaskCategoryPlugin
    : public plugins::TypedCategoryPlugin<ProbabilisticPlanningTask> {
public:
    ProbabilisticTaskCategoryPlugin()
        : TypedCategoryPlugin("ProbabilisticPlanningTask")
    {
        document_synopsis("Represents a probabilistic planning task");
    }
} _category_plugin_collection;

} // namespace probfd