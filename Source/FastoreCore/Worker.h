// $Id$
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
//	boost::thread thread;

public:
	Worker( const PodID podId, 
			const string& path, 
			int port,
			const boost::shared_ptr<Scheduler> pscheduler );

	void run() { 
		endpoint.Run(); 
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

