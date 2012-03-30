#include "Schema\standardtypes.h"
#include "Table\table.h"
#include "Column\HashBuffer.h"
#include "Column\UniqueBuffer.h"

#include <conio.h>
#include <tbb\queuing_mutex.h>
#include <iostream>
#include "Util/Stopwatch.h"
#include <sstream>
#include <bitset>
#include "TransactionID.h"
#include "Change.h"



using namespace std;
using namespace standardtypes;

fs::wstring RandomString(int length)
{
	const wchar_t* _chars = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	wstringstream result;

	for (int i = 0; i < length; i++)
	{
		int index = rand() % 26;
		result << _chars[index];
	}
								 
	return result.str();
}

void StringTest()
{
	cout << "Testing Random Strings...";
	
	long numrows = 1000000;

	BTree tree(GetStringType(), GetStringType());	
	BTree tree2(GetPStringType(), GetPStringType());

	Stopwatch* watch = new Stopwatch();
	Stopwatch* watch2 = new Stopwatch();

	Node* dummy;
	Node* dummy2;
	for(int i = 0; i < numrows; i++)
	{
		fs::wstring insert = RandomString(rand() % 8 + 1);	
		watch->StartTimer();
		BTree::Path path = tree.GetPath(&insert);
		tree.Insert(path, &insert, &insert);	
		watch->StopTimer();

		fs::wstring* insert2 = new fs::wstring(insert);
		watch2->StartTimer();
		BTree::Path path2 = tree.GetPath(&insert2);
		tree2.Insert(path2, &insert2, &insert2);
		watch2->StopTimer();
	}

	double secs = watch->TotalTime();
	cout << " secs: " << secs << "\r\n";	
	cout << "\tRows per second WString: " << numrows / secs << "\r\n";

	double secs2 = watch2->TotalTime();
	cout << " secs: " << secs2 << "\r\n";	
	cout << "\tRows per second WString*: " << numrows / secs2 << "\r\n";
}

//void SequentialPLongTest()
//{
//	cout << "Testing Sequential PLongs...";
//	
//	long numrows = 10000000;
//
//	BTree tree(GetPLongType(), GetPLongType());	
//
//	Stopwatch watch;
//
//	Leaf* dummy;
//	for(int i = 0; i < numrows; i++)
//	{
//		long* item = new long;
//
//		*item = i;
//
//		watch.StartTimer();
//		tree.Insert(&item, &item, &dummy);	
//		watch.StopTimer();
//	}
//
//	double secs = watch.TotalTime();
//	cout << " secs: " << secs << "\r\n";
//	
//	cout << "\tRows per second: " << numrows / secs << "\r\n";
//
//	//wcout << tree->ToString();
//}

