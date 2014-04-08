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
            return val;
		}
	};

	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			HashMap<int, int> hash(4);
			hash.insert(1, 10);
			int count = hash.count();
            Assert::AreEqual(1, count);
            HashMap<int, int>::iterator i = hash.find(1);
            Assert::AreEqual(10, i->value);
		}

	};
}