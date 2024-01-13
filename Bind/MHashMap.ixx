module;

// #include "emhash/hash_table5.hpp"
#include <unordered_map>

export module MHashMap;

export template <typename KeyT, typename ValueT, typename HashT = std::hash<KeyT>, typename EqT = std::equal_to<KeyT>>
using MMMAP = std::unordered_map<KeyT, ValueT, HashT, EqT>;
