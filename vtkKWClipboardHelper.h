/*=========================================================================

  Module:    vtkKWClipboardHelper.h,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWClipboardHelper - A clipboard class
// .SECTION Description
// This class offers some convenient methods to copy text or vtkImageData onto
// the system clipboard, so that they can be pasted into other applications.
// The system clipboard will be cleared before the new contents are added to it.

#ifndef __vtkKWClipboardHelper_h
#define __vtkKWClipboardHelper_h

#include "vtkObject.h"
#include "vtkKWWidgets.h" // Needed for export symbols directives

class vtkImageData;

class KWWidgets_EXPORT vtkKWClipboardHelper : public vtkObject
{
public:
  // Description:
  // Standard New and type methods
  static vtkKWClipboardHelper* New();
  vtkTypeRevisionMacro(vtkKWClipboardHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Copy some texts onto the system clipboard.
  // Return 1 on success; 0 on failure.
  int CopyTextToClipboard(const char* text);

  // Copy a vtkImageData onto the system clipboard. The data
  // will be converted into a bitmap first.
  // Return 1 on success; 0 on failure.
  int CopyImageToClipboard(vtkImageData* imgdata);

protected:
  vtkKWClipboardHelper();
  ~vtkKWClipboardHelper();

private:
  vtkKWClipboardHelper(const vtkKWClipboardHelper&); // Not implemented
  void operator=(const vtkKWClipboardHelper&); // Not implemented
};

#endif
