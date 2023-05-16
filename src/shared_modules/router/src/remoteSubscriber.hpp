/*
 * Wazuh router
 * Copyright (C) 2015, Wazuh Inc.
 * March 25, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _REMOTE_SUBSCRIBER_HPP
#define _REMOTE_SUBSCRIBER_HPP

#include "epollWrapper.hpp"
#include "nlohmann/json.hpp"
#include "observer.hpp"
#include "remoteStateHelper.hpp"
#include "routerProvider.hpp"
#include "socketClient.hpp"
#include <functional>

class RemoteSubscriber final
{
private:
    std::shared_ptr<SocketClient<Socket<OSPrimitives>, EpollWrapper>> m_socketClient {};
    std::string m_endpointName {};
    std::string m_subscriberId {};
    bool m_isRegistered;

public:
    explicit RemoteSubscriber(std::string endpoint,
                              std::string subscriberId,
                              const std::function<void(const std::vector<char>&)>& callback,
                              const std::string& socketPath)
        : m_endpointName {std::move(endpoint)}
        , m_subscriberId {std::move(subscriberId)}
        , m_isRegistered {false}
    {
        std::promise<void> promise;
        m_socketClient =
            std::make_shared<SocketClient<Socket<OSPrimitives>, EpollWrapper>>(socketPath + m_endpointName);
        m_socketClient->connect(
            [&, callback](const char* body, uint32_t bodySize, const char*, uint32_t)
            {
                static const std::string OK {"OK"};
                if (!m_isRegistered)
                {
                    if (bodySize != OK.size() || std::string(body, body + bodySize).compare(OK) != 0)
                    {
                        promise.set_exception(std::make_exception_ptr(std::runtime_error("Connection refused")));
                    }
                    else
                    {
                        m_isRegistered = true;
                        promise.set_value();
                    }
                }
                else
                {
                    callback(std::vector<char>(body, body + bodySize));
                }
            });

        nlohmann::json jsonMessage;
        jsonMessage["type"] = "subscribe";
        jsonMessage["subscriberId"] = m_subscriberId;
        auto jsonMessageString = jsonMessage.dump();

        m_socketClient->send(jsonMessageString.c_str(), jsonMessageString.length());
        auto future {promise.get_future()};

        if (future.wait_for(std::chrono::seconds(10)) == std::future_status::timeout)
        {
            throw std::runtime_error("Connection refused");
        }
    }

    ~RemoteSubscriber()
    {
        nlohmann::json jsonMsg {
            {"EndpointName", m_endpointName}, {"MessageType", "RemoveSubscriber"}, {"SubscriberId", m_subscriberId}};

        RemoteStateHelper::sendRegistrationMessage(jsonMsg);
    }
};

#endif // _REMOTE_SUBSCRIBER_HPP
