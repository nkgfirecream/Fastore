#pragma once

struct TreeEntry
{
    void* key;
    void* value;

    TreeEntry();
    TreeEntry(void* k) : key(k) {};
    TreeEntry(void* k, void* v) : key(k), value(v) {};

    TreeEntry(const TreeEntry& entry) : key(entry.key), value(entry.value) {};
};