/*=========================================================================

  Module:    $RCSfile: vtkXMLKWUserInterfaceNotebookManagerReader.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkXMLKWUserInterfaceNotebookManagerReader - vtkKWUserInterfaceNotebookManager XML Reader.
// .SECTION Description
// vtkXMLKWUserInterfaceNotebookManagerReader provides XML reading functionality to 
// vtkKWUserInterfaceNotebookManager.
// .SECTION See Also
// vtkXMLKWUserInterfaceNotebookManagerWriter

#ifndef __vtkXMLKWUserInterfaceNotebookManagerReader_h
#define __vtkXMLKWUserInterfaceNotebookManagerReader_h

#include "vtkXMLKWUserInterfaceManagerReader.h"

class VTK_EXPORT vtkXMLKWUserInterfaceNotebookManagerReader : public vtkXMLKWUserInterfaceManagerReader
{
public:
  static vtkXMLKWUserInterfaceNotebookManagerReader* New();
  vtkTypeRevisionMacro(vtkXMLKWUserInterfaceNotebookManagerReader, vtkXMLKWUserInterfaceManagerReader);

  // Description:
  // Parse an XML tree.
  // Return 1 on success, 0 on error.
  virtual int Parse(vtkXMLDataElement*);

  // Description:
  // Return the name of the root element of the XML tree this reader
  // is supposed to read and process.
  virtual char* GetRootElementName();

protected:  
  vtkXMLKWUserInterfaceNotebookManagerReader() {};
  ~vtkXMLKWUserInterfaceNotebookManagerReader() {};

private:
  vtkXMLKWUserInterfaceNotebookManagerReader(const vtkXMLKWUserInterfaceNotebookManagerReader&); // Not implemented
  void operator=(const vtkXMLKWUserInterfaceNotebookManagerReader&); // Not implemented    
};

#endif

