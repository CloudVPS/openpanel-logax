// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

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
			
			if (cf.id() == s)
			{
				
				// Lock file, we are buzzy with it
				protate->lockfile (s, "slogt");
				string outfile = "%s/%s.log" %format (logdir,s);
				
				// ::printf (outfile.str());
			
				f.openappend (outfile);
	
				foreach (logline, cf)
					f.writeln (logline.sval());
				
				f.close ();
				
				protate->unlockfile (s, "slogt");
			}
		}
	}
}
