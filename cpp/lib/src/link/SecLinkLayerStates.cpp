/*
 * Copyright 2013-2020 Automatak, LLC
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Automatak
 * LLC (www.automatak.com) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Automatak LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "SecLinkLayerStates.h"

#include "link/LinkLayer.h"
#include "logging/LogMacros.h"

#include "opendnp3/logging/LogLevels.h"

namespace opendnp3
{

////////////////////////////////////////
// SecStateBase
////////////////////////////////////////

SecStateBase& SecStateBase::OnTxReady(LinkContext& ctx)
{
	if(!ctx.secDeferredActions.empty())
	{
		ctx.secDeferredActions.front()();
		ctx.secDeferredActions.pop_front();
	}
	return *this;
}

////////////////////////////////////////////////////////
//	Class SLLS_NotReset
////////////////////////////////////////////////////////
SLLS_NotReset SLLS_NotReset::instance;

void DoOrDeferQTx(LinkContext& ctx, std::function<void()> action)
{
	//if the link is already in secondary transmit mode, defer the action.
	//if it's in primary mode or idle, the queue-tx action can be executed
	if(ctx.txMode != LinkTransmitMode::Secondary)
		action();
	else
		ctx.secDeferredActions.push_back(action);
}

SecStateBase& SLLS_NotReset::OnTestLinkStatus(LinkContext& ctx, uint16_t /*source*/, bool /*fcb*/)
{
    ++ctx.statistics.numUnexpectedFrame;
    SIMPLE_LOG_BLOCK(ctx.logger, flags::WARN, "TestLinkStatus ignored: secondary not reset");
    return *this;
}

SecStateBase& SLLS_NotReset::OnConfirmedUserData(
    LinkContext& ctx, uint16_t /*source*/, bool /*fcb*/, bool /*isBroadcast*/, const Message& /*message*/)
{
    ++ctx.statistics.numUnexpectedFrame;
    SIMPLE_LOG_BLOCK(ctx.logger, flags::WARN, "ConfirmedUserData ignored: secondary not reset");
    return *this;
}

SecStateBase& SLLS_NotReset::OnResetLinkStates(LinkContext& ctx, uint16_t source)
{
	DoOrDeferQTx(ctx,[&]()
	{
		ctx.QueueAck(source);
	});
	ctx.ResetReadFCB();
	ctx.listener->OnStateChange(LinkStatus::RESET);
	return SLLS_Reset::Instance();
}

SecStateBase& SLLS_NotReset::OnRequestLinkStatus(LinkContext& ctx, uint16_t source)
{
	DoOrDeferQTx(ctx,[&]()
	{
		ctx.QueueLinkStatus(source);
	});
	return *this;
}

////////////////////////////////////////////////////////
//	Class SLLS_Reset
////////////////////////////////////////////////////////
SLLS_Reset SLLS_Reset::instance;

SecStateBase& SLLS_Reset::OnTestLinkStatus(LinkContext& ctx, uint16_t source, bool fcb)
{
	if (ctx.nextReadFCB == fcb)
	{
		DoOrDeferQTx(ctx,[&]()
		{
			ctx.QueueAck(source);
		});
		ctx.ToggleReadFCB();
		return *this;
	}

	//resend the last ACK/NACK response
	if(ctx.testLinkErr.is_not_empty())
	{
		DoOrDeferQTx(ctx,[&,resend{ctx.testLinkErr}]()
		{
			ctx.QueueTransmit(resend,false);
		});
	}
	SIMPLE_LOG_BLOCK(ctx.logger, flags::WARN, "Received TestLinkStatus with invalid FCB");
	return *this;
}

SecStateBase& SLLS_Reset::OnConfirmedUserData(
    LinkContext& ctx, uint16_t source, bool fcb, bool isBroadcast, const Message& message)
{
	if (!isBroadcast)
	{
		DoOrDeferQTx(ctx,[&]()
		{
			ctx.QueueAck(source);
		});
	}

	if (ctx.nextReadFCB == fcb)
	{
		ctx.ToggleReadFCB();
		ctx.PushDataUp(message);
	}
	else
	{
		SIMPLE_LOG_BLOCK(ctx.logger, flags::WARN, "ConfirmedUserData ignored: unexpected frame count bit (FCB)");
	}

	return *this;
}

SecStateBase& SLLS_Reset::OnResetLinkStates(LinkContext& ctx, uint16_t source)
{
	DoOrDeferQTx(ctx,[&]()
	{
		ctx.QueueAck(source);
	});
	ctx.ResetReadFCB();
	return *this;
}

SecStateBase& SLLS_Reset::OnRequestLinkStatus(LinkContext& ctx, uint16_t source)
{
	DoOrDeferQTx(ctx,[&]()
	{
		ctx.QueueLinkStatus(source);
	});
	return *this;
}

} // namespace opendnp3
