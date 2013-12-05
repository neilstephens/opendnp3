#include <boost/test/unit_test.hpp>

#include "TestHelpers.h"

#include <opendnp3/EventBufferBase.h>

#include <limits>
#include <set>

using namespace std;
using namespace opendnp3;


template <class T>
struct SetWrapper {
	struct ValueOrder {
		bool operator()(const T& a, const T& b) {
			return a.mValue < b.mValue;
		}
	};

	typedef std::set<T, ValueOrder> Type;
};

template <class T>
class SimpleBuffer : public EventBufferBase<T, SetWrapper<T> >
{
public:

	SimpleBuffer(size_t aMaxEvents) : EventBufferBase<T, SetWrapper<T> >(aMaxEvents)
	{}
};

BOOST_AUTO_TEST_SUITE(TestEventBufferBase)
typedef EventInfo<int> intevt;

BOOST_AUTO_TEST_CASE(SetInitState)
{
	SimpleBuffer< intevt > b(2);

	BOOST_REQUIRE_EQUAL(b.ClearWrittenEvents(), 0);
	BOOST_REQUIRE_EQUAL(b.Deselect(), 0);
}

BOOST_AUTO_TEST_CASE(SetUpdate)
{
	SimpleBuffer< intevt > b(2);

	b.Update(1, PC_CLASS_1, 0);
	BOOST_REQUIRE_EQUAL(b.Size(), 1);
	b.Update(1, PC_CLASS_1, 0);
	BOOST_REQUIRE_EQUAL(b.Size(), 1);
}

BOOST_AUTO_TEST_CASE(SimpleOverflow)
{
	SimpleBuffer< intevt > b(1);

	b.Update(1, PC_CLASS_1, 0);
	BOOST_REQUIRE_EQUAL(b.Size(), 1);
	b.Update(2, PC_CLASS_1, 0);
	BOOST_REQUIRE_EQUAL(b.Size(), 1);
	BOOST_REQUIRE(b.IsOverflown());
}

BOOST_AUTO_TEST_CASE(OverflowDuringSelection)
{
	SimpleBuffer< intevt > b(2);

	b.Update(1, PC_CLASS_1, 0);
	b.Update(2, PC_CLASS_1, 1);

	BOOST_REQUIRE_EQUAL(b.Select(PC_CLASS_1, 1), 1);

	b.Update(3, PC_CLASS_1, 0);

	// b/c the selected events don't count towards overflow, this should still be false
	BOOST_REQUIRE_FALSE(b.IsOverflown());

	b.Update(4, PC_CLASS_1, 2);
	BOOST_REQUIRE(b.IsOverflown()); //now we lose an event

	//clear the buffer and make sure the overflown flag gets cleared

	b.Begin()->mWritten = true;
	BOOST_REQUIRE_EQUAL(b.ClearWrittenEvents(), 1);
	BOOST_REQUIRE_EQUAL(b.Size(), 2);
	BOOST_REQUIRE(b.IsOverflown()); // still overflow until we remove an event

	b.Select(PC_CLASS_1, 1);
	b.Begin()->mWritten = true;
	BOOST_REQUIRE_EQUAL(b.ClearWrittenEvents(), 1);
	BOOST_REQUIRE_FALSE(b.IsOverflown());
}

BOOST_AUTO_TEST_CASE(SelectDeselect)
{
	SimpleBuffer< intevt > b(5);

	b.Update(1, PC_CLASS_1, 0);
	b.Update(2, PC_CLASS_1, 0);

	BOOST_REQUIRE_EQUAL(b.Select(PC_CLASS_2), 0);
	BOOST_REQUIRE_EQUAL(b.Select(PC_CLASS_1), 2);
	BOOST_REQUIRE_EQUAL(b.Select(PC_CLASS_1), 0); //performing an additional select should add no more values

	BOOST_REQUIRE_EQUAL(b.Deselect(), 2); // all events should go back into the buffer
	BOOST_REQUIRE_EQUAL(b.Deselect(), 0); // all events should go back into the buffer
}

BOOST_AUTO_TEST_CASE(SelectAndClear)
{
	SimpleBuffer< intevt > b(5);

	b.Update(1, PC_CLASS_1, 0);
	b.Update(2, PC_CLASS_1, 0);

	BOOST_REQUIRE_EQUAL(b.Select(PC_CLASS_1), 2);
	BOOST_REQUIRE_EQUAL(b.NumSelected(), 2);
	BOOST_REQUIRE_EQUAL(b.Select(PC_CLASS_1), 0); //performing an additional select should add no more values
	BOOST_REQUIRE_EQUAL(b.NumSelected(), 2);
	EvtItr<intevt>::Type itr = b.Begin();

	itr->mWritten = true;

	BOOST_REQUIRE_EQUAL(b.ClearWrittenEvents(), 1);
	BOOST_REQUIRE_EQUAL(b.NumSelected(), 1);
}
BOOST_AUTO_TEST_SUITE_END()
