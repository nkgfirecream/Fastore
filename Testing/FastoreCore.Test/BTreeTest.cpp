#include "stdafx.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "BTree.h"
#include "Schema\standardtypes.h"
#include <hash_set>
#include <algorithm>

using namespace std;


TEST_CLASS(BTreeTest)
{
public:
	wstring RandomString(int length)
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

	template<typename T>
	void IterateType(ScalarType type)
	{
		BTree etree(type, type);

		auto begin = etree.begin();
		auto end = etree.end();

		//Empty tree should return end pointer
		Assert::IsTrue(begin == end);

		T i = 2;
		auto find = etree.find(&i);
		Assert::IsTrue(find == end);

		bool match;
		find = etree.findNearest(&i, match);
		Assert::IsTrue(find == end);
		Assert::IsFalse(match);

		int countarray[] = {50 , 5000, 100000 };
		for (T i = 0; i < 3; i++)
		{

			BTree tree(type, type);

			//Put stuff in tree
			int numrows =  countarray[i];
			for (T k = 0; k <= numrows; k += 2)
			{
				auto path = tree.GetPath(&k);
				Assert::IsFalse(path.Match);
				tree.Insert(path, &k, &k);
			}

			//Iteration should cover the entire tree with no missing ends;
			begin = tree.begin();
			end = tree.end();
			T j = 0;
			while (begin != end)
			{
				Assert::AreEqual<T>((*(T*)(*begin).key), j);
				Assert::AreEqual<T>((*(T*)(*begin).value), j);
				++begin;
				j += 2;
			}

			Assert::AreEqual<T>(j, numrows + 2);

			//we should be able to iterate backwards as well with no problems.
			for (int i = numrows; i >= 0; i -= 2)
			{	
				--end;
				Assert::AreEqual<T>((*(T*)(*end).key), i);
				Assert::AreEqual<T>((*(T*)(*end).value), i);
			}	

			//Incrementing at the end should throw an exception
			end = tree.end();
			bool exthrown = false;

			try
			{
				end++;
			}
			catch(...)
			{
				exthrown = true;
			}

			Assert::IsTrue(exthrown);

			//Dereferencing at the end should throw an exception
			end = tree.end();
			exthrown = false;
			try
			{
				(*end);
			}
			catch(...)
			{
				exthrown = true;
			}
		
			Assert::IsTrue(exthrown);

			//Decrementing at the beginning should throw an exception
			begin = tree.begin();
			exthrown = false;
			try
			{
				begin--;
			}
			catch(...)
			{
				exthrown = true;
			}
		
			Assert::IsTrue(exthrown);


			//find should return a path to an item if found, or the end path if not found
			T test = 3;
			find = tree.find(&test);

			Assert::IsTrue(find == tree.end());

			test = 4;
			find = tree.find(&test);

			Assert::AreEqual<T>((*(T*)(*find).key), 4);

			//first nearest should return the item if found, or the next high item if not found, or the end if there is no higher value
			find = tree.findNearest(&test, match);
			Assert::AreEqual<T>((*(T*)(*find).key), 4);
			Assert::IsTrue(match);

			test = 3;
			find = tree.findNearest(&test, match);
			Assert::AreEqual<T>((*(T*)(*find).key), 4);
			Assert::IsFalse(match);

			test = numrows + 1;
			find = tree.findNearest(&test, match);
			Assert::IsTrue(find == tree.end());
			Assert::IsFalse(match);
		}
	}

	TEST_METHOD(IteratorBehavior)
	{
		IterateType<int64_t>(standardtypes::Long);
		IterateType<int>(standardtypes::Int);		
	}

	TEST_METHOD(Sequential)
	{
		//Test is done on various sizes of trees to ensure that we build correctly no matter if the tree
		//is flat, shallow, or deep.
		int countarray[] = {50 , 5000, 100000 };
		for (int i = 0; i < 3; i++)
		{
			//ASSUMPTION: Type compare logic is correct
			//This is a basic test to ensure that everything entered in a sequential manner
			//will still be present in the tree and in the same order.
			BTree tree(standardtypes::Int, standardtypes::Int);

			//Note: We need at least DefaultListSize * DefaultListSize values to ensure we get more than one level deep
			//on the btree. Currently DefaultListSize is set at 128.
			int numrows = countarray[i];

			for (int i = 0; i <= numrows; i++)
			{
				auto path = tree.GetPath(&i);
				Assert::IsFalse(path.Match);
				tree.Insert(path, &i, &i);
			}

			auto begin = tree.begin();
			for (int i = 0; i <= numrows; i++)
			{
				Assert::AreEqual<int>((*(int*)(*begin).key), i);
				Assert::AreEqual<int>((*(int*)(*begin).value), i);
				begin++;
			}

			for (int i = 0; i <= numrows; i++)
			{
				auto path = tree.GetPath(&i);
				Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].value), i);
				Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].key), i);
			}
		}
	}

	TEST_METHOD(Reverse)
	{
		int countarray[] = {50 , 5000, 100000 };
		for (int i = 0; i < 3; i++)
		{
			//If we insert items in reverse order, when we pull them out they should be in forward order
			//(according to the comparison operator of the type) and they should all be present.
			BTree tree(standardtypes::Int, standardtypes::Int);

			int numrows = countarray[i];

			for (int i = numrows; i >= 0; i--)
			{
				auto path = tree.GetPath(&i);
				Assert::IsFalse(path.Match);
				tree.Insert(path, &i, &i);
			}

			auto begin = tree.begin();
			for (int i = 0; i <= numrows; i++)
			{
				Assert::AreEqual<int>((*(int*)(*begin).key), i);
				Assert::AreEqual<int>((*(int*)(*begin).value), i);
				begin++;
			}
		}
	}

	TEST_METHOD(Random)
	{
		int countarray[] = {50 , 5000, 20000 };
		for (int i = 0; i < 3; i++)
		{
			//Generate a bunch of random numbers and then insert them into the BTree.
			//Once inserted, the resulting tree should be in the same order as a sorted
			//vector of the same values.
			int numrows = countarray[i];
			std::hash_set<int> hash;
			std::vector<int> vec;
			BTree tree(standardtypes::Int, standardtypes::Int);

			while(hash.size() < numrows)
			{
				hash.insert(rand());
			}

			auto hb = hash.begin();
			while (hb != hash.end())
			{
				int i = (*hb);
				auto path = tree.GetPath(&i);
				Assert::IsFalse(path.Match);
				tree.Insert(path, &i, &i);

				vec.push_back(i);

				++hb;
			}

			std::sort(vec.begin(), vec.end());

			auto begin = tree.begin();
			auto end = tree.end();
			int j = 0;
			while (begin != end)
			{
				Assert::AreEqual<int>((*(int*)(*begin).key), vec[j]);
				j++;
				begin++;
			}

			Assert::AreEqual<size_t>(j, vec.size());
		}
	}

	TEST_METHOD(Deletion)
	{
		BTree tree(standardtypes::Int, standardtypes::Int);

		//Put stuff in tree
		int numrows = 4096;
		for (int i = 0; i < numrows; i++)
		{
			auto path = tree.GetPath(&i);
			tree.Insert(path, &i, &i);
			path = tree.GetPath(&i);
			Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].value), i);
			Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].key), i);
		}

		for (int i = 0; i < numrows; i++)
		{
			auto path = tree.GetPath(&i);
			Assert::IsTrue(path.Match);
			Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].value), i);
			Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].key), i);

			tree.Delete(path);

			path = tree.GetPath(&i);
			Assert::IsFalse(path.Match);
		}

		for (int i = 0; i < numrows; i++)
		{
			auto path = tree.GetPath(&i);
			tree.Insert(path, &i, &i);
			path = tree.GetPath(&i);
			Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].value), i);
			Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].key), i);
		}

		for (int i = numrows - 1; i >= 0; i--)
		{
			auto path = tree.GetPath(&i);
			Assert::IsTrue(path.Match);
			Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].value), i);
			Assert::AreEqual<int>((*(int*)(*path.Leaf)[path.LeafIndex].key), i);

			tree.Delete(path);

			path = tree.GetPath(&i);
			Assert::IsFalse(path.Match);
		}
	}
};