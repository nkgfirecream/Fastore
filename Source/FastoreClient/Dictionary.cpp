#include "Dictionary.h"

using namespace fastore::client;

const int Dictionary::ColumnColumns[] = {ColumnID, ColumnName, ColumnValueType, ColumnRowIDType, ColumnBufferType};
const int Dictionary::TopologyColumns[] = {TopologyID};
const int Dictionary::HostColumns[] = {HostID};
const int Dictionary::TablePodColumns[] = {PodID, PodHostID};
const int Dictionary::PodColumnColumns[] = {PodColumnPodID, PodColumnColumnID};
const int Dictionary::GeneratorColumns[] = {GeneratorNextValue};