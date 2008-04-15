/*=========================================================================

  Module:    $RCSfile: vtkKWCoreWidget.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWCoreWidget.h"

#include "vtkKWApplication.h"
#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWIcon.h"
#include "vtkKWOptions.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWCoreWidget );
vtkCxxRevisionMacro(vtkKWCoreWidget, "$Revision: 1.21 $");

//----------------------------------------------------------------------------
class vtkKWCoreWidgetInternals
{
public:

  // Some temporary storage var that do not need to be exposed in the .h

  double ConfigurationOptionAsColorTemp[3];
  vtksys_stl::string ConvertInternalStringToTclStringTemp;
  vtksys_stl::string ConvertTclStringToInternalStringTemp;

  // For performance, let's cache the -state option value so that we
  // can query it faster. This is important as setting a lot of other
  // options on a widget can only be achieved given a specific state.

  int CachedStateOption; 
};

//----------------------------------------------------------------------------
vtkKWCoreWidget::vtkKWCoreWidget()
{
  this->Internals = new vtkKWCoreWidgetInternals;
  this->Internals->CachedStateOption = vtkKWOptions::StateUnknown;
}

//----------------------------------------------------------------------------
vtkKWCoreWidget::~vtkKWCoreWidget()
{
  delete this->Internals;
  this->Internals = NULL;
}

//----------------------------------------------------------------------------
void vtkKWCoreWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();
}

//----------------------------------------------------------------------------
int vtkKWCoreWidget::SetConfigurationOption(
  const char *option, const char *value)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return 0;
    }

  if (!option)
    {
    vtkWarningMacro("Missing option !");
    return 0;
    }

  const char *res = 
    this->Script("%s configure %s {%s}", 
                 this->GetWidgetName(), option, value ? value : "");

  // 'configure' is not supposed to return anything, so let's assume
  // any output is an error

  if (res && *res)
    {
    vtksys_stl::string err_msg(res);
    vtksys_stl::string tcl_name(this->GetTclName());
    vtksys_stl::string widget_name(this->GetWidgetName());
    vtksys_stl::string type(this->GetType());
    vtkErrorMacro(
      "Error configuring " << tcl_name.c_str() << " (" << type.c_str() << ": " 
      << widget_name.c_str() << ") with option: [" << option 
      << "] and value [" << value << "] => " << err_msg.c_str());
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWCoreWidget::HasConfigurationOption(const char *option)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return 0;
    }

  return (this->GetApplication() && 
          !this->GetApplication()->EvaluateBooleanExpression(
            "catch {%s cget %s}",
            this->GetWidgetName(), option));
}

//----------------------------------------------------------------------------
const char* vtkKWCoreWidget::GetConfigurationOption(const char *option)
{
  if (!this->HasConfigurationOption(option))
    {
    return NULL;
    }

  return this->Script("%s cget %s", this->GetWidgetName(), option);
}

//----------------------------------------------------------------------------
int vtkKWCoreWidget::SetConfigurationOptionAsInt(
  const char *option, int value)
{
  char buffer[20];
  sprintf(buffer, "%d", value);
  return this->SetConfigurationOption(option, buffer);
}

//----------------------------------------------------------------------------
int vtkKWCoreWidget::GetConfigurationOptionAsInt(const char *option)
{
  if (!this->HasConfigurationOption(option))
    {
    return 0;
    }

  return atoi(this->Script("%s cget %s", this->GetWidgetName(), option));
}

//----------------------------------------------------------------------------
int vtkKWCoreWidget::SetConfigurationOptionAsDouble(
  const char *option, double value)
{
  char buffer[2048];
  sprintf(buffer, "%f", value);
  return this->SetConfigurationOption(option, buffer);
}

//----------------------------------------------------------------------------
double vtkKWCoreWidget::GetConfigurationOptionAsDouble(const char *option)
{
  if (!this->HasConfigurationOption(option))
    {
    return 0.0;
    }

  return atof(this->Script("%s cget %s", this->GetWidgetName(), option));
}

//----------------------------------------------------------------------------
void vtkKWCoreWidget::GetConfigurationOptionAsColor(
  const char *option, double *r, double *g, double *b)
{
  vtkKWTkUtilities::GetOptionColor(this, option, r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWCoreWidget::GetConfigurationOptionAsColor(const char *option)
{
  // Now this is still not super-safe if in a multi-thread context this
  // method is called with different 'option' argument...

  this->GetConfigurationOptionAsColor(
    option, 
    this->Internals->ConfigurationOptionAsColorTemp, 
    this->Internals->ConfigurationOptionAsColorTemp + 1, 
    this->Internals->ConfigurationOptionAsColorTemp + 2);

  return this->Internals->ConfigurationOptionAsColorTemp;
}

//----------------------------------------------------------------------------
void vtkKWCoreWidget::SetConfigurationOptionAsColor(
  const char *option, double r, double g, double b)
{
  vtkKWTkUtilities::SetOptionColor(this, option, r, g, b);
}

//----------------------------------------------------------------------------
const char* vtkKWCoreWidget::ConvertInternalStringToTclString(
  const char *source, int options)
{
  if (!source || !this->IsCreated())
    {
    return NULL;
    }

  vtksys_stl::string &dest = 
    this->Internals->ConvertInternalStringToTclStringTemp;
  const char *res = source;

  // Handle the encoding

  int app_encoding = this->GetApplication()->GetCharacterEncoding();
  if (app_encoding != VTK_ENCODING_NONE &&
      app_encoding != VTK_ENCODING_UNKNOWN)
    {
    // Get the Tcl encoding name

    const char *tcl_encoding_name = 
      vtkKWOptions::GetCharacterEncodingAsTclOptionValue(app_encoding);

    // Check if we have that encoding
    
    Tcl_Encoding tcl_encoding = 
      Tcl_GetEncoding(
        this->GetApplication()->GetMainInterp(), tcl_encoding_name);
    if (tcl_encoding != NULL)
      {
      Tcl_FreeEncoding(tcl_encoding);
      
      // Convert from that encoding
      // We need to escape interpretable chars to perform that conversion

      dest = vtksys::SystemTools::EscapeChars(source, "[]$\"\\");
      res = source = this->Script(
        "encoding convertfrom %s \"%s\"", tcl_encoding_name, dest.c_str());
      }
    }

  // Escape
  
  vtksys_stl::string escape_chars;
  if (options)
    {
    if (options & vtkKWCoreWidget::ConvertStringEscapeCurlyBraces)
      {
      escape_chars += "{}";
      }
    if (options & vtkKWCoreWidget::ConvertStringEscapeInterpretable)
      {
      escape_chars += "[]$\"\\";
      }
    dest = 
      vtksys::SystemTools::EscapeChars(source, escape_chars.c_str());
    res = source = dest.c_str();
    }

  return res;
}

//----------------------------------------------------------------------------
const char* vtkKWCoreWidget::ConvertTclStringToInternalString(
  const char *source, int options)
{
  if (!source || !this->IsCreated())
    {
    return NULL;
    }

  vtksys_stl::string &dest = 
    this->Internals->ConvertTclStringToInternalStringTemp;
  const char *res = source;

  // Handle the encoding

  int app_encoding = this->GetApplication()->GetCharacterEncoding();
  if (app_encoding != VTK_ENCODING_NONE &&
      app_encoding != VTK_ENCODING_UNKNOWN)
    {
    // Convert from that encoding
    // We need to escape interpretable chars to perform that conversion

    dest = vtksys::SystemTools::EscapeChars(source, "[]$\"\\");
    res = source = this->Script(
      "encoding convertfrom identity \"%s\"", dest.c_str());
    }
  
  // Escape
  
  vtksys_stl::string escape_chars;
  if (options)
    {
    if (options & vtkKWCoreWidget::ConvertStringEscapeCurlyBraces)
      {
      escape_chars += "{}";
      }
    if (options & vtkKWCoreWidget::ConvertStringEscapeInterpretable)
      {
      escape_chars += "[]$\"\\";
      }
    dest = 
      vtksys::SystemTools::EscapeChars(source, escape_chars.c_str());
    res = source = dest.c_str();
    }

  return res;
}

//----------------------------------------------------------------------------
void vtkKWCoreWidget::SetTextOption(const char *option, const char *value)
{
  if (!option || !this->IsCreated())
    {
    return;
    }

  const char *val = this->ConvertInternalStringToTclString(
    value, vtkKWCoreWidget::ConvertStringEscapeInterpretable);
  this->Script("%s configure %s \"%s\"", 
               this->GetWidgetName(), option, val ? val : "");
}

//----------------------------------------------------------------------------
const char* vtkKWCoreWidget::GetTextOption(const char *option)
{
  if (!option || !this->IsCreated())
    {
    return "";
    }

  return this->ConvertTclStringToInternalString(
    this->GetConfigurationOption(option));
}

//----------------------------------------------------------------------------
void vtkKWCoreWidget::SetState(int state)
{
  if (this->GetState() != state && this->IsAlive())
    {
    this->Internals->CachedStateOption = state;
    this->SetConfigurationOption(
      "-state", vtkKWOptions::GetStateAsTkOptionValue(state));
    }
}

void vtkKWCoreWidget::SetStateToDisabled() 
{ 
  this->SetState(vtkKWOptions::StateDisabled); 
};

void vtkKWCoreWidget::SetStateToNormal() 
{ 
  this->SetState(vtkKWOptions::StateNormal); 
};

void vtkKWCoreWidget::SetStateToReadOnly() 
{ 
  this->SetState(vtkKWOptions::StateReadOnly); 
};

//----------------------------------------------------------------------------
int vtkKWCoreWidget::GetState()
{
  if (this->Internals->CachedStateOption == vtkKWOptions::StateUnknown &&
      this->IsAlive())
    {
    this->Internals->CachedStateOption = 
      vtkKWOptions::GetStateFromTkOptionValue(
        this->GetConfigurationOption("-state"));
    }
  return this->Internals->CachedStateOption;
}

//----------------------------------------------------------------------------
const char* vtkKWCoreWidget::GetType()
{
  const char *res = vtkKWTkUtilities::GetWidgetClass(this);
  if (!res)
    {
    return "None";
    }
  return res;
}

//----------------------------------------------------------------------------
void vtkKWCoreWidget::Raise()
{
  if (this->IsCreated())
    {
    this->Script("raise %s", this->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWCoreWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

