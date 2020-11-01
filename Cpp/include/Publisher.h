/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2018-2020 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "OPSTypeDefs.h"
#include "OPSObject.h"
#include "OPSMessage.h"
#include "Topic.h"
#include "SendDataHandler.h"
#include "MemoryMap.h"
#include "Participant.h"
#include "OPSExport.h"
#include "DebugHandler.h"
#include "Listener.h"
#include "ConnectStatus.h"
#include "Lockable.h"

namespace ops
{
class OPS_EXPORT Publisher : protected Listener<ConnectStatus>, public Notifier<ConnectStatus>
#ifdef OPS_ENABLE_DEBUG_HANDLER
	, DebugNotifyInterface
#endif
{
public:
	Publisher(Topic t);
    virtual ~Publisher();

	void start();
	void stop();

    Topic getTopic() const;

    void setName(ObjectName_T name);
    void setKey(ObjectKey_T key) noexcept;
    ObjectKey_T getKey() const noexcept;
	ObjectName_T getName() const noexcept;

	bool writeOPSObject(OPSObject* obj);

    // Ack specifics
    void AddExpectedAckSender(const char* subkey);
    bool CheckAckSender(const char* subkey);    // nullptr --> check all
    void RemoveExpectedAckSender(const char* subkey);
    void Activate();

protected:
    int64_t currentPublicationID{ 0 };
    std::shared_ptr<SendDataHandler> sendDataHandler{ nullptr };

    virtual bool write(OPSObject* data);

    bool writeSerializedBuffer();

	// Called from SendDataHandler (TCPServer)
	virtual void onNewEvent(Notifier<ConnectStatus>* sender, ConnectStatus arg) override
	{
		UNUSED(sender)
		// Forward status to all connected
		notifyNewEvent(arg);
	}

private:
    Topic topic;
	MemoryMap memMap;
    ByteBuffer buf;

	OPSMessage message;
 
    Participant* participant{ nullptr };

	ObjectName_T name;
    ObjectKey_T key;

    bool started{ false };

#ifdef OPS_ENABLE_DEBUG_HANDLER
    volatile int64_t _dbgSkip{ 0 };
	Lockable _dbgLock;
	std::vector<OPSObject*> _replace;
	virtual void onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp) override;
	bool internalWrite(OPSObject* data);
#endif

    // Ack specifics
    struct AckSubscriber;
    AckSubscriber* _ackSub{ nullptr };
    Lockable _pubLock;
    int64_t _ackTimeout{ 0 };
    const int64_t _ackTimeoutInc;
    int _resendsLeft{ 0 };
    bool resendLatest();

public:
	//Send behavior parameters
    int64_t sendSleepTime{ 1 };
    int sleepEverySendPacket{ 100000 };
};

}
