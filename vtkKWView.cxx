/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWView.cxx,v $
  Language:  C++
  Date:      $Date: 2002-11-12 22:26:38 $
  Version:   $Revision: 1.94 $

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
#include "vtkKWView.h"

#include "vtkBMPWriter.h"
#include "vtkImageData.h"
#include "vtkJPEGWriter.h"
#include "vtkKWApplication.h"
#include "vtkKWChangeColorButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCompositeCollection.h"
#include "vtkKWCornerAnnotation.h"
#include "vtkKWEntry.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWGenericComposite.h"
#include "vtkKWIcon.h"
#include "vtkKWLabeledFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWNotebook.h"
#include "vtkKWSaveImageDialog.h"
#include "vtkKWSerializer.h"
#include "vtkKWWidgetCollection.h"
#include "vtkKWWindow.h"
#include "vtkPNGWriter.h"
#include "vtkPNMWriter.h"
#include "vtkPostScriptWriter.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkString.h"
#include "vtkTIFFWriter.h"
#include "vtkTextActor.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkViewport.h"
#include "vtkWindowToImageFilter.h"

#ifdef _WIN32
#include "vtkWin32OpenGLRenderWindow.h"
#else
#include "vtkXOpenGLRenderWindow.h"
#include "vtkKWMessageDialog.h"
int vtkKWViewFoundMatch;
Bool vtkKWRenderViewPredProc(Display *vtkNotUsed(disp), XEvent *event, 
                             char * vtkNotUsed(arg))
{  
  if (event->type == Expose)
    {
    vtkKWViewFoundMatch = 1;
    }
  if (event->type == ConfigureNotify)
    {
    vtkKWViewFoundMatch = 2;
    }
  if (event->type == ButtonPress)
    {
    vtkKWViewFoundMatch = 2;
    }
  return 0;
}
#endif

vtkCxxRevisionMacro(vtkKWView, "$Revision: 1.94 $");

//----------------------------------------------------------------------------
int vtkKWViewCommand(ClientData cd, Tcl_Interp *interp,
                     int argc, char *argv[]);

//----------------------------------------------------------------------------
void KWViewAbortCheckMethod( void *arg )
{
  vtkKWView *me = (vtkKWView *)arg;

  // if we are printing then do not abort
  if (me->GetPrinting())
    {
    return;
    }

  if ( me->ShouldIAbort() == 2 )
    {
    me->GetRenderWindow()->SetAbortRender(1);    
    }
}

//----------------------------------------------------------------------------
vtkKWView::vtkKWView()
{
  this->SupportPrint        = 1;
  this->SupportSaveAsImage  = 1;
  this->SupportCopy         = 1;
  this->SupportControlFrame = 0;
  
  this->Frame = vtkKWWidget::New();
  this->Frame->SetParent(this);
  this->Frame2 = vtkKWWidget::New();
  this->Frame2->SetParent(this->Frame);
  this->ControlFrame = vtkKWWidget::New();
  this->ControlFrame->SetParent(this->Frame);
  this->Label = vtkKWWidget::New();
  this->Label->SetParent(this->Frame2);
  this->VTKWidget = vtkKWWidget::New();
  this->VTKWidget->SetParent(this->Frame);
  this->InExpose = 0;
  this->ParentWindow = NULL;
  this->PropertiesParent = NULL;
  this->CommandFunction = vtkKWViewCommand;
  this->Composites = vtkKWCompositeCollection::New();
  this->SelectedComposite = NULL;
  this->SharedPropertiesParent = 0;
  this->Notebook = vtkKWNotebook::New();

  this->AnnotationProperties = vtkKWFrame::New();
  this->HeaderFrame = vtkKWLabeledFrame::New();
  this->HeaderDisplayFrame = vtkKWWidget::New();
  this->HeaderEntryFrame = vtkKWWidget::New();
  this->HeaderButton = vtkKWCheckButton::New();
  this->HeaderColor = vtkKWChangeColorButton::New();
  this->HeaderLabel = vtkKWWidget::New();
  this->HeaderEntry = vtkKWEntry::New();

  this->HeaderMapper = vtkTextMapper::New();
  this->HeaderMapper->GetTextProperty()->SetJustificationToCentered();
  this->HeaderMapper->GetTextProperty()->SetVerticalJustificationToTop();
  this->HeaderMapper->GetTextProperty()->SetFontSize(15);  
  this->HeaderMapper->GetTextProperty()->ShadowOff();  

  this->HeaderProp = vtkTextActor::New();
  this->HeaderProp->ScaledTextOn();
  this->HeaderProp->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  this->HeaderProp->GetPositionCoordinate()->SetValue(0.2,0.88);
  this->HeaderProp->GetPosition2Coordinate()->SetValue(0.6, 0.1);

  this->HeaderProp->SetMapper(this->HeaderMapper);
  this->HeaderComposite = vtkKWGenericComposite::New();
  this->HeaderComposite->SetProp(this->HeaderProp);

  this->CornerAnnotation = vtkKWCornerAnnotation::New();
  this->CornerAnnotation->SetTraceReferenceObject(this);
  this->CornerAnnotation->SetTraceReferenceCommand("GetCornerAnnotation");
  
  this->PropertiesCreated = 0;
  this->InteractiveUpdateRate = 5.0;
  this->NumberOfStillUpdates  = 1;
  this->StillUpdateRates = new float[1];
  this->StillUpdateRates[0] = 1.0;
  this->RenderMode = VTK_KW_STILL_RENDER;
  this->RenderState = 1;
  this->MultiPassStillAbortCheckMethod = NULL;
  this->MultiPassStillAbortCheckMethodArg = NULL;

  this->GeneralProperties = vtkKWFrame::New();

  this->ColorsFrame = vtkKWLabeledFrame::New();
  this->BackgroundColor = vtkKWChangeColorButton::New();

  this->Printing = 0;
  
  this->MenuEntryName = NULL;
  this->MenuEntryHelp = NULL;
  this->MenuEntryUnderline = -1;
  
  this->Renderer = vtkRenderer::New();
  this->RenderWindow = vtkRenderWindow::New();
  this->RenderWindow->AddRenderer(this->Renderer);
  this->RenderWindow->SetAbortCheckMethod(KWViewAbortCheckMethod, (void*)this);

}

