#ifndef _LOGQUEUE_H
#define _LOGQUEUE_H 1

#include <grace/array.h>
#include <grace/str.h>
#include <grace/value.h>

/// Log queue class
class logqueue
{
	public:
				 /// Constructor
				 logqueue (void) 
				 {
				 }
				 
				 /// Destructor
				~logqueue (void)
				 {
				 }
	
				 /// Add logable item
		void	 add (const value &info, 
					  const string &logline);
					  
				 /// Pulls the first item out of
				 /// our queue and returns a copy
		value	*getfirst (void);
		
				 /// Get queue count
		int		 count (void)
				 {
				 	lck.lockw ();
				 	
				 	int count =  q.count ();
				 	
				 	lck.unlock ();
				 	
				 	return count;
				 }
				 
				 
				 /// RESERVED
		bool	 savequeue (void);
	
	
	private:
		array <value>	q;
		lock  <bool>	lck;

};

#endif
