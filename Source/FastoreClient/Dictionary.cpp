#include "Dictionary.h"

using namespace fastore::client;

const ColumnID Dictionary::_ColumnColumns[] = {ColumnID, ColumnName, ColumnValueType, ColumnRowIDType, ColumnBufferType};
const ColumnIDs  Dictionary::ColumnColumns(_ColumnColumns, _ColumnColumns + (sizeof(_ColumnColumns) / sizeof(_ColumnColumns[0])));

const ColumnID Dictionary::_TopologyColumns[] = {TopologyID};
const ColumnIDs  Dictionary::TopologyColumns(_TopologyColumns, _TopologyColumns + (sizeof(_TopologyColumns) / sizeof(_TopologyColumns[0])));

const ColumnID Dictionary::_HostColumns[] = {HostID};
const ColumnIDs  Dictionary::HostColumns(_HostColumns, _HostColumns + (sizeof(_HostColumns) / sizeof(_HostColumns[0])));

const ColumnID Dictionary::_TablePodColumns[] = {PodID, PodHostID};
const ColumnIDs  Dictionary::TablePodColumns(_TablePodColumns, _TablePodColumns + (sizeof(_TablePodColumns) / sizeof(_TablePodColumns[0])));

const ColumnID Dictionary::_PodColumnColumns[] = {PodColumnPodID, PodColumnColumnID};
const ColumnIDs  Dictionary::PodColumnColumns(_PodColumnColumns, _PodColumnColumns + (sizeof(_PodColumnColumns) / sizeof(_PodColumnColumns[0])));

const ColumnID Dictionary::_GeneratorColumns[] = {GeneratorNextValue};
const ColumnIDs  Dictionary::GeneratorColumns(_GeneratorColumns, _GeneratorColumns + (sizeof(_GeneratorColumns) / sizeof(_GeneratorColumns[0])));

