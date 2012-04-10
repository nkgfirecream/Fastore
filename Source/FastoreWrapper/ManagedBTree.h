#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/BTree.h"
#include "..\FastoreCore\Schema\standardtypes.h"
#pragma managed(pop)

namespace Wrapper
{
	public ref class ManagedBTree
	{
		private:
			BTree* _nativeTree;

		public:
			ManagedBTree()
			{
				_nativeTree = new BTree(standardtypes::GetIntType(), standardtypes::GetIntType());			
			}

			void Insert(int key, int value);
			void Delete(int key);
			int Get(int key);
	};
}