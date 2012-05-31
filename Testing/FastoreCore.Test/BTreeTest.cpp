#include "StdAfx.h"
#include <cfixcc.h>

#include <sstream>
#include <iostream>
#include <hash_set>
#include <algorithm>

#include "BTree.h"
#include "Schema\standardtypes.h"

using namespace std;


class BTreeTest : public cfixcc::TestFixture
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

	void IteratorBehavior()
	{
		BTree etree(standardtypes::Int, standardtypes::Int);

		auto begin = etree.begin();
		auto end = etree.end();

		//Empty tree should return end pointer
		CFIX_ASSERT(begin == end);

		int i = 2;
		auto find = etree.find(&i);
		CFIX_ASSERT(find == end);

		bool match;
		find = etree.findNearest(&i, match);
		CFIX_ASSERT(find == end);
		CFIX_ASSERT(match == false);

		int countarray[] = {50 , 5000, 100000 };
		for (int i = 0; i < 3; i++)
		{

			BTree tree(standardtypes::Int, standardtypes::Int);

			//Put stuff in tree
			int numrows =  countarray[i];
			for (int i = 0; i <= numrows; i += 2)
			{
				auto path = tree.GetPath(&i);
				CFIX_ASSERT(path.Match == false);
				tree.Insert(path, &i, &i);
			}

			//Iteration should cover the entire tree with no missing ends;
			begin = tree.begin();
			end = tree.end();
			int j = 0;
			while (begin != end)
			{
				CFIX_ASSERT((*(int*)(*begin).key) == j);
				CFIX_ASSERT((*(int*)(*begin).value) == j);
				++begin;
				j += 2;
			}

			CFIX_ASSERT(j == numrows + 2);

			//we should be able to iterate backwards as well with no problems.
			for (int i = numrows; i >= 0; i -= 2)
			{	
				--end;
				CFIX_ASSERT((*(int*)(*end).key) == i);
				CFIX_ASSERT((*(int*)(*end).value) == i);
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

			CFIX_ASSERT(exthrown == true);

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
		
			CFIX_ASSERT(exthrown == true);

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
		
			CFIX_ASSERT(exthrown == true);


			//find should return a path to an item if found, or the end path if not found
			int test = 3;
			find = tree.find(&test);

			CFIX_ASSERT(find == tree.end());

			test = 4;
			find = tree.find(&test);

			CFIX_ASSERT((*(int*)(*find).key) == 4);

			//first nearest should return the item if found, or the next high item if not found, or the end if there is no higher value
			find = tree.findNearest(&test, match);
			CFIX_ASSERT((*(int*)(*find).key) == 4);
			CFIX_ASSERT(match == true);

			test = 3;
			find = tree.findNearest(&test, match);
			CFIX_ASSERT((*(int*)(*find).key) == 4);
			CFIX_ASSERT(match == false);

			test = numrows + 1;
			find = tree.findNearest(&test, match);
			CFIX_ASSERT(find == tree.end());
			CFIX_ASSERT(match == false);
		}

	}

	void Sequential()
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
				CFIX_ASSERT(path.Match == false);
				tree.Insert(path, &i, &i);
			}

			auto begin = tree.begin();
			for (int i = 0; i <= numrows; i++)
			{
				CFIX_ASSERT((*(int*)(*begin).key) == i);
				CFIX_ASSERT((*(int*)(*begin).value) == i);
				begin++;
			}
		}
	}

	void Reverse()
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
				CFIX_ASSERT(path.Match == false);
				tree.Insert(path, &i, &i);
			}

			auto begin = tree.begin();
			for (int i = 0; i <= numrows; i++)
			{
				CFIX_ASSERT((*(int*)(*begin).key) == i);
				CFIX_ASSERT((*(int*)(*begin).value) == i);
				begin++;
			}
		}
	}

	void Random()
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
				CFIX_ASSERT(path.Match == false);
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
				CFIX_ASSERT((*(int*)(*begin).key) == vec[j]);
				j++;
				begin++;
			}

			CFIX_ASSERT(j == vec.size());
		}
	}

	void Deletion()
	{
		BTree tree(standardtypes::Int, standardtypes::Int);

		//Put stuff in tree
		int numrows = 10000;
		for (int i = 0; i <= numrows; i++)
		{
			auto path = tree.GetPath(&i);
			tree.Insert(path, &i, &i);
		}

		for (int i = 0; i <= numrows; i++)
		{
			auto path = tree.GetPath(&i);
			CFIX_ASSERT(path.Match == true);
			tree.Delete(path);
			path = tree.GetPath(&i);
			CFIX_ASSERT(path.Match == false);
		}
	}

};

CFIXCC_BEGIN_CLASS( BTreeTest )
	CFIXCC_METHOD( IteratorBehavior )
	CFIXCC_METHOD( Sequential )
	CFIXCC_METHOD( Reverse )
	CFIXCC_METHOD( Random )
	CFIXCC_METHOD( Deletion )
CFIXCC_END_CLASS()