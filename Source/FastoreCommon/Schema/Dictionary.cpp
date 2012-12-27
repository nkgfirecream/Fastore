#include "Dictionary.h"

#define COUNTOF(a) (sizeof(a)/sizeof(*a))

using namespace fastore::common;
using fastore::communication::ColumnID;

const ColumnID Dictionary::MaxSystemColumnID(9999);
const ColumnID Dictionary::MaxClientColumnID(19999);
const ColumnID Dictionary::ColumnID(0);
const ColumnID Dictionary::ColumnName(1);
const ColumnID Dictionary::ColumnValueType(2);
const ColumnID Dictionary::ColumnRowIDType(3);
const ColumnID Dictionary::ColumnBufferType(4);	
const ColumnID Dictionary::ColumnRequired(5);
const ColumnID Dictionary::TopologyID(100);
const ColumnID Dictionary::HostID(200);
const ColumnID Dictionary::PodID(300);
const ColumnID Dictionary::PodHostID(301);
const ColumnID Dictionary::PodColumnPodID(400);
const ColumnID Dictionary::PodColumnColumnID(401);
const ColumnID Dictionary::StashID(500);
const ColumnID Dictionary::StashHostID(501);
const ColumnID Dictionary::StashColumnStashID(600);
const ColumnID Dictionary::StashColumnColumnID(601);


const ColumnID Dictionary::_ColumnColumns[] = {ColumnID, ColumnName, ColumnValueType, ColumnRowIDType, ColumnBufferType, ColumnRequired };
const ColumnIDs  Dictionary::ColumnColumns(_ColumnColumns, _ColumnColumns + COUNTOF(_ColumnColumns));

const ColumnID Dictionary::_TopologyColumns[] = {TopologyID};
const ColumnIDs  Dictionary::TopologyColumns(_TopologyColumns, _TopologyColumns + COUNTOF(_TopologyColumns));

const ColumnID Dictionary::_HostColumns[] = {HostID};
const ColumnIDs  Dictionary::HostColumns(_HostColumns, _HostColumns + COUNTOF(_HostColumns));

const ColumnID Dictionary::_TablePodColumns[] = {PodID, PodHostID};
const ColumnIDs  Dictionary::TablePodColumns(_TablePodColumns, _TablePodColumns + COUNTOF(_TablePodColumns));

const ColumnID Dictionary::_PodColumnColumns[] = {PodColumnPodID, PodColumnColumnID};
const ColumnIDs  Dictionary::PodColumnColumns(_PodColumnColumns, _PodColumnColumns + COUNTOF(_PodColumnColumns));

const ColumnID Dictionary::_StashColumns[] = {StashID, StashHostID};
const ColumnIDs Dictionary::StashColumns(_StashColumns, _StashColumns + COUNTOF(_StashColumns));

const ColumnID Dictionary::_StashColumnColumns[] = {StashColumnStashID, StashColumnColumnID};
const ColumnIDs Dictionary::StashColumnColumns(_StashColumnColumns, _StashColumnColumns + COUNTOF(_StashColumnColumns));
