// $Id$
#include <functional>
#include <boost/thread.hpp>
#include "Endpoint.h"
#include "WorkerHandler.h"
#include "../FastoreCommon/Server_types.h"

class Worker
{
public:
	enum status_t { idle, running, stopped } ; 
private:
	Wal _wal;
	boost::shared_ptr<WorkerHandler>  phandler;
	WorkerState state;
	boost::shared_ptr<WorkerProcessor> pprocessor;
	const EndpointConfig config;
	boost::shared_ptr<Endpoint> pendpoint;
	status_t _status;
	boost::shared_ptr<boost::thread>  pthread;

public:
	Worker( const PodID podId, 
			const string& path, 
			uint64_t port,
			const boost::shared_ptr<Scheduler> pscheduler );

	bool run();
	status_t status() const { return _status; }

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

