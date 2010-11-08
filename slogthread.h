// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

#ifndef _SLOGTHREAD_H
#define _SLOGTHREAD_H 1

#include <grace/thread.h>
#include <grace/str.h>
#include <grace/value.h>

#include "logqueue.h"

/// Apache service log thread
class slogthread : public thread
{

	public:
						 /// Constructor
						 slogthread (logqueue *lq,
						 			 const string &logdir,
						 			 int iv,
						 			 class tlogrotate *pr) 
						 			 : thread ()
						 {
						 	this->lq 	 = lq;
						 	this->logdir = logdir;
						 	protate = pr;
						 	_sinterval = iv;
						 }
						 
						 /// Destructor
						~slogthread ()
						 {
						 }


		virtual void 	 run (void);			// Thread implementation
		void		 	 shutdown (void);		// Shut's down the thread
	
	
	
	private:

		conditional	 	 shutdownCondition;
		conditional	 	 startupCondition;				   

		logqueue 		*lq;					// external log queue
		value			 writequeue;			// Queue to write to disk
		
		string			 logdir;				// Directory to write logfiles
		
		class 
		tlogrotate		*protate;
		
		int 			 _sinterval;
		
						 /// Write logs to disk
		void			 writelogs (const value &v);
};

#endif 
