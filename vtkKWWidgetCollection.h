/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWWidgetCollection.h,v $
  Language:  C++
  Date:      $Date: 2000-02-17 03:58:11 $
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
// .NAME vtkKWWidgetCollection - a collection of widgets
// .SECTION DescriptionCollection
// vtkKWWidgetCollection represents and provides methods to manipulate a list 
// of widgets. The list is unsorted and duplicate entries are not prevented.

#ifndef __vtkKWWidgetC_h
#define __vtkKWWidgetC_h

#include "vtkCollection.h"
class vtkKWWidget;

class VTK_EXPORT vtkKWWidgetCollection : public vtkCollection
{
public:
  static vtkKWWidgetCollection *New();
  vtkTypeMacro(vtkKWWidgetCollection,vtkCollection);

  // Description:
  // Add an KWWidget to the list.
  void AddItem(vtkKWWidget *a);

  // Description:
  // Remove an KWWidget from the list.
  void RemoveItem(vtkKWWidget *a);

  // Description:
  // Determine whether a particular KWWidget is present. 
  // Returns its position in the list.
  int IsItemPresent(vtkKWWidget *a);

  // Description:
  // Get the next KWWidget in the list.
  vtkKWWidget *GetNextKWWidget();

  // Description:
  // Get the last KWWidget in the list.
  vtkKWWidget *GetLastKWWidget();

protected:
  vtkKWWidgetCollection() {};
  ~vtkKWWidgetCollection() {};
  vtkKWWidgetCollection(const vtkKWWidgetCollection&) {};
  void operator=(const vtkKWWidgetCollection&) {};

};

inline void vtkKWWidgetCollection::AddItem(vtkKWWidget *a) 
{
  this->vtkCollection::AddItem((vtkObject *)a);
}

inline void vtkKWWidgetCollection::RemoveItem(vtkKWWidget *a) 
{
  this->vtkCollection::RemoveItem((vtkObject *)a);
}

inline int vtkKWWidgetCollection::IsItemPresent(vtkKWWidget *a) 
{
  return this->vtkCollection::IsItemPresent((vtkObject *)a);
}

inline vtkKWWidget *vtkKWWidgetCollection::GetNextKWWidget() 
{ 
  return (vtkKWWidget *)(this->GetNextItemAsObject());
}

inline vtkKWWidget *vtkKWWidgetCollection::GetLastKWWidget() 
{ 
  if ( this->Bottom == NULL )
    {
    return NULL;
    }
  else
    {
    return (vtkKWWidget *)(this->Bottom->Item);
    }
}

#endif