//----------------------------------------------------------------------------
vtkKWView::~vtkKWView()
{
  if (this->Renderer)
    {
    this->Renderer->Delete();
    this->Renderer = NULL;
    }
  
  if (this->RenderWindow)
    {
    this->RenderWindow->Delete();
    this->RenderWindow = NULL;
    }
    
  // Remove all binding
  const char *wname = this->VTKWidget->GetWidgetName();
  if ( this->Application )
    {
    this->Script("bind %s <Expose> {}",wname);
    this->Script("bind %s <Any-ButtonPress> {}",wname);
    this->Script("bind %s <Any-ButtonRelease> {}",wname);
    this->Script("bind %s <Shift-Any-ButtonPress> {}",wname);
    this->Script("bind %s <Shift-Any-ButtonRelease> {}",wname);
    this->Script("bind %s <Control-Any-ButtonPress> {}", wname);
    this->Script("bind %s <Control-Any-ButtonRelease> {}",wname);
    this->Script("bind %s <B1-Motion> {}",wname);
    this->Script("bind %s <B2-Motion> {}",wname);
    this->Script("bind %s <B3-Motion> {}",wname);
    this->Script("bind %s <Shift-B1-Motion> {}",wname);
    this->Script("bind %s <Shift-B2-Motion> {}",wname);
    this->Script("bind %s <Shift-B3-Motion> {}",wname);
    this->Script("bind %s <Control-B1-Motion> {}",wname);
    this->Script("bind %s <Control-B2-Motion> {}",wname);
    this->Script("bind %s <Control-B3-Motion> {}",wname);
    this->Script("bind %s <KeyPress> {}",wname);
    this->Script("bind %s <Enter> {}",wname);
    }
  this->GeneralProperties->Delete();
  this->ColorsFrame->Delete();
  this->BackgroundColor->Delete();

  this->AnnotationProperties->Delete();
  this->HeaderComposite->Delete();
  this->HeaderProp->Delete();
  this->HeaderMapper->Delete();
  this->HeaderFrame->Delete();
  this->HeaderDisplayFrame->Delete();
  this->HeaderColor->Delete();
  this->HeaderEntryFrame->Delete();
  this->HeaderButton->Delete();
  this->HeaderLabel->Delete();
  this->HeaderEntry->Delete();

  this->CornerAnnotation->Delete();
  
  this->Notebook->SetParent(NULL);
  this->Notebook->Delete();
  this->VTKWidget->Delete();
  this->Composites->Delete();
  this->Label->Delete();
  this->Frame->Delete();
  this->Frame2->Delete();
  this->ControlFrame->Delete();
  if (this->PropertiesParent)
    {
    this->PropertiesParent->Delete();
    }

  delete [] this->StillUpdateRates;
  
  this->SetMenuEntryName(NULL);
  this->SetMenuEntryHelp(NULL);
  
}

//----------------------------------------------------------------------------
// Return 1 to mean abort but keep trying, 2 to mean hard abort
int vtkKWView::ShouldIAbort()
{
  int flag = 0;
  
#ifdef _WIN32
  MSG msg;

  // Check all four - can't get the range right in one call without
  // including events we don't want

  if (PeekMessage(&msg,NULL,WM_LBUTTONDOWN,WM_LBUTTONDOWN,PM_NOREMOVE))
    {
    flag = 2;
    }
  if (PeekMessage(&msg,NULL,WM_NCLBUTTONDOWN,WM_NCLBUTTONDOWN,PM_NOREMOVE))
    {
    flag = 2;
    }
  if (PeekMessage(&msg,NULL,WM_MBUTTONDOWN,WM_MBUTTONDOWN,PM_NOREMOVE))
    {
    flag = 2;
    }
  if (PeekMessage(&msg,NULL,WM_RBUTTONDOWN,WM_RBUTTONDOWN,PM_NOREMOVE))
    {
    flag = 2;
    }
  if (PeekMessage(&msg,NULL,WM_WINDOWPOSCHANGING,WM_WINDOWPOSCHANGING,PM_NOREMOVE))
    {
    flag = 2;
    }
  if (PeekMessage(&msg,NULL,WM_WINDOWPOSCHANGED,WM_WINDOWPOSCHANGED,PM_NOREMOVE))
    {
    flag = 2;
    }
  if (PeekMessage(&msg,NULL,WM_SIZE,WM_SIZE,PM_NOREMOVE))
    {
    flag = 2;
    }

  if ( !flag )
    {
    // Check some other events to make sure UI isn't being updated
    if (PeekMessage(&msg,NULL,WM_SYNCPAINT,WM_SYNCPAINT,PM_NOREMOVE))
      {
      flag = 1;
      }
    if (PeekMessage(&msg,NULL,WM_NCPAINT,WM_NCPAINT,PM_NOREMOVE))
      {
      flag = 1;
      }
    if (PeekMessage(&msg,NULL,WM_PAINT,WM_PAINT,PM_NOREMOVE))
      {
      flag = 1;
      }
    if (PeekMessage(&msg,NULL,WM_ERASEBKGND,WM_ERASEBKGND,PM_NOREMOVE))
      {
      flag = 1;
      }
    if (PeekMessage(&msg,NULL,WM_ACTIVATE,WM_ACTIVATE,PM_NOREMOVE))
      {
      flag = 1;
      }
    if (PeekMessage(&msg,NULL,WM_NCACTIVATE,WM_NCACTIVATE,PM_NOREMOVE))
      {
      flag = 1;
      }
    }
  
#else
  XEvent report;
  
  vtkKWViewFoundMatch = 0;
  Display *dpy = ((vtkXOpenGLRenderWindow*)this->GetRenderWindow())->GetDisplayId();
  XSync(dpy,0);
  XCheckIfEvent(dpy, &report, vtkKWRenderViewPredProc, NULL);
  XSync(dpy,0);
  flag = vtkKWViewFoundMatch;
#endif
  
  return flag;
  
}

//----------------------------------------------------------------------------
void vtkKWView::SetStillUpdateRates( int count, float *rates )
{
  if ( count < 1 || count > 5 )
    {
    vtkErrorMacro( << "Number of still updates should be between 1 and 5" );
    return;
    }

  if ( count != this->NumberOfStillUpdates )
    {
    delete [] this->StillUpdateRates;
    this->StillUpdateRates = new float[count];
    this->NumberOfStillUpdates = count;
    }
  
  memcpy( this->StillUpdateRates, rates, count*sizeof(float) );
}

//----------------------------------------------------------------------------
// Specify a function to be called to check and see if an abort
// of the multi-pass still rendering in progress is desired
void vtkKWView::SetMultiPassStillAbortCheckMethod(int (*f)(void *), void *arg)
{
  if ( f != this->MultiPassStillAbortCheckMethod || 
       arg != this->MultiPassStillAbortCheckMethodArg )
    {
    this->MultiPassStillAbortCheckMethod = f;
    this->MultiPassStillAbortCheckMethodArg = arg;
    }
}


//----------------------------------------------------------------------------
void vtkKWView::Close()
{
  vtkKWComposite *c;
       
  if (this->PropertiesCreated)
    {
    if (this->HeaderButton->GetState())
      {
      this->RemoveComposite(this->HeaderComposite);
      }
    this->CornerAnnotation->Close();
    }
    
  // first unselect any composites
  this->SetSelectedComposite(NULL);
  
  this->Composites->InitTraversal();
  while((c = this->Composites->GetNextKWComposite()))
    {
    c->Close();
    c->SetView(NULL);
    this->GetViewport()->RemoveProp(c->GetProp());
    }
  this->Composites->RemoveAllItems();
}

