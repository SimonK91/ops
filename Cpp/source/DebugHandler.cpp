/**
*
* Copyright (C) 2018 Lennart Andersson.
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

#include <map>
#include <mutex>

#include "DataNotifier.h"
#include "Participant.h"
#include "Subscriber.h"
#include "Publisher.h"
#include "BasicError.h"
#include "KeyFilterQoSPolicy.h"

#include "DebugHandler.h"

#ifdef OPS_ENABLE_DEBUG_HANDLER

namespace ops {

	// Key used as filter when listening on the debug topic for DebugRequestResponseData messages
	static ObjectKey_T gKey;

	void DebugHandler::SetKey(const ObjectKey_T& key)
	{
		gKey = key;
	}

	class DebugHandler::InternalDebugListener : public DataListener
	{
	public:
		InternalDebugListener(Participant* part) : _part(part), _sub(nullptr), _pub(nullptr) {}
		virtual ~InternalDebugListener() { remove(); }

		// Register/Unregister with the debug handler
		void RegisterPub(DebugNotifyInterface* client, ObjectName_T topicName)
		{
			std::lock_guard<std::mutex> lck(_mapLock);
			if (_pubMap.find(topicName) == _pubMap.end()) {
				_pubMap[topicName] = client;
			}
		}

		void UnregisterPub(DebugNotifyInterface* client, ObjectName_T topicName)
		{
			std::lock_guard<std::mutex> lck(_mapLock);
			std::map<ObjectName_T, DebugNotifyInterface*>::iterator it = _pubMap.find(topicName);
			if (it == _pubMap.end()) return;
			if (it->second != client) return;
			_pubMap.erase(it);
		}

		void RegisterSub(DebugNotifyInterface* client, ObjectName_T topicName)
		{
			std::lock_guard<std::mutex> lck(_mapLock);
			if (_subMap.find(topicName) == _subMap.end()) {
				_subMap[topicName] = client;
			}
		}

		void UnregisterSub(DebugNotifyInterface* client, ObjectName_T topicName)
		{
			std::lock_guard<std::mutex> lck(_mapLock);
			std::map<ObjectName_T, DebugNotifyInterface*>::iterator it = _subMap.find(topicName);
			if (it == _subMap.end()) return;
			if (it->second != client) return;
			_subMap.erase(it);
		}

		void Start()
		{
			// We require the key to be set to enable the topics
			if (gKey == "") return;

			// Check if already started
			if (_sub) return;

			setup();
		}

		void Stop()
		{
			remove();
		}

		void onNewData(DataNotifier* notifier)
		{
			Subscriber* sub = dynamic_cast<Subscriber*> (notifier);
			if (sub) {
				opsidls::DebugRequestResponseData* req = dynamic_cast<opsidls::DebugRequestResponseData*>(sub->getMessage()->getData());
				if (req) {
					if (req->Command == 0) return;  // We don't care about responses

					{
						std::lock_guard<std::mutex> lck(_mapLock);

						switch (req->Entity) {
						case 0: // Debug
							onRequest(*req, _response);
							break;

						case 1: // Participant
							break;

						case 2: // Publisher
							if (_pubMap.find(req->Name) == _pubMap.end()) break;
							_pubMap[req->Name]->onRequest(*req, _response);
							break;

						case 3: // Subscriber
							if (_subMap.find(req->Name) == _subMap.end()) break;
							_subMap[req->Name]->onRequest(*req, _response);
							break;
						}
					}
					_response.Entity = req->Entity;
					_response.Name = req->Name;
					_response.Command = 0;	// Always a response

					if (_pub) _pub->writeOPSObject(&_response);

					_response.Param3.clear();

				} else {
					BasicError err("DebugHandler", "onNewData", "Data could not be cast as expected.");
					_part->reportError(&err);
				}
			} else {
				BasicError err("DebugHandler", "onNewData", "Subscriber could not be cast as expected.");
				_part->reportError(&err);
			}
		}

	private:
		void setup()
		{
			_sub = new Subscriber(_part->createDebugTopic());
			_sub->addDataListener(this);
			_sub->addFilterQoSPolicy(new KeyFilterQoSPolicy(gKey));
			_sub->start();
			_pub = new Publisher(_part->createDebugTopic());
			_pub->start();
		}

		void remove()
		{
			if (_sub) delete _sub;
			_sub = nullptr;
			if (_pub) delete _pub;
			_pub = nullptr;
		}

		// Called with _mapLock held
		void onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp)
		{
			switch (req.Command) {
			case 2: // List
				resp.Param1 = 0;
				if (req.Param1 == 2) {
					for (std::map<ObjectName_T, DebugNotifyInterface*>::iterator it = _pubMap.begin(); it != _pubMap.end(); ++it) {
						resp.Param3.push_back(it->first.c_str());
					}
					resp.Result1 = 2;
				}
				if (req.Param1 == 3) {
					for (std::map<ObjectName_T, DebugNotifyInterface*>::iterator it = _subMap.begin(); it != _subMap.end(); ++it) {
						resp.Param3.push_back(it->first.c_str());
					}
					resp.Result1 = 3;
				}
				break;
			}
		}

		Participant* _part;
		Subscriber* _sub;
		Publisher* _pub;

		opsidls::DebugRequestResponseData _response;

		std::mutex _mapLock;
		std::map<ObjectName_T, DebugNotifyInterface*> _pubMap;
		std::map<ObjectName_T, DebugNotifyInterface*> _subMap;
	};

	DebugHandler::DebugHandler(Participant* part) : _pimpl(new InternalDebugListener(part))
	{
	}
	
	DebugHandler::~DebugHandler()
	{
		if (_pimpl) delete _pimpl;
	}

	void DebugHandler::Start()
	{
		_pimpl->Start();
	}

	// Register/Unregister with the debug handler
	void DebugHandler::RegisterPub(DebugNotifyInterface* client, ObjectName_T topicName)
	{
		_pimpl->RegisterPub(client, topicName);
	}
	
	void DebugHandler::UnregisterPub(DebugNotifyInterface* client, ObjectName_T topicName)
	{
		_pimpl->UnregisterPub(client, topicName);
	}

	void DebugHandler::RegisterSub(DebugNotifyInterface* client, ObjectName_T topicName)
	{
		_pimpl->RegisterSub(client, topicName);
	}
	
	void DebugHandler::UnregisterSub(DebugNotifyInterface* client, ObjectName_T topicName)
	{
		_pimpl->UnregisterSub(client, topicName);
	}

}
#endif
