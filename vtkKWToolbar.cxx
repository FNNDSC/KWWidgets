/*=========================================================================

  Module:    $RCSfile: vtkKWToolbar.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWToolbar.h"

#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWRadioButton.h"
#include "vtkObjectFactory.h"
#include "vtkVector.txx"
#include "vtkVectorIterator.txx"

#ifndef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION

template class VTK_EXPORT vtkAbstractList<vtkKWWidget*>;
template class VTK_EXPORT vtkVector<vtkKWWidget*>;
template class VTK_EXPORT vtkAbstractIterator<vtkIdType,vtkKWWidget*>;
template class VTK_EXPORT vtkVectorIterator<vtkKWWidget*>;

#endif

#ifdef _WIN32
static int vtkKWToolbarGlobalFlatAspect        = 1;
static int vtkKWToolbarGlobalWidgetsFlatAspect = 1;
#else
static int vtkKWToolbarGlobalFlatAspect        = 0;
static int vtkKWToolbarGlobalWidgetsFlatAspect = 0;
#endif

int vtkKWToolbar::GetGlobalFlatAspect() 
{ 
  return vtkKWToolbarGlobalFlatAspect; 
}
void vtkKWToolbar::SetGlobalFlatAspect(int val) 
{ 
  vtkKWToolbarGlobalFlatAspect = val; 
};

int vtkKWToolbar::GetGlobalWidgetsFlatAspect() 
{ 
  return vtkKWToolbarGlobalWidgetsFlatAspect; 
}
void vtkKWToolbar::SetGlobalWidgetsFlatAspect(int val) 
{ 
  vtkKWToolbarGlobalWidgetsFlatAspect = val; 
};

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWToolbar );
vtkCxxRevisionMacro(vtkKWToolbar, "$Revision: 1.35 $");

int vtkKWToolbarCommand(ClientData cd, Tcl_Interp *interp,
                       int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWToolbar::vtkKWToolbar()
{
  this->CommandFunction = vtkKWToolbarCommand;

  this->Frame   = vtkKWFrame::New();
  this->Handle  = vtkKWFrame::New();

  this->Widgets                   = vtkVector<vtkKWWidget*>::New();

  this->FlatAspect                = vtkKWToolbar::GetGlobalFlatAspect();
  this->WidgetsFlatAspect         = vtkKWToolbar::GetGlobalWidgetsFlatAspect();
  this->Resizable                 = 0;
  this->Expanding                 = 0;

  this->WidgetsFlatAdditionalPadX = 1;
  this->WidgetsFlatAdditionalPadY = 0;

#if defined(WIN32)
  this->PadX = 0;
  this->PadY = 0;
#else
  this->PadX = 1;
  this->PadY = 1;
#endif

  // This widget is used to keep track of default options

  this->DefaultOptionsWidget = vtkKWRadioButton::New();
}

//----------------------------------------------------------------------------
vtkKWToolbar::~vtkKWToolbar()
{
  if (this->Frame)
    {
    this->Frame->Delete();
    this->Frame = 0;
    }

  if (this->Handle)
    {
    this->Handle->Delete();
    this->Handle = 0;
    }

  if (this->Widgets)
    {
    this->Widgets->Delete();
    this->Widgets = NULL;
    }

  if (this->DefaultOptionsWidget)
    {
    this->DefaultOptionsWidget->Delete();
    this->DefaultOptionsWidget = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWToolbar::Create(vtkKWApplication *app)
{
  // Set the application

  if (this->IsCreated())
    {
    vtkErrorMacro("widget already created");
    return;
    }

  this->SetApplication(app);

  // Create the main frame for this widget

  this->Script("frame %s", this->GetWidgetName());

  this->Script("bind %s <Configure> {%s ScheduleResize}",
               this->GetWidgetName(), this->GetTclName());

  // Create the widgets container itself

  this->Frame->SetParent(this);
  this->Frame->Create(app, "");

  // Create a "toolbar handle"

  this->Handle->SetParent(this);
  this->Handle->Create(app, "-bd 2 -relief raised");

  // Create the default options repository (never packed, just a way
  // to keep track of default options)

  this->DefaultOptionsWidget->SetParent(this);
  this->DefaultOptionsWidget->Create(app, "");

  // Update aspect

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::AddWidget(vtkKWWidget *widget)
{
  if (this->Widgets->AppendItem(widget) == VTK_OK)
    {
    widget->SetEnabled(this->Enabled);
    this->UpdateWidgets();
    }
  else
    {
    vtkErrorMacro("Unable to add widget to toolbar");
    }
}

//----------------------------------------------------------------------------
void vtkKWToolbar::InsertWidget(vtkKWWidget *location, vtkKWWidget *widget)
{
  int res;
  if (!location)
    {
    res = this->Widgets->PrependItem(widget);
    }

  vtkIdType loc = 0;
  if (this->Widgets->FindItem(location, loc) == VTK_OK)
    {
    res = this->Widgets->InsertItem(loc, widget);
    }
  else
    {
    res = this->Widgets->PrependItem(widget);
    }

  if (res)
    {
    widget->SetEnabled(this->Enabled);
    this->UpdateWidgets();
    }
  else
    {
    vtkErrorMacro("Unable to insert widget in toolbar");
    }
}

//----------------------------------------------------------------------------
void vtkKWToolbar::RemoveWidget(vtkKWWidget *widget)
{
  vtkIdType loc = 0;
  if (this->Widgets->FindItem(widget, loc) == VTK_OK)
    {
    if (this->Widgets->RemoveItem(loc) == VTK_OK)
      {
      this->UpdateWidgets();
      return;
      }
    }
  vtkErrorMacro("Unable to remove widget from toolbar");
}

//----------------------------------------------------------------------------
vtkKWWidget* vtkKWToolbar::GetWidget(const char *name)
{
  vtkKWWidget *found = 0;

  if (name && this->Widgets)
    {
    const char *options[4] = { "-label", "-text", "-image", "-selectimage" };

    vtkVectorIterator<vtkKWWidget*>* it = this->Widgets->NewIterator();
    it->InitTraversal();
    while (!it->IsDoneWithTraversal())
      {
      vtkKWWidget* widget = 0;
      if (it->GetData(widget) == VTK_OK)
        {
        for (int i = 0; i < 4; i++)
          {
          if (widget->HasConfigurationOption(options[i]))
            {
            const char *option = 
              this->Script("%s cget %s", widget->GetWidgetName(), options[i]);
            if (!strcmp(name, option))
              {
              found = widget;
              break;
              }
            }
          }
        }
      it->GoToNextItem();
      }
    it->Delete();
    }

  return found;
}

//----------------------------------------------------------------------------
vtkKWWidget* vtkKWToolbar::AddRadioButtonImage(int value, 
                                               const char *image_name, 
                                               const char *select_image_name, 
                                               const char *variable_name, 
                                               vtkKWObject *object, 
                                               const char *method,
                                               const char *help,
                                               const char *extra)
{
  if (!this->IsCreated())
    {
    return 0;
    }

  vtkKWRadioButton *rb = vtkKWRadioButton::New();

  rb->SetParent(this->GetFrame());
  rb->Create(this->GetApplication(), "");
  rb->SetIndicator(0);
  rb->SetValue(value);

  if (image_name)
    {
    this->Script(
      "%s configure -highlightthickness 0 -image %s -selectimage %s", 
      rb->GetWidgetName(), 
      image_name, 
      select_image_name ? select_image_name : image_name);
    }

  if (object && method)
    {
    rb->SetCommand(object, method);
    }

  if (variable_name)
    {
    rb->SetVariableName(variable_name);
    }

  if (help)
    {
    rb->SetBalloonHelpString(help);
    }

  if (extra)
    {
    this->Script("%s configure %s", rb->GetWidgetName(), extra);
    }

  this->AddWidget(rb);

  rb->Delete();

  return rb;
}

//----------------------------------------------------------------------------
void vtkKWToolbar::ScheduleResize()
{  
  if (this->Expanding)
    {
    return;
    }
  this->Expanding = 1;
  this->Script("after idle {%s Resize}", this->GetTclName());
}

//----------------------------------------------------------------------------
void vtkKWToolbar::Resize()
{
  this->UpdateWidgets();
  this->Expanding = 0;
}

//----------------------------------------------------------------------------
void vtkKWToolbar::UpdateWidgetsAspect()
{
  if (this->Widgets->GetNumberOfItems() <= 0)
    {
    return;
    }

  vtkVectorIterator<vtkKWWidget*>* it = this->Widgets->NewIterator();
  ostrstream s;

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkKWWidget* widget = 0;

    // Change the relief of buttons (let's say that everything that
    // has a -command will qualify, -state could have been used, or
    // a match on the widget type, etc).

    if (it->GetData(widget) == VTK_OK && 
        widget->HasConfigurationOption("-command"))
      {
      int use_relief = widget->HasConfigurationOption("-relief");
      if (widget->HasConfigurationOption("-indicatoron"))
        {
        this->Script("%s cget -indicatoron", widget->GetWidgetName());
        use_relief = this->GetIntegerResult(this->GetApplication());
        }
        
      if (use_relief)
        {
        s << widget->GetWidgetName() << " config -relief " 
          << (this->WidgetsFlatAspect ? "flat" : "raised") << endl;
        }
      else
        {
        // Can not use -relief, try to hack -bd by specifying
        // an empty border as the negative current value (i.e.
        // the negative value will be handled as 0, but still will enable
        // us to retrieve the old value using abs() later on).

        if (widget->HasConfigurationOption("-bd"))
          {
          this->Script("%s cget -bd", widget->GetWidgetName());
          int bd = this->GetIntegerResult(this->GetApplication());
          s << widget->GetWidgetName() << " config -bd "
            << (this->WidgetsFlatAspect ? -abs(bd) : abs(bd)) << endl;
          }
        }

      // If radiobutton, remove the select color border in flat aspect

      if (widget->HasConfigurationOption("-selectcolor"))
        {
        if (this->WidgetsFlatAspect)
          {
          s << widget->GetWidgetName() << " config -selectcolor [" 
            << widget->GetWidgetName() << " cget -bg]" << endl; 
          }
        else
          {
          s << widget->GetWidgetName() << " config -selectcolor [" 
            << this->DefaultOptionsWidget->GetWidgetName() 
            << " cget -selectcolor]" << endl; 
          }
        }

      // Do not use active background in flat mode either

      if (widget->HasConfigurationOption("-activebackground"))
        {
        if (this->WidgetsFlatAspect)
          {
          s << widget->GetWidgetName() << " config -activebackground [" 
            << widget->GetWidgetName() << " cget -bg]" << endl; 
          }
        else
          {
          s << widget->GetWidgetName() << " config -activebackground [" 
            << this->DefaultOptionsWidget->GetWidgetName() 
            << " cget -activebackground]" << endl; 
          }
        }
      }
    it->GoToNextItem();
    }
  it->Delete();

  s << ends;
  this->Script(s.str());
  s.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWToolbar::ConstrainWidgetsLayout()
{
  if (this->Widgets->GetNumberOfItems() <= 0)
    {
    return;
    }

  int totReqWidth = 0;

  vtkVectorIterator<vtkKWWidget*>* it = this->Widgets->NewIterator();
    
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkKWWidget* widget = 0;
    if (it->GetData(widget) == VTK_OK)
      {
      this->Script("winfo reqwidth %s", widget->GetWidgetName());
      totReqWidth += this->GetIntegerResult(this->GetApplication()) + this->PadX;
      if (this->WidgetsFlatAspect)
        {
        totReqWidth += this->WidgetsFlatAdditionalPadX;
        }
      }
    it->GoToNextItem();
    }

  this->Script("winfo width %s", this->GetWidgetName());
  int width = this->GetIntegerResult(this->GetApplication());

  int widthWidget = totReqWidth / this->Widgets->GetNumberOfItems();
  int numPerRow = width / widthWidget;

  if ( numPerRow > 0 )
    {
    int row = 0, num = 0;
    ostrstream s;

    it->InitTraversal();
    while (!it->IsDoneWithTraversal())
      {
      vtkKWWidget* widget = 0;
      if (it->GetData(widget) == VTK_OK)
        {
        s << "grid " << widget->GetWidgetName() << " -row " 
          << row << " -column " << num << " -sticky news "
          << " -ipadx " 
          << (this->PadX + (this->WidgetsFlatAspect ? 
                            this->WidgetsFlatAdditionalPadX : 0))
          << " -ipady "
          << (this->PadY + (this->WidgetsFlatAspect ? 
                            this->WidgetsFlatAdditionalPadY : 0))
          << endl;
        num++;
        if ( num == numPerRow ) 
          { 
          row++; 
          num=0;
          }
        }
      it->GoToNextItem();
      }
    s << ends;
    this->Script(s.str());
    s.rdbuf()->freeze(0);
    }
  it->Delete();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::UpdateWidgetsLayout()
{
  if (!this->IsCreated() || this->Widgets->GetNumberOfItems() <= 0)
    {
    return;
    }

  this->GetFrame()->UnpackChildren();

  // If this toolbar is resizable, then constrain it to the current size

  if (this->Resizable)
    {
    this->ConstrainWidgetsLayout();
    return;
    }

  ostrstream s;
  s << "grid "; 

  vtkVectorIterator<vtkKWWidget*>* it = this->Widgets->NewIterator();

  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkKWWidget* widget = 0;
    if (it->GetData(widget) == VTK_OK)
      {
      s << " " << widget->GetWidgetName();
      }
    it->GoToNextItem();
    }
  it->Delete();

  s << " -sticky news -row 0 "
    << " -ipadx " 
    << (this->PadX + (this->WidgetsFlatAspect ? 
                      this->WidgetsFlatAdditionalPadX : 0))
    << " -ipady "
    << (this->PadY + (this->WidgetsFlatAspect ? 
                      this->WidgetsFlatAdditionalPadY : 0))
    << ends;

  this->Script(s.str());
  s.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWToolbar::UpdateWidgets()
{
  this->UpdateWidgetsAspect();
  this->UpdateWidgetsLayout();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::SetPadX(int arg)
{
  if (arg == this->PadX)
    {
    return;
    }

  this->PadX = arg;
  this->Modified();

  this->UpdateWidgetsLayout();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::SetPadY(int arg)
{
  if (arg == this->PadX)
    {
    return;
    }

  this->PadY = arg;
  this->Modified();

  this->UpdateWidgetsLayout();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::SetWidgetsFlatAdditionalPadX(int arg)
{
  if (arg == this->WidgetsFlatAdditionalPadX)
    {
    return;
    }

  this->WidgetsFlatAdditionalPadX = arg;
  this->Modified();

  this->UpdateWidgetsLayout();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::SetWidgetsFlatAdditionalPadY(int arg)
{
  if (arg == this->WidgetsFlatAdditionalPadX)
    {
    return;
    }

  this->WidgetsFlatAdditionalPadY = arg;
  this->Modified();

  this->UpdateWidgetsLayout();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::Update()
{
  this->UpdateEnableState();

  const char *common_opts = " -side left -anchor nw -fill both -expand n";

  if (this->FlatAspect)
    {
    this->Script("%s config -relief flat -bd 0", this->GetWidgetName());

    this->Script("pack %s -ipadx 0 -ipady 0 -padx 0 -pady 0 %s",
                 this->Frame->GetWidgetName(), common_opts);
    }
  else
    {
    this->Script("%s config -relief raised -bd 1", this->GetWidgetName());

    this->Script("pack %s -ipadx 1 -ipady 1 -padx 0 -pady 0 %s",
                 this->Frame->GetWidgetName(), common_opts);
    }
}

//----------------------------------------------------------------------------
void vtkKWToolbar::SetFlatAspect(int f)
{
  if (this->FlatAspect == f)
    {
    return;
    }

  this->FlatAspect = f;
  this->Modified();

  this->Update();
  this->UpdateWidgets();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::SetWidgetsFlatAspect(int f)
{
  if (this->WidgetsFlatAspect == f)
    {
    return;
    }

  this->WidgetsFlatAspect = f;
  this->Modified();

  this->UpdateWidgets();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::SetResizable(int r)
{
  if (this->Resizable == r)
    {
    return;
    }

  this->Resizable = r;
  this->Modified();

  this->Update();
  this->UpdateWidgets();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  vtkVectorIterator<vtkKWWidget*>* it = this->Widgets->NewIterator();
  it->InitTraversal();
  while (!it->IsDoneWithTraversal())
    {
    vtkKWWidget* widget = 0;
    if (it->GetData(widget) == VTK_OK)
      {
      widget->SetEnabled(this->Enabled);
      }
    it->GoToNextItem();
    }
  it->Delete();
}

//----------------------------------------------------------------------------
void vtkKWToolbar::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Frame: " << this->Frame << endl;
  os << indent << "Resizable: " << (this->Resizable ? "On" : "Off") << endl;
  os << indent << "FlatAspect: " << (this->FlatAspect ? "On" : "Off") << endl;
  os << indent << "WidgetsFlatAspect: " << (this->WidgetsFlatAspect ? "On" : "Off") << endl;
  os << indent << "PadX: " << this->PadX << endl;
  os << indent << "PadY: " << this->PadY << endl;
  os << indent << "WidgetsFlatAdditionalPadX: " << this->WidgetsFlatAdditionalPadX << endl;
  os << indent << "WidgetsFlatAdditionalPadY: " << this->WidgetsFlatAdditionalPadY << endl;
}