//----------------------------------------------------------------------------
void vtkKWView::CreateViewProperties()
{
  vtkKWApplication *app = this->Application;

  this->Notebook->SetParent(this->GetPropertiesParent());
  this->Notebook->Create(this->Application,"");

  vtkKWIcon *ico = vtkKWIcon::New();
  ico->SetImageData(vtkKWIcon::ICON_GENERAL);
  this->Notebook->AddPage(
    "General", "Set the general properties of the image view", ico);
  ico->SetImageData(vtkKWIcon::ICON_ANNOTATE);
  this->Notebook->AddPage(
    "Annotate", "Set the header and corner annotation", ico);
  ico->Delete();
  
  this->AnnotationProperties->SetParent(this->Notebook->GetFrame("Annotate"));
  this->AnnotationProperties->Create(app,"-scrollable");
  this->Script("pack %s -pady 2 -padx 2 -fill both -expand yes -anchor n",
               this->Notebook->GetWidgetName());
  this->Script("pack %s -pady 2 -fill both -expand yes -anchor n",
               this->AnnotationProperties->GetWidgetName());
  this->Notebook->Raise("Annotate");
  
  // create the anno widgets
  this->HeaderFrame->SetParent( this->AnnotationProperties->GetParent() );  
  this->HeaderFrame->Create(app, 0);
  this->HeaderFrame->SetLabel("Header Annotation");
  this->HeaderDisplayFrame->SetParent(this->HeaderFrame->GetFrame());
  this->HeaderDisplayFrame->Create(app,"frame","");
  this->HeaderEntryFrame->SetParent(this->HeaderFrame->GetFrame());
  this->HeaderEntryFrame->Create(app,"frame","");
  this->Script("pack %s -padx 2 -pady 2 -fill x -expand yes -anchor w",
               this->HeaderFrame->GetWidgetName());
  this->Script("pack %s %s -side top -padx 2 -pady 4 -expand 1 -fill x -anchor nw",
               this->HeaderDisplayFrame->GetWidgetName(),
               this->HeaderEntryFrame->GetWidgetName());

  this->HeaderButton->SetParent(this->HeaderDisplayFrame);
  this->HeaderButton->Create(this->Application,
                             "-text {Display Header Annotation}");
  this->HeaderButton->SetBalloonHelpString("Toggle the visibility of the header text");
  this->HeaderButton->SetCommand(this, "OnDisplayHeader");
  this->HeaderColor->SetParent(this->HeaderDisplayFrame);
  this->HeaderColor->Create(this->Application, "");
  this->HeaderColor->SetCommand( this, "SetHeaderTextColor" );
  this->HeaderColor->SetBalloonHelpJustificationToRight();
  this->HeaderColor->SetBalloonHelpString("Change the color of the header text");
  this->Script("pack %s -side left -padx 2 -pady 4 -anchor nw",
               this->HeaderButton->GetWidgetName());
  this->Script("pack %s -side right -padx 2 -pady 4 -anchor ne",
               this->HeaderColor->GetWidgetName());
  this->HeaderLabel->SetParent(this->HeaderEntryFrame);
  this->HeaderLabel->Create(app,"label","-text Header:");
  this->HeaderLabel->SetBalloonHelpString("Set the header text string");
  this->HeaderEntry->SetParent(this->HeaderEntryFrame);
  this->HeaderEntry->Create(app,"-width 20");
  this->Script("bind %s <Return> {%s HeaderChanged}",
               this->HeaderEntry->GetWidgetName(),this->GetTclName());
  this->Script("pack %s -side left -anchor w -padx 4 -expand no",
               this->HeaderLabel->GetWidgetName());
  this->Script("pack %s -side left -anchor w -padx 4 -expand yes -fill x",
               this->HeaderEntry->GetWidgetName());

  this->CornerAnnotation->SetParent(this->AnnotationProperties->GetFrame());
  this->CornerAnnotation->SetView(this);
  this->CornerAnnotation->ShowHideFrameOn();
  this->CornerAnnotation->Create(app);
  this->CornerAnnotation->SetLabel("Corner Annotation");
  this->Script("pack %s -padx 2 -pady 4 -fill x -expand yes -anchor w",
               this->CornerAnnotation->GetWidgetName());

  
  this->GeneralProperties->SetParent(this->Notebook->GetFrame("General"));
  this->GeneralProperties->Create(app,"-scrollable");
  this->Script("pack %s -pady 2 -padx 2 -fill both -expand yes -anchor n",
               this->Notebook->GetWidgetName());
  this->Script("pack %s -pady 2 -fill both -expand yes -anchor n",
               this->GeneralProperties->GetWidgetName());  

  this->ColorsFrame->SetParent( this->GeneralProperties->GetFrame() );
  this->ColorsFrame->ShowHideFrameOn();
  this->ColorsFrame->Create( app,0 );
  this->ColorsFrame->SetLabel("Colors");
  this->Script("pack %s -padx 2 -pady 2 -fill x -expand yes -anchor w",
               this->ColorsFrame->GetWidgetName());

  float c[3];  c[0] = 0.0;  c[1] = 0.0;  c[2] = 0.0;
  this->BackgroundColor->SetParent( this->ColorsFrame->GetFrame() );
  this->BackgroundColor->SetColor( c );
  this->BackgroundColor->SetText("Set Background Color");
  this->BackgroundColor->Create( app, "" );
  this->BackgroundColor->SetCommand( this, "SetBackgroundColor" );
  this->BackgroundColor->SetBalloonHelpString("Set the background color");
  this->Script("pack %s -side top -padx 15 -pady 4 -expand 1 -fill x",
               this->BackgroundColor->GetWidgetName());

  this->PropertiesCreated = 1;
}

//----------------------------------------------------------------------------
void vtkKWView::SetHeaderTextColor( float r, float g, float b )
{
  if ( r < 0 || g < 0 || b < 0 )
    {
    return;
    }
  float *ff = this->GetHeaderTextColor();
  if ( ff[0] == r && ff[1] == g && ff[2] == b )
    {
    return;
    }
  this->HeaderColor->SetColor( r, g, b );
  this->HeaderProp->GetProperty()->SetColor( r, g, b );
  float color[3];
  color[0] = r;
  color[1] = g;
  color[2] = b;
  this->InvokeEvent( vtkKWEvent::AnnotationColorChangedEvent, color );
  this->InvokeEvent( vtkKWEvent::ViewAnnotationChangedEvent, 0);
}

//----------------------------------------------------------------------------
float* vtkKWView::GetHeaderTextColor()
{
  return this->HeaderProp->GetProperty()->GetColor( );
}

//----------------------------------------------------------------------------
void vtkKWView::GetHeaderTextColor( float *r, float *g, float *b )
{
  float *ff = this->GetHeaderTextColor();
  *r = ff[0];
  *g = ff[1];
  *b = ff[2];
}


