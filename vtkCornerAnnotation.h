/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkCornerAnnotation.h,v $
  Language:  C++
  Date:      $Date: 2000-06-05 15:15:49 $
  Version:   $Revision: 1.1 $

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
// .NAME vtkKWCornerAnnotation - simple annotation for a data set.
// .SECTION Description
// A property class used for annotation of a data set. Provides simple
// functionality such as a bounding box, scalar bar, and labeled axes.
//
// .SECTION See Also
// vtkActor2D vtkTextMapper

#ifndef __vtkCornerAnnotation_h
#define __vtkCornerAnnotation_h

#include "vtkActor2D.h"
#include "vtkTextMapper.h"

class VTK_EXPORT vtkCornerAnnotation : public vtkActor2D
{
public:
  vtkTypeMacro(vtkCornerAnnotation,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object with a rectangle in normaled view coordinates
  // of (0.2,0.85, 0.8, 0.95).
  static vtkCornerAnnotation *New();
  
  // Description:
  // Draw the scalar bar and annotation text to the screen.
  int RenderOpaqueGeometry(vtkViewport* viewport);
  int RenderTranslucentGeometry(vtkViewport* ) {return 0;};
  int RenderOverlay(vtkViewport* viewport);

  // Description:
  // Set/Get the maximum height of a line of text as a 
  // percentage of the vertical area allocated to this
  // scaled text actor. Defaults to 1.0
  vtkSetMacro(MaximumLineHeight,float);
  vtkGetMacro(MaximumLineHeight,float);
  
  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Set the text to be displayed for each corner
  void SetText(int i,const char *text);
  
protected:
  vtkCornerAnnotation();
  ~vtkCornerAnnotation();
  vtkCornerAnnotation(const vtkCornerAnnotation&) {};
  void operator=(const vtkCornerAnnotation&) {};

  float MaximumLineHeight;
  
  vtkActor2D    *TextActor[4];
  vtkTimeStamp   BuildTime;
  int            LastSize[2];
  vtkTextMapper *TextMapper[4];

private:
};


#endif

