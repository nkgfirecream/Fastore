// $Id$
#include <functional>
#include <boost/thread.hpp>
#include "Endpoint.h"
#include "WorkerHandler.h"
#include "../FastoreCommunication/Server_types.h"

class Worker
{
	boost::shared_ptr<WorkerHandler>  phandler;
	WorkerState state;
	boost::shared_ptr<WorkerProcessor> pprocessor;
	const EndpointConfig config;
	Endpoint endpoint;
#if USE_WAL
	Wal _wal;
#endif
	boost::shared_ptr<boost::thread>  pthread;

public:
	Worker( const PodID podId, 
			const string& path, 
			int port,
			const boost::shared_ptr<Scheduler> pscheduler );

	void run() { 
		pthread = boost::shared_ptr<boost::thread>( new boost::thread(std::mem_fun(&Endpoint::Run), &endpoint) );
	}

	class InitWith
	{
		const boost::shared_ptr<Scheduler> pscheduler;
		bool fRun;
	public:
		InitWith( const boost::shared_ptr<Scheduler> pscheduler, bool fRun = true )
			: pscheduler(pscheduler), fRun(fRun)
		{}
		Worker operator()( const WorkerState&  state, 
						   const fastore::server::Path& path ) const {
			Worker w( state.podID, 
					  path, 
					  state.port,
					  pscheduler );

			if( fRun ) 
				w.run();
			return w;
		}
	};

private:
};

