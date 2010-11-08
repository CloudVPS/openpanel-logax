// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

#include "logsplitter.h"
#include "slogthread.h"
#include "trafcount.h"
#include "tlogrotate.h"

#include <grace/filesystem.h>

APPOBJECT(logsplitterApp);

//  =========================================================================
/// Main method.
//  =========================================================================
int logsplitterApp::main (void)
{
	// -b burst write interval
	// --burst_write_interval
	
	// -h hookdir
	// --hookdir
	
	// -i rotate interval
	// --rotate_interval
		
	// -m max logfile size in bytes
	// --max_logfile_size
	
	// -r enable rotate	
	// --rotate_enable
	
	// -s statdir to write daily usage in bytes	
	// --statdir
	
	if (argv.exists ("--help") || argv.count() < 2 || argv["*"].count() > 1)
	{
	  fout.printf (
		 "\nUsage: logsplit [OPTIONS]... LOGDIR \n"
		 "Split apache logs, output to LOGDIR input from stdin\n"
		 "Example: logsplit -r /var/log/ourlogdir < some.log\n\n"
		 "Log output control options:\n"
		 "  -b, --burst_write_interval <SEC>   Interval between the logs are written.\n\n"
		 "Log rotation:\n"
		 "  -r, --rotate_enable                Enable log rotation.\n"
		 "  -i, --rotate_interval      <SEC>   Check every (x) seconds if rotation is\n"
		 "                                     neccesary.\n"
		 "  -m, --max_logfile_size     <SIZE>  If file exceeds (maxsize) rotate the file.\n"
		 "  -o, --hook_dir             <DIR>   Execute the files in this directory before\n"
		 "                                     rotate.\n\n"
		 "Daily bandwidth measurement:\n"
		 "  -s, --statdir <DIR>                Directory to output daily statistics.\n\n"
		 "Option parameters:\n"
		 "  <SEC>          Time in seconds.\n"
		 "  <SIZE>         Size in bytes.\n"
		 "  <DIR>          Directory name.\n\n"
		 
		);
	  return 1;			
	}
	
	if (argv.exists ("--burst_write_interval"))
	{
		_log_statinterval = argv["--burst_write_interval"].ival();
	}
	else
	{
		_log_statinterval = 1; // user does not want to tweak timing
	}
	
	if (argv.exists ("--hookdir"))
	{
		if (! argv.exists ("--rotate_enable"))
		{
			fout.printf ("WARNING: hookdir ignored, rotating not enabled\n");
		}
		else
			_rot_prehookdir = argv["--hookdir"].sval();
	}

	if (argv.exists ("--rotate_interval"))
	{
		if (! argv.exists ("--rotate_enable"))
		{
			fout.printf ("WARNING: rotate_interval ignored, rotating not enabled\n");
		}

		_rot_statinterval = argv["--rotate_interval"].ival();
	}
	else
	{	
		// Check file sizes every minute
		_rot_statinterval = 60;
	}
	
	if (argv.exists ("--max_logfile_size"))
	{
		if (! argv.exists ("--rotate_enable"))
		{
			fout.printf ("WARNING: max_logfile_size ignore, rotating not enabled\n");
		}
		
		_rot_logmaxfilesize = argv["--max_logfile_size"].ival();
	}
	else
	{
		// max file is is default 500K
		_rot_logmaxfilesize = 500000;
	}

	if (argv.exists ("--statdir"))
	{
		_log_statdir = argv["--statdir"].sval();
		_acceptstats = true;
		
		if (! fs.exists (_log_statdir))
		{
			fout.printf ("ERROR: Path: <%s> does not exist\n", 
						 _log_statdir.cval());
			return 1;
		}		
	}
	else
		_acceptstats = false;



	if (! fs.exists (argv["*"][0].sval()))
	{
		fout.printf ("ERROR: Path: <%s> does not exist\n", 
					 argv["*"][0].cval());
		return 1;
	}
	else
		_log_outdir = argv["*"][0].sval(); 


	// Setup application
	if (! init ())
		return 1;



	// Apache logs to our stdin
	while (string in = fin.gets())
	{
		value 	s 	 = transll (in);
		string  alog = in.cutat (" "); // seperate our custom log part
	
		// add line to log queue
		_logqueue.add (s, in);
		
		if (_acceptstats)
			_tlogt->addbcount (s["domain"].sval(), s["size"].ival());
	}
	

	// Shutdown application
	shutdown ();


	return 0;
}


//  =========================================================================
///	METHOD: logsplitterApp::transll
//  =========================================================================
value *logsplitterApp::transll (const string &s)
{
	returnclass (value) v 	 retain;
	string				tmp (s);

	v["domain"]		= tmp.cutat (" ");
	v["ip"]			= tmp.cutat (" ");
	v["user1"]		= tmp.cutat (" ");
	v["user2"]		= tmp.cutat (" [");
	v["822-date"]	= tmp.cutat ("] ");
	
	value rest = strutil::splitquoted (tmp, ' ');
	string httpreq = rest[0];
	v["cmd"] = httpreq.cutat (" ");
	v["uri"] = httpreq.cutat (" ");
	v["protocol"] = httpreq;
	
	v["result"] = rest[1].ival();
	v["size"] = rest[2].ival();
	return &v;
}

//  =========================================================================
///	METHOD logsplitterApp::shutdown
//  =========================================================================
void logsplitterApp::shutdown ()
{
	// TODO:
	//   shutdown actions in here
	// - shutdown logthread, empty queue
	// - remove queue
	// ::printf ("WAITING FOR SHUTDOWN...\n");
	
	_slogt->shutdown ();
	_tlogt->shutdown ();
	_trott->shutdown ();
	
	// ::printf ("...OK!\n");
}


//  =========================================================================
///	METHOD logsplitterApp::init
//  =========================================================================
bool logsplitterApp::init ()
{
	// TODO:
	//   All initialisations in here
	// - setup logqueue
	// - setup slogthread
	// - setup slogrotation (future candy)

	try 
	{
		// Load logqueue if there is logable data left from
		// a crash or whatever
		// Todo...

		// Setup thread for custom logrotation
		_trott = new tlogrotate (_log_outdir, _rot_statinterval, 
							  	 _rot_logmaxfilesize, _rot_prehookdir);

		// Only start the logrotating thread if neccesarry
		if (argv.exists ("--rotate_enable"))
			_trott->start ();
		
		// Setup the logthread
		_slogt = new slogthread (&_logqueue, _log_outdir, 
								 _log_statinterval, _trott);
		_slogt->start ();
		
		
		if (_acceptstats)
		{
			// Setup daily traffic log facility
			_tlogt = new ttrafcount (_log_statdir, _log_statinterval);
			_tlogt->start ();
		}
	}
	catch (...)
	{
		// something has gone wrong serious
		fout.printf ("Unidentified exception at init()\n");
		
		return false;
	}
	
	return true;
}

