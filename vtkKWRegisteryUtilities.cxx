/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWRegisteryUtilities.cxx,v $
  Language:  C++
  Date:      $Date: 2002-01-10 16:18:28 $
  Version:   $Revision: 1.3 $

Copyright (c) 2000-2001 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of Kitware nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific 
   prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "vtkKWRegisteryUtilities.h"

void vtkKWRegisteryUtilities::ReadAValue(HKEY hKey,char *val,char *key, 
					 char *adefault)
{
  DWORD dwType, dwSize;
  
  dwType = REG_SZ;
  dwSize = 1023;
  if(RegQueryValueEx(hKey,key, NULL, &dwType, 
                     (BYTE *)val, &dwSize) != ERROR_SUCCESS)
    {
    strcpy(val,adefault);
    }
}

unsigned long vtkKWRegisteryUtilities::DeleteKey(HKEY hKey, char *key)
{
  unsigned long res = 0;
  res =  RegDeleteKey( hKey, key );
  cout << "Delete key: " << key << " (" << res << ")" << endl;
  LPVOID lpMsgBuf;
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
    );
// Process any inserts in lpMsgBuf.
// ...
// Display the string.
  cout << (LPCTSTR)lpMsgBuf << endl;
// Free the buffer.
  LocalFree( lpMsgBuf );    
 
  return res;
}

unsigned long vtkKWRegisteryUtilities::DeleteValue(HKEY hKey, char *value)
{
  unsigned long res = 0;
  res =  RegDeleteValue( hKey, value );
  cout << "Delete value: " << value << " (" << res << ")" << endl;
  LPVOID lpMsgBuf;
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
    );
// Process any inserts in lpMsgBuf.
// ...
// Display the string.
  cout << (LPCTSTR)lpMsgBuf << endl;
// Free the buffer.
  LocalFree( lpMsgBuf );    
 
  return res;
}

unsigned long vtkKWRegisteryUtilities::SetValue(HKEY hKey,char *key,char *value)
{
  unsigned long res = 0;
  res = RegSetValueEx(hKey, key, 0, REG_SZ, 
		(CONST BYTE *)(const char *)value, strlen(value)+1);
  return res;
}
