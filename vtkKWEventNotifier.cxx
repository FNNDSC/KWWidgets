/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWEventNotifier.cxx,v $
  Language:  C++
  Date:      $Date: 2000-06-28 15:43:03 $
  Version:   $Revision: 1.3 $

Copyright (c) 1998-1999 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "vtkKWEventNotifier.h"
#include "vtkObjectFactory.h"
#include <ctype.h>


vtkKWEventNotifier* vtkKWEventNotifier::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkKWEventNotifier");
  if(ret)
    {
    return (vtkKWEventNotifier*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkKWEventNotifier;
}


int vtkKWEventNotifierCommand(ClientData cd, Tcl_Interp *interp,
                       int argc, char *argv[]);

vtkKWEventNotifier::vtkKWEventNotifier()
{
  this->Callbacks = new vtkKWCallbackSpecification *[26];
  for ( int i=0; i < 26; i++ )
    {
    this->Callbacks[i] = NULL;
    }
  this->CommandFunction = vtkKWEventNotifierCommand;
}

vtkKWEventNotifier::~vtkKWEventNotifier()
{
  if ( this->Callbacks )
    {
    for ( int i; i < 26; i++ )
      {
      if ( this->Callbacks[i] )
	{
	this->Callbacks[i]->UnRegister(this);
	this->Callbacks[i] = NULL;
	}
      }
    delete [] this->Callbacks;
    }
}

void vtkKWEventNotifier::AddCallback( const char *event, 
				      vtkKWWindow *window,
				      vtkKWObject *object, 
				      const char *command )
{
  int                         index;
  vtkKWCallbackSpecification  *tmp1, *tmp2;

  // Find out where to put it
  index  = this->ComputeIndex(event);

  // Create a callback specification object to store it in
  tmp1   = (vtkKWCallbackSpecification *)vtkKWCallbackSpecification::New();
  tmp1->SetEventString  ( event   );
  tmp1->SetCalledObject ( object  );
  tmp1->SetWindow( window );
  tmp1->SetCommandString( command );

  // If we already have another callback in this bucket
  if ( this->Callbacks[index] )
    {
    // Find the end of the list and put this callback there
    tmp2 = this->Callbacks[index];
    while (tmp2->GetNextCallback())
      {
      tmp2 = tmp2->GetNextCallback();
      }    
    tmp2->SetNextCallback(tmp1);
    }
  // Otherwise, this is the first callback. 
  else
    {
    this->Callbacks[index] = tmp1;
    // We must register it because we are going to delete it below
    this->Callbacks[index]->Register( this );
    }

  tmp1->Delete();
}

void vtkKWEventNotifier::RemoveCallback( const char *event, 
					 vtkKWWindow *window,
					 vtkKWObject *object, 
					 const char *command )
{
  int                         index;
  vtkKWCallbackSpecification  *tmp1, *tmp2, *tmp3;

  index  = this->ComputeIndex(event);

  // Are there any callbacks in the right list? If not, this must not
  // be a callback that we have - just ignore it. If so, then find it in
  // that list, and remove it.
  if ( this->Callbacks[index] )
    {
      // Is it the first callback? Treat this case specially
      if ( !strcmp( this->Callbacks[index]->GetEventString(), event ) &&
	   !strcmp( this->Callbacks[index]->GetCommandString(), command ) &&
	   this->Callbacks[index]->GetWindow() == window &&
	   this->Callbacks[index]->GetCalledObject() == object )
	{
	// Get the next callback
	tmp1 = this->Callbacks[index]->GetNextCallback();

	// Is the next one non-null? If so, we need to register it so
	// that it is not deleted when unregister is called on 
	// this->Callbacks[index].
	if ( tmp1 )
	  {
	  tmp1->Register( this );
	  this->Callbacks[index]->UnRegister(this);
	  this->Callbacks[index] = tmp1;
	  }
	// If the next one is null, then this whole list becomes null after
	// we delete its only entry
	else
	  {
	  this->Callbacks[index]->UnRegister(this);
	  this->Callbacks[index] = NULL;
	  }
	}
      // Otherwise, it is not the first callback and we have to keep
      // looking for it.
      else
	{
	tmp1 = this->Callbacks[index];
	tmp2 = this->Callbacks[index]->GetNextCallback();
	while ( tmp2 &&
		( strcmp( tmp2->GetEventString(), event ) ||
		  strcmp( tmp2->GetCommandString(), command ) ||
		  tmp2->GetWindow() != window ||
		  tmp2->GetCalledObject() != object ) )
	  {
	  tmp1 = tmp2;
	  tmp2 = tmp2->GetNextCallback();
	  }

	// If tmp2 is null, then we can't find the callback to remove,
	// so just silently ignore it. If tmp2 is non null, then this
	// is the callback we have to remove, and tmp1 is the one right
	// before it in the list.
	if ( tmp2 )
	  {
	  // If there is another callback after the one we are deleting,
	  // reconnect the list
	  if ( tmp2->GetNextCallback() )
	    {
	    // Hang on to it so it isn't deleted when the event before it
	    // is unregistered. Then when we set it as the next event it
	    // is registered so we can unregister it again.
	    tmp3 = tmp2->GetNextCallback();
	    tmp3->Register(this);
	    tmp1->SetNextCallback( tmp2->GetNextCallback() );
	    tmp3->UnRegister(this);
	    }
	  // Otherwise we are removing the last callback in the list.
	  else
	    {
	    tmp1->SetNextCallback(NULL);
	    }
	  }
	}
    }
}


void vtkKWEventNotifier::InvokeCallbacks( const char *event,
					  vtkKWWindow *window,
					  const char *args )
{
  this->InvokeCallbacks( NULL, event, window, args );
}

void vtkKWEventNotifier::InvokeCallbacks( vtkKWObject *object,
                                          const char *event,
					  vtkKWWindow *window,
					  const char *args )
{
  int                         index;
  vtkKWCallbackSpecification  *tmp;

  // Find out where to look for callbacks matching the event
  index  = this->ComputeIndex(event);

  // If there are some callbacks in this bucket, invoke any that match
  if ( this->Callbacks[index] )
    {
    tmp = this->Callbacks[index];
    while (tmp)
      {
      // If we have a match, invoke it. Either a window is NULL, or the
      // window arguments must match as well as the event string. The object
      // argument must not match - this is used to exclude the calling
      // object from invoking its own callbacks.
      if ( !strcmp( tmp->GetEventString(), event ) &&
	   ( window == NULL || tmp->GetWindow() == NULL ||
	     tmp->GetWindow() == window ) &&
           tmp->GetCalledObject() != object )
	{
	this->Script("eval %s %s %s", tmp->GetCalledObject()->GetTclName(),
		     tmp->GetCommandString(), args );
	}
      tmp = tmp->GetNextCallback();
      }    
    }
}

int vtkKWEventNotifier::ComputeIndex( const char *event )
{
  if ( !isalpha(event[0]) )
    {
    return 25;
    }
  else if ( isupper(event[0]) )
    {
    return (int)(event[0] - 'A');
    }
  else
    {
    return (int)(event[0] - 'a');
    }
}

