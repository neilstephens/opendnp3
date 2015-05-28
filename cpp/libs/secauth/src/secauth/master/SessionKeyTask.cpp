/**
* Licensed to Green Energy Corp (www.greenenergycorp.com) under one or
* more contributor license agreements. See the NOTICE file distributed
* with this work for additional information regarding copyright ownership.
* Green Energy Corp licenses this file to you under the Apache License,
* Version 2.0 (the "License"); you may not use this file except in
* compliance with the License.  You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* This project was forked on 01/01/2013 by Automatak, LLC and modifications
* may have been made to this file. Automatak, LLC licenses these modifications
* to you under the terms of the License.
*/

#include "SessionKeyTask.h"

#include <opendnp3/objects/Group120.h>

using namespace opendnp3;
using namespace openpal;

namespace secauth
{

SessionKeyTask::SessionKeyTask(	opendnp3::IMasterApplication& application,
								openpal::TimeDuration retryPeriod_,
								openpal::Logger logger,
								const User& user_,
								MSState& msstate) :

							opendnp3::IMasterTask(application, openpal::MonotonicTimestamp::Min(), logger, nullptr, -1),
							retryPeriod(retryPeriod_),
							user(user_),
							pmsstate(&msstate)
{
	
}

void SessionKeyTask::BuildRequest(opendnp3::APDURequest& request, uint8_t seq)
{
	if (state == State::GetStatus)
	{
		this->BuildStatusRequest(request, seq);
	}
	else
	{
		this->BuildSessionKeyRequest(request, seq);
	}
}

void SessionKeyTask::Initialize()
{
	this->state = State::GetStatus;
}

bool SessionKeyTask::IsEnabled() const
{
	return false;
}

IMasterTask::ResponseResult SessionKeyTask::_OnResponse(const APDUResponseHeader& header, const openpal::ReadBufferView& objects)
{
	if (ValidateSingleResponse(header))
	{
		return (state == State::GetStatus) ? OnStatusResponse(header, objects) : OnChangeResponse(header, objects);
	}
	else
	{
		return ResponseResult::ERROR_BAD_RESPONSE;
	}
}

void SessionKeyTask::OnResponseError(openpal::MonotonicTimestamp now)
{
	expiration = now.Add(this->retryPeriod);
}

void SessionKeyTask::OnResponseOK(openpal::MonotonicTimestamp now)
{
	expiration = MonotonicTimestamp::Max();
}

void SessionKeyTask::_OnResponseTimeout(openpal::MonotonicTimestamp now)
{
	expiration = now.Add(this->retryPeriod);
}

void SessionKeyTask::_OnLowerLayerClose(openpal::MonotonicTimestamp now)
{
	expiration = 0;
}

void SessionKeyTask::BuildStatusRequest(opendnp3::APDURequest& request, uint8_t seq)
{
	request.ConfigureHeader(FunctionCode::AUTH_REQUEST, seq);

	Group120Var4 ksrequest;
	ksrequest.userNum = user.GetId();

	request.GetWriter().WriteSingleValue<UInt8, Group120Var4>(QualifierCode::UINT8_CNT, ksrequest);
}

void SessionKeyTask::BuildSessionKeyRequest(opendnp3::APDURequest& request, uint8_t seq)
{
	
}

IMasterTask::ResponseResult SessionKeyTask::OnStatusResponse(const APDUResponseHeader& response, const ReadBufferView& objects)
{
	return ResponseResult::ERROR_BAD_RESPONSE; // TODO
}

IMasterTask::ResponseResult SessionKeyTask::OnChangeResponse(const APDUResponseHeader& response, const ReadBufferView& objects)
{
	return ResponseResult::ERROR_BAD_RESPONSE; // TODO
}

}

