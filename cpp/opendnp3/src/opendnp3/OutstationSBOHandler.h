#ifndef __OUTSTATION_SBO_HANDLER_H_
#define __OUTSTATION_SBO_HANDLER_H_

#include <openpal/Location.h>
#include <openpal/Visibility.h>
#include <openpal/TimeDuration.h>
#include <openpal/IExecutor.h>

#include <opendnp3/ICommandHandler.h>
#include <opendnp3/APDUConstants.h>

#include "gen/QualifierCode.h"

#include <map>

namespace opendnp3
{

class DLL_LOCAL OutstationSBOHandler
{

	template <class T>
	class SelectInfo
	{
	public:
		SelectInfo(const T& arCommand, uint8_t aSequence, QualifierCode aCode, openpal::MonotonicTimestamp aTimestamp) :
			mCommand(arCommand),
			mSequence(aSequence),
			mCode(aCode),
			mTimestamp(aTimestamp),
			mOperated(false)
		{}

		SelectInfo() :
			mSequence(0),
			mCode(QualifierCode::UNDEFINED),
			mTimestamp(),
			mOperated(false)
		{}

		T mCommand;
		uint8_t mSequence;
		QualifierCode mCode;
		openpal::MonotonicTimestamp mTimestamp;
		bool mOperated;
	};

	typedef std::map<size_t, SelectInfo<ControlRelayOutputBlock>> CROBSelectMap;
	typedef std::map<size_t, SelectInfo<AnalogOutputInt16>> Analog16SelectMap;
	typedef std::map<size_t, SelectInfo<AnalogOutputInt32>> Analog32SelectMap;
	typedef std::map<size_t, SelectInfo<AnalogOutputFloat32>> AnalogFloatSelectMap;
	typedef std::map<size_t, SelectInfo<AnalogOutputDouble64>> AnalogDoubleSelectMap;

public:
	OutstationSBOHandler(openpal::TimeDuration aSelectTimeout, ICommandHandler* apCmdHandler, openpal::IExecutor* apExecutor);

	CommandStatus Select(const ControlRelayOutputBlock& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);
	CommandStatus Operate(const ControlRelayOutputBlock& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);

	CommandStatus Select(const AnalogOutputInt16& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);
	CommandStatus Operate(const AnalogOutputInt16& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);

	CommandStatus Select(const AnalogOutputInt32& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);
	CommandStatus Operate(const AnalogOutputInt32& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);

	CommandStatus Select(const AnalogOutputFloat32& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);
	CommandStatus Operate(const AnalogOutputFloat32& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);

	CommandStatus Select(const AnalogOutputDouble64& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);
	CommandStatus Operate(const AnalogOutputDouble64& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode);

private:

	void ClearAll();

	template <class T>
	CommandStatus Select(const T& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode, std::map<size_t, SelectInfo<T>>& arMap);

	template <class T>
	CommandStatus Operate(const T& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode, std::map<size_t, SelectInfo<T>>& arMap);

	openpal::TimeDuration mSelectTimeout;
	ICommandHandler* mpCmdHandler;
	openpal::IExecutor* mpExecutor;
	uint8_t mCurrentSequenceNum;

	CROBSelectMap mCROBSelectMap;
	Analog16SelectMap mAnalog16SelectMap;
	Analog32SelectMap mAnalog32SelectMap;
	AnalogFloatSelectMap mAnalogFloatSelectMap;
	AnalogDoubleSelectMap mAnalogDoubleSelectMap;
};

template <class T>
CommandStatus OutstationSBOHandler::Select(const T& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode, std::map<size_t, SelectInfo<T>>& arMap)
{
	if(aSequence != mCurrentSequenceNum) {
		this->ClearAll();
	}

	mCurrentSequenceNum = aSequence;

	CommandStatus status =  mpCmdHandler->Select(arCommand, aIndex);
	if(status == CommandStatus::SUCCESS) { //outstation supports this point
		auto time = mpExecutor->GetTime();
		SelectInfo<T> info(arCommand, aSequence, aCode, time);
		arMap[aIndex] = info; // record the select by index
	}
	return status;
}

template <class T>
CommandStatus OutstationSBOHandler::Operate(const T& arCommand, size_t aIndex, uint8_t aSequence, QualifierCode aCode, std::map<size_t, SelectInfo<T>>& arMap)
{
	auto iter = arMap.find(aIndex);
	if(iter == arMap.end()) {
		this->ClearAll();
		return CommandStatus::NO_SELECT; //no prior select
	}
	else {
		// what should the sequence number be?
		uint8_t expectedSeq = (iter->second.mSequence + 1) % 16;
		// are all values what we expect them to be?
		if(expectedSeq == aSequence && aCode == iter->second.mCode && arCommand == iter->second.mCommand) {
			// now check the timestamp
			auto now = mpExecutor->GetTime();
			if((now.milliseconds - iter->second.mTimestamp.milliseconds) < mSelectTimeout.GetMilliseconds()) {
				if(iter->second.mOperated) {
					return CommandStatus::SUCCESS;
				}
				else {
					iter->second.mOperated = true;
					return mpCmdHandler->Operate(arCommand, aIndex);
				}
			}
			else return CommandStatus::TIMEOUT;
		}
		else {
			this->ClearAll();
			return CommandStatus::NO_SELECT;
		}
	}
}

}

#endif
