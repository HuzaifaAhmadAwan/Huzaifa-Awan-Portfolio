#include "downward/fact_pair.h"

#include <iostream>

using namespace std;

FactPair::FactPair(int var, int value)
    : var(var)
    , value(value)
{
}

const FactPair FactPair::no_fact = FactPair(-1, -1);

ostream& operator<<(ostream& os, const FactPair& fact_pair)
{
    os << fact_pair.var << "=" << fact_pair.value;
    return os;
}

namespace utils {
void feed(HashState& hash_state, const FactPair& fact)
{
    feed(hash_state, fact.var);
    feed(hash_state, fact.value);
}
} // namespace utils
