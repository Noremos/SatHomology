#ifdef USE_MODULE
module;
#endif

// #include "emhash/hash_table5.hpp"
#include <unordered_map>

#ifdef USE_MODULE
export module MHashMap;
#define MEXPORT export
#else
#define MEXPORT
#endif

MEXPORT template <typename KeyT, typename ValueT, typename HashT = std::hash<KeyT>, typename EqT = std::equal_to<KeyT>>
using MMMAP = std::unordered_map<KeyT, ValueT, HashT, EqT>;
