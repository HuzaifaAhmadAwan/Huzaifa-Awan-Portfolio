#include "downward/open_list_factory.h"

#include "downward/plugins/options.h"
#include "downward/plugins/plugin.h"

using namespace std;

template <>
unique_ptr<StateOpenList> OpenListFactory::create_open_list()
{
    return create_state_open_list();
}

template <>
unique_ptr<EdgeOpenList> OpenListFactory::create_open_list()
{
    return create_edge_open_list();
}

void add_open_list_options_to_feature(plugins::Feature&)
{
}

tuple<> get_open_list_arguments_from_options(const plugins::Options&)
{
    return make_tuple();
}

static class OpenListFactoryCategoryPlugin
    : public plugins::TypedCategoryPlugin<OpenListFactory> {
public:
    OpenListFactoryCategoryPlugin()
        : TypedCategoryPlugin("OpenList")
    {
        // TODO: use document_synopsis() for the wiki page.
    }
} _category_plugin;