//----------------------------------------------------------------------------
void vtkKWView::SetPropertiesParent(vtkKWWidget *args)
{
  if (this->PropertiesParent != args)
    {
    if (this->PropertiesParent != NULL)
      { 
      this->PropertiesParent->UnRegister(this);
      }
    this->PropertiesParent = args;
    if (this->PropertiesParent != NULL)
      { 
      this->PropertiesParent->Register(this); 
      }
    this->Modified();
    this->SharedPropertiesParent = 0;
    }                                                           
}

//----------------------------------------------------------------------------
vtkKWWidget *vtkKWView::GetPropertiesParent()
{
  // if already set then return
  if (this->PropertiesParent)
    {
    return this->PropertiesParent;
    }
  
  // if the window has defined one then use it
  if (this->ParentWindow && 
      this->ParentWindow->GetPropertiesParent())
    {
    // if the views props have not been defined the define them now
    this->PropertiesParent = vtkKWWidget::New();
    this->PropertiesParent->SetParent
      (this->ParentWindow->GetPropertiesParent());
    this->PropertiesParent->Create(this->Application,"frame","-bd 0");
    this->SharedPropertiesParent = 1;
    }
  return this->PropertiesParent;
}

//----------------------------------------------------------------------------
// if you are not using window based properties then you are probably 
// using view based properties
void vtkKWView::CreateDefaultPropertiesParent()
{
  if (!this->PropertiesParent)
    {
    this->PropertiesParent = vtkKWWidget::New();
    this->PropertiesParent->SetParent(this);
    this->PropertiesParent->Create(this->Application,"frame","-bd 0");
    this->Script("pack %s -before %s -fill y -side left -anchor nw",
                 this->PropertiesParent->GetWidgetName(),
                 this->Frame->GetWidgetName());
    }
  else
    {
    vtkDebugMacro("Properties Parent Already Set for view");
    }
}

//----------------------------------------------------------------------------
void vtkKWView::ShowViewProperties()
{
  this->ParentWindow->ShowProperties();
  
  // make sure we have an applicaiton
  if (!this->Application)
    {
    vtkErrorMacro("attempt to update properties without an application set");
    }

  if (this->MenuEntryName)
    {
    // make sure the variable is set, otherwise set it
    this->ParentWindow->GetMenuView()->CheckRadioButton(
      this->ParentWindow->GetMenuView(), "Radio", VTK_KW_VIEW_MENU_INDEX);
    }

  // unpack any current children
  this->Script("catch {eval pack forget [pack slaves %s]}",
               this->GetPropertiesParent()->GetWidgetName());
  
  // do we need to create the props ?
  if (!this->PropertiesCreated)
    {
    this->CreateViewProperties();
    }
  this->Script("pack %s -pady 2 -padx 2 -fill both -expand yes -anchor n",
               this->Notebook->GetWidgetName());
  this->PackProperties();
}

