#pragma once
#include "typedefs.h"

struct TreeEntry
{
    fs::Key key;
    fs::Value value;

    TreeEntry();
    TreeEntry(const fs::Key& k) : key(k) {};
    TreeEntry(const fs::Key& k, const fs::Value& v) : key(k), value(v) {};

    TreeEntry(const TreeEntry& entry) : key(entry.key), value(entry.value) {};
};