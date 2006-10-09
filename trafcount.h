#ifndef _TRAFCOUNT_H
#define _TRAFCOUNT_H 1

#include <grace/str.h>
#include <grace/value.h>
#include <grace/thread.h>
#include <grace/lock.h>
#include <grace/timestamp.h>
#include <grace/system.h>
#include <grace/dictionary.h>
#include <grace/file.h>

#include <stdlib.h>

#define _DEBUG 1

/// User's daily traffic
class userdtraf
{
	public:
							userdtraf (void)
							{
								_count = 0;
							}
							userdtraf (const char *c)
							{
								setcount (c);
							}
							
						   ~userdtraf (void) {}
	
		void				setcount (const char *size)
							{
								_count = atoll (size);
							}	
		
	
		void				setcount (long long set)
							{
								_count = set;
							}	
	
		void				addcount (long long add)
							{
								_count += add;
							}
		
		long long  			getcount (void)
							{
								return _count;	
							}
							
		userdtraf		   &operator= (const userdtraf &v)
							{
								_count = v._count;
								return *this;
							}
							
		userdtraf		   &operator+= (long long add)
							{
								_count += add;
								return *this;
							}
							
		string			   *sval (void)
							{
								returnclass (string) ret retain;	
								ret.printf ("%U", _count);
								return &ret;
							}
							
		void				setid (const string &v)
							{
								_id = v;
							}
		const string	   &getid (void)
							{
								return _id;
							}
							
	private:
		long long 			_count;
		string				_id;
	
};


/// Trafic logger thread
class ttrafcount : public thread
{

	public:
	
						 /// Constructor
						 ttrafcount (const string &dbfile,
						 			int sinterval) 
									: thread ()
						 {
						 	_dbfile 	= dbfile;
						 	_sinterval 	= sinterval;
						 	
						 	// Get timestamp
						 	_lastupdate = kernel.time.now ();
						 	
						 	// Load existing stats
							loadstats ();
							
							#ifdef _DEBUG
							string debugfile;
							debugfile.printf ("%s/debug.out",
											  dbfile.str());
							_df.openappend (debugfile);
							#endif
						 }
						 
						 /// Destructor
						~ttrafcount (void)
						 {
						 }
				 
		virtual	void	 run (void);
		void	 		 shutdown (void);
	
				
		
						 /// Adds a bytecount to a specific vhost
		void			 addbcount (const string &vhost, int count);

	private:

		conditional	 	 		shutdownCondition;
		conditional	 	 		startupCondition;				   

		string			 		_dbfile;
		int				 		_sinterval;	 // Interval in seconds
		dictionary <userdtraf> 	_trafdata;	 // Only todays data
		lock <bool>		 		_lck;
		
		value					_wdata;		 // Complete db image, this will
											 // be upd. with collected trafd.
											
		timestamp 				_lastupdate; //when data has been last updated
		
		#ifdef _DEBUG
		file					_df; 		 // Debug file
		#endif
		
		void			 update (void);		 // Update internal data structure
		void			 loadstats (void);
		void			 savestats (void);

};

#endif
