/*=========================================================================

  Module:    vtkKWEntry.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWEntry.h"

#include "vtkKWOptions.h"
#include "vtkObjectFactory.h"

#include <vtksys/stl/string>

#if defined (__BORLANDC__)  && (__BORLANDC__ >= 0x0580)
#include <ctype.h> // for isalpha
#endif

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWEntry);
vtkCxxRevisionMacro(vtkKWEntry, "$Revision: 1.97 $");

//----------------------------------------------------------------------------
vtkKWEntry::vtkKWEntry()
{
  this->Width               = -1;
  this->ReadOnly            = 0;
  this->InternalValueString = NULL;
  this->Command             = NULL;
  this->ValidationCommand   = NULL;
  this->RestrictValue       = vtkKWEntry::RestrictNone;
  this->CommandTrigger      = (vtkKWEntry::TriggerOnFocusOut | 
                               vtkKWEntry::TriggerOnReturnKey);
}

//----------------------------------------------------------------------------
vtkKWEntry::~vtkKWEntry()
{
  if (this->Command)
    {
    delete [] this->Command;
    this->Command = NULL;
    }

  if (this->ValidationCommand)
    {
    delete [] this->ValidationCommand;
    this->ValidationCommand = NULL;
    }

  this->SetInternalValueString(NULL);
}

//----------------------------------------------------------------------------
void vtkKWEntry::CreateWidget()
{
  // Call the superclass to set the appropriate flags then create manually

  if (!vtkKWWidget::CreateSpecificTkWidget(this, 
        "entry", "-highlightthickness 0 -bd 2 -background white"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  this->Script("%s configure -textvariable %s_Value",
               this->GetWidgetName(), this->GetTclName());

  // Design choice: we assume a keypress is meant for this widget only

  this->Script(
    "bind Entry <KeyPress> {::tk::CancelRepeat ; ::tk::EntryInsert %%W %%A; break}");

  this->Configure();
}

//----------------------------------------------------------------------------
void vtkKWEntry::Configure()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (!this->ReadOnly &&
      this->CommandTrigger & vtkKWEntry::TriggerOnFocusOut)
    {
    this->SetBinding("<FocusOut>", this, "ValueCallback");
    }
  else
    {
    this->RemoveBinding("<FocusOut>", this, "ValueCallback");
    }

  if (this->CommandTrigger & vtkKWEntry::TriggerOnReturnKey)
    {
    this->SetBinding("<Return>", this, "ValueCallback");
    }
  else
    {
    this->RemoveBinding("<Return>", this, "ValueCallback");
    }

  if (this->Width >= 0)
    {
    this->SetConfigurationOptionAsInt("-width", this->Width);
    }

  // When the entry is unmapped, trigger the callback as well. Here is
  // the rationale: a user may have edited the value in the entry but
  // neither did he press Enter or clicked on any other widget that
  // made that entry lose its focus. When the user closes the window, or when
  // the user switch to a different notebook tab than the one in which the 
  // entry was packled, or when the user skip to a different wizard step
  // than the one in which the entry was packed, then this entry should
  // definitely be validated/acknowledged. This is what this event will
  // take care of.

  this->SetBinding("<Unmap>", this, "ValueCallback");

  this->ConfigureValidation();

  if (this->CommandTrigger & vtkKWEntry::TriggerOnAnyChange)
    {
    this->ConfigureTraceCallback(1);
    }
}

//----------------------------------------------------------------------------
void vtkKWEntry::ConfigureTraceCallback(int state)
{
  if (!this->IsCreated())
    {
    return;
    }

  this->Script(
    "trace %s variable %s_Value write {%s TracedVariableChangedCallback}",
    (state ? "add" : "remove"),
    this->GetTclName(), this->GetTclName());
}

//----------------------------------------------------------------------------
const char* vtkKWEntry::GetValue()
{
  if (!this->IsCreated())
    {
    return NULL;
    }

  const char *val = this->Script("%s get", this->GetWidgetName());
  this->SetInternalValueString(this->ConvertTclStringToInternalString(val));
  return this->GetInternalValueString();
}

//----------------------------------------------------------------------------
int vtkKWEntry::GetValueAsInt()
{
  if (!this->IsCreated())
    {
    return 0;
    }

  // Do not call this->GetValue() here to speed up things (GetValue() copies
  // the buffer to a string each time, for safety reasons)

  const char *val = this->Script("%s get", this->GetWidgetName());
  if (!val || !*val)
    {
    return 0;
    }
  return atoi(val);
}

//----------------------------------------------------------------------------
double vtkKWEntry::GetValueAsDouble()
{
  if (!this->IsCreated())
    {
    return 0;
    }

  // Do not call this->GetValue() here to speed up things (GetValue() copies
  // the buffer to a string each time, for safety reasons)

  const char *val = this->Script("%s get", this->GetWidgetName());
  if (!val || !*val)
    {
    return 0;
    }
  return atof(val);
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetValue(const char *s)
{
  if (!this->IsAlive())
    {
    return;
    }

  // Save the old -validate option, which seems to be reset to none
  // whenever the entry was set to something invalid
  vtksys_stl::string old_validate;
  if (this->RestrictValue != vtkKWEntry::RestrictNone)
    {
    old_validate = this->GetConfigurationOption("-validate");
    }
  
  int old_state = this->GetState();
  this->SetStateToNormal();

  // We need to empty the entry first before inserting the value
  // Let's make sure we are not triggering the trace callback while
  // doing so.

  if (this->CommandTrigger & vtkKWEntry::TriggerOnAnyChange)
    {
    this->ConfigureTraceCallback(0);
    }
  this->Script("%s delete 0 end", this->GetWidgetName());
  if (this->CommandTrigger & vtkKWEntry::TriggerOnAnyChange)
    {
    this->ConfigureTraceCallback(1);
    }

  if (s)
    {
    const char *val = this->ConvertInternalStringToTclString(
      s, vtkKWCoreWidget::ConvertStringEscapeInterpretable);
    if (this->RestrictValue == vtkKWEntry::RestrictInteger)
      {
      this->Script("if {[string is integer \"%s\"]} {%s insert 0 \"%s\"}", 
                   val ? val : "", this->GetWidgetName(), val ? val : "");
      }
    else if (this->RestrictValue == vtkKWEntry::RestrictHexadecimal)
      {
      this->Script("if {[string is xdigit \"%s\"]} {%s insert 0 \"%s\"}", 
                   val ? val : "", this->GetWidgetName(), val ? val : "");
      }
    else if (this->RestrictValue == vtkKWEntry::RestrictDouble)
      {
      this->Script("if {[string is double \"%s\"]} {%s insert 0 \"%s\"}", 
                   val ? val : "", this->GetWidgetName(), val ? val : "");
      }
    else
      {
      this->Script("%s insert 0 \"%s\"", 
                   this->GetWidgetName(), val ? val : "");
      }
    }

  this->SetState(old_state);

  if (this->RestrictValue != vtkKWEntry::RestrictNone)
    {
    this->SetConfigurationOption("-validate", old_validate.c_str());
    }

  this->InvokeEvent(vtkKWEntry::EntryValueChangedEvent, (void*)s);
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetValueAsInt(int i)
{
  if (!this->IsCreated())
    {
    return;
    }

  // Do not call this->GetValue() here to speed up things (GetValue() copies
  // the buffer to a string each time, for safety reasons)

  const char *val = this->Script("%s get", this->GetWidgetName());
  if (val && *val && !isalpha(*val) && i == atoi(val))
    {
    return;
    }

  char tmp[1024];
  sprintf(tmp, "%d", i);
  this->SetValue(tmp);
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetValueAsDouble(double f)
{
  if (!this->IsCreated())
    {
    return;
    }

  // Do not call this->GetValue() here to speed up things (GetValue() copies
  // the buffer to a string each time, for safety reasons)

  const char *val = this->Script("%s get", this->GetWidgetName());
  if (val && *val && !isalpha(*val) && f == atof(val))
    {
    return;
    }

  char tmp[1024];
  sprintf(tmp, "%.5g", f);
  this->SetValue(tmp);
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetValueAsFormattedDouble(double f, int size)
{
  if (!this->IsCreated())
    {
    return;
    }

  // Do not call this->GetValue() here to speed up things (GetValue() copies
  // the buffer to a string each time, for safety reasons)

  const char *val = this->Script("%s get", this->GetWidgetName());
  if (val && *val && !isalpha(*val) && f == atof(val))
    {
    return;
    }

  char format[1024];
  sprintf(format, "%%.%dg", size);

  char tmp[1024];
  sprintf(tmp, format, f);

  this->SetValue(tmp);
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetHexadecimalValueAsRGB(int r, int g, int b)
{
  int old_r, old_g, old_b;
  this->GetHexadecimalValueAsRGB(old_r, old_g, old_b);
  if (old_r < 0 || old_g < 0 || old_b < 0 ||
      r != old_r || g != old_g || b != old_b)
    {
    static char buffer[20];
    sprintf(buffer, "%02x%02x%02x", r, g, b);
    this->SetValue(buffer);
    }
}

//----------------------------------------------------------------------------
void vtkKWEntry::GetHexadecimalValueAsRGB(int &r, int &g, int &b)
{
  const char *value = this->GetValue();
  if (!value || !*value || strlen(value) != 6 ||
      sscanf(value, "%02x%02x%02x", &r, &g, &b) != 3)
    {
    r = g = b = -1;
    }
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetReadOnly(int arg)
{
  if (this->ReadOnly == arg)
    {
    return;
    }

  this->ReadOnly = arg;
  this->Modified();
  this->Configure();
  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetRestrictValue(int arg)
{
  if (this->RestrictValue == arg)
    {
    return;
    }

  this->RestrictValue = arg;
  this->Modified();

  this->ConfigureValidation();
}

void vtkKWEntry::SetRestrictValueToInteger()
{ 
  this->SetRestrictValue(vtkKWEntry::RestrictInteger); 
}

void vtkKWEntry::SetRestrictValueToDouble()
{ 
  this->SetRestrictValue(vtkKWEntry::RestrictDouble); 
}

void vtkKWEntry::SetRestrictValueToHexadecimal()
{ 
  this->SetRestrictValue(vtkKWEntry::RestrictHexadecimal); 
}

void vtkKWEntry::SetRestrictValueToNone()
{ 
  this->SetRestrictValue(vtkKWEntry::RestrictNone); 
}

//----------------------------------------------------------------------------
void vtkKWEntry::ConfigureValidation()
{
  if (!this->IsCreated())
    {
    return;
    }

  if (this->RestrictValue == vtkKWEntry::RestrictNone &&
      (!this->ValidationCommand || !*this->ValidationCommand))
    {
    this->SetConfigurationOption("-validate", "none");
    }
  else
    {
    this->SetConfigurationOption("-validate", "all");
    vtksys_stl::string command(this->GetTclName());
    command += " ValidationCallback {%P}";
    this->SetConfigurationOption("-validatecommand", command.c_str());
    }
}

//----------------------------------------------------------------------------
int vtkKWEntry::ValidationCallback(const char *value)
{
  int res = 1;
  if (this->RestrictValue == vtkKWEntry::RestrictInteger)
    {
    // Sadly we have to use the "append 0" trick to allow people to enter
    // a negative sign
    res &= atoi(this->Script(
          "expr {[string is integer %s] || [string is integer \"%s0\"]}", 
          value, value));
    }
  else if (this->RestrictValue == vtkKWEntry::RestrictHexadecimal)
    {
    res &= atoi(this->Script("expr {[string is xdigit %s]}", value));
    }
  else if (this->RestrictValue == vtkKWEntry::RestrictDouble)
    {
    // Sadly we have to use the "append 0" trick to allow people to enter
    // a negative sign or decimal point
    res &= atoi(this->Script(
          "expr {[string is double %s] || [string is double \"%s0\"]}", 
          value, value));
    }
  if (!res)
    {
    return 0;
    }
  if (this->ValidationCommand && *this->ValidationCommand)
    {
    res &= this->InvokeValidationCommand(value);
    }
  return res;
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetWidth(int width)
{
  if (this->Width == width)
    {
    return;
    }

  this->Width = width;
  this->Modified();

  if (this->IsCreated() && this->Width >= 0)
    {
    this->SetConfigurationOptionAsInt("-width", this->Width);
    }
}

//----------------------------------------------------------------------------
void vtkKWEntry::GetBackgroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-background", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWEntry::GetBackgroundColor()
{
  return this->GetConfigurationOptionAsColor("-background");
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetBackgroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-background", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWEntry::GetForegroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-foreground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWEntry::GetForegroundColor()
{
  return this->GetConfigurationOptionAsColor("-foreground");
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetForegroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-foreground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWEntry::GetDisabledBackgroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-disabledbackground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWEntry::GetDisabledBackgroundColor()
{
  return this->GetConfigurationOptionAsColor("-disabledbackground");
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetDisabledBackgroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-disabledbackground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWEntry::GetDisabledForegroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-disabledforeground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWEntry::GetDisabledForegroundColor()
{
  return this->GetConfigurationOptionAsColor("-disabledforeground");
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetDisabledForegroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-disabledforeground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWEntry::GetReadOnlyBackgroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-readonlybackground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWEntry::GetReadOnlyBackgroundColor()
{
  return this->GetConfigurationOptionAsColor("-readonlybackground");
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetReadOnlyBackgroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-readonlybackground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetHighlightThickness(int width)
{
  this->SetConfigurationOptionAsInt("-highlightthickness", width);
}

//----------------------------------------------------------------------------
int vtkKWEntry::GetHighlightThickness()
{
  return this->GetConfigurationOptionAsInt("-highlightthickness");
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetBorderWidth(int width)
{
  this->SetConfigurationOptionAsInt("-bd", width);
}

//----------------------------------------------------------------------------
int vtkKWEntry::GetBorderWidth()
{
  return this->GetConfigurationOptionAsInt("-bd");
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetPasswordMode(int arg)
{
  if (this->GetPasswordMode() == arg)
    {
    return;
    }

  this->SetConfigurationOption("-show", arg ? "*" : "");
}

//----------------------------------------------------------------------------
int vtkKWEntry::GetPasswordMode()
{
  const char *res = this->GetConfigurationOption("-show");
  if (res && *res)
    {
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetRelief(int relief)
{
  this->SetConfigurationOption(
    "-relief", vtkKWOptions::GetReliefAsTkOptionValue(relief));
}

void vtkKWEntry::SetReliefToRaised()     
{ 
  this->SetRelief(vtkKWOptions::ReliefRaised); 
};
void vtkKWEntry::SetReliefToSunken() 
{ 
  this->SetRelief(vtkKWOptions::ReliefSunken); 
};
void vtkKWEntry::SetReliefToFlat() 
{ 
  this->SetRelief(vtkKWOptions::ReliefFlat); 
};
void vtkKWEntry::SetReliefToRidge() 
{ 
  this->SetRelief(vtkKWOptions::ReliefRidge); 
};
void vtkKWEntry::SetReliefToSolid() 
{ 
  this->SetRelief(vtkKWOptions::ReliefSolid); 
};
void vtkKWEntry::SetReliefToGroove() 
{ 
  this->SetRelief(vtkKWOptions::ReliefGroove); 
};

//----------------------------------------------------------------------------
int vtkKWEntry::GetRelief()
{
  return vtkKWOptions::GetReliefFromTkOptionValue(
    this->GetConfigurationOption("-relief"));
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetFont(const char *font)
{
  this->SetConfigurationOption("-font", font);
}

//----------------------------------------------------------------------------
const char* vtkKWEntry::GetFont()
{
  return this->GetConfigurationOption("-font");
}

//----------------------------------------------------------------------------
void vtkKWEntry::ValueCallback()
{
  this->InvokeCommand(this->GetValue());
}

//----------------------------------------------------------------------------
void vtkKWEntry::TracedVariableChangedCallback(
  const char *, const char *, const char *)
{
  if (this->CommandTrigger & vtkKWEntry::TriggerOnAnyChange)
    {
    this->ValueCallback();
    }
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetCommandTrigger(int arg)
{
  if (this->CommandTrigger == arg)
    {
    return;
    }

  this->CommandTrigger = arg;
  this->Modified();

  this->Configure();
}

void vtkKWEntry::SetCommandTriggerToReturnKeyAndFocusOut()
{ 
  this->SetCommandTrigger(
    vtkKWEntry::TriggerOnFocusOut | vtkKWEntry::TriggerOnReturnKey); 
}

void vtkKWEntry::SetCommandTriggerToAnyChange()
{ 
  this->SetCommandTrigger(vtkKWEntry::TriggerOnAnyChange); 
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetCommand(vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->Command, object, method);
}

//----------------------------------------------------------------------------
void vtkKWEntry::InvokeCommand(const char *value)
{
  if (this->Command && *this->Command && this->GetApplication())
    {
    const char *val = this->ConvertInternalStringToTclString(
      value, vtkKWCoreWidget::ConvertStringEscapeInterpretable);
    this->Script("%s \"%s\"", this->Command, val ? val : "");
    }
  this->InvokeEvent(vtkKWEntry::EntryValueChangedEvent, (void*)value);
}

//----------------------------------------------------------------------------
void vtkKWEntry::SetValidationCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->ValidationCommand, object, method);
  this->ConfigureValidation();
}

//----------------------------------------------------------------------------
int vtkKWEntry::InvokeValidationCommand(const char *value)
{
  if (this->ValidationCommand && *this->ValidationCommand && 
      this->GetApplication())
    {
    const char *val = this->ConvertInternalStringToTclString(
      value, vtkKWCoreWidget::ConvertStringEscapeInterpretable);
    return atoi(
      this->Script("%s \"%s\"", this->ValidationCommand, val ? val : ""));
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWEntry::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  if (this->GetEnabled() && this->ReadOnly)
    {
    this->SetStateToReadOnly();
    }
  else
    {
    this->SetState(this->GetEnabled());
    }
}

//----------------------------------------------------------------------------
void vtkKWEntry::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Width: " << this->GetWidth() << endl;
  os << indent << "Readonly: " << (this->ReadOnly ? "On" : "Off") << endl;
  os << indent << "RestrictValue: " << this->RestrictValue << endl;
  os << indent << "CommandTrigger: " << this->CommandTrigger << endl;
}