void SequentialLongTest()
{
	cout << "Testing Sequential Longs...";
	
	long numrows = 1000000;

	BTree tree(GetLongType(), GetLongType());	

	Stopwatch watch;

	for (long i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		BTree::Path path = tree.GetPath(&i);
		tree.Insert(path, &i, &i);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "\tRows per second: " << numrows / secs << "\r\n";
	//wcout << tree->ToString();
}

void SequentialIntTest()
{
	cout << "Testing Sequential Ints...";
	
	long numrows = 1000000;

	BTree tree(GetIntType(), GetIntType());	

	Stopwatch watch;

	for (int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		BTree::Path path = tree.GetPath(&i);
		tree.Insert(path, &i, &i);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "\tRows per second: " << numrows / secs << "\r\n";

	//wcout << tree->ToString();
}

void SequentialIntArrayTest()
{
	cout << "Testing Sequential Array Ints...";
	
	long numrows = 1000000;

	eastl::vector<int> a;
	//a.set_capacity(numrows);	// preallocation

	Stopwatch watch;

	for (int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		a.push_back(i);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "\tRows per second: " << numrows / secs << "\r\n";

	//wcout << tree->ToString();
}

void ReverseSequentialIntTest()
{
	cout << "Testing Reverse Sequential Ints...";
	
	long numrows = 1000000;

	BTree tree(GetIntType(), GetIntType());	

	Stopwatch watch;

	for (int i = numrows; i >= 0; --i)
	{
		watch.StartTimer();
		BTree::Path path = tree.GetPath(&i);
		tree.Insert(path, &i, &i);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "\tRows per second: " << numrows / secs << "\r\n";

	//wcout << tree->ToString();
}

void RandomIntTest()
{
	cout << "Testing Random Ints...\r\n";
	
	long numrows = 1000000;

	auto intType = GetIntType();
	BTree tree(intType, intType);	

	Stopwatch watch;

	cout << "	Inserts...";

	for (int i = 0; i < numrows; i++)
	{
		int x = (rand() << 16) | rand();
		watch.StartTimer();
		BTree::Path path = tree.GetPath(&x);
		if (!path.Match)
			tree.Insert(path, &x, &x);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "		Rows per second: " << numrows / secs << "\r\n";

	//wcout << tree->ToString();

	watch.Reset();
	cout << "	Finds...";

	for (int i = 0; i < numrows; i++)
	{
		int x = (rand() << 16) | rand();
		watch.StartTimer();
		BTree::Path path = tree.GetPath(&x);
		watch.StopTimer();
	}

	secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "		Rows per second: " << numrows / secs << "\r\n";

	//wcout << tree.ToString();
}

void RandomLongTest()
{
	cout << "Testing Random Longs...";
	
	long numrows = 1000000;

	auto longType = GetLongType();
	BTree tree(longType, longType);	

	Stopwatch watch;

	for (int i = 0; i < numrows; i++)
	{
		long long x = (long long)rand() << 16 | rand();
		watch.StartTimer();
		BTree::Path path = tree.GetPath(&x);
		if (!path.Match)
			tree.Insert(path, &x, &x);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "\tRows per second: " << numrows / secs << "\r\n";

	//wcout << tree->ToString();
}

void RandomStringTest()
{
	cout << "Testing Random Strings...";
	
	long numrows = 100;

	auto stringType = GetStringType();
	BTree tree(stringType, stringType);	

	Stopwatch watch;

	for (int i = 0; i < numrows; i++)
	{
		fs::wstring x = RandomString(8);
		watch.StartTimer();
		BTree::Path path = tree.GetPath(&x);
		if (!path.Match)
			tree.Insert(path, &x, &x);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "\tRows per second: " << numrows / secs << "\r\n";

	wcout << tree.ToString();
}

void GuidTest()
{
	//To be implemented
}

void QueueingMutexTest()
{
	tbb::queuing_mutex qm;

	cout << "Testing Queueing mutex...";
	Stopwatch watch;
	long numrows = 100000000;
	
	for(int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		tbb::queuing_mutex::scoped_lock lock(qm);
		lock.release();
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	cout << "Mutexes per second: " << numrows / secs << "\r\n";
}

void InterlockedTest()
{
	cout << "Testing InterlockSpeed...";
	Stopwatch watch;
	long numrows = 100000000;
	
	unsigned int val = 0;
	for(int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		while(InterlockedCompareExchange(&val, 1, 0) == 1)	
		InterlockedDecrement(&val);		
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	cout << "Interlocks per second: " << numrows / secs << "\r\n";
}

void ArrayCopyTest()
{
	int numrows = 10000000;

	int* intarray = (int*)alloca(numrows);

	Stopwatch watch;

	cout << "Testing Assignment speed...";

	for(int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		intarray[i] = i;
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	cout << "Assignments per second: " << numrows / secs << "\r\n";

	watch.Reset();
	cout << "Testing Copy speed...";
	for(int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		memcpy(&intarray[i], &i, sizeof(int));
		watch.StopTimer();
	}

	secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	cout << "Copies per second: " << numrows / secs << "\r\n";
}

//void BTreeIteratorTest()
//{
//	BTree tree(GetLongType(),GetLongType());
//	long numrows = 1000;
//	Leaf* dummy;
//	long i = 0;
//	for(i = 0; i < numrows; i++)
//	{
//		tree.Insert(&i,&i,&dummy);
//	}
//
//	BTree::iterator start = tree.begin();
//
//	i = 0;
//	Stopwatch watch;
//	watch.GetFrequency();
// 	while(!start.End())
//	{		
//		cout << *(long*)(*start).first << "\n\r";
//		watch.StartTimer();
//		start++;
//		watch.StopTimer();
//	}
//
//	/*start = tree.begin();
//	
//	while (start != end)
//	{
//		cout << *(long*)*end << "\n\r";
//		end--;
//	}*/
//
//	double secs = watch.TotalTime();
//	cout << " secs: " << secs << "\r\n";
//	cout << "iterations per second: " << numrows / secs << "\r\n";
//
//}

void OutputResult(const GetResult& result, const ScalarType& keyType, const ScalarType& valueType)
{
	for (unsigned int i = 0; i < result.Data.size(); i++)
	{
		wcout << keyType.ToString(result.Data[i].first) <<"\n\r";
		auto keys = result.Data[i].second;
		for (int j = 0; j < keys.size(); j++)
		{
			wcout << "\t" << valueType.ToString(keys[j]) <<"\n\r";		
		}
	}
}

void HashBufferTest()
{
	ScalarType longType = GetLongType();
	ScalarType stringType = GetStringType();
	cout << "Testing HashBuffer...\n\r";

	HashBuffer* hash = new HashBuffer(longType, longType);

	/*long* i = new long(0);
	hash->Include(i,i);
	auto result = hash->GetValue(i);
	wcout << *(long*)result;*/
	long numvalues = 100;
	long rowspervalue = 10;
	
	long rowId = 0;
	//Leave gaps so we can test match/no match
	//for (long i = 0; i < numvalues * 2; i = i + 2)
	//{
	//	for (long j = 0; j < rowspervalue; j++)
	//	{
	//		long* v = new long(i);
	//		long* r = new long(rowId);
	//		hash->Include(v, r);
	//		rowId++;
	//	}
	//}

	//rowId = 0;
	///*for (long i = 0; i < 1; i++)
	//{
	//	for (long j = 0; j < rowspervalue; j++)
	//	{
	//		long* r = new long(rowId);
	//		wcout << *(long*)hash->GetValue(r) << "\r\n";
	//		rowId++;
	//	}
	//}*/

	////Ascending, inclusive, first 30 rows;
	//Range range;
	//range.Ascending = true;
	//range.Limit = 400;

	//
	//RangeBound start;
	//start.Inclusive = false;
	//long i = 2;
	//start.Value = &i;

	//range.Start = start;

	//RangeBound end;
	//end.Inclusive = true;	
	//long j = 46;
	//end.Value = &j; 

	//range.End = end;

	//auto result = hash->GetRows(range);

	//OutputResult(result, longType, longType);

	HashBuffer* hash2 = new HashBuffer(longType, stringType);
	
	rowId = 0;
	for (long i = 0; i < numvalues * 2; i = i + 2)
	{
		fs::wstring s = RandomString(8);
		for (long j = 0; j < rowspervalue; j++)
		{
			hash2->Include(&s, &rowId);
			rowId++;
		}
	}

	/*rowId = 0;
	for (long i = 0; i < numvalues / 10; i++)
	{
		for (long j = 0; j < rowspervalue; j++)
		{
			wcout << *(fs::wstring*)hash2->GetValue(&rowId) << "\r\n";
			rowId++;
		}
	}*/

	//Ascending, inclusive, first 30 rows;
	Range range;
	range.Ascending = true;
	range.Limit = 400;

	RangeBound start;
	start.Inclusive = true;
	wstringstream stream;
	stream << "AAFBGARC";
	start.Value = new fs::wstring(stream.str());

	range.Start = start;

	RangeBound end;
	end.Inclusive = true;
	wstringstream stream2;
	stream2 << "BAFBGARC";
	end.Value = new fs::wstring(stream2.str());

	range.End = end;

	auto result2 = hash2->GetRows(range);

	OutputResult(result2, longType, stringType);

	
	cout << "Rows inserted";
}

void UniqueBufferTest()
{
	ScalarType longType = GetLongType();
	ScalarType stringType = GetStringType();
	cout << "Testing UniqueBuffer...\n\r";

	UniqueBuffer* unique = new UniqueBuffer(longType, longType);

	/*long* i = new long(0);
	unique->Include(i,i);
	auto result = unique->GetValue(i);
	wcout << *(long*)result;*/
	long numvalues = 100;
	long rowspervalue = 10;
	
	long rowId = 0;
	//Leave gaps so we can test match/no match
	//for (long i = 0; i < numvalues * 2; i = i + 2)
	//{
	//	for (long j = 0; j < rowspervalue; j++)
	//	{
	//		long* v = new long(i);
	//		long* r = new long(rowId);
	//		unique->Include(v, r);
	//		rowId++;
	//	}
	//}

	//rowId = 0;
	///*for (long i = 0; i < 1; i++)
	//{
	//	for (long j = 0; j < rowspervalue; j++)
	//	{
	//		long* r = new long(rowId);
	//		wcout << *(long*)unique->GetValue(r) << "\r\n";
	//		rowId++;
	//	}
	//}*/

	////Ascending, inclusive, first 30 rows;
	//Range range;
	//range.Ascending = true;
	//range.Limit = 400;

	//
	//RangeBound start;
	//start.Inclusive = false;
	//long i = 2;
	//start.Value = &i;

	//range.Start = start;

	//RangeBound end;
	//end.Inclusive = true;	
	//long j = 46;
	//end.Value = &j; 

	//range.End = end;

	//auto result = unique->GetRows(range);

	//OutputResult(result, longType, longType);

	UniqueBuffer* unique2 = new UniqueBuffer(longType, stringType);
	
	rowId = 0;
	for (long i = 0; i < numvalues * 2; i = i + 2)
	{
		for (long j = 0; j < rowspervalue; j++)
		{
			fs::wstring* s = new fs::wstring(RandomString(8));
			long* r = new long(rowId);
			unique2->Include(s, r);
			rowId++;
		}
	}

	/*rowId = 0;
	for (long i = 0; i < numvalues / 10; i++)
	{
		for (long j = 0; j < rowspervalue; j++)
		{
			wcout << *(fs::wstring*)unique2->GetValue(&rowId) << "\r\n";
			rowId++;
		}
	}*/

	//Ascending, inclusive, first 30 rows;
	Range range;
	range.Ascending = true;
	range.Limit = 400;

	RangeBound start;
	start.Inclusive = true;
	wstringstream stream;
	stream << "AAFBGARC";
	start.Value = new fs::wstring(stream.str());

	range.Start = start;

	RangeBound end;
	end.Inclusive = true;
	wstringstream stream2;
	stream2 << "BAFBGARC";
	end.Value = new fs::wstring(stream2.str());

	range.End = end;

	auto result2 = unique2->GetRows(range);

	OutputResult(result2, longType, stringType);

	
	cout << "Rows inserted";
}

void TestEAHashSet()
{
	ScalarType type = GetPLongType();
	//fshash_set<void*> set(type.Hash, type.HashCompare);

	eastl::hash_set<void*, ScalarType, ScalarType> set(32, type, type);
	
	long numrows = 200;
	for (long i = 0; i < numrows; i++)
	{
		long* s = new long(i);
		set.insert(s);
	}

	auto start = set.begin();
	auto end = set.end();
	while(start != end)
	{
		wcout << type.ToString(*start) << "\n\r";
		start++;
	}

	ScalarType type2 = GetStringType();

	eastl::hash_set<void*, ScalarType, ScalarType> set2(32, type2, type2);
	
	for (long i = 0; i < numrows; i++)
	{
		auto s = new fs::wstring(RandomString(8));
		set2.insert(s);
	}

	start = set2.begin();
	end = set2.end();
	while(start != end)
	{
		wcout << type2.ToString(*start) << "\n\r";
		start++;
	}
	
}

//void BTreeDeleteTest()
//{
//	int numrows = 1000;
//	ScalarType t = GetIntType();
//	BTree tree(t,t);	
//	wcout << L"Inserting:\n\r";
//	Leaf* dummy;
//	for(int i = 0; i < numrows; i++)
//	{
//		tree.Insert(&i, &i, &dummy);	
//	}
//	wcout << L"Result of insert:\n\r";
//	auto it = tree.begin();
//	while(!it.End())
//	{		
//		wcout << t.ToString((*it).second) << "\n\r";
//		it++;
//	}
//	wcout << L"deleting all from high:\n\r";
//	for(int i = numrows; i > 0; i--)
//	{
//
//		tree.Delete(&i);	
//	}
//
//	it = tree.begin();
//	wcout << L"Result of deleting all from top:\n\r";
//	while(!it.End())
//	{		
//		wcout << t.ToString((*it).second) << "\n\r";
//		it++;
//	}
//	_getch();
//
//	wcout << L"Inserting:\n\r";
//	for(int i = 0; i < numrows; i++)
//	{
//		tree.Insert(&i, &i, &dummy);	
//	}
//	wcout << L"Result of insert:\n\r";
//	it = tree.begin();
//	while(!it.End())
//	{		
//		wcout << t.ToString((*it).second) << "\n\r";
//		it++;
//	}
//
//	wcout << L"deleting all from 0:\n\r";
//	for(int i = 0; i < numrows; i++)
//	{
//
//		tree.Delete(&i);	
//	}
//
//	it = tree.begin();
//	wcout << L"Result of deleting all from 0:\n\r";
//	while(!it.End())
//	{		
//		wcout << t.ToString((*it).second) << "\n\r";
//		it++;
//	}
//}


DataSet CreateRandomDataSet(TupleType tt)
{
	static long rowID = 0;
	rowID++;
	DataSet ds(tt,1);

	ds.SetCell(0, 0, &rowID);

	auto string = new fs::wstring(RandomString(8));
	ds.SetCell(0, 1, string);

	return ds;
}

//void TableTest()
//{
//	ColumnType ct1;
//	ColumnType ct2;
//
//	ct1.IsRequired = true;
//	ct2.IsRequired = true;
//
//	ct1.Name = L"ID";
//	ct2.Name = L"Text";
//
//	ct1.IsUnique = true;
//	ct2.IsUnique = false;
//
//	ct1.Type = GetLongType();
//	ct2.Type = GetStringType();
//
//	eastl::vector<ColumnType> columns;
//
//	columns.push_back(ct1);
//	columns.push_back(ct2);
//
//	TupleType tt(columns);
//
//	Table t(tt);
//
//	eastl::vector<int> nums;
//	nums.push_back(0);
//	nums.push_back(1);
//
//	Ranges ranges;
//	ColumnRange range;
//	range.ColumnNumber = 0;
//	ranges.push_back(range);
//
//	for(int i = 0; i < 100; i++)
//	{
//		DataSet ds = CreateRandomDataSet(tt);
//		t.Include(ranges, ds, nums);
//	}
//}

void BTreePathTest()
{
	int numrows = 1000;
	ScalarType t = GetIntType();
	BTree tree(t,t);	
	wcout << L"Inserting:\n\r";
	for (int i = 0; i < numrows; i++)
	{
		auto path = tree.GetPath(&i);
		tree.Insert(path, &i, &i);
	}
	wcout << L"Result of insert:\n\r";
	auto it = tree.begin();
	while (!it.End())
	{		
		wcout << t.ToString((*it).value) << "\n\r";
		it++;
	}

	wcout << L"deleting all from high:\n\r";
	for (int i = numrows - 1; i >= 0; i--)
	{
		auto path = tree.GetPath(&i);
		if(!path.Match)
			throw;
		tree.Delete(path);	
	}

	it = tree.begin();
	wcout << L"Result of deleting all from top:\n\r";
	while (!it.End())
	{		
		wcout << t.ToString((*it).value) << "\n\r";
		it++;
	}
	_getch();

	wcout << L"Inserting:\n\r";
	for (int i = 0; i < numrows; i++)
	{
		auto path = tree.GetPath(&i);
		tree.Insert(path, &i, &i);
	}

	wcout << L"Result of insert:\n\r";
	it = tree.begin();
	while(!it.End())
	{		
		wcout << t.ToString((*it).value) << "\n\r";
		it++;
	}

	wcout << L"deleting all from 0:\n\r";
	for(int i = 0; i < numrows; i++)
	{
		auto path = tree.GetPath(&i);
		if(!path.Match)
			throw;
		tree.Delete(path);	
	}

	it = tree.begin();
	wcout << L"Result of deleting all from 0:\n\r";
	while(!it.End())
	{		
		wcout << t.ToString((*it).value) << "\n\r";
		it++;
	}
}

void TestTransactionID()
{
	TransactionID trans;

	trans.SetRevision(100);
	trans.SetTransaction(5);


	cout << "Revision: " << trans.GetRevision() << "\n\r";
	cout << "Transaction: " << trans.GetTransaction() << "\n\r";

	trans.SetRevision(126);
	cout << "Revision: " << trans.GetRevision() << "\n\r";
	cout << "Transaction: " << trans.GetTransaction() << "\n\r";

	trans.SetTransaction(446);
	cout << "Revision: " << trans.GetRevision() << "\n\r";
	cout << "Transaction: " << trans.GetTransaction() << "\n\r";
}

void TestChange()
{
	Change change;
	change.RowID = new long(1);
	change.Value = new long(1);
	change.Operation = change.Include;

	ChangeSet set;
	set.push_back(change);

	//TODO: actually test changes...
	cout << "Change created and added to set";


}

void main()
{
	
	//BTreeIteratorTest();
	//BTreeDeleteTest();
	//QueueingMutexTest();
	//StringTest();
	//SequentialLongTest();
	//SequentialIntArrayTest();
	//SequentialIntTest();
	//ReverseSequentialIntTest();
	//RandomIntTest();
	//RandomLongTest();
	RandomStringTest();
	//SequentialPLongTest();
	//InterlockedTest();
	//ArrayCopyTest();
	//GuidTest();
	//HashBufferTest();
	//UniqueBufferTest();
    //TestEAHashSet();
	//TableTest();
	//BTreePathTest();
	//TestTransactionID();
	//TestChange();
	_getch();
}


