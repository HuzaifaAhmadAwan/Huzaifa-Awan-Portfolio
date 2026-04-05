#include "downward/heuristics/best_supporter_function.h"

#include "downward/plugins/plugin.h"

using namespace std;

static class BestSupporterFunctionCategoryPlugin
    : public plugins::TypedCategoryPlugin<BestSupporterFunction> {
public:
    BestSupporterFunctionCategoryPlugin()
        : TypedCategoryPlugin("BestSupporterFunction")
    {
    }
} _category_plugin;