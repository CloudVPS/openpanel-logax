// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

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
