#pragma once
#include "../Tree/BTree.h"
#include "IColumnBuffer.h"

class TreeBuffer : public IColumnBuffer
{
	public:
		TreeBuffer(const ScalarType& rowType, const ScalarType &valueType);	

		vector<OptionalValue> GetValues(const vector<std::string>& rowIds);		
		OptionalValue GetValue(const std::string &rowId);
		void Apply(const ColumnWrites& writes);
		RangeResult GetRows(const RangeRequest& range);
		Statistic GetStatistic();
		const ScalarType& GetRowIdType();
		const ScalarType& GetValueType();

	private:
		bool Include(void* rowId, void* value);
		bool Exclude(void* rowId);
		bool Exclude(void* rowId, void* value);
		void* GetValue(void* rowId);

		void ValuesMoved(void*, Node*);
		const ScalarType& _rowType;
		const ScalarType& _valueType;
		std::unique_ptr<BTree> _rows;
		std::unique_ptr<BTree> _values;	
		long long _unique;
		long long _total;
};