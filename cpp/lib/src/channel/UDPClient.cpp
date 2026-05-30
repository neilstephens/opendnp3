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

#include "channel/UDPClient.h"

#include "channel/SocketHelpers.h"

#include <sstream>
#include <utility>

namespace opendnp3
{

UDPClient::UDPClient(const std::shared_ptr<exe4cpp::StrandExecutor>& executor)
    : executor(executor), socket(*executor->get_context()), resolver(*executor->get_context())
{
}

void UDPClient::Cancel()
{
    if (this->canceled)
	  return;

    std::error_code ec;
    socket.cancel(ec);
    socket.close(ec);
    resolver.cancel();
    this->canceled = true;
}

void UDPClient::Open(const IPEndpoint& localEndpoint, const IPEndpoint& remoteEndpoint, connect_callback_t callback)
{
    if (canceled)
	  return;

    std::error_code ec;
    SocketHelpers::BindToLocalAddress<asio::ip::udp>(localEndpoint.address, localEndpoint.port, this->socket, ec);

    if (ec)
    {
	  this->PostConnectError(callback, ec);
	  return;
    }

    // Find remote address
    const auto address = asio::ip::make_address(remoteEndpoint.address, ec);
    if (ec)
    {
	    //try name resolution
	    std::stringstream portstr;
	    portstr << remoteEndpoint.port;
	    resolver.async_resolve(remoteEndpoint.address, portstr.str()
	    ,executor->wrap([self = shared_from_this(),callback](const std::error_code& ec, asio::ip::udp::resolver::results_type endpoints)
	    {
		    if(self->canceled)
			    return;
		    if(ec)
		    {
			    self->PostConnectError(callback, ec);
			    return;
		    }
		    if(endpoints.size() == 0)
		    {
			    asio::error_code ec = asio::error::make_error_code(asio::error::host_not_found);
			    self->PostConnectError(callback, ec);
			    return;
		    }
		    callback(self->executor, std::move(self->socket), *endpoints.begin(), ec);
	    }));
	    return;
    }

    asio::ip::udp::endpoint rem_ep(address, remoteEndpoint.port);
    callback(executor, std::move(socket), rem_ep, ec);
}

void UDPClient::PostConnectError(const connect_callback_t& callback, const std::error_code& ec)
{
    auto cb = [self = shared_from_this(), ec, callback]() {
        if (!self->canceled)
        {
		callback(self->executor, std::move(self->socket), asio::ip::udp::endpoint(), ec);
        }
    };
    executor->post(cb);
}

} // namespace opendnp3
