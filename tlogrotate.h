#ifndef _TLOGROTATE_H
#define _TLOGROTATE_H

#include <grace/str.h>
#include <grace/value.h>
#include <grace/thread.h>
#include <grace/dictionary.h>


namespace tlog
{
	typedef struct
	{
		bool	locked;
		string	owner;
	} filelock;
};

/// Logsplitter logrotate thread
/// This is directory watcher and rotates 
/// all of the files in the given directory 
/// when they exceed a given length.
/// Rotates will be done by given intervals
class tlogrotate : public thread
{

	public:
						 /// Constructor
						 tlogrotate (const string &logdir, int sinterval,
						 			 long long maxfilesize, 
						 			 const string &hookdir)
						 			 : thread ()
						 {
							_logdir			= logdir;
							_sinterval 		= sinterval;
							_maxfilesize	= maxfilesize;
							_hookdir		= hookdir;
						 }
						 /// Destructor
						~tlogrotate (void)
						 {
						 }
						 
		virtual void	 run (void);		///< Thread implementation
		void			 shutdown (void);	///< Shutdown thread
	
		
						 /// Lock file for writing
		inline void		 lockfile 	(const string &vhost, 
									 const string &owner="trotate")
						 {
						 	_filelock[vhost].lockw ();
						 	_filelock[vhost].o.locked = true;
						 	_filelock[vhost].o.owner  = owner;
						 	_filelock[vhost].unlock ();
						 }
						 
						 /// Unlock file for writing
		inline void		 unlockfile	(const string &vhost, 
									 const string &owner="trorate")
						 {
						 	_filelock[vhost].lockw ();
						 	_filelock[vhost].o.locked = false;
						 	_filelock[vhost].o.owner  = owner;
						 	_filelock[vhost].unlock ();
						 }
						 
						 /// Check if a lock has already been created
		inline bool		 lockexists (const string &vhost)
						 {
						 	return _filelock.exists (vhost);
						 }
						 
		string			*getlockowner (const string &vhost)
						 {
						 	returnclass (string) ret retain;
						 	_filelock[vhost].lockw ();
						 	ret = _filelock[vhost].o.owner;
						 	_filelock[vhost].unlock ();
						 	return &ret;
						 }
		
				
						 // If returns true you may not open
						 // .new files, skip and take them the
						 // next round, files are getting rotated 
						 // by now
		bool			 testgloballock (void)
						 {
						 	bool isglob;

							_globlck.lockw ();
							isglob = _globlck.o;
							_globlck.unlock ();

							return isglob;
						 }
		
	private:
		value			*flistoversized (void);
		void			 exechooks (void);
		void			 rotate (void);
		
						 /// Lock files for rotation
		void			 lockrfiles (const value &v);
		
						 /// Release our own file locks
		void			 unlockrfiles (const value &v);
		
						 /// Rotate log file
		void			 rotatefile (const string &vhost);

						 
						 /// Extract the vhost name from an given log
						 /// file name
		inline string	*getvhostfromfile (const string &filename)
						 {
						 	returnclass (string) ret retain;
						 	ret = filename.copyuntil (".log");
						 	return &ret;
						 }
						 

						 /// Start, Shutdown conditionals	
		conditional	 	 shutdownCondition;
		conditional	 	 startupCondition;
		
						 /// Lock filesystem to prevent writing in 
						 /// unwanted file, If locked we should
		dictionary<lock <tlog::filelock> >		_filelock;
		lock <bool>		 _globlck;
		
		string			_logdir;
		string			_hookdir;
		int				_sinterval;
		long long		_maxfilesize;
};

#endif