//----------------------------------------------------------------------------
void vtkKWView::PackProperties()
{
  // make sure the view is packed if necc
  if (this->SharedPropertiesParent)
    {
    // if the windows prop is not currently this views prop
    this->Script("pack slaves %s",
                 this->PropertiesParent->GetParent()->GetWidgetName());
    if (strcmp(this->Application->GetMainInterp()->result,
               this->PropertiesParent->GetWidgetName()))
      {
      // forget current props
      this->Script("pack forget [pack slaves %s]",
                   this->PropertiesParent->GetParent()->GetWidgetName());  
      this->Script("pack %s -side left -anchor nw -fill y",
                   this->PropertiesParent->GetWidgetName());
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWView::SetSelectedComposite(vtkKWComposite *_arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting SelectedComposite to " << _arg ); 
  if (this->SelectedComposite != _arg) 
    { 
    if (this->SelectedComposite != NULL) 
      { 
      if (this->ParentWindow->GetSelectedView() == this)
        {
        this->SelectedComposite->Deselect(this);
        }
      this->SelectedComposite->UnRegister(this); 
      }
    this->SelectedComposite = _arg; 
    if (this->SelectedComposite != NULL) 
      { 
      this->SelectedComposite->Register(this); 
      if (this->ParentWindow->GetSelectedView() == this)
        {
        this->SelectedComposite->Select(this);
        } 
      } 
    this->Modified(); 
    } 
}

//----------------------------------------------------------------------------
void vtkKWView::AddComposite(vtkKWComposite *c)
{
  c->SetView(this);
  // never allow a composite to be added twice
  if (this->Composites->IsItemPresent(c))
    {
    return;
    }
  this->Composites->AddItem(c);
  if (c->GetProp() != NULL)
    {
    this->GetViewport()->AddProp(c->GetProp());
    }
}

//----------------------------------------------------------------------------
void vtkKWView::RemoveComposite(vtkKWComposite *c)
{
  c->SetView(NULL);
  this->GetViewport()->RemoveProp(c->GetProp());
  this->Composites->RemoveItem(c);
}

//----------------------------------------------------------------------------
void vtkKWView::Enter(int /*x*/, int /*y*/)
{
//  this->Script("focus %s",this->VTKWidget->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWView::PrintView()
{
  this->Printing = 1;
  vtkWindow *vtkWin = this->GetVTKWindow();

#ifdef _WIN32  
  int oldrm = this->GetRenderMode();
  this->SetRenderModeToSingle();

  int size[2];
  memcpy(size,vtkWin->GetSize(),sizeof(int)*2);

  PRINTDLG  pd;
  DOCINFO di;
  
  // Initialize a PRINTDLG struct and call PrintDlg to allow user to
  //   specify various printing options...
  //
  memset ((void *) &pd, 0, sizeof(PRINTDLG));
  
  pd.lStructSize = sizeof(PRINTDLG);
  pd.hwndOwner   = (HWND)vtkWin->GetGenericWindowId();
  pd.Flags       = PD_RETURNDC;
  pd.hInstance   = NULL;
  
  PrintDlg(&pd);
  HDC ghdc = pd.hDC;
  
  if (!ghdc)
    {
    return;
    }

  if (pd.hDevMode)
    {
    GlobalFree(pd.hDevMode);
    }
  if (pd.hDevNames)
    {
    GlobalFree(pd.hDevNames);
    }
  
  this->Script("%s configure -cursor watch; update",
               this->ParentWindow->GetWidgetName());  

  di.cbSize      = sizeof(DOCINFO);
  di.lpszDocName = "Kitware Test";
  di.lpszOutput  = NULL;
  
  StartDoc  (ghdc, &di);
  StartPage (ghdc);

  this->Print(ghdc, ghdc);
  
  EndPage   (ghdc);
  EndDoc    (ghdc);
  DeleteDC  (ghdc);

  this->SetRenderMode(oldrm);
  this->Script("%s configure -cursor top_left_arrow",
               this->ParentWindow->GetWidgetName());
#else

  vtkWindowToImageFilter *w2i = vtkWindowToImageFilter::New();
  float DPI=0;
  if (this->GetParentWindow())
    {
    // Is this right? Should DPI be int or float?
    DPI = this->GetParentWindow()->GetPrintTargetDPI();
    }
  if (DPI >= 150.0)
    {
    w2i->SetMagnification(2);
    }
  if (DPI >= 300.0)
    {
    w2i->SetMagnification(3);
    }
  w2i->SetInput(vtkWin);
  w2i->Update();
  
  this->Script("tk_getSaveFile -title \"Save Postscript\" -filetypes {{{Postscript} {.ps}}}");
  char* path = 
    strcpy(new char[strlen(this->Application->GetMainInterp()->result)+1], 
           this->Application->GetMainInterp()->result);
  if (strlen(path) != 0)
    {
    vtkPostScriptWriter *psw = vtkPostScriptWriter::New();
    psw->SetInput(w2i->GetOutput());
    psw->SetFileName(path);
    psw->Write();
    psw->Delete();

    vtkKWMessageDialog *dlg = vtkKWMessageDialog::New();
    dlg->SetMasterWindow(this->ParentWindow);
    dlg->Create(this->Application,"");
    dlg->SetText(
      "A postscript file has been generated. You will need to\n"
      "print this file using a print command appropriate for\n"
      "your system. Typically this command is lp or lpr. For\n"
      "additional information on printing a postscript file\n"
      "please contact your system administrator.");
    dlg->Invoke();
    }
  w2i->Delete();
#endif
  
  this->Printing = 0;
}

#ifdef _WIN32
void vtkKWView::Print(HDC ghdc, HDC adc)
{
  if (!ghdc || !adc)
    {
    return;
    }
  
  // get size of printer page (in pixels)
  int cxPage = GetDeviceCaps(ghdc,HORZRES);
  int cyPage = GetDeviceCaps(ghdc,VERTRES);
  // get printer pixels per inch
  int cxInch = GetDeviceCaps(ghdc,LOGPIXELSX);
  int cyInch = GetDeviceCaps(ghdc,LOGPIXELSY);

  this->Print(ghdc, adc, cxPage, cyPage, cxInch, cyInch,
              0.0, 0.0, 1.0, 1.0);
}

void vtkKWView::Print(HDC ghdc, HDC,
                      int printerPageSizeX, int printerPageSizeY,
                      int printerDPIX, int printerDPIY,
                      float minX, float minY, float scaleX, float scaleY)
{
  RECT rcDest;  
  vtkWindow *vtkWin = this->GetVTKWindow();  
  int size[2];
  memcpy(size,vtkWin->GetSize(),sizeof(int)*2);

  this->SetupPrint(rcDest, ghdc, printerPageSizeX, printerPageSizeY,
                   printerDPIX, printerDPIY,
                   scaleX, scaleY, size[0], size[1]);
  float scale;
  // target DPI specified here
  if (this->GetParentWindow())
    {
    scale = printerDPIX/this->GetParentWindow()->GetPrintTargetDPI();
    }
  else
    {
    scale = printerDPIX/100.0;
    }

  this->Render();
  
  SetStretchBltMode(ghdc,HALFTONE);
  StretchBlt(ghdc, rcDest.right*minX, rcDest.top*minY,
             rcDest.right*scaleX, rcDest.top*scaleY,
             (HDC)this->GetMemoryDC(), 0, 0,
             rcDest.right/scale*scaleX,
             rcDest.top/scale*scaleY,
             SRCCOPY);
  
  this->ResumeScreenRendering();
}

void vtkKWView::SetupPrint(RECT &rcDest, HDC ghdc,
                           int printerPageSizeX, int printerPageSizeY,
                           int printerDPIX, int printerDPIY,
                           float scaleX, float scaleY,
                           int screenSizeX, int screenSizeY)
{
  float scale;
  int cxDIB = screenSizeX;         // Size of DIB - x
  int cyDIB = screenSizeY;         // Size of DIB - y
  
  // target DPI specified here
  if (this->GetParentWindow())
    {
    scale = printerDPIX/this->GetParentWindow()->GetPrintTargetDPI();
    }
  else
    {
    scale = printerDPIX/100.0;
    }
  

  // Best Fit case -- create a rectangle which preserves
  // the DIB's aspect ratio, and fills the page horizontally.
  //
  // The formula in the "->bottom" field below calculates the Y
  // position of the printed bitmap, based on the size of the
  // bitmap, the width of the page, and the relative size of
  // a printed pixel (printerDPIY / printerDPIX).
  //
  rcDest.bottom = rcDest.left = 0;
  if (((float)cyDIB*(float)printerPageSizeX/(float)printerDPIX) > 
      ((float)cxDIB*(float)printerPageSizeY/(float)printerDPIY))
    {
    rcDest.top = printerPageSizeY;
    rcDest.right = (static_cast<float>(printerPageSizeY)*printerDPIX*cxDIB) /
      (static_cast<float>(printerDPIY)*cyDIB);
    }
  else
    {
    rcDest.right = printerPageSizeX;
    rcDest.top = (static_cast<float>(printerPageSizeX)*printerDPIY*cyDIB) /
      (static_cast<float>(printerDPIX)*cxDIB);
    } 
  
  this->SetupMemoryRendering(rcDest.right/scale*scaleX,
                             rcDest.top/scale*scaleY, ghdc);
}
#endif

//----------------------------------------------------------------------------
void vtkKWView::SaveAsImage() 
{
  char *path = 0;
  
  // first get the file name
  vtkKWSaveImageDialog *dlg = vtkKWSaveImageDialog::New();
  dlg->SetParent(this->GetWindow());
  dlg->Create(this->Application,"");  
  dlg->Invoke();
  path = dlg->GetFileName();

  // make sure we have a file name
  if (path && vtkString::Length(path) > 1)
    {
    this->SaveAsImage(path);
    }
  dlg->Delete();
}

//----------------------------------------------------------------------------
void vtkKWView::SaveAsImage(const char* filename) 
{
  if ( !filename || !*filename )
    {
    vtkErrorMacro("Filename not specified");
    return;
    }
  
  // first get the file name
  vtkWindow *vtkWin = this->GetVTKWindow();
  vtkWindowToImageFilter *w2i = vtkWindowToImageFilter::New();
  w2i->SetInput(vtkWin);
  w2i->Update();
  
  if (!strcmp(filename + strlen(filename) - 4,".bmp"))
    {
    vtkBMPWriter *bmp = vtkBMPWriter::New();
    bmp->SetInput(w2i->GetOutput());
    bmp->SetFileName((char *)filename);
    bmp->Write();
    bmp->Delete();
    }
  else if (!strcmp(filename + strlen(filename) - 4,".tif"))
    {
    vtkTIFFWriter *tif = vtkTIFFWriter::New();
    tif->SetInput(w2i->GetOutput());
    tif->SetFileName((char *)filename);
    tif->Write();
    tif->Delete();
    }
  else if (!strcmp(filename + strlen(filename) - 4,".ppm"))
    {
    vtkPNMWriter *pnm = vtkPNMWriter::New();
    pnm->SetInput(w2i->GetOutput());
    pnm->SetFileName((char *)filename);
    pnm->Write();
    pnm->Delete();
    }
  else if (!strcmp(filename + strlen(filename) - 4,".png"))
    {
    vtkPNGWriter *png = vtkPNGWriter::New();
    png->SetInput(w2i->GetOutput());
    png->SetFileName((char *)filename);
    png->Write();
    png->Delete();
    }
  else if (!strcmp(filename + strlen(filename) - 4,".jpg"))
    {
    vtkJPEGWriter *jpg = vtkJPEGWriter::New();
    jpg->SetInput(w2i->GetOutput());
    jpg->SetFileName((char *)filename);
    jpg->Write();
    jpg->Delete();
    }

  w2i->Delete();
}

//----------------------------------------------------------------------------
void vtkKWView::EditCopy()
{
  vtkWindow *vtkWin = this->GetVTKWindow();
  vtkWindowToImageFilter *w2i = vtkWindowToImageFilter::New();
  w2i->SetInput(vtkWin);
  w2i->Update();

#ifdef _WIN32
  // get the pointer to the data
  unsigned char *ptr = 
    (unsigned char *)(w2i->GetOutput()->GetScalarPointer());
  
  LPBITMAPINFOHEADER  lpbi;       // pointer to BITMAPINFOHEADER
  DWORD               dwLen;      // size of memory block
  HANDLE              hDIB = NULL;  // handle to DIB, temp handle
  int *size = this->GetVTKWindow()->GetSize();
  int dataWidth = ((size[0]*3+3)/4)*4;
  int srcWidth = size[0]*3;
  
  if (::OpenClipboard((HWND)this->GetVTKWindow()->GetGenericWindowId()))
    {
    EmptyClipboard();
    
    dwLen = sizeof(BITMAPINFOHEADER) + dataWidth*size[1];
    hDIB = ::GlobalAlloc(GHND, dwLen);
    lpbi = (LPBITMAPINFOHEADER) ::GlobalLock(hDIB);
    
    lpbi->biSize = sizeof(BITMAPINFOHEADER);
    lpbi->biWidth = size[0];
    lpbi->biHeight = size[1];
    lpbi->biPlanes = 1;
    lpbi->biBitCount = 24;
    lpbi->biCompression = BI_RGB;
    lpbi->biClrUsed = 0;
    lpbi->biClrImportant = 0;
    lpbi->biSizeImage = dataWidth*size[1];
    
    // copy the data to the clipboard
    unsigned char *dest = (unsigned char *)lpbi + lpbi->biSize;
    int i,j;
    for (i = 0; i < size[1]; i++)
      {
      for (j = 0; j < size[0]; j++)
        {
        *dest++ = ptr[2];
        *dest++ = ptr[1];
        *dest++ = *ptr;
        ptr += 3;
        }
      dest = dest + (dataWidth - srcWidth);
      }
    
    SetClipboardData (CF_DIB, hDIB);
    ::GlobalUnlock(hDIB);
    CloseClipboard();
    }           
#endif
  w2i->Delete();
}

//----------------------------------------------------------------------------
void vtkKWView::Select(vtkKWWindow *pw)
{
  if (this->MenuEntryName)
    {
    // now add property options
    char *rbv = 
      pw->GetMenuView()->CreateRadioButtonVariable(
        pw->GetMenuView(),"Radio");

    pw->GetMenuView()->AddRadioButton(VTK_KW_VIEW_MENU_INDEX, 
                                      this->MenuEntryName, 
                                      rbv, 
                                      this, 
                                      "ShowViewProperties", 
                                      this->GetMenuEntryUnderline(),
                                      this->MenuEntryHelp ? 
                                      this->MenuEntryHelp :
                                      this->MenuEntryName
      );
    delete [] rbv;
    }

  if ( this->SupportSaveAsImage )
    {
    // add the save as image option
    pw->GetMenuFile()->InsertSeparator(this->ParentWindow->GetFileMenuIndex());
    pw->GetMenuFile()->InsertCommand(this->ParentWindow->GetFileMenuIndex(),
                                     "Save View Image",
                                     this, 
                                     "SaveAsImage", 8,
                                     "Save an image of the current view contents");
    }
  
  if ( this->SupportPrint )
    {
    // add the Print option
    // If there is a "Page Setup" menu, insert below
    int clidx;
    if (pw->GetMenuFile()->IsItemPresent(VTK_KW_PAGE_SETUP_MENU_LABEL))
      {
      clidx = pw->GetMenuFile()->GetIndex(VTK_KW_PAGE_SETUP_MENU_LABEL) + 1;  
      }
    else
      {
      clidx = this->ParentWindow->GetFileMenuIndex();  
      }
    pw->GetMenuFile()->InsertCommand(clidx, "Print", this, "PrintView", 0);
    }
  
  if ( this->SupportCopy )
    {
#ifdef _WIN32
  // add the edit copy option
  pw->GetMenuEdit()->AddCommand("Copy View Image",this,"EditCopy", "Copy an image of current view contents to the clipboard");
#endif
    }
  
  // change the color of the frame
  this->Script("%s configure -bg #008", this->Label->GetWidgetName());
  this->Script("%s configure -bg #008", this->Frame2->GetWidgetName());
  
  // forward to selected composite
  if (this->SelectedComposite)
    {
    this->SelectedComposite->Select(this);
    }
  
  
  // map the property sheet as needed
  if (this->SharedPropertiesParent && this->MenuEntryName)
    {
    // if the window prop is empty then pack this one
    if (this->ParentWindow->GetMenuView()->GetRadioButtonValue(
      this->ParentWindow->GetMenuView(), "Radio") >= VTK_KW_VIEW_MENU_INDEX)
      {
      this->Script("pack %s -side left -anchor nw -fill y",
                   this->PropertiesParent->GetWidgetName());
      }
    }
  this->InvokeEvent(vtkKWEvent::ViewSelectedEvent, 0);
}



//----------------------------------------------------------------------------
void vtkKWView::Deselect(vtkKWWindow *pw)
{
  if (this->MenuEntryName)
    {
    pw->GetMenuView()->DeleteMenuItem(this->MenuEntryName);
    }
      
  if ( this->SupportPrint )
    {
    pw->GetMenuFile()->DeleteMenuItem("Print");
    }
  
  if ( this->SupportSaveAsImage )
    {
    pw->GetMenuFile()->DeleteMenuItem("Save Image");
    }
  
  if ( this->SupportCopy )
    {
#ifdef _WIN32
  // add the edit copy option
  pw->GetMenuEdit()->DeleteMenuItem("Copy");
#endif
    }
  
  // change the color of the frame
  this->Script("%s configure -bg #888", this->Label->GetWidgetName());
  this->Script("%s configure -bg #888", this->Frame2->GetWidgetName());
  
  // forward to selected composite
  if (this->SelectedComposite)
    {
    this->SelectedComposite->Deselect(this);
    }

  // forget the properties parent as necc
  if (this->SharedPropertiesParent)
    {
    this->Script("pack forget %s", this->PropertiesParent->GetWidgetName());
    }
}


//----------------------------------------------------------------------------
void vtkKWView::MakeSelected()
{
  this->Script("focus %s",this->VTKWidget->GetWidgetName());
  if (this->ParentWindow)
    {
    this->ParentWindow->SetSelectedView(this);
    }
}

//----------------------------------------------------------------------------
void vtkKWView::SetupBindings()
{
  const char *wname = this->VTKWidget->GetWidgetName();
  const char *tname = this->GetTclName();

  // setup some default bindings
  this->Script("bind %s <Expose> {%s Exposed}",wname,tname);
  
  this->Script("bind %s <Any-ButtonPress> {%s AButtonPress %%b %%x %%y}",
               wname, tname);

  this->Script("bind %s <Any-ButtonRelease> {%s AButtonRelease %%b %%x %%y}",
               wname, tname);

  this->Script(
    "bind %s <Shift-Any-ButtonPress> {%s AShiftButtonPress %%b %%x %%y}",
    wname, tname);

  this->Script(
    "bind %s <Shift-Any-ButtonRelease> {%s AShiftButtonRelease %%b %%x %%y}",
    wname, tname);

  this->Script(
    "bind %s <Control-Any-ButtonPress> {%s AControlButtonPress %%b %%x %%y}",
    wname, tname);

  this->Script(
    "bind %s <Control-Any-ButtonRelease> {%s AControlButtonRelease %%b %%x %%y}",
    wname, tname);

  this->Script("bind %s <B1-Motion> {%s Button1Motion %%x %%y}",
               wname, tname);

  this->Script("bind %s <B2-Motion> {%s Button2Motion %%x %%y}", 
               wname, tname);
  
  this->Script("bind %s <B3-Motion> {%s Button3Motion %%x %%y}", 
               wname, tname);

  this->Script("bind %s <Shift-B1-Motion> {%s ShiftButton1Motion %%x %%y}", 
               wname, tname);

  this->Script("bind %s <Shift-B2-Motion> {%s ShiftButton2Motion %%x %%y}", 
               wname, tname);
  
  this->Script("bind %s <Shift-B3-Motion> {%s ShiftButton3Motion %%x %%y}", 
               wname, tname);

  this->Script("bind %s <Control-B1-Motion> {%s ControlButton1Motion %%x %%y}",
               wname, tname);

  this->Script("bind %s <Control-B2-Motion> {%s ControlButton2Motion %%x %%y}",
               wname, tname);
  
  this->Script("bind %s <Control-B3-Motion> {%s ControlButton3Motion %%x %%y}",
               wname, tname);

  this->Script("bind %s <KeyPress> {%s AKeyPress %%A %%x %%y}", 
               wname, tname);
  
  this->Script("bind %s <Enter> {%s Enter %%x %%y}", wname, tname);

}


//----------------------------------------------------------------------------
void vtkKWView::UnRegister(vtkObjectBase *o)
{
  if (!this->DeletingChildren)
    {
    // delete the children if we are about to be deleted
    if (this->ReferenceCount == this->Composites->GetNumberOfItems() + 
        this->Children->GetNumberOfItems() + 2)
      {
      if (!(this->Composites->IsItemPresent((vtkKWComposite *)o) ||
            this->Children->IsItemPresent((vtkKWWidget *)o)))
        {
        vtkKWWidget *child;
        vtkKWComposite *c;
        
        this->DeletingChildren = 1;
        this->Children->InitTraversal();
        while ((child = this->Children->GetNextKWWidget()))
          {
          child->SetParent(NULL);
          }
        this->Composites->InitTraversal();
        while ((c = this->Composites->GetNextKWComposite()))
          {
          c->SetView(NULL);
          }
        this->CornerAnnotation->SetView(NULL);
        this->DeletingChildren = 0;
        }
      }
    }
  
  this->vtkObject::UnRegister(o);
}

//----------------------------------------------------------------------------
void vtkKWView::SetParentWindow(vtkKWWindow *_arg)
{ 
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ParentWindow to " << _arg ); 
  if (this->ParentWindow != _arg) 
    { 
    if (this->ParentWindow != NULL) { this->ParentWindow->UnRegister(this); }
    this->ParentWindow = _arg; 
    if (this->ParentWindow != NULL) { this->ParentWindow->Register(this); } 
    this->Modified(); 
    } 
} 

//----------------------------------------------------------------------------
void vtkKWView::SetTitle(const char *title)
{
  this->Script("%s configure -text {%s}", 
               this->Label->GetWidgetName(), title);
  //  this->Script("update idletasks");
}

//----------------------------------------------------------------------------
void vtkKWView::OnDisplayHeader() 
{
  if (this->HeaderButton->GetState())
    {
    this->AddComposite(this->HeaderComposite);
    this->HeaderMapper->SetInput(this->HeaderEntry->GetValue());
    this->Render();
    }
  else
    {
    this->RemoveComposite(this->HeaderComposite);
    this->Render();
    }
  
  this->InvokeEvent(vtkKWEvent::ViewAnnotationChangedEvent, 0);
}

//----------------------------------------------------------------------------
void vtkKWView::HeaderChanged() 
{
  this->HeaderMapper->SetInput(this->HeaderEntry->GetValue());
  if (this->HeaderButton->GetState())
    {
    this->Render();
    }
  this->InvokeEvent(vtkKWEvent::ViewAnnotationChangedEvent, 0);
}

//----------------------------------------------------------------------------
void vtkKWView::InteractOn()
{
  this->SetRenderModeToInteractive();
}

//----------------------------------------------------------------------------
void vtkKWView::InteractOff()
{
  this->SetRenderModeToStill();
  this->Render();
}

//----------------------------------------------------------------------------
// Description:
// Chaining method to serialize an object and its superclasses.
void vtkKWView::SerializeSelf(ostream& os, vtkIndent indent)
{
  // invoke superclass
  this->vtkKWWidget::SerializeSelf(os,indent);

  // write out the composite
  if (this->PropertiesCreated)
    {
    os << indent << "CornerAnnotation ";
    this->CornerAnnotation->Serialize(os,indent);
    
    os << indent << "HeaderEntry ";
    vtkKWSerializer::WriteSafeString(os, this->HeaderEntry->GetValue());
    os << endl;
    os << indent << "HeaderButton " << this->HeaderButton->GetState() << endl;
    
    os << indent << "HeaderColor ";
    this->HeaderColor->Serialize(os,indent);

    os << indent << "BackgroundColor ";
    this->BackgroundColor->Serialize(os,indent);
    }
}

//----------------------------------------------------------------------------
void vtkKWView::SerializeToken(istream& is, const char token[1024])
{
  int i;
  char tmp[1024];
  
  // do we need to create the props ?
  if (!this->PropertiesCreated)
    {
    this->CreateViewProperties();
    }

  // if this file is from an old version then look for the 
  // old corner annotation code version 1.7 or earlier
  if (!this->VersionsLoaded || 
      this->CompareVersions(this->GetVersion("vtkKWView"),"1.7") <= 0)
    {
    if (!strcmp(token,"CornerButton"))
      {
      is >> i;
      this->CornerAnnotation->SetVisibility(i);
      return;
      }
    if (!strcmp(token,"CornerText"))
      {
      vtkKWSerializer::GetNextToken(&is,tmp);
      this->CornerAnnotation->SetCornerText(tmp,0);
      return;
      }
    }

  if (!strcmp(token,"HeaderButton"))
    {
    is >> i;
    this->HeaderButton->SetState(i);
    this->OnDisplayHeader();
    return;
    }
  if (!strcmp(token,"HeaderEntry"))
    {
    vtkKWSerializer::GetNextToken(&is,tmp);
    this->HeaderEntry->SetValue(tmp);
    this->OnDisplayHeader();
    return;
    }
  if (!strcmp(token,"HeaderColor"))
    {
    this->HeaderColor->Serialize(is);
    return;
    }
  if (!strcmp(token,"CornerAnnotation"))
    {
    this->CornerAnnotation->Serialize(is);
    return;
    }
  
  if (!strcmp(token,"BackgroundColor"))
    {
    this->BackgroundColor->Serialize(is);
    return;
    }

  vtkKWWidget::SerializeToken(is,token);
}

//----------------------------------------------------------------------------
void vtkKWView::SerializeRevision(ostream& os, vtkIndent indent)
{
  vtkKWWidget::SerializeRevision(os,indent);
  os << indent << "vtkKWView ";
  this->ExtractRevision(os,"$Revision: 1.94 $");
}

//----------------------------------------------------------------------------
void vtkKWView::SetupMemoryRendering(
#ifdef _WIN32
  int x, int y, void *cd
#else
  int, int, void*
#endif
  ) 
{
#ifdef _WIN32
  if (!cd)
    {
    cd = this->RenderWindow->GetGenericContext();
    }
  vtkWin32OpenGLRenderWindow::
    SafeDownCast(this->RenderWindow)->SetupMemoryRendering(x,y,(HDC)cd);
#endif
}

//----------------------------------------------------------------------------
void vtkKWView::ResumeScreenRendering() 
{
#ifdef _WIN32
  vtkWin32OpenGLRenderWindow::
    SafeDownCast(this->RenderWindow)->ResumeScreenRendering();
#endif
}

//----------------------------------------------------------------------------
void *vtkKWView::GetMemoryDC()
{
#ifdef _WIN32   
  return (void *)vtkWin32OpenGLRenderWindow::
    SafeDownCast(this->RenderWindow)->GetMemoryDC();
#else
  return NULL;
#endif
}

//----------------------------------------------------------------------------
unsigned char *vtkKWView::GetMemoryData()
{
#ifdef _WIN32   
  return vtkWin32OpenGLRenderWindow::
    SafeDownCast(this->RenderWindow)->GetMemoryData();
#else
  return NULL;
#endif
}

//----------------------------------------------------------------------------
void vtkKWView::SetBackgroundColor( float r, float g, float b )
{
  if ( r < 0 || g < 0 || b < 0 )
    {
    return;
    }
  float *ff = this->Renderer->GetBackground( );
  if ( ff[0] == r && ff[1] == g && ff[2] == b )
    {
    return;
    }

  this->BackgroundColor->SetColor( r, g, b );
  this->Renderer->SetBackground( r, g, b );
  this->Render();
  float color[3];
  color[0] = r;
  color[1] = g;
  color[2] = b;
  this->InvokeEvent( vtkKWEvent::BackgroundColorChangedEvent, color );
}

//----------------------------------------------------------------------------
float* vtkKWView::GetBackgroundColor( )
{
  return this->Renderer->GetBackground( );
}

//----------------------------------------------------------------------------
void vtkKWView::GetBackgroundColor( float *r, float *g, float *b )
{
  float *ff = this->Renderer->GetBackground( );
  *r = ff[0];
  *g = ff[1];
  *b = ff[2];
}


//----------------------------------------------------------------------------
void vtkKWView::SetCornerTextColor( float rgb[3] )
{
  if ( rgb[0] < 0 || rgb[1] < 0 || rgb[2] < 0 )
    {
    return;
    }
  this->CornerAnnotation->SetTextColor( rgb );
  this->InvokeEvent(vtkKWEvent::ViewAnnotationChangedEvent, 0);
}

//----------------------------------------------------------------------------
float *vtkKWView::GetCornerTextColor()
{
  return this->CornerAnnotation->GetTextColor();
}

//----------------------------------------------------------------------------
vtkWindow *vtkKWView::GetVTKWindow() 
{ 
  return this->RenderWindow; 
}

//----------------------------------------------------------------------------
vtkViewport *vtkKWView::GetViewport() 
{ 
  return this->Renderer; 
}

//----------------------------------------------------------------------------
void vtkKWView::Render() 
{
  this->GetVTKWindow()->Render();
}

//----------------------------------------------------------------------------
void vtkKWView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ControlFrame: " << this->GetControlFrame() << endl;
  os << indent << "CornerAnnotation: " << this->GetCornerAnnotation() << endl;
  os << indent << "HeaderButton: " << this->GetHeaderButton() << endl;
  os << indent << "HeaderEntry: " << this->GetHeaderEntry() << endl;
  os << indent << "InExpose: " << this->GetInExpose() << endl;
  os << indent << "InteractiveUpdateRate: " 
     << this->GetInteractiveUpdateRate() << endl;
  os << indent << "LastPosition: " << this->GetLastPosition() << endl;
  os << indent << "MenuEntryHelp: " << this->GetMenuEntryHelp() 
     << endl;
  os << indent << "MenuEntryName: " << this->GetMenuEntryName() 
     << endl;
  os << indent << "MenuEntryUnderline: " 
     << this->GetMenuEntryUnderline() << endl;
  os << indent << "Notebook: " << this->GetNotebook() << endl;
  os << indent << "NumberOfStillUpdates: " << this->GetNumberOfStillUpdates()
     << endl;
  os << indent << "ParentWindow: " << this->GetParentWindow() << endl;
  os << indent << "Printing: " << this->GetPrinting() << endl;
  os << indent << "RenderMode: " << this->GetRenderMode() << endl;
  os << indent << "RenderState: " << this->GetRenderState() << endl;
  os << indent << "RenderWindow: " << this->GetRenderWindow() << endl;
  os << indent << "Renderer: " << this->GetRenderer() << endl;
  os << indent << "SelectedComposite: " << this->GetSelectedComposite() 
     << endl;
  os << indent << "SupportControlFrame: " << this->GetSupportControlFrame() 
     << endl;
  os << indent << "SupportPrint: " << this->GetSupportPrint() << endl;
  os << indent << "SupportSaveAsImage: " << this->GetSupportSaveAsImage() 
     << endl;
  os << indent << "SupportCopy: " << this->GetSupportCopy() << endl;
}
