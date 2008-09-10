/*=========================================================================

  Module:    $RCSfile: vtkKWMenuButton.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMenuButton.h"

#include "vtkKWMenu.h"
#include "vtkObjectFactory.h"
#include "vtkKWIcon.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWOptions.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMenuButton );
vtkCxxRevisionMacro(vtkKWMenuButton, "$Revision: 1.44 $");

//----------------------------------------------------------------------------
vtkKWMenuButton::vtkKWMenuButton()
{
  this->CurrentValue      = NULL;
  this->Menu              = vtkKWMenu::New();
  this->MaximumLabelWidth = 0;
}

//----------------------------------------------------------------------------
vtkKWMenuButton::~vtkKWMenuButton()
{
  this->SetCurrentValue(NULL);

  if (this->Menu)
    {
    this->Menu->Delete();
    this->Menu = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::CreateWidget()
{
  // Call the superclass to create the widget and set the appropriate flags

  if (!vtkKWWidget::CreateSpecificTkWidget(this, 
        "menubutton", "-indicatoron 1 -relief raised -bd 2 -highlightthickness 0 -anchor center -direction flush"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  this->Menu->SetParent(this);
  this->Menu->Create();
  this->Menu->SetTearOff(0);

  this->SetConfigurationOption("-menu", this->Menu->GetWidgetName());

  this->Script("set %s_RB_group {}", this->Menu->GetTclName());
  this->Script(
    "trace variable %s_RB_group w {%s TracedVariableChangedCallback}",
    this->Menu->GetTclName(), 
    this->GetTclName());

  this->AddCallbackCommandObservers();
}

//----------------------------------------------------------------------------
const char *vtkKWMenuButton::GetValue()
{
  if (this->IsCreated())
    {
    // Why do we re-assign to CurrentValue each time GetValue() is 
    // called ?
    // That's because the value of the internal variable is set by Tk
    // through the -variable settings of the radiobutton entries that
    // have been added to the menu. Therefore, if a radiobutton entry has
    // a command that will use the value (very likely), there is no
    // guarantee the variable has been changed before or after calling the
    // callback. To ensure it is true, always refresh the value from
    // the variable itself.
    this->SetCurrentValue(
      this->Script("set %s_RB_group", this->Menu->GetTclName()));
    }
  return this->CurrentValue;  
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetValue(const char *s)
{
  if (this->IsCreated() && s && strcmp(s, this->GetValue()))
    {
    this->Script("set %s_RB_group {%s}", this->Menu->GetTclName(), s);

    if (this->Menu && *s)
      {
      // TODO: this does not support multiple-level of menus (i.e. cascade)
      int nb_items = this->Menu->GetNumberOfItems();
      for (int i = 0; i < nb_items; i++)
        {
        const char *image = this->Menu->GetItemOption(i, "-image");
        if (image && !strcmp(image, s))
          {
          this->SetConfigurationOption("-image", s);
          break;
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::NextValue()
{
  const char *value = this->GetValue();
  if (!this->Menu || !this->Menu->IsCreated())
    {
    return;
    }
  int pos;
  if (!this->Menu->HasItem(value))
    {
    pos = 0;
    }
  else
    {
    pos = this->Menu->GetIndexOfItem(value) + 1;
    if (pos >= this->Menu->GetNumberOfItems())
      {
      pos = 0;
      }
    }
  this->Menu->InvokeItem(pos);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::PreviousValue()
{
  const char *value = this->GetValue();
  if (!this->Menu || !this->Menu->IsCreated() || !this->Menu->HasItem(value))
    {
    return;
    }
  int pos;
  if (!this->Menu->HasItem(value))
    {
    pos = this->Menu->GetNumberOfItems() - 1;
    }
  else
    {
    pos = this->Menu->GetIndexOfItem(value) - 1;
    if (pos < 0)
      {
      pos = this->Menu->GetNumberOfItems() - 1;
      }
    }
  this->Menu->InvokeItem(pos);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::TracedVariableChangedCallback(
  const char *, const char *, const char *)
{
  this->UpdateMenuButtonLabel();
}

//----------------------------------------------------------------------------
int vtkKWMenuButton::UpdateMenuButtonLabelFromMenu(
  const char *varname, const char *value, vtkKWMenu *menu)
{
  const char *label = 
    menu->GetItemLabel(
      menu->GetIndexOfItemWithVariableAndSelectedValue(varname, value));

  if (label)
    {
    if (this->MaximumLabelWidth <= 0)
      {
      this->SetConfigurationOption("-text", label);
      }
    else
      {
      vtksys_stl::string cropped = vtksys::SystemTools::CropString(
        label, (size_t)this->MaximumLabelWidth);
      this->SetConfigurationOption("-text", cropped.c_str());
      }
    return 1;
    }

  // Not found? Try harder in the submenus (cascade)

  int i, nb_items = menu->GetNumberOfItems();
  for (i = 0; i < nb_items; i++)
    {
    if (menu->GetItemType(i) == vtkKWMenu::CascadeItemType &&
        this->UpdateMenuButtonLabelFromMenu(
          varname, value, this->Menu->GetItemCascade(i)))
      {
      return 1;
      }
    }
  
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::UpdateMenuButtonLabel()
{
  if (!this->IsCreated())
    {
    return;
    }

  // When a radiobutton entry is inserted in the internal menu, we override
  // its selected value to match the label of the entry, for convenience.
  // However, if the user choose to reset that selected value to something
  // else, chances are, it is still the label that should be displayed.
  // The code below takes care of it (if the user didn't interfere, it will
  // return the selected value, which was already set to match the label)

  vtksys_stl::string varname(this->Menu->GetTclName());
  varname += "_RB_group";

  vtksys_stl::string value(this->GetValue());

  int found = this->UpdateMenuButtonLabelFromMenu(
    varname.c_str(), value.c_str(), this->Menu);
  if (!found)
    {
    this->SetConfigurationOption("-text", "");
    }
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetMaximumLabelWidth(int arg)
{ 
  if (this->MaximumLabelWidth == arg)
    {
    return;
    }

  this->MaximumLabelWidth = arg;
  this->Modified();

  this->UpdateMenuButtonLabel();
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetIndicatorVisibility(int arg)
{
  this->SetConfigurationOptionAsInt("-indicatoron", arg);
}

//----------------------------------------------------------------------------
int vtkKWMenuButton::GetIndicatorVisibility()
{
  return this->GetConfigurationOptionAsInt("-indicatoron");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetWidth(int width)
{
  this->SetConfigurationOptionAsInt("-width", width);
}

//----------------------------------------------------------------------------
int vtkKWMenuButton::GetWidth()
{
  return this->GetConfigurationOptionAsInt("-width");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::GetBackgroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-background", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMenuButton::GetBackgroundColor()
{
  return this->GetConfigurationOptionAsColor("-background");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetBackgroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-background", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::GetForegroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-foreground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMenuButton::GetForegroundColor()
{
  return this->GetConfigurationOptionAsColor("-foreground");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetForegroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-foreground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetHighlightThickness(int width)
{
  this->SetConfigurationOptionAsInt("-highlightthickness", width);
}

//----------------------------------------------------------------------------
int vtkKWMenuButton::GetHighlightThickness()
{
  return this->GetConfigurationOptionAsInt("-highlightthickness");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::GetActiveBackgroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-activebackground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMenuButton::GetActiveBackgroundColor()
{
  return this->GetConfigurationOptionAsColor("-activebackground");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetActiveBackgroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-activebackground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::GetActiveForegroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-activeforeground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMenuButton::GetActiveForegroundColor()
{
  return this->GetConfigurationOptionAsColor("-activeforeground");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetActiveForegroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-activeforeground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::GetDisabledForegroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-disabledforeground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMenuButton::GetDisabledForegroundColor()
{
  return this->GetConfigurationOptionAsColor("-disabledforeground");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetDisabledForegroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-disabledforeground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetBorderWidth(int width)
{
  this->SetConfigurationOptionAsInt("-bd", width);
}

//----------------------------------------------------------------------------
int vtkKWMenuButton::GetBorderWidth()
{
  return this->GetConfigurationOptionAsInt("-bd");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetRelief(int relief)
{
  this->SetConfigurationOption(
    "-relief", vtkKWOptions::GetReliefAsTkOptionValue(relief));
}

void vtkKWMenuButton::SetReliefToRaised()     
{ 
  this->SetRelief(vtkKWOptions::ReliefRaised); 
};
void vtkKWMenuButton::SetReliefToSunken() 
{ 
  this->SetRelief(vtkKWOptions::ReliefSunken); 
};
void vtkKWMenuButton::SetReliefToFlat() 
{ 
  this->SetRelief(vtkKWOptions::ReliefFlat); 
};
void vtkKWMenuButton::SetReliefToRidge() 
{ 
  this->SetRelief(vtkKWOptions::ReliefRidge); 
};
void vtkKWMenuButton::SetReliefToSolid() 
{ 
  this->SetRelief(vtkKWOptions::ReliefSolid); 
};
void vtkKWMenuButton::SetReliefToGroove() 
{ 
  this->SetRelief(vtkKWOptions::ReliefGroove); 
};

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetFont(const char *font)
{
  this->SetConfigurationOption("-font", font);
}

//----------------------------------------------------------------------------
const char* vtkKWMenuButton::GetFont()
{
  return this->GetConfigurationOption("-font");
}

//----------------------------------------------------------------------------
int vtkKWMenuButton::GetRelief()
{
  return vtkKWOptions::GetReliefFromTkOptionValue(
    this->GetConfigurationOption("-relief"));
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetPadX(int arg)
{
  this->SetConfigurationOptionAsInt("-padx", arg);
}

//----------------------------------------------------------------------------
int vtkKWMenuButton::GetPadX()
{
  return this->GetConfigurationOptionAsInt("-padx");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetPadY(int arg)
{
  this->SetConfigurationOptionAsInt("-pady", arg);
}

//----------------------------------------------------------------------------
int vtkKWMenuButton::GetPadY()
{
  return this->GetConfigurationOptionAsInt("-pady");
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetAnchor(int anchor)
{
  this->SetConfigurationOption(
    "-anchor", vtkKWOptions::GetAnchorAsTkOptionValue(anchor));
}

void vtkKWMenuButton::SetAnchorToNorth() 
{ 
  this->SetAnchor(vtkKWOptions::AnchorNorth); 
};
void vtkKWMenuButton::SetAnchorToNorthEast() 
{ 
  this->SetAnchor(vtkKWOptions::AnchorNorthEast); 
};
void vtkKWMenuButton::SetAnchorToEast() 
{ 
  this->SetAnchor(vtkKWOptions::AnchorEast); 
};
void vtkKWMenuButton::SetAnchorToSouthEast() 
{ 
  this->SetAnchor(vtkKWOptions::AnchorSouthEast); 
};
void vtkKWMenuButton::SetAnchorToSouth() 
{ 
  this->SetAnchor(vtkKWOptions::AnchorSouth); 
};
void vtkKWMenuButton::SetAnchorToSouthWest() 
{ 
  this->SetAnchor(vtkKWOptions::AnchorSouthWest); 
};
void vtkKWMenuButton::SetAnchorToWest() 
{ 
  this->SetAnchor(vtkKWOptions::AnchorWest); 
};
void vtkKWMenuButton::SetAnchorToNorthWest() 
{ 
  this->SetAnchor(vtkKWOptions::AnchorNorthWest); 
};
void vtkKWMenuButton::SetAnchorToCenter() 
{ 
  this->SetAnchor(vtkKWOptions::AnchorCenter); 
};

//----------------------------------------------------------------------------
int vtkKWMenuButton::GetAnchor()
{
  return vtkKWOptions::GetAnchorFromTkOptionValue(
    this->GetConfigurationOption("-anchor"));
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetImageToPredefinedIcon(int icon_index)
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  this->SetImageToIcon(icon);
  icon->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetImageToIcon(vtkKWIcon* icon)
{
  if (icon)
    {
    this->SetImageToPixels(
      icon->GetData(), 
      icon->GetWidth(), icon->GetHeight(), icon->GetPixelSize());
    }
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetImageToPixels(const unsigned char* pixels, 
                                       int width, 
                                       int height,
                                       int pixel_size,
                                       unsigned long buffer_length)
{
  vtkKWTkUtilities::SetImageOptionToPixels(
    this, pixels, width, height, pixel_size, buffer_length);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::SetCompoundMode(int mode)
{
  this->SetConfigurationOption(
    "-compound", vtkKWOptions::GetCompoundModeAsTkOptionValue(mode));
}

void vtkKWMenuButton::SetCompoundModeToNone()     
{ 
  this->SetCompoundMode(vtkKWOptions::CompoundModeNone); 
};
void vtkKWMenuButton::SetCompoundModeToLeft() 
{ 
  this->SetCompoundMode(vtkKWOptions::CompoundModeLeft); 
};
void vtkKWMenuButton::SetCompoundModeToCenter() 
{ 
  this->SetCompoundMode(vtkKWOptions::CompoundModeCenter); 
};
void vtkKWMenuButton::SetCompoundModeToRight() 
{ 
  this->SetCompoundMode(vtkKWOptions::CompoundModeRight); 
};
void vtkKWMenuButton::SetCompoundModeToTop() 
{ 
  this->SetCompoundMode(vtkKWOptions::CompoundModeTop); 
};
void vtkKWMenuButton::SetCompoundModeToBottom() 
{ 
  this->SetCompoundMode(vtkKWOptions::CompoundModeBottom); 
};

//----------------------------------------------------------------------------
int vtkKWMenuButton::GetCompoundMode()
{
  return vtkKWOptions::GetCompoundModeFromTkOptionValue(
    this->GetConfigurationOption("-compound"));
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->SetState(this->GetEnabled());
  this->PropagateEnableState(this->Menu);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::AddCallbackCommandObservers()
{
  this->Superclass::AddCallbackCommandObservers();

  this->AddCallbackCommandObserver(
    this->Menu, vtkKWMenu::RadioButtonItemAddedEvent);

  this->AddCallbackCommandObserver(
    this->Menu, vtkKWMenu::CascadeItemAddedEvent);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::RemoveCallbackCommandObservers()
{
  this->Superclass::RemoveCallbackCommandObservers();

  this->RemoveCallbackCommandObserver(
    this->Menu, vtkKWMenu::RadioButtonItemAddedEvent);

  this->RemoveCallbackCommandObserver(
    this->Menu, vtkKWMenu::CascadeItemAddedEvent);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::ProcessCallbackCommandEvents(vtkObject *caller,
                                                   unsigned long event,
                                                   void *calldata)
{
  // Make sure all radiobuttons share the same variable as we 
  // are using internally.

  vtkKWMenu *caller_menu = vtkKWMenu::SafeDownCast(caller);
  vtkKWMenu *cascade_menu = NULL;
  vtksys_stl::string varname;

  if (caller_menu)
    {
    int index = *(static_cast<int*>(calldata));
    switch (event)
      {
      case vtkKWMenu::RadioButtonItemAddedEvent:
        varname = this->Menu->GetTclName();
        varname += "_RB_group";
        caller_menu->SetItemVariable(index, varname.c_str());
        caller_menu->SetItemSelectedValue(
          index, caller_menu->GetItemLabel(index));
        break;

      case vtkKWMenu::CascadeItemAddedEvent:
        cascade_menu = caller_menu->GetItemCascade(index);
        if (cascade_menu)
          {
          this->AddCallbackCommandObserver(
            cascade_menu, vtkKWMenu::RadioButtonItemAddedEvent);
          this->AddCallbackCommandObserver(
            cascade_menu, vtkKWMenu::CascadeItemAddedEvent);
          }
        break;
      }
    }

  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}

//----------------------------------------------------------------------------
void vtkKWMenuButton::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Menu: " << this->Menu << endl;
  os << indent << "MaximumLabelWidth: " << this->MaximumLabelWidth << endl;
}

