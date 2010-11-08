// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

#include "tlogrotate.h"

#include <grace/filesystem.h>
#include <grace/system.h>

// #define _DEBUG 1

//	===========================================================================
/// METHOD: tlogrotate::run
//	===========================================================================
void tlogrotate::run (void)
{
	value 	ev;		// Thread event

	// We are running now
	startupCondition.broadcast();
	
	// keep thread running
	while (true)
	{
		// catch thread event
		ev = waitevent(_sinterval*1000);
		
		// Rotate file that are in need of some rotation
		#ifdef _DEBUG
		::printf ("Start rotate\n");
		#endif

		rotate ();

		if (ev.exists ("tlogrotate") &&
				(ev["tlogrotate"] == "die"))
		{
			// exit: while (true)			
			break;
		}
	}
	
	// Thread has finished running,
	// now we die gently
	shutdownCondition.broadcast();
}


//	===========================================================================
/// METHOD: tlogrotate::shutdown
//	===========================================================================
void tlogrotate::shutdown (void)
{
	value death;
	death["tlogrotate"] = "die";
	sendevent (death);
	
	// Wait until the thread has 
	// finished it's current job
	shutdownCondition.wait();
}


//	===========================================================================
/// METHOD: tlogrotate::flistoversized
//	===========================================================================
value *tlogrotate::flistoversized (void)
{
	returnclass (value) ret retain;
	value				result;

	result = fs.ls (_logdir);
		
	foreach (v, result)
	{
		// Is this a normal file
		if (v["type"] == 1)
		{
			// If ile limit exceeded and the extension is: .log
			if ((_maxfilesize <= atoll (v["size"].cval()))
				&& (v.id().sval().globcmp ("*.log")))
			{
				ret[v.name()] = v;
				ret[v.name()]["vhost"] = getvhostfromfile (v.name());
				
				#ifdef _DEBUG
				::printf ("Oversized file: %s\n", v.name());
				#endif				
			}
		}
	}
	
	return &ret;
}


//	===========================================================================
/// METHOD: tlogrotate::exechooks
//	===========================================================================
void tlogrotate::exechooks (void)
{
	value files;
	
	files = fs.ls (_hookdir);
	
	foreach (v, files)
	{
		if ((v["mode"] == "700" || v["mode"] == "500") && v["type"] == 1);
			kernel.sh (v["path"]);
	}

}


//	===========================================================================
/// METHOD: tlogrotate::rotate
//	===========================================================================
void tlogrotate::rotate (void)
{
	value candidates;

	// First of all we have to get a list of 
	// candidate files to rotate
	candidates = flistoversized ();

	// Get a lock on all candidates, the log thread will
	// use temporary log files named: <vhostname>.log.new
	lockrfiles (candidates);
	
	// Since we have set all locks we execute all pre-rotate hooks
	if (_hookdir != "")
		exechooks ();
	
	// Now we can rotate the files, this requires a global lock
	// we do this after the hooks cause we don't want locking 
	// caused by external features.
	_globlck.lockw ();
	_globlck.o = true;
	_globlck.unlock ();
	
	
	// Rotate every file
	foreach (v, candidates)
		rotatefile (v["vhost"].sval());
	
	
	// Release all locks
	unlockrfiles (candidates);
	
	// Unlock global write lock, Logging will resume normal
	_globlck.lockw ();
	_globlck.o = false;
	_globlck.unlock ();
}


//	===========================================================================
/// METHOD: tlogrotate::lockrfiles
//	===========================================================================
void tlogrotate::lockrfiles (const value &v)
{
	foreach (file, v)
		lockfile (v["vhost"].sval());
}


//	===========================================================================
/// METHOD: tlogrotate::unlockrfiles
//	===========================================================================
void tlogrotate::unlockrfiles (const value &v)
{
	foreach (file, v)
		unlockfile (v["vhost"].sval());
}


//	===========================================================================
/// METHOD: tlogrotate::rotatefile
//	===========================================================================
void tlogrotate::rotatefile (const string &vhost)
{
	value allfiles = fs.ls (_logdir);
	value files;
		
	string 	curlogname, newlogname;
			curlogname.printf ("%s.log", 	 vhost.str ());
			newlogname.printf ("%s.log.new", vhost.str ());
	
	// Get all logfile names that belong to
	// this vhost
	foreach (v, allfiles)
	{
		string vhcmp;
		vhcmp.printf ("*%s.log*", vhost.str());
		if (v.id().sval().globcmp(vhcmp))
		{
			files[v.id()] = v;
		}
	}

	// Sort the files 
	files.sort (labelSort);


	#ifdef _DEBUG
	::printf ("Vhost file count: %d\n", files.count ());
	#endif


	// rotate the files, loop reverse
	for (int i=(files.count()-1); i >= 0; i--)
	{
		value v = files[i];
	
		#ifdef _DEBUG
		::printf ("Validating: %s\n", v.name ());
		#endif
	
	
		if ((v.id () != curlogname)
			&& (v.id () != newlogname))
		{
			string nr = v["path"]; 
			string newname = nr.cutatlast ('.');
			
			if (nr.toint() >= 5)
			{
				// Remove file
				fs.rm (v["path"]);
			}
			else
			{
				newname.printf (".%d", nr.toint()+1);
			
				// Move the log file
				fs.mv (v["path"], newname);
			
				#ifdef _DEBUG
				::printf (" Move\n");
				#endif
			}

		}
	}
	
	string oldfile, newfile;
	oldfile.printf ("%s/%s", 	_logdir.str(), curlogname.str ());
	newfile.printf ("%s/%s.0",	_logdir.str(), curlogname.str ());
	
	#ifdef _DEBUG
	::printf ("Old file: %s\n", oldfile.str());
	::printf ("New file: %s\n", newfile.str());
	#endif	
	
	fs.mv (oldfile, newfile);
		
	if (files.exists (newlogname))
	{
		oldfile.clear (); 
		newfile.clear ();

		oldfile.printf ("%s/%s", 	_logdir.str(), newlogname.str ());
		newfile.printf ("%s/%s",	_logdir.str(), curlogname.str ());

		#ifdef _DEBUG
		::printf ("Old file: %s\n", oldfile.str());
		::printf ("New file: %s\n", newfile.str());
		#endif
	
		fs.mv (oldfile, newfile);
	}
}

