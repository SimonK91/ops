#ifndef ops_ReceiveDataHandlerFactory_h
#define ops_ReceiveDataHandlerFactory_h

#include <map>
#include <vector>
#include <string>

#include "OPSTypeDefs.h"
#include "Lockable.h"
#include "IOService.h"

namespace ops
{
    //Forward declaration..
    class ReceiveDataHandler;
	class Participant;
	class Topic;

    class ReceiveDataHandlerFactory
    {
    private:
        ///By Singelton, one ReceiveDataHandler per Topic (name) on this Participant
        std::map<InternalKey_T, ReceiveDataHandler*> receiveDataHandlerInstances;

        ///Garbage vector for ReceiveDataHandlers, these can safely be deleted.
        std::vector<ReceiveDataHandler*> garbageReceiveDataHandlers;
        ops::Lockable garbageLock;

        inline InternalKey_T makeKey(Topic& top, IOService* ioServ);

    public:
        explicit ReceiveDataHandlerFactory(Participant* participant);
        ReceiveDataHandler* getReceiveDataHandler(Topic& top, Participant* participant);
        void cleanUpReceiveDataHandlers();
        void releaseReceiveDataHandler(Topic& top, Participant* participant);
		bool cleanUpDone();
        ~ReceiveDataHandlerFactory();
    };

}
#endif
