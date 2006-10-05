#include "logqueue.h"

//	============================================================================
/// METHOD: logqueue::add
//	============================================================================
void logqueue::add (const value &info, const string &logline)
{
	// Lock 
	lck.lockw ();

	// Add item
	q.add (new value);
	q[-1].type (info["domain"].cval ());
	q[-1] 			= logline;
	q[-1]("size") 	= info["size"].ival ();
	
	// Unlock
	lck.unlock ();
}
			  

//	============================================================================
///	METHOD: logqueue::getfirst
//	============================================================================
value *logqueue::getfirst (void)
{
	returnclass (value) ret retain;
	
	lck.lockw ();
	
	// Else an empty value object 
	// wil be returned
	if (q.count() > 0)
	{
		ret = q[0];
		q.remove (0);	
	}
	
	lck.unlock ();
	
	return &ret;
}
