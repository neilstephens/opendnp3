#include "TransportLayer.h"

#include <openpal/Exception.h>
#include <opendnp3/TransportConstants.h>

#include <openpal/LoggableMacros.h>

#include "TransportStates.h"

#include <assert.h>
#include <sstream>

using namespace std;
using namespace openpal;

namespace opendnp3
{

TransportLayer::TransportLayer(Logger aLogger, size_t aFragSize) :
	Loggable(aLogger),
	IUpperLayer(aLogger),
	ILowerLayer(aLogger),
	mpState(TLS_Closed::Inst()),
	M_FRAG_SIZE(aFragSize),
	mReceiver(aLogger, this, aFragSize),
	mTransmitter(aLogger, this, aFragSize),
	mThisLayerUp(false)
{

}

///////////////////////////////////////
// Actions
///////////////////////////////////////

void TransportLayer::ThisLayerUp()
{
	mThisLayerUp = true;
	if(mpUpperLayer != nullptr) mpUpperLayer->OnLowerLayerUp();
}

void TransportLayer::ThisLayerDown()
{
	mReceiver.Reset();
	mThisLayerUp = false;
	if(mpUpperLayer != nullptr) mpUpperLayer->OnLowerLayerDown();
}

void TransportLayer::ChangeState(TLS_Base* apNewState)
{
	LOG_BLOCK(LogLevel::Debug, "State Change: " << mpState->Name() << " -> " << apNewState->Name());
	mpState = apNewState;
}

void TransportLayer::TransmitAPDU(const openpal::ReadOnlyBuffer& arBuffer)
{
	mTransmitter.Send(arBuffer);
}

void TransportLayer::TransmitTPDU(const openpal::ReadOnlyBuffer& arBuffer)
{
	if(mpLowerLayer != nullptr) mpLowerLayer->Send(arBuffer);
}

void TransportLayer::ReceiveTPDU(const openpal::ReadOnlyBuffer& arBuffer)
{
	mReceiver.HandleReceive(arBuffer);
}

void TransportLayer::ReceiveAPDU(const openpal::ReadOnlyBuffer& arBuffer)
{
	if(mpUpperLayer != nullptr) mpUpperLayer->OnReceive(arBuffer);
}

bool TransportLayer::ContinueSend()
{
	return !mTransmitter.SendSuccess();
}

void TransportLayer::SignalSendSuccess()
{
	if(mpUpperLayer != nullptr) mpUpperLayer->OnSendSuccess();
}

void TransportLayer::SignalSendFailure()
{
	if(mpUpperLayer != nullptr) mpUpperLayer->OnSendFailure();
}

///////////////////////////////////////
// ILayerDown NVII implementations
///////////////////////////////////////
void TransportLayer::_Send(const ReadOnlyBuffer& arBuffer)
{
	if(arBuffer.IsEmpty() || arBuffer.Size() > M_FRAG_SIZE) {
		MACRO_THROW_EXCEPTION_COMPLEX(ArgumentException, "Illegal arg: " << arBuffer.Size() << ", Array length must be in the range [1," << M_FRAG_SIZE << "]");
	}

	mpState->Send(arBuffer, this);
}

///////////////////////////////////////
// ILayerUp NVII implementations
///////////////////////////////////////
void TransportLayer::_OnReceive(const ReadOnlyBuffer& arBuffer)
{
	mpState->HandleReceive(arBuffer, this);
}

void TransportLayer::_OnSendSuccess()
{
	mpState->HandleSendSuccess(this);
}

void TransportLayer::_OnSendFailure()
{
	mpState->HandleSendFailure(this);
}

void TransportLayer::_OnLowerLayerUp()
{
	mpState->LowerLayerUp(this);
}

void TransportLayer::_OnLowerLayerDown()
{
	mpState->LowerLayerDown(this);
}

///////////////////////////////////////
// Helpers
///////////////////////////////////////

#ifndef OPENDNP3_STRIP_LOG_MESSAGES
std::string TransportLayer::ToString(uint8_t aHeader)
{
	std::ostringstream oss;
	oss << "TL: ";
	if((aHeader & TL_HDR_FIR) != 0) oss << "FIR ";
	if((aHeader & TL_HDR_FIN) != 0) oss << "FIN ";
	oss << "#" << static_cast<int>(aHeader & TL_HDR_SEQ);
	return oss.str();
}
#endif

} //end namespace

