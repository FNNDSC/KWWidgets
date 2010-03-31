/*=========================================================================

  Module:    $RCSfile: vtkKWFileBrowserUtilities.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWFileBrowserUtilities - some constants
// This work is part of the National Alliance for Medical Image
// Computing (NAMIC), funded by the National Institutes of Health
// through the NIH Roadmap for Medical Research, Grant U54 EB005149.
// Information on the National Centers for Biomedical Computing
// can be obtained from http://nihroadmap.nih.gov/bioinformatics.

#ifndef __vtkKWFileBrowserUtilities_h
#define __vtkKWFileBrowserUtilities_h

#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/string>

#ifdef _WIN32
#define KWFileBrowser_PATH_SEPARATOR "\\"
#else
#define KWFileBrowser_PATH_SEPARATOR "/"
#endif
    
#define KWFileBrowser_UNIX_ROOT_DIRECTORY "/"
#define KWFileBrowser_ESCAPE_CHARS "{}[]$\"\\"
#define VTK_KW_FAVORITE_TOPLEVEL "KWFileBrowserFavorites"

static char* KWFileBrowser_GetUnixPath(const char* path)                                    
{
  if(path && *path)
    {
    vtksys_stl::string sBuffer = path;
    vtksys::SystemTools::ConvertToUnixSlashes(sBuffer);
    static char buffer[512];
    strcpy(buffer, sBuffer.c_str());
    return buffer;
    }
  return NULL;
};

static int KWFileBrowser_HasTrailingSlash(const char *dir)
{ 
  size_t dir_len = strlen(dir);
  int has_slash = 
   (dir_len && (dir[dir_len - 1] == '/' || dir[dir_len - 1] == '\\'));

  return has_slash;
};

static bool KWFileBrowser_ComparePath(const char *dir1, const char* dir2)
{ 
  if(!dir1 || !dir2)
    {
    return false;
    }
  vtksys_stl::string path1 = dir1;
  vtksys_stl::string path2 = dir2;
  int dirslash1 = KWFileBrowser_HasTrailingSlash(dir1);
  int dirslash2 = KWFileBrowser_HasTrailingSlash(dir2);
  if(!dirslash1 && dirslash2)
    {
    path1 += "/";
    }
  else if(dirslash1 && !dirslash2)
    {
    path2 += "/";
    }
  vtksys::SystemTools::ConvertToUnixSlashes(path1);
  vtksys::SystemTools::ConvertToUnixSlashes(path2);
  return vtksys::SystemTools::ComparePath(path1.c_str(), path2.c_str());
};

#endif
