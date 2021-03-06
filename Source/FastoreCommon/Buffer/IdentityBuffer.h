#pragma once
#include "../Tree/BTree.h"
#include "IColumnBuffer.h"

class IdentityBuffer : public IColumnBuffer
{
	public:
		IdentityBuffer(const ScalarType& type);

		vector<OptionalValue> GetValues(const vector<std::string>& rowIds);		
		OptionalValue GetValue(const std::string &rowId);
		void Apply(const ColumnWrites& writes);
		RangeResult GetRows(const RangeRequest& range);
		Statistic GetStatistic();
		const ScalarType& GetRowIdType();
		const ScalarType& GetValueType();

	private:
		bool Include(void* rowId);
		bool Exclude(void* rowId);

		const ScalarType& _type;
		std::unique_ptr<BTree> _rows;
		long long _count;
};
