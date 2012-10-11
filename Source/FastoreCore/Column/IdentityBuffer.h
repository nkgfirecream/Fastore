#pragma once

#include "../BTree.h"
#include "../Column/IColumnBuffer.h"

class IdentityBuffer : public IColumnBuffer
{
	public:
		IdentityBuffer(const ScalarType& type);

		vector<OptionalValue> GetValues(const vector<std::string>& rowIds);		
		void Apply(const ColumnWrites& writes);
		RangeResult GetRows(const RangeRequest& range);
		Statistic GetStatistic();

	private:
		bool Include(void* rowId);
		bool Exclude(void* rowId);

		ScalarType _type;
		std::unique_ptr<BTree> _rows;
		long long _count;
};
