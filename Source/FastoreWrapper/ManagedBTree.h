#pragma once
#include "stdafx.h"
#include "../FastoreCore/BTree.h"
#include "..\FastoreCore\Schema\standardtypes.h"

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