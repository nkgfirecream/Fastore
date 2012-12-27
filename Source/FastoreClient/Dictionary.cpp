#include "Dictionary.h"

#define COUNTOF(a) (sizeof(a)/sizeof(*a))

using namespace fastore::client;
using fastore::communication::ColumnID;

const ColumnID Dictionary::GeneratorNextValue(10000);

const ColumnID Dictionary::_GeneratorColumns[] = {GeneratorNextValue};
const ColumnIDs  Dictionary::GeneratorColumns(_GeneratorColumns, _GeneratorColumns + COUNTOF(_GeneratorColumns));

