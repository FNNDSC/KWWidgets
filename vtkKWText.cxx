/*=========================================================================

  Module:    $RCSfile: vtkKWText.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWText.h"

#include "vtkKWApplication.h"
#include "vtkKWOptions.h"
#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"

#include <vtksys/stl/string>
#include <vtksys/stl/list>
#include <vtksys/RegularExpression.hxx>

const char *vtkKWText::MarkerBold = "**";
const char *vtkKWText::MarkerItalic = "~~";
const char *vtkKWText::MarkerUnderline = "__";

const char *vtkKWText::TagBold = "_bold_tag_";
const char *vtkKWText::TagItalic = "_italic_tag_";
const char *vtkKWText::TagUnderline = "_underline_tag_";
const char *vtkKWText::TagFgNavy = "_fg_navy_tag_";
const char *vtkKWText::TagFgRed = "_fg_red_tag_";
const char *vtkKWText::TagFgBlue = "_fg_blue_tag_";
const char *vtkKWText::TagFgDarkGreen = "_fg_dark_green_tag_";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWText);
vtkCxxRevisionMacro(vtkKWText, "$Revision: 1.63 $");

//----------------------------------------------------------------------------
class vtkKWTextInternals
{
public:

  class TagMatcher
  {
  public:
    vtksys_stl::string Regexp;
    vtksys_stl::string Tag;
  };

  typedef vtksys_stl::list<TagMatcher> TagMatchersContainer;
  typedef vtksys_stl::list<TagMatcher>::iterator TagMatchersContainerIterator;

  TagMatchersContainer TagMatchers;
};

//----------------------------------------------------------------------------
vtkKWText::vtkKWText()
{
  this->InternalTextString = NULL;
  this->ReadOnly            = 0;
  this->QuickFormatting     = 0;
  this->Command             = NULL;
  this->CommandTrigger      = (vtkKWText::TriggerOnFocusOut | 
                               vtkKWText::TriggerOnReturnKey);

  this->Internals = new vtkKWTextInternals;
}

//----------------------------------------------------------------------------
vtkKWText::~vtkKWText()
{
  if (this->Command)
    {
    delete [] this->Command;
    this->Command = NULL;
    }

  this->SetInternalTextString(NULL);

  // Delete all presets

  if (this->Internals)
    {
    delete this->Internals;
    this->Internals = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWText::Configure(int disable)
{
  if (!this->IsCreated())
    {
    return;
    }

  if (!this->ReadOnly &&
      this->CommandTrigger & vtkKWText::TriggerOnFocusOut &&
      !disable)
    {
    this->SetBinding("<FocusOut>", this, "ValueCallback");
    }
  else
    {
    this->RemoveBinding("<FocusOut>", this, "ValueCallback");
    }

  if (this->CommandTrigger & vtkKWText::TriggerOnReturnKey &&
      !disable)
    {
    this->SetBinding("<Return>", this, "ValueCallback");
    }
  else
    {
    this->RemoveBinding("<Return>", this, "ValueCallback");
    }

  if (this->CommandTrigger & vtkKWText::TriggerOnAnyChange &&
      !disable)
    {
    this->SetBinding("<<Modified>>", this, "ValueCallback");
    }
  else
    {
    this->RemoveBinding("<<Modified>>", this, "ValueCallback");
    }

  // When the text is unmapped, trigger the callback as well. Here is
  // the rationale: a user may have edited the value in the text but
  // neither did he press Enter or clicked on any other widget that
  // made that text lose its focus. When the user closes the window, or when
  // the user switch to a different notebook tab than the one in which the 
  // text was packed, or when the user skip to a different wizard step
  // than the one in which the text was packed, then this text should
  // definitely be validated/acknowledged. This is what this event will
  // take care of.

  if (!this->ReadOnly &&
      !disable)
    {
    this->SetBinding("<Unmap>", this, "ValueCallback");
    }
  else
    {
    this->RemoveBinding("<Unmap>", this, "ValueCallback");
    }
}

//----------------------------------------------------------------------------
char *vtkKWText::GetText()
{
  if (!this->IsCreated())
    {
    return 0;
    }

  const char *val = this->Script("%s get 1.0 {end -1 chars}", 
                                 this->GetWidgetName());
  this->SetInternalTextString(this->ConvertTclStringToInternalString(val));
  return this->GetInternalTextString();
}

//----------------------------------------------------------------------------
void vtkKWText::SetText(const char *s)
{
  this->SetText(s, NULL);
}

//----------------------------------------------------------------------------
void vtkKWText::SetText(const char *s, const char *tag)
{
  if (!this->IsCreated())
    {
    return;
    }

  // Delete everything

  int state = this->GetState();
  this->SetStateToNormal();
  this->Configure(1); // disable all bindings so that deleting doesn't trigger
  this->Script("%s delete 1.0 end", this->GetWidgetName());
  if (this->CommandTrigger & vtkKWText::TriggerOnAnyChange)
    {
    this->SetModifiedFlag(0);
    }
  this->Configure(0); // re-enable all bindings
  this->SetState(state);

  // Append to the end

  this->AppendText(s, tag);
}

//----------------------------------------------------------------------------
void vtkKWText::AppendText(const char *s)
{
  this->AppendText(s, NULL);
}

//----------------------------------------------------------------------------
void vtkKWText::AppendText(const char *s, const char *tag)
{
  if (!this->IsCreated())
    {
    return;
    }

  int state = this->GetState();
  this->SetStateToNormal();

  this->AppendTextInternalTagging(s, tag);
  
  this->SetState(state);
}

//----------------------------------------------------------------------------
void vtkKWText::AppendTextInternalTagging(const char *str, const char *tag)
{
  // Don't check for this->Created() for speed, since it is called
  // by AppendText which does the check already

  // In QuickFormatting mode, look for markers, and use tags accordingly

  if (this->QuickFormatting && str)
    {
    const int nb_markers = 3;
    const char* markertag[nb_markers * 2] = 
      {
        vtkKWText::MarkerBold, vtkKWText::TagBold,
        vtkKWText::MarkerItalic, vtkKWText::TagItalic,
        vtkKWText::MarkerUnderline, vtkKWText::TagUnderline
      };

    // First find the closest known marker

    const char *closest_marker = NULL;
    int i, closest_marker_id = -1;
    for (i = 0; i < nb_markers; i++)
      {
      const char *find_marker = strstr(str, markertag[i * 2]);
      if (find_marker && (!closest_marker || find_marker < closest_marker))
        {
        closest_marker = find_marker;
        closest_marker_id = i;
        }
      }

    // Then find its counterpart end marker, if any

    if (closest_marker)
      {
      int len_marker = (int)strlen(markertag[closest_marker_id * 2]);
      const char *end_marker = 
        strstr(closest_marker + len_marker, markertag[closest_marker_id * 2]);
      if (end_marker)
        {
        // Text before the marker, using the current tag

        vtksys_stl::string before;
        before.append(str, closest_marker - str);
        this->AppendTextInternalTagging(before.c_str(), tag);

        // Zone inside the marker, using the current tag + the marker's tag

        vtksys_stl::string new_tag;
        if (tag)
          {
          new_tag.append(tag);
          }
        new_tag.append(" ").append(markertag[closest_marker_id * 2 + 1]);
        vtksys_stl::string zone;
        zone.append(closest_marker + len_marker, 
                    end_marker - closest_marker - len_marker);
        this->AppendTextInternalTagging(zone.c_str(), new_tag.c_str());

        // Text after the marker, using the current tag

        vtksys_stl::string after;
        after.append(end_marker + len_marker);
        this->AppendTextInternalTagging(after.c_str(), tag);

        return;
        }
      }
    }

  // The tag matchers

  vtkKWTextInternals::TagMatchersContainerIterator it = 
    this->Internals->TagMatchers.begin();
  vtkKWTextInternals::TagMatchersContainerIterator end = 
    this->Internals->TagMatchers.end();
  int found_regexp = 0;
  for (; it != end; ++it)
    {
    vtksys::RegularExpression re((*it).Regexp.c_str());
    if (re.find(str))
      {
      // Text before the regexp, using the current tag

      vtksys_stl::string before;
      before.append(str, re.start());

      // Zone inside the regexp, using the current tag + the marker's tag

      vtksys_stl::string new_tag;
      if (tag)
        {
        new_tag.append(tag);
        }
      new_tag.append(" ").append((*it).Tag);
      vtksys_stl::string zone;
      zone.append(str + re.start(), re.end() - re.start());

      // Text after the regexp, using the current tag

      vtksys_stl::string after;
      after.append(str + re.end());

      this->AppendTextInternalTagging(before.c_str(), tag);
      this->AppendTextInternal(zone.c_str(), new_tag.c_str());
      this->AppendTextInternalTagging(after.c_str(), tag);
      found_regexp = 1;
      break;
      }
    }

  if (!found_regexp)
    {
    this->AppendTextInternal(str, tag);
    }
}

//----------------------------------------------------------------------------
void vtkKWText::AppendTextInternal(const char *s, const char *tag)
{
  // Don't check for this->Created() for speed, since it is called
  // by AppendText which does the check already

  const char *val = this->ConvertInternalStringToTclString(
    s, vtkKWCoreWidget::ConvertStringEscapeInterpretable);

  this->Script("%s insert end \"%s\" %s", 
               this->GetWidgetName(),
               val ? val : "", tag ? tag : "");
}

//----------------------------------------------------------------------------
void vtkKWText::CreateWidget()
{
  // Call the superclass to set the appropriate flags then create manually

  vtksys_stl::string options;
  int tcl_major, tcl_minor, tcl_patch_level;
  Tcl_GetVersion(&tcl_major, &tcl_minor, &tcl_patch_level, NULL);
  if (tcl_major > 8 || (tcl_major == 8 && tcl_minor >= 5))
    {
    options = "-width 20 -wrap word -height 5 -highlightthickness 0 -background #ffffff -bd 2 -font TkDefaultFont";
    }
  else
    {
    options = "-width 20 -wrap word -height 5 -highlightthickness 0 -background #ffffff";
    }

  if (!vtkKWWidget::CreateSpecificTkWidget(this, "text", options.c_str()))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  // Create the default tags

  const char *wname = this->GetWidgetName();
  vtksys_stl::string font(this->GetConfigurationOption("-font"));

  char bold_font[512], italic_font[512];
  vtkKWTkUtilities::ChangeFontWeightToBold(
    vtkKWApplication::GetMainInterp(), font.c_str(), bold_font);
  vtkKWTkUtilities::ChangeFontSlantToItalic(
    vtkKWApplication::GetMainInterp(), font.c_str(), italic_font);

  this->Script("%s tag config %s -font \"%s\"", 
               wname, vtkKWText::TagBold, bold_font);

  this->Script("%s tag config %s -font \"%s\"", 
               wname, vtkKWText::TagItalic, italic_font);

  this->Script("%s tag config %s -underline 1", 
               wname, vtkKWText::TagUnderline);

  this->Script("%s tag config %s -foreground #000080", 
               wname, vtkKWText::TagFgNavy);

  this->Script("%s tag config %s -foreground #FF0000", 
               wname, vtkKWText::TagFgRed);

  this->Script("%s tag config %s -foreground #0000FF", 
               wname, vtkKWText::TagFgBlue);

  this->Script("%s tag config %s -foreground #006400", 
               wname, vtkKWText::TagFgDarkGreen);

  // Design choice: we assume a keypress is meant for this widget only

  this->SetGenericBinding(
    "Text", 
    "<KeyPress>", NULL, "::tk::TextInsert %W %A; break");
}

//----------------------------------------------------------------------------
void vtkKWText::SetReadOnly(int arg)
{
  if (this->ReadOnly == arg)
    {
    return;
    }

  this->ReadOnly = arg;
  this->UpdateEnableState();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWText::SetQuickFormatting(int arg)
{
  if (this->QuickFormatting == arg)
    {
    return;
    }

  this->QuickFormatting = arg;
  this->Modified();

  this->SetText(this->GetText());
}

//----------------------------------------------------------------------------
void vtkKWText::GetBackgroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-background", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWText::GetBackgroundColor()
{
  return this->GetConfigurationOptionAsColor("-background");
}

//----------------------------------------------------------------------------
void vtkKWText::SetBackgroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-background", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWText::GetForegroundColor(double *r, double *g, double *b)
{
  this->GetConfigurationOptionAsColor("-foreground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWText::GetForegroundColor()
{
  return this->GetConfigurationOptionAsColor("-foreground");
}

//----------------------------------------------------------------------------
void vtkKWText::SetForegroundColor(double r, double g, double b)
{
  this->SetConfigurationOptionAsColor("-foreground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWText::SetHighlightThickness(int width)
{
  this->SetConfigurationOptionAsInt("-highlightthickness", width);
}

//----------------------------------------------------------------------------
int vtkKWText::GetHighlightThickness()
{
  return this->GetConfigurationOptionAsInt("-highlightthickness");
}

//----------------------------------------------------------------------------
void vtkKWText::SetBorderWidth(int width)
{
  this->SetConfigurationOptionAsInt("-bd", width);
}

//----------------------------------------------------------------------------
int vtkKWText::GetBorderWidth()
{
  return this->GetConfigurationOptionAsInt("-bd");
}

//----------------------------------------------------------------------------
void vtkKWText::SetRelief(int relief)
{
  this->SetConfigurationOption(
    "-relief", vtkKWOptions::GetReliefAsTkOptionValue(relief));
}

void vtkKWText::SetReliefToRaised()     
{ 
  this->SetRelief(vtkKWOptions::ReliefRaised); 
};
void vtkKWText::SetReliefToSunken() 
{ 
  this->SetRelief(vtkKWOptions::ReliefSunken); 
};
void vtkKWText::SetReliefToFlat() 
{ 
  this->SetRelief(vtkKWOptions::ReliefFlat); 
};
void vtkKWText::SetReliefToRidge() 
{ 
  this->SetRelief(vtkKWOptions::ReliefRidge); 
};
void vtkKWText::SetReliefToSolid() 
{ 
  this->SetRelief(vtkKWOptions::ReliefSolid); 
};
void vtkKWText::SetReliefToGroove() 
{ 
  this->SetRelief(vtkKWOptions::ReliefGroove); 
};

//----------------------------------------------------------------------------
int vtkKWText::GetRelief()
{
  return vtkKWOptions::GetReliefFromTkOptionValue(
    this->GetConfigurationOption("-relief"));
}

//----------------------------------------------------------------------------
void vtkKWText::SetFont(const char *font)
{
  this->SetConfigurationOption("-font", font);
}

//----------------------------------------------------------------------------
const char* vtkKWText::GetFont()
{
  return this->GetConfigurationOption("-font");
}

//----------------------------------------------------------------------------
void vtkKWText::SetPadX(int arg)
{
  this->SetConfigurationOptionAsInt("-padx", arg);
}

//----------------------------------------------------------------------------
int vtkKWText::GetPadX()
{
  return this->GetConfigurationOptionAsInt("-padx");
}

//----------------------------------------------------------------------------
void vtkKWText::SetPadY(int arg)
{
  this->SetConfigurationOptionAsInt("-pady", arg);
}

//----------------------------------------------------------------------------
int vtkKWText::GetPadY()
{
  return this->GetConfigurationOptionAsInt("-pady");
}

//----------------------------------------------------------------------------
void vtkKWText::SetWidth(int width)
{
  this->SetConfigurationOptionAsInt("-width", width);
}

//----------------------------------------------------------------------------
int vtkKWText::GetWidth()
{
  return this->GetConfigurationOptionAsInt("-width");
}

//----------------------------------------------------------------------------
void vtkKWText::SetHeight(int height)
{
  this->SetConfigurationOptionAsInt("-height", height);
}

//----------------------------------------------------------------------------
int vtkKWText::GetHeight()
{
  return this->GetConfigurationOptionAsInt("-height");
}

//----------------------------------------------------------------------------
void vtkKWText::SetWrapToNone()
{
  this->SetConfigurationOption("-wrap", "none");
}

//----------------------------------------------------------------------------
void vtkKWText::SetWrapToChar()
{
  this->SetConfigurationOption("-wrap", "char");
}

//----------------------------------------------------------------------------
void vtkKWText::SetWrapToWord()
{
  this->SetConfigurationOption("-wrap", "word");
}

//----------------------------------------------------------------------------
void vtkKWText::SetResizeToGrid(int arg)
{
  this->SetConfigurationOptionAsInt("-setgrid", arg);
}

//----------------------------------------------------------------------------
int vtkKWText::GetResizeToGrid()
{
  return this->GetConfigurationOptionAsInt("-setgrid");
}

//----------------------------------------------------------------------------
void vtkKWText::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->SetState(this->ReadOnly ? 0 : this->GetEnabled());
}

//----------------------------------------------------------------------------
void vtkKWText::AddTagMatcher(const char *regexp, const char *tag)
{
  if (!this->Internals || !regexp || !tag)
    {
    return;
    }

  vtkKWTextInternals::TagMatcher tagmatcher;
  tagmatcher.Regexp = regexp;
  tagmatcher.Tag = tag;
  this->Internals->TagMatchers.push_back(tagmatcher);
} 

//----------------------------------------------------------------------------
void vtkKWText::SeeEnd()
{
  if (this->IsCreated())
    {
    this->Script("%s see end", this->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWText::ValueCallback()
{
  this->InvokeCommand(this->GetText());
  if (this->CommandTrigger & vtkKWText::TriggerOnAnyChange)
    {
    this->SetModifiedFlag(0);
    }
}

//----------------------------------------------------------------------------
void vtkKWText::SetCommandTrigger(int arg)
{
  if (this->CommandTrigger == arg)
    {
    return;
    }

  this->CommandTrigger = arg;
  this->Modified();

  this->Configure();
}

void vtkKWText::SetCommandTriggerToReturnKeyAndFocusOut()
{ 
  this->SetCommandTrigger(
    vtkKWText::TriggerOnFocusOut | vtkKWText::TriggerOnReturnKey); 
}

void vtkKWText::SetCommandTriggerToAnyChange()
{ 
  this->SetCommandTrigger(vtkKWText::TriggerOnAnyChange); 
}

//----------------------------------------------------------------------------
void vtkKWText::SetCommand(vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->Command, object, method);
}

//----------------------------------------------------------------------------
void vtkKWText::InvokeCommand(const char *value)
{
  if (this->Command && *this->Command && this->GetApplication())
    {
    const char *val = this->ConvertInternalStringToTclString(
      value, vtkKWCoreWidget::ConvertStringEscapeInterpretable);
    this->Script("%s \"%s\"", this->Command, val ? val : "");
    }
}

//----------------------------------------------------------------------------
int vtkKWText::GetModifiedFlag()
{
  if (this->IsCreated())
    {
    return atoi(this->Script("%s edit modified", this->GetWidgetName()));
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWText::SetModifiedFlag(int flag)
{
  if (this->IsCreated())
    {
    this->Script("%s edit modified %d", this->GetWidgetName(), flag ? 1 : 0);
    }
}

//----------------------------------------------------------------------------
void vtkKWText::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ReadOnly: " 
     << (this->ReadOnly ? "On" : "Off") << endl;
  os << indent << "QuickFormatting: " 
     << (this->QuickFormatting ? "On" : "Off") << endl;
  os << indent << "CommandTrigger: " << this->CommandTrigger << endl;
}
