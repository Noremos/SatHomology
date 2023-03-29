module;

#include "emhash/hash_table5.hpp"

export module MHashMap;

export template <typename KeyT, typename ValueT, typename HashT = std::hash<KeyT>, typename EqT = std::equal_to<KeyT>>
using MMMAP = emhash5::HashMap<KeyT, ValueT, HashT, EqT>;
