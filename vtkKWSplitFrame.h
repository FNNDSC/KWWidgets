/*=========================================================================

  Module:    $RCSfile: vtkKWSplitFrame.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWSplitFrame - A Frame that contains two adjustable sub frames.
// .SECTION Description
// The split frame allows the use to select the size of the two frames.
// It uses a spearator that can be dragged interactively.


#ifndef __vtkKWSplitFrame_h
#define __vtkKWSplitFrame_h

#include "vtkKWWidget.h"
class vtkKWApplication;

class VTK_EXPORT vtkKWSplitFrame : public vtkKWWidget
{
public:
  static vtkKWSplitFrame* New();
  vtkTypeRevisionMacro(vtkKWSplitFrame,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a Tk widget
  virtual void Create(vtkKWApplication *app);
  
  // Description:
  // Get the vtkKWWidget for the left internal frame.
  vtkKWWidget *GetFrame1() {return this->Frame1;};

  // Description:
  // Get the vtkKWWidget for the right internal frame.
  vtkKWWidget *GetFrame2() {return this->Frame2;};

  // Description:
  // Set/Get The minimum widths for the two frames.
  // They default to 50 pixels.
  vtkGetMacro(Frame1MinimumSize, int);
  vtkGetMacro(Frame2MinimumSize, int);
  void SetFrame1MinimumSize(int minSize);
  void SetFrame2MinimumSize(int minSize);

  // Description:
  // Set/Get the width of the left frame.  
  // For now, we cannot direclty set the width of the second frame.
  vtkGetMacro(Frame1Size, int);
  vtkGetMacro(Frame2Size, int);
  void SetFrame1Size(int minSize);

  // Description:
  // Set/Get the visibility of the left or right frame.  
  virtual void SetFrame1Visibility(int flag);
  vtkGetMacro(Frame1Visibility, int);
  vtkBooleanMacro(Frame1Visibility, int);  
  virtual void SetFrame2Visibility(int flag);
  vtkGetMacro(Frame2Visibility, int);
  vtkBooleanMacro(Frame2Visibility, int);  

  // Description:
  // This sets the separators narrow dimension. Horizontal=> size = width.  
  // If the size is 0, then the two frames cannot be adjusted by the user.
  void SetSeparatorSize(int size);
  vtkGetMacro(SeparatorSize, int);

  // Callbacks used internally to adjust the widths,
  void DragCallback();
  void ConfigureCallback();

  // Description:
  // Vertical orientation has the first frame on top of the second frame.
  // Horizontal orientation has first frame to left of second.
  // At the moment, this state has to be set before the
  // widget is created.  I should extend it in the future to be more flexible.
  vtkSetMacro(Orientation, int);
  vtkGetMacro(Orientation, int);
  void SetOrientationToHorizontal();
  void SetOrientationToVertical();

//BTX
  enum OrientationStates {
    Horizontal = 0,
    Vertical
  };
//ETX

protected:
  vtkKWSplitFrame();
  ~vtkKWSplitFrame();

  vtkKWWidget *Frame1;
  vtkKWWidget *Separator;
  vtkKWWidget *Frame2;

  int Size;
  int Frame1Size;
  int Frame2Size;
  int SeparatorSize;

  int Frame1Visibility;
  int Frame2Visibility;

  int Frame1MinimumSize;
  int Frame2MinimumSize;

  int Orientation;

  // Reset the actual windows to match our width IVars.
  void Update();
private:
  vtkKWSplitFrame(const vtkKWSplitFrame&); // Not implemented
  void operator=(const vtkKWSplitFrame&); // Not implemented
};


#endif



