#include "trafcount.h"

#include <grace/filesystem.h>

//	==================================================================
/// Sends die event to thread
/// and wait's until the thread has finished
//	==================================================================
void ttrafcount::shutdown (void)
{
       value death;
       death["trafcount"] = "die";
       sendevent (death);
	   
	   // Wait until the thread has 
	   // finished it's current job
       shutdownCondition.wait();
}


//	==================================================================
/// Thread's implementation
/// This thread does... run
//	==================================================================
void ttrafcount::run (void)
{
	value 	ev;		// Thread event

	// We are running now
	startupCondition.broadcast();
	
	// keep thread running
	while (true)
	{
		// catch thread event
		ev = waitevent(_sinterval*1000);

		#ifdef _DEBUG
		_df.printf ("Running update\n");
		#endif
		
		update ();


		if (ev.exists ("trafcount") &&
				(ev["trafcount"] == "die"))
		{
			// exit: while (true)
			#ifdef _DEBUG
			_df.printf ("Cathed event: die\n");
			#endif
			
			break;
		}
	}
	
	// Thread has finished running,
	// now we die gently
	shutdownCondition.broadcast();
}


//	==================================================================
///	METHOD: trafcount::addbcount
//	==================================================================
void ttrafcount::addbcount (const string &vhost, int count)
{
	_lck.lockw ();
	if (! _trafdata.exists (vhost))
	{
		#ifdef _DEBUG
		_df.printf ("Setting host id: %s\n", vhost.str());
		#endif
		
		_trafdata[vhost].setid (vhost);
	}
	#ifdef _DEBUG
	_df.printf ("Traffic data item count: %d\n", _trafdata.count());
	#endif

	_trafdata[vhost] += (long long) count;

	#ifdef _DEBUG
	_df.printf ("Traffic data item count: %d\n", _trafdata.count());
	string Acount = _trafdata[vhost].sval();
	_df.printf ("Current trafic bytecount: %s\n", Acount.str());
	#endif



	_lck.unlock ();
}


//	==================================================================
/// METHOD: trafcount::loadstats
//	==================================================================
void ttrafcount::loadstats (void)
{	

	string timets;
	string fname;

	timestamp 	now (kernel.time.now());	
	timets = 	now.format ("%G%m");
	fname.printf ("%s/%s.bin.xml", _dbfile.str(), timets.str());


	try
	{
		_lck.lockw ();
	
		string 		dayidx;
		dayidx = 	now.format ("%G%m%d");
	
		// Load data if the db file exists
		if (fs.exists (fname))
		{
			_wdata.loadxml (fname);
		
			// Load existing data in our dictionary
			if (_wdata.exists (dayidx))
				foreach (v, _wdata[dayidx])
				{
					_trafdata[v.name()] = userdtraf (v.cval());
					_trafdata[v.name()].setid (v.name());
				}
		}
		else
			savestats();
			
		_lck.unlock ();
	}
	catch (...)
	{
		// prob. no db file, could not create ...
		// do nothing
		// TODO: clean this up
	}
}


//	==================================================================
///	METHOD: trafcount::update
//	==================================================================
void ttrafcount::update (void)
{
	// Check if we have to reset the counters before we proceed
	// we reset the counters daily with a possible lack of the 
	// given interval
	
	_lck.lockw ();
	
	timestamp ts (kernel.time.now());
	string    now, last;
	
	now  = ts.format ("%G%m%d");
	last = _lastupdate.format ("%G%m%d");
	
	#ifdef _DEBUG
	_df.printf ("Now: %s - Last: %s\n", now.str(), last.str());
	#endif
	
	// Update timestamp
	_lastupdate = kernel.time.now ();
	
	foreach (v, _trafdata)
	{
		#ifdef _DEBUG
		_df.printf ("ID: %s\n", v.getid().str());
		#endif _DEBUG
		
		_wdata[last][v.getid()] = v.sval();
	}
	
	// It's another day then our current count, 
	// reset counters to zero.
	if (now != last)
	{
		#ifdef _DEBUG
		_df.printf ("Clearing trafdata\n");
		#endif _DEBUG
	
		_trafdata.clear ();
	}
	
	#ifdef _DEBUG
	_df.printf ("Saving trafic stats\n");
	#endif _DEBUG

	savestats ();
	
	_lck.unlock ();	
}


//	==================================================================
///	METHOD: trafcount::savestats
//	==================================================================
void ttrafcount::savestats (void)
{
	string timets;
	string fname;
	
	timets = _lastupdate.format ("%G%m");
	fname.printf ("%s/%s.bandwidth.log", _dbfile.str(), timets.str());

	_wdata.savexml (fname);
}
