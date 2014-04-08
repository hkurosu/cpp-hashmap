#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace cpphashmap
{		
	template <>
	class HashFunc<int>
	{
	public:
		uint32_t operator() (const int val) const
		{
			uint32_t x = static_cast<uint32_t>(val);
			return x + (x >> 3) + (x >> 13) + (x >> 23);
		}
	};

	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			HashMap<int, int> hash;
			hash.insert(1, 1);
			int count = hash.count();
		}

	};
}