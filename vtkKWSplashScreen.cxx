/*=========================================================================

  Module:    $RCSfile: vtkKWSplashScreen.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWSplashScreen.h"

#include "vtkKWApplication.h"
#include "vtkKWCanvas.h"
#include "vtkKWIcon.h"
#include "vtkObjectFactory.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWTkUtilities.h"

#include <vtksys/stl/string>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWSplashScreen );
vtkCxxRevisionMacro(vtkKWSplashScreen, "$Revision: 1.41 $");

//----------------------------------------------------------------------------
vtkKWSplashScreen::vtkKWSplashScreen()
{
  this->Canvas = vtkKWCanvas::New();

  this->ImageName = NULL;
  this->ProgressMessageVerticalOffset = -10;
  this->DisplayPosition = vtkKWTopLevel::DisplayPositionScreenCenter;
  this->HideDecoration  = 1;
  this->Discard = 0;
}

//----------------------------------------------------------------------------
vtkKWSplashScreen::~vtkKWSplashScreen()
{
  if (this->Canvas)
    {
    this->Canvas->Delete();
    this->Canvas = NULL;
    }

  this->SetImageName(NULL);
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // Create and pack the canvas

  this->Canvas->SetParent(this);
  this->Canvas->Create();
  this->Canvas->SetBorderWidth(0);
  this->Canvas->SetHighlightThickness(0);

  this->Script("pack %s -side top -fill both -expand y",
               this->Canvas->GetWidgetName());

  this->Canvas->AddBinding("<ButtonPress>", this, "ButtonPressCallback");

  // Insert the image

  this->Script("%s create image 0 0 -tags image -anchor nw", 
               this->Canvas->GetWidgetName());

  // Insert the text

  this->Script("%s create text 0 0 -tags msg -anchor c", 
               this->Canvas->GetWidgetName());

  this->UpdateImageInCanvas();
  this->UpdateProgressMessagePosition();
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::Display()
{
  this->Discard = 0;
  this->Superclass::Display();
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::ButtonPressCallback()
{
  this->Withdraw();
  this->Discard = 1;
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::UpdateCanvasSize()
{
  // Resize the canvas according to the image

  if (this->IsCreated() && this->ImageName)
    {
    vtkKWApplication *app = this->GetApplication();
    this->Canvas->SetWidth(
      vtkKWTkUtilities::GetPhotoWidth(app, this->ImageName));
    this->Canvas->SetHeight(
      vtkKWTkUtilities::GetPhotoHeight(app, this->ImageName));
    this->UpdateProgressMessagePosition();
    }
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::UpdateProgressMessagePosition()
{
  if (this->IsCreated())
    {
    int width = this->Canvas->GetWidth();
    int height = this->Canvas->GetHeight();
    
    this->Script("%s coords msg %lf %d", 
                 this->Canvas->GetWidgetName(), 
                 (double)width * 0.5, 
                 (this->ProgressMessageVerticalOffset < 0 
                  ? height + ProgressMessageVerticalOffset 
                  : ProgressMessageVerticalOffset));
    }
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::UpdateImageInCanvas()
{
  if (this->Canvas && this->Canvas->IsCreated())
    {
    const char *res = this->Canvas->Script(
      "%s itemconfigure image -image {%s}", 
      this->Canvas->GetWidgetName(), this->ImageName ? this->ImageName : "");
    this->UpdateCanvasSize();
    if (res && *res)
      {
      vtkErrorMacro("Error setting ImageName: " << res);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::SetImageName(const char* _arg)
{
  if (this->ImageName == NULL && _arg == NULL) 
    { 
    return;
    }

  if (this->ImageName && _arg && (!strcmp(this->ImageName, _arg))) 
    {
    return;
    }

  if (this->ImageName) 
    { 
    delete [] this->ImageName; 
    }

  if (_arg)
    {
    this->ImageName = new char[strlen(_arg)+1];
    strcpy(this->ImageName, _arg);
    }
  else
    {
    this->ImageName = NULL;
    }

  this->Modified();

  this->UpdateImageInCanvas();
} 

//----------------------------------------------------------------------------
int vtkKWSplashScreen::ReadImage(const char *filename)
{
  int width, height, pixel_size;
  unsigned char *image_buffer = NULL;

  // Try to load the image

  if (!vtkKWResourceUtilities::ReadImage(
        filename, &width, &height, &pixel_size, &image_buffer))
    {
    vtkErrorMacro("Error reading image: " << (filename ? filename : ""));
    return 0;
    }

  this->SetImageToPixels(image_buffer, width, height, pixel_size);

  delete [] image_buffer;
  return 1;
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::SetImageToPredefinedIcon(int icon_index)
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  this->SetImageToIcon(icon);
  icon->Delete();
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::SetImageToIcon(vtkKWIcon* icon)
{
  if (icon)
    {
    this->SetImageToPixels(
      icon->GetData(), 
      icon->GetWidth(), icon->GetHeight(), icon->GetPixelSize());
    }
  else
    {
    this->SetImageToPixels(NULL, 0, 0, 0);
    }
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::SetImageToPixels(const unsigned char* pixels, 
                                         int width, 
                                         int height,
                                         int pixel_size,
                                         unsigned long buffer_length)
{
  if (!pixels || !width || !height || !pixel_size)
    {
    this->SetImageName(NULL);
    return;
    }

  // If no image name, make up one

  vtksys_stl::string new_image_name;
  if (!this->ImageName)
    {
    new_image_name = this->GetTclName();
    new_image_name += "Photo";
    }
  const char *image_name = 
    (this->ImageName ? this->ImageName : new_image_name.c_str());

  // Update the Tk image (or create it if it did not exist)

  int res = vtkKWTkUtilities::UpdatePhoto(
    this->GetApplication(), 
    image_name, 
    pixels, width, height, pixel_size, buffer_length);
  if (!res)
    {
    vtkErrorMacro("Error updating photo: " << image_name);
    }

  // Assign the new image name (now that it has been created)
  // This will update the canvas as well

  if (new_image_name.size())
    {
    this->SetImageName(new_image_name.c_str());
    }
  else
    {
    this->UpdateImageInCanvas();
    }
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::SetProgressMessage(const char *txt)
{
  if (!this->IsCreated() || !txt)
    {
    return;
    }

  const char *val = this->ConvertInternalStringToTclString(
    txt, vtkKWCoreWidget::ConvertStringEscapeInterpretable);
  this->Script("%s itemconfigure msg -text \"%s\"",
               this->Canvas->GetWidgetName(), (val ? val : ""));

  if (!this->IsMapped() && !this->Discard)
    {
    this->Display();
    }

  // As much as call to 'update' are evil, this is the only way to bring
  // the splashscreen up-to-date and in front. 'update idletasks' will not
  // do the trick because this code is usually executed during initialization
  // or creation of the UI, not in the event loop

  this->GetApplication()->ProcessPendingEvents();
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::SetProgressMessageVerticalOffset(int _arg)
{
  if (this->ProgressMessageVerticalOffset == _arg)
    {
    return;
    }

  this->ProgressMessageVerticalOffset = _arg;
  this->Modified();

  this->UpdateProgressMessagePosition();
}

//----------------------------------------------------------------------------
int vtkKWSplashScreen::GetRequestedWidth()
{
  if (this->IsCreated() && this->ImageName)
    {
    return vtkKWTkUtilities::GetPhotoWidth(this->GetApplication(), 
                                           this->ImageName);
    }
  return this->Superclass::GetRequestedWidth();
}

//----------------------------------------------------------------------------
int vtkKWSplashScreen::GetRequestedHeight()
{
  if (this->IsCreated() && this->ImageName)
    {
    return vtkKWTkUtilities::GetPhotoHeight(this->GetApplication(), 
                                            this->ImageName);
    }
  return this->Superclass::GetRequestedHeight();
}

// ---------------------------------------------------------------------------
void vtkKWSplashScreen::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->Canvas);
}

//----------------------------------------------------------------------------
void vtkKWSplashScreen::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ProgressMessageVerticalOffset: " 
     << this->ProgressMessageVerticalOffset << endl;
  os << indent << "ImageName: " 
     << (this->ImageName ? this->ImageName : "(none)") << endl;
}

