#ifndef STATE_ID_H
#define STATE_ID_H

#include <compare>
#include <iostream>

// For documentation on classes relevant to storing and working with registered
// states see the file state_registry.h.

class StateID {
    friend class StateRegistry;
    friend std::ostream& operator<<(std::ostream& os, StateID id);
    template <typename>
    friend class PerStateInformation;
    template <typename>
    friend class PerStateArray;
    friend class PerStateBitset;

    int value;

    // No implementation to prevent default construction
    StateID();

public:
    static const StateID no_state;

    // HACK
    explicit StateID(int value_)
        : value(value_)
    {
    }

    friend auto operator<=>(StateID left, StateID right) = default;

    // HACK
    int get_value() const { return value; }
};

#endif
