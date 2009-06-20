#include "slogthread.h"

#include "tlogrotate.h"

#include <grace/file.h>

//	==================================================================
/// Sends die event to thread
/// and wait's until the thread has finished
//	==================================================================
void slogthread::shutdown (void)
{
       value death;
       death["logthread"] = "die";
       sendevent (death);
	   
	   // Wait until the thread has 
	   // finished it's current job
       shutdownCondition.wait();
}


//	==================================================================
/// Thread's implementation
/// This thread does... run
//	==================================================================
void slogthread::run (void)
{
	value 	ev;		// Thread event

	// We are running now
	startupCondition.broadcast();
	
	// keep thread running
	while (true)
	{
		// catch thread event
		ev = waitevent(_sinterval*1000);
					
		// Collection of data to write to disk
		value out;	
				
		
		while (lq->count() > 0)
		{		
			value v;
			v = lq->getfirst ();
			
			out[v.type()].newval();
			out[v.type()][-1] = v.sval();
			out[v.type()][-1]("total") = v("size");
		}
		
		
		// Append logs and update 
		// statistics
		if (out.count() > 0)
		{
			writelogs 	(out);
		}


		// if thread event is die we are suposed to
		// cancel out operations
		if (ev.exists ("logthread") &&
				(ev["logthread"] == "die"))
		{
			// exit: while (true)
			break;
		}
	}
	
	// Thread has finished running,
	// now we die gently
	shutdownCondition.broadcast();
}


//	==================================================================
///	METHOD: slogthread::writelogs
//	==================================================================
void slogthread::writelogs (const value &v)
{
	foreach (cf, v)
	{
		file 	f;
		
		// Check if the rotate tread is moving files
		if (! protate->testgloballock ())
		{
			string s = cf.id();
			s = s.filter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQR"
						 "STUVWXYZ0123456789-.");
						 
			// Lock file, we are buzzy with it
			protate->lockfile (s, "slogt");
			string outfile = "%s/%s.log" %format (logdir,s);
			
			// ::printf (outfile.str());
		
			f.openappend (outfile);

			foreach (logline, cf)
				f.writeln (logline.sval());
			
			f.close ();
			
			protate->unlockfile (cf.name (), "slogt");
		}
	}
}
