#pragma once
#include "Cursor.h"

using namespace fastore::module;

Cursor::Cursor(fastore::module::Table* table) : _table(table) { }

void Cursor::next()
{

}

void Cursor::eof()
{

}

std::string Cursor::column(int index)
{

}

std::string Cursor::rowId()
{

}