/*=========================================================================

  Module:    $RCSfile: vtkKWWidgets.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __vtkKWWidgets_h
#define __vtkKWWidgets_h
#include "vtkKWWidgetsConfigure.h"

#if defined(WIN32) && !defined(KWSTATIC)
# if defined(vtkKWWidgetsTCL_EXPORTS)
#  define KW_WIDGETS_EXPORT __declspec(dllexport)
# else
#  define KW_WIDGETS_EXPORT __declspec(dllimport)
# endif
#else
# define KW_WIDGETS_EXPORT
#endif

#endif
