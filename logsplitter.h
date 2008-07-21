#ifndef _logsplitter_H
#define _logsplitter_H 1

#include <grace/application.h>

#include "logqueue.h"

//  -------------------------------------------------------------------------
/// Main application class.
//  -------------------------------------------------------------------------
class logsplitterApp : public application
{
	public:
				 	 logsplitterApp (void) 
						: application ("logsplitter")
					 {
					 	opt = $("-h", $("long","--help")) ->
					 		  $("-b", $("long","--burst_write_interval")) ->
					 		  $("-r", $("long","--rotate_enable")) ->
					 		  $("-i", $("long","--rotate_interval")) ->
					 		  $("-m", $("long","--max_logfile_size")) ->
					 		  $("-o", $("long","--hookdir")) ->
					 		  $("-s", $("long","--statdir")) ->
					 		  $("--help", $("argc", 0)) ->
					 		  $("--burst_write_interval", $("argc",1)) ->
					 		  $("--rotate_enable", $("argc",0)) ->
					 		  $("--rotate_interval", $("argc",1)) ->
					 		  $("--max_logfile_size", $("argc",1)) ->
					 		  $("--hookdir", $("argc",1)) ->
					 		  $("--statdir", $("argc",1));
					 }
					~logsplitterApp (void)
					 {
					 }
	
		int		 	 main (void);
	
		 			 // trans a log line in a value object
		value		*transll (const string &);
	
	private:
		void		 shutdown ();
		bool		 init ();
			
		// Log queue
		logqueue	 _logqueue;
		
		// sLog thread
		class
		slogthread	*_slogt;
		
		class
		ttrafcount	*_tlogt;
		
		// Rotate thread
		class
		tlogrotate	*_trott;
		
		
		string		_log_statdir;
		string		_log_outdir;
		string		_rot_prehookdir;
		int			_log_statinterval;
		int			_rot_statinterval;
		long long 	_rot_logmaxfilesize;
		bool		_acceptstats;
		value runopt;
};

#endif
