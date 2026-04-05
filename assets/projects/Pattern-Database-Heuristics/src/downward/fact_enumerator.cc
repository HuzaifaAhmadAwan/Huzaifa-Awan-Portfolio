#include "downward/fact_enumerator.h"

#include "downward/task_proxy.h"

#include "downward/fact_pair.h"

using namespace std;

FactEnumerator::FactEnumerator(const VariablesProxy& variables)
    : indices(variables.size())
    , num_facts(0)
{
    for (size_t i = 0; i != variables.size(); ++i) {
        indices[i] = num_facts;
        num_facts += variables[i].get_domain_size();
    }
}

int FactEnumerator::get_fact_index(int var, int value) const
{
    return indices[var] + value;
}

int FactEnumerator::get_fact_index(const FactPair& fact_pair) const
{
    return indices[fact_pair.var] + fact_pair.value;
}

int FactEnumerator::get_fact_index(const FactProxy& fact) const
{
    return indices[fact.get_variable().get_id()] + fact.get_value();
}

int FactEnumerator::get_num_facts() const
{
    return num_facts;
}
