#ifndef __RESPONSE_LOADER_H_
#define __RESPONSE_LOADER_H_

#include <opendnp3/IMeasurementHandler.h>
#include <opendnp3/ObjectInterfaces.h>
#include <opendnp3/MeasurementUpdate.h>

#include <openpal/Visibility.h>
#include <openpal/Loggable.h>
#include <openpal/LoggableMacros.h>

#include "CTOHistory.h"
#include "ObjectReadIterator.h"

namespace opendnp3
{

class HeaderReadIterator;

/**
 * Dedicated class for processing response data in the master.
 */
class DLL_LOCAL ResponseLoader : openpal::Loggable
{
public:
	/**
	 * Creates a new ResponseLoader instance.
	 *
	 * @param arLogger			the Logger that the loader should use for
	 * 						message reporting
	 * @param aUpdate		the callback to use for dispatching measurements
	 * @param apVtoReader	the VtoReader for any responses that match
	 *
	 * @return				a new ResponseLoader instance
	 */
	ResponseLoader(openpal::Logger& arLogger,
	               const std::function<void (MeasurementUpdate&)>& aUpdate);

	/**
	* When the response loader destructs, it flushes an update to the publisher if
	* any measurements were loaded via Process()
	*/
	~ResponseLoader();

	/**
	 * Processes a DNP3 object received by the Master.  The real heavy
	 * lifting is done in ResponseLoader::ProcessData().
	 *
	 * @param itr			the header iterator that provides access to
	 * 						the group, variation, data, etc.
	 */
	void Process(HeaderReadIterator& itr);

private:

	/**
	 * Processes the data field of a DNP3 object received by the Master.
	 * This function multiplexes objects to the Read(), ReadBitfield(), or
	 * ReadCTO() as is appropriate for the group/variation tuple.
	 *
	 * @param itr		the header iterator that provides access to the
	 * 					group, variation, data, etc.
	 * @param aGrp		the DNP3 group object id
	 * @param aVar		the DNP3 variation object id
	 */
	void ProcessData(HeaderReadIterator& itr, int aGrp, int aVar);

	/**
	* Processes size by variation objects received by the master.  The
	* variation id ('aVar') is the size of the data object.
	*
	* @param itr		the header iterator that provides access to the
	* 					group, variation, data, etc.
	* @param aGrp		the DNP3 group object id
	* @param aVar		the DNP3 variation id
	*/
	void ProcessSizeByVariation(HeaderReadIterator& itr, int aGrp, int aVar);

	template <class T>
	void Read(HeaderReadIterator& arIter, StreamObject<T>* apObj);

	template <class T>
	void ReadBitfield(HeaderReadIterator& arHeader);

	template <class T>
	void ReadCTO(HeaderReadIterator& arIter);

	/**
	 * Convert an incoming data stream for DNP3 Object Groups 112 or 113
	 * into a VtoData object and pass the object to the user application
	 * via the VtoReader instance.
	 *
	 * @param arIter	the header iterator that provides access to the
	 * 					group, variation, data, etc.	 
	 */
	void ReadOctetData(HeaderReadIterator& arIter, const std::function<void (const uint8_t*, size_t, uint32_t)>& process);

	std::function<void (MeasurementUpdate&)> mCallback;
	MeasurementUpdate mUpdate;
	CTOHistory mCTO;
};

template <class T>
void ResponseLoader::ReadCTO(HeaderReadIterator& arIter)
{
	ObjectReadIterator i = arIter.BeginRead();

	if (i.Count() != 1) {
		LOG_BLOCK(openpal::LogLevel::Warning, "Invalid number of CTO objects");
		return;
	}

	auto t = T::Inst()->mTime.Get(*i);
	mCTO.SetCTO(t);
}

template <class T>
void ResponseLoader::Read(HeaderReadIterator& arIter, StreamObject<T>* apObj)
{
	int64_t t = 0;

	if (apObj->UseCTO() && !mCTO.GetCTO(t)) {
		LOG_BLOCK(openpal::LogLevel::Error,
		          "No CTO for relative time type " << apObj->Name());
		return;
	}

	ObjectReadIterator obj = arIter.BeginRead();
	LOG_BLOCK(openpal::LogLevel::Interpret,
	          "Converting " << obj.Count() << " " << apObj->Name());

	for ( ; !obj.IsEnd(); ++obj) {
		size_t index = obj->Index();
		T value = apObj->Read(*obj);

		/* Make sure the value has time information */
		if (apObj->UseCTO()) {
			value.SetTime(t + value.GetTime());
		}

		/* Make sure the value has quality information */
		if (!apObj->HasQuality()) {
			value.SetQuality(T::ONLINE);
		}

		mUpdate.Add(value, index);
		
	}
}

template <class T>
void ResponseLoader::ReadBitfield(HeaderReadIterator& arIter)
{
	Binary b; b.SetQuality(Binary::ONLINE);

	ObjectReadIterator obj = arIter.BeginRead();
	LOG_BLOCK(openpal::LogLevel::Interpret,
	          "Converting " << obj.Count() << " " << T::Inst()->Name() << " "
	          "To Binary");

	for (; !obj.IsEnd(); ++obj) {
		bool val = BitfieldObject::StaticRead(*obj, obj->Start(), obj->Index());
		b.SetValue(val);
		mUpdate.Add(b, obj->Index());
	}
}

}

/* vim: set ts=4 sw=4: */

#endif

