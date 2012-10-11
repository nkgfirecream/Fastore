#pragma once
#include "../BTree.h"
#include "../Column/IColumnBuffer.h"

const int UniqueBufferRowMapInitialSize = 32;

class UniqueBuffer : public IColumnBuffer
{
	public:
		UniqueBuffer(const ScalarType& rowType, const ScalarType& valueType);

		vector<OptionalValue> GetValues(const vector<std::string>& rowIds);		
		void Apply(const ColumnWrites& writes);
		RangeResult GetRows(const RangeRequest& range);
		Statistic GetStatistic();

	private:
		bool Include(void* rowId, void* value);
		bool Exclude(void* rowId);
		bool Exclude(void* rowId, void* value);
		void* GetValue(void* rowId);

		void ValuesMoved(void* value, Node* leaf);		
		ScalarType _rowType;
		ScalarType _valueType;
		ScalarType _nodeType;
		std::unique_ptr<BTree> _rows;
		std::unique_ptr<BTree> _values;
		long long _count;
};