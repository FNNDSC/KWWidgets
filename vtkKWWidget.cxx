/*=========================================================================

  Module:    $RCSfile: vtkKWWidget.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWWidget.h"

#include "vtkKWApplication.h"
#include "vtkKWIcon.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWWidgetCollection.h"
#include "vtkKWWindow.h"
#include "vtkKWDragAndDropHelper.h"
#include "vtkObjectFactory.h"

#include <kwsys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWWidget );
vtkCxxRevisionMacro(vtkKWWidget, "$Revision: 1.115 $");

int vtkKWWidgetCommand(ClientData cd, Tcl_Interp *interp,
                       int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkKWWidget::vtkKWWidget()
{
  this->CommandFunction          = vtkKWWidgetCommand;

  this->WidgetName               = NULL;
  this->Parent                   = NULL;

  this->Children                 = NULL;
  this->DeletingChildren         = 0;

  this->BalloonHelpString        = NULL;  
  this->BalloonHelpJustification = 0;
  this->BalloonHelpInitialized   = 0;

  this->Enabled                  = 1;

  this->WidgetIsCreated          = 0;

  this->DragAndDropHelper       = NULL;
}

//----------------------------------------------------------------------------
vtkKWWidget::~vtkKWWidget()
{
  if (this->DragAndDropHelper)
    {
    this->DragAndDropHelper->Delete();
    }

  if (this->BalloonHelpString)
    {
    this->SetBalloonHelpString(NULL);
    }

  if (this->Children)
    {
    this->Children->UnRegister(this);
    this->Children = NULL;
    }
  
  if (this->IsCreated())
    {
    this->Script("destroy %s",this->GetWidgetName());
    }

  if (this->WidgetName)
    {
    delete [] this->WidgetName;
    }

  this->SetParent(NULL);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetParent(vtkKWWidget *p)
{
  if (this->Parent && p)
    {
    vtkErrorMacro("Error attempt to reparent a widget!");
    return;
    }
  if (this->Parent)
    {
    vtkKWWidget *tmp = this->Parent;
    this->Parent = NULL;
    tmp->UnRegister(this);
    tmp->RemoveChild(this);
    }
  else if (p)
    {
    this->Parent = p;
    p->Register(this);
    p->AddChild(this);
    }
}

//----------------------------------------------------------------------------
const char *vtkKWWidget::GetWidgetName()
{
  static unsigned long count = 0;

  // is the name is already set the just return it
  if (this->WidgetName)
    {
    return this->WidgetName;
    }

  // create this widgets name
  char local[256];
  // get the parents name
  if (this->Parent)
    {
    const char *tmp = this->Parent->GetWidgetName();
    sprintf(local,"%s.%lu",tmp,count);
    }
  else
    {
    sprintf(local,".%lu",count);
    }
  count++;
  this->WidgetName = new char [strlen(local)+1];
  strcpy(this->WidgetName,local);
  return this->WidgetName;
}

//----------------------------------------------------------------------------
int vtkKWWidget::Create(vtkKWApplication *app, 
                        const char *type, 
                        const char *args)
{
  if (this->IsCreated())
    {
    if (type)
      {
      vtkErrorMacro(
        << this->GetClassName() << " (" << type << ") already created");
      }
    else
      {
      vtkErrorMacro(<< this->GetClassName() << " already created");
      }
    return 0;
    }

  if (!app)
    {
    vtkErrorMacro("Can not create widget with NULL application. Make sure you did not SafeDowncast a vtkKWApplication to a more specific subclass.");
    return 0;
    }

  this->SetApplication(app);

  if (this->HasDragAndDropHelper())
    {
    this->GetDragAndDropHelper()->SetApplication(app);
    }

  this->WidgetIsCreated = 1;

  if (type)
    {
    if (args)
      {
      this->Script("%s %s %s", type, this->GetWidgetName(), args);
      }
    else
      {
      this->Script("%s %s", type, this->GetWidgetName());
      }

    /* Update enable state
       At this point, the widget is considered created, although for all 
       all subclasses calling this method, only a part of the widget has
       really been created (for ex., this method will be used to create the
       main container for the widget, like a frame, and the subclass will
       create the subwidgets to put in). 
       Ideally, we could have a function that explicitly sets when a widget
       is fully created or not.
       Anyway, most subclasses override the virtual UpdateEnableState()
       method to propagate the Enabled state to their subwidgets components.
       If we call it now, it will immediately go inside the overriden method
       and try to act on widget that have propably not been created yet: it
       is therefore important that the overriden UpdateEnableState() is smart
       enough and test if each subwidget has really been created 
       (subobj->IsCreated() instead of just this->IsCreated(), which will 
       return true at the moment). It is the case at the moment.
       Also, each subclass should call UpdateEnableState() at the end of their
       own Create() method, so that the code that is supposed to be executed
       in the overriden UpdateEnableState() for the subwidgets is really 
       executed now that they have been created.
       The call here, even if it goes down to the subclass, is still needed.
    */
   
    this->UpdateEnableState();
    }

  return 1;
}

// ---------------------------------------------------------------------------
int vtkKWWidget::IsCreated()
{
  return (this->GetApplication() != NULL && this->WidgetIsCreated);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetUpBalloonHelpBindings()
{
  this->Script("bind %s <Enter> {+%s BalloonHelpTrigger %s}", 
               this->GetWidgetName(), this->GetApplication()->GetTclName(),
               this->GetTclName());
  this->Script("bind %s <ButtonPress> {+%s BalloonHelpWithdraw}", 
               this->GetWidgetName(), this->GetApplication()->GetTclName());
  this->Script("bind %s <KeyPress> {+%s BalloonHelpWithdraw}", 
               this->GetWidgetName(), this->GetApplication()->GetTclName());
  this->Script("bind %s <Leave> {+%s BalloonHelpCancel}", 
               this->GetWidgetName(), this->GetApplication()->GetTclName());
  this->Script("bind %s <B1-Motion> {+%s BalloonHelpWithdraw}", 
               this->GetWidgetName(), this->GetApplication()->GetTclName());  
}

//----------------------------------------------------------------------------
vtkKWWidgetCollection* vtkKWWidget::GetChildren()
{
  // Lazy allocation. Create the children collection only when it is needed

  if (!this->Children)
    {
    this->Children = vtkKWWidgetCollection::New();

    // Make tracking memory leaks easier.

    this->Children->Register(this);
    this->Children->Delete();
    }
  return this->Children;
}

//----------------------------------------------------------------------------
int vtkKWWidget::HasChildren()
{
  if (!this->Children)
    {
    return 0;
    }
  return this->Children->GetNumberOfItems() ? 1 : 0;
}

//----------------------------------------------------------------------------
void vtkKWWidget::RemoveChild(vtkKWWidget *w) 
{
  // Removing items in the middle of a traversal is a bad thing.
  // UnRegister will handle removing all of the children.

  if (!this->DeletingChildren && this->HasChildren())
    {
    this->GetChildren()->RemoveItem(w);
    }
}

//----------------------------------------------------------------------------
void vtkKWWidget::AddChild(vtkKWWidget *w) 
{
  this->GetChildren()->AddItem(w);
}

//----------------------------------------------------------------------------
int  vtkKWWidget::GetNetReferenceCount() 
{
  int childCounts = 0;

  if (this->HasChildren())
    {
    vtkKWWidget *child;
    vtkKWWidgetCollection *children = this->GetChildren();
    children->InitTraversal();
    while ((child = children->GetNextKWWidget()))
      {
      childCounts += child->GetNetReferenceCount();
      }
    childCounts -= 2 * children->GetNumberOfItems();
    }

  return this->ReferenceCount + childCounts;
}

//----------------------------------------------------------------------------
vtkKWWidget *vtkKWWidget::GetChildWidgetWithName(const char *name)
{
  if (name && this->HasChildren())
    {
    vtkKWWidget *child;
    vtkKWWidgetCollection *children = this->GetChildren();
    children->InitTraversal();
    while ((child = children->GetNextKWWidget()))
      {
      const char *wname = child->GetWidgetName();
      if (wname && !strcmp(wname, name))
        {
        return child;
        }
      }
    }

  return NULL;
}
       
//----------------------------------------------------------------------------
void vtkKWWidget::UnRegister(vtkObjectBase *o)
{
  if (!this->DeletingChildren && this->HasChildren())
    {
    // delete the children if we are about to be deleted
    vtkKWWidgetCollection *children = this->GetChildren();
    if (this->ReferenceCount == children->GetNumberOfItems() + 1)
      {
      vtkKWWidget *child;
      
      this->DeletingChildren = 1;
      children->InitTraversal();
      while ((child = children->GetNextKWWidget()))
        {
        child->SetParent(NULL);
        }
      children->RemoveAllItems();
      this->DeletingChildren = 0;
      }
    }
  
  this->Superclass::UnRegister(o);
}

//----------------------------------------------------------------------------
void vtkKWWidget::Focus()
{
  if (this->IsCreated())
    {
    this->Script("focus %s", this->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetBind(vtkKWObject* CalledObject, const char *Event, const char *CommandString)
{
  this->Script("bind %s %s { %s %s }", this->GetWidgetName(), 
               Event, CalledObject->GetTclName(), CommandString);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetBind(const char *Event, const char *CommandString)
{
  this->Script("bind %s %s { %s }", this->GetWidgetName(), Event, CommandString);
}

//----------------------------------------------------------------------------
void vtkKWWidget::UnsetBind(const char *Event)
{
  this->SetBind(Event, "");
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetBind(const char *event, const char *widget, const char *command)
{
  this->Script("bind %s %s { %s %s }", this->GetWidgetName(), event, widget, command);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetBindAll(const char *event, const char *widget, const char *command)
{
  this->Script("bind all %s { %s %s }", event, widget, command);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetBindAll(const char *event, const char *command)
{
  this->Script("bind all %s { %s }", event, command);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetCommand(vtkKWObject* CalledObject, const char * CommandString)
{
  if (!this->IsCreated())
    {
    return;
    }

  char *command = NULL;
  this->SetObjectMethodCommand(&command, CalledObject, CommandString);
  this->Script("%s configure -command {%s}",
               this->GetWidgetName(), command);
  delete [] command;
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetBalloonHelpString(const char *str)
{
  // A little overkill.
  if (this->BalloonHelpString == NULL && str == NULL)
    {
    return;
    }

  // Normal string stuff.
  if (this->BalloonHelpString)
    {
    delete [] this->BalloonHelpString;
    this->BalloonHelpString = NULL;
    }
  if (str != NULL)
    {
    this->BalloonHelpString = new char[strlen(str)+1];
    strcpy(this->BalloonHelpString, str);
    }
  
  if ( this->BalloonHelpString && this->GetApplication() && 
       !this->BalloonHelpInitialized )
    {
    this->SetUpBalloonHelpBindings();
    this->BalloonHelpInitialized = 1;
    }
}

//----------------------------------------------------------------------------
vtkKWWindow* vtkKWWidget::GetWindow()
{
  vtkKWWindow* win =0;
  vtkKWWidget* widget = this->GetParent();
  while(widget)
    {
    if((win = vtkKWWindow::SafeDownCast(widget)))
      {
      return win;
      }
    widget = widget->GetParent();
    }
  return win;
}

//----------------------------------------------------------------------------
const char* vtkKWWidget::GetType()
{
  if (this->IsCreated())
    {
    return this->Script("winfo class %s", this->GetWidgetName());
    }
  return "None";
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetEnabled(int e)
{
  if ( this->Enabled == e )
    {
    return;
    }
  this->Enabled = e;

  this->UpdateEnableState();

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWWidget::UpdateEnableState()
{
  // Use this piece of debug code to detect which widgets are created using
  // the old fashion way (vtkKWWidget, then this->Script("%s foobar", ...),
  // preventing the overriden UpdateEnableState() to be called

#if 0
  int is_widget = (this->IsCreated() &&
                   this->HasConfigurationOption("-state") &&
                   !strcmp(this->GetClassName(), "vtkKWWidget"));

  /*
    Enable this one to get a rough idea of how many vtkKWWidget are not
    *qualified* enough (i.e., they are widgets with a -state option, like
    a button, canvas, scale, but were created using vtkKWWidget instead of
    a more qualified vtkKWPushButton, vtkKWCanvas, vtkKWScale, etc.). 
    For speed reasons the -state flag is now updated *only* in subclasses
    where that flag actually means something (like a button, a scale, etc.),
    hence the vtkKWWidget listed here should be "qualified" to a meaningful
    subclass.
    Note that Listbox and Scale will show up but there is nothing
    you can do about it, because the implementation of vtkKWListbox and
    vtkKWScale use a vtkKWWidget internally to store the actual Tk "scale"
    or "listbox", so that they can be embedded inside a frame, with label
    around, etc.

    Pipe the output of the application to a file, then sort this file and 
    get the uniq entries:
    paraview > state.txt
    sort state.txt | uniq -c
  */

#if 1
  if (is_widget)
    {
    cout << this->GetClassName() << " (" << this->GetType() << ")" << endl;
    }
#endif

  /*
    Enable this one to get more accurate info about the vtkKWWidget to change
    (TclName, -text and -label option, etc).
  */

#if 0
  if (is_widget)
    {
    cout << this->GetClassName() << " (" << this->GetType() << ")"
         << " (" << this->GetTclName() << ")";
    if (this->HasConfigurationOption("-text"))
      {
      res = this->Script("%s cget -text", this->GetWidgetName());
      cout << " (-text: " << res << ")";
      }
    if (this->HasConfigurationOption("-label"))
      {
      res = this->Script("%s cget -label", this->GetWidgetName());
      cout << " (-label: " << res << ")";
      }
    cout << endl;
    }
#endif

#endif
}

//----------------------------------------------------------------------------
void vtkKWWidget::PropagateEnableState(vtkKWWidget* widget)
{
  if ( !widget || widget == this )
    {
    return;
    }
  widget->SetEnabled(this->Enabled);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetStateOption(int flag)
{
  if (this->IsAlive())
    {
    this->Script("%s configure -state %s", 
                 this->GetWidgetName(), (flag ? "normal" : "disabled")); 
    }
}

//----------------------------------------------------------------------------
int vtkKWWidget::GetStateOption()
{
  if (this->IsAlive())
    {
    const char *state = this->Script("%s cget -state", this->GetWidgetName());
    return !strcmp(state, "normal") ? 1 : 0; 
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWWidget::GetBackgroundColor(int *r, int *g, int *b)
{
  vtkKWTkUtilities::GetBackgroundColor(this, r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetBackgroundColor(int r, int g, int b)
{
  char color[10];
  sprintf(color, "#%02x%02x%02x", r, g, b);
  this->Script("%s config -bg %s", this->GetWidgetName(), color);
}

//----------------------------------------------------------------------------
void vtkKWWidget::GetBackgroundColor(double *r, double *g, double *b)
{
  int ir, ig, ib;
  this->GetBackgroundColor(&ir, &ig, &ib);

  *r = (double)ir / 255.0;
  *g = (double)ig / 255.0;
  *b = (double)ib / 255.0;
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetBackgroundColor(double r, double g, double b)
{
  this->SetBackgroundColor(
    (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
}

//----------------------------------------------------------------------------
void vtkKWWidget::GetForegroundColor(int *r, int *g, int *b)
{
  vtkKWTkUtilities::GetOptionColor(this, "-fg", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetForegroundColor(int r, int g, int b)
{
  char color[10];
  sprintf(color, "#%02x%02x%02x", r, g, b);
  this->Script("%s config -fg %s", this->GetWidgetName(), color);
}

//----------------------------------------------------------------------------
void vtkKWWidget::GetForegroundColor(double *r, double *g, double *b)
{
  int ir, ig, ib;
  this->GetForegroundColor(&ir, &ig, &ib);

  *r = (double)ir / 255.0;
  *g = (double)ig / 255.0;
  *b = (double)ib / 255.0;
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetForegroundColor(double r, double g, double b)
{
  char color[10];
  sprintf(color, "#%02x%02x%02x", 
          (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
  this->Script("%s config -fg %s", this->GetWidgetName(), color);
}

//----------------------------------------------------------------------------
int vtkKWWidget::HasConfigurationOption(const char* option)
{
  return (this->GetApplication() && 
          !this->GetApplication()->EvaluateBooleanExpression(
            "catch {%s cget %s}",
            this->GetWidgetName(), option));
}

//----------------------------------------------------------------------------
int vtkKWWidget::GetConfigurationOptionAsInt(const char* option)
{
  if (!this->HasConfigurationOption(option))
    {
    return 0;
    }

  return atoi(this->Script("%s cget %s", this->GetWidgetName(), option));
}

//----------------------------------------------------------------------------
const char* vtkKWWidget::GetTclCharacterEncodingAsString(int encoding)
{
  switch (encoding)
    {
    case VTK_ENCODING_US_ASCII:
      return "ascii";

    case VTK_ENCODING_UNICODE:
      return "unicode";

    case VTK_ENCODING_UTF_8:
      return "utf-8";

    case VTK_ENCODING_ISO_8859_1:
      return "iso8859-1";

    case VTK_ENCODING_ISO_8859_2:
      return "iso8859-2";

    case VTK_ENCODING_ISO_8859_3:
      return "iso8859-3";

    case VTK_ENCODING_ISO_8859_4:
      return "iso8859-4";

    case VTK_ENCODING_ISO_8859_5:
      return "iso8859-5";

    case VTK_ENCODING_ISO_8859_6:
      return "iso8859-5";

    case VTK_ENCODING_ISO_8859_7:
      return "iso8859-7";

    case VTK_ENCODING_ISO_8859_8:
      return "iso8859-8";

    case VTK_ENCODING_ISO_8859_9:
      return "iso8859-9";

    case VTK_ENCODING_ISO_8859_10:
      return "iso8859-10";

    case VTK_ENCODING_ISO_8859_11:
      return "iso8859-11";

    case VTK_ENCODING_ISO_8859_12:
      return "iso8859-12";

    case VTK_ENCODING_ISO_8859_13:
      return "iso8859-13";

    case VTK_ENCODING_ISO_8859_14:
      return "iso8859-14";

    case VTK_ENCODING_ISO_8859_15:
      return "iso8859-15";

    case VTK_ENCODING_ISO_8859_16:
      return "iso8859-16";

    default:
      return "identity";
    }
}

//----------------------------------------------------------------------------
const char* vtkKWWidget::ConvertInternalStringToTclString(
  const char *str, int no_curly_braces)
{
  if (!str || !this->IsCreated())
    {
    return NULL;
    }

  // Shall we remove the curly braces so that we can use the {%s}
  // syntax to use the string inside this->Script ?

  char *clean_str = NULL;
  if (no_curly_braces && (strchr(str, '{') || strchr(str, '}')))
    {
    clean_str = kwsys::SystemTools::RemoveChars(str, "{}");
    str = clean_str;
    }

  // No encoding known ? The fast way, return unchanged

  int app_encoding = this->GetApplication()->GetCharacterEncoding();
  if (app_encoding != VTK_ENCODING_NONE &&
      app_encoding != VTK_ENCODING_UNKNOWN)
    {
    // Get the Tcl encoding name

    const char *tcl_encoding_name = 
      vtkKWWidget::GetTclCharacterEncodingAsString(app_encoding);

    // Check if we have that encoding
    
    Tcl_Encoding tcl_encoding = 
      Tcl_GetEncoding(
        this->GetApplication()->GetMainInterp(), tcl_encoding_name);
    if (tcl_encoding != NULL)
      {
      Tcl_FreeEncoding(tcl_encoding);
      
      // Convert from that encoding
      
      const char *res = 
        this->Script("encoding convertfrom %s {%s}", tcl_encoding_name, str);

      if (clean_str)
        {
        delete [] clean_str;
        }

      return res;
      }
    }

  // Otherwise just return unchanged (or without the braces)

  if (clean_str)
    {
    const char *res = this->Script("set __foo__ {%s}", str);
    delete [] clean_str;
    return res;
    }
  
  return str;
}

//----------------------------------------------------------------------------
const char* vtkKWWidget::ConvertTclStringToInternalString(
  const char *str, int no_curly_braces)
{
  if (!str || !this->IsCreated())
    {
    return NULL;
    }

  // Shall we remove the curly braces so that we can use the {%s}
  // syntax to use the string inside this->Script ?
  
  char *clean_str = NULL;
  if (no_curly_braces && (strchr(str, '{') || strchr(str, '}')))
    {
    clean_str = kwsys::SystemTools::RemoveChars(str, "{}");
    str = clean_str;
    }

  // No encoding known ? The fast way, return unchanged

  int app_encoding = this->GetApplication()->GetCharacterEncoding();
  if (app_encoding != VTK_ENCODING_NONE &&
      app_encoding != VTK_ENCODING_UNKNOWN)
    {
    // Otherwise try to encode/decode

    const char *res = 
      this->Script("encoding convertfrom identity {%s}", str);
    
    if (clean_str)
      {
      delete [] clean_str;
      }
    
    return res;
    }
  
  // Otherwise just return unchanged (or without the braces)
  
  if (clean_str)
    {
    const char *res = this->Script("set __foo__ {%s}", str);
    delete [] clean_str;
    return res;
    }
  
  return str;
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetTextOption(const char *str, const char *option)
{
  if (!option || !this->IsCreated())
    {
    return;
    }

  const char *val = this->ConvertInternalStringToTclString(str);
  this->Script("catch {%s configure %s {%s}}", 
               this->GetWidgetName(), option, val ? val : "");
}

//----------------------------------------------------------------------------
const char* vtkKWWidget::GetTextOption(const char *option)
{
  if (!option || !this->IsCreated())
    {
    return "";
    }

  const char *val = this->Script("%s cget %s", this->GetWidgetName(), option);
  return this->ConvertTclStringToInternalString(val);
}

//----------------------------------------------------------------------------
int vtkKWWidget::IsAlive()
{
  if (!this->IsCreated())
    {
    return 0;
    }

  return atoi(this->Script("winfo exists %s", this->GetWidgetName()));
}

//----------------------------------------------------------------------------
int vtkKWWidget::IsMapped()
{
  return this->IsAlive() && this->GetApplication()->EvaluateBooleanExpression(
    "winfo ismapped %s", this->GetWidgetName());
}

//----------------------------------------------------------------------------
int vtkKWWidget::IsPacked()
{
  return this->IsCreated() && !this->GetApplication()->EvaluateBooleanExpression(
    "catch {pack info %s}", this->GetWidgetName());
}

//----------------------------------------------------------------------------
int vtkKWWidget::GetNumberOfPackedChildren()
{
  if (!this->IsCreated())
    {
    return 0;
    }
  return atoi(this->Script("llength [pack slaves %s]", this->GetWidgetName()));
}

//----------------------------------------------------------------------------
void vtkKWWidget::Unpack()
{
  this->Script("catch {eval pack forget %s}",
               this->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWWidget::UnpackSiblings()
{
  if (this->GetParent())
    {
    this->Script("catch {eval pack forget [pack slaves %s]} \n "
                 "catch {eval grid forget [grid slaves %s]}",
                 this->GetParent()->GetWidgetName(),
                 this->GetParent()->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWWidget::UnpackChildren()
{
  this->Script("catch {eval pack forget [pack slaves %s]} \n "
               "catch {eval grid forget [grid slaves %s]}",
               this->GetWidgetName(),this->GetWidgetName());
}

//----------------------------------------------------------------------------
int vtkKWWidget::HasDragAndDropHelper()
{
  return this->DragAndDropHelper ? 1 : 0;
}

//----------------------------------------------------------------------------
vtkKWDragAndDropHelper* vtkKWWidget::GetDragAndDropHelper()
{
  // Lazy allocation. Create the drag and drop container only when it is needed

  if (!this->DragAndDropHelper)
    {
    this->DragAndDropHelper = vtkKWDragAndDropHelper::New();
    this->DragAndDropHelper->SetApplication(this->GetApplication());
    this->DragAndDropHelper->SetSource(this);
    }

  return this->DragAndDropHelper;
}

//----------------------------------------------------------------------------
const char* vtkKWWidget::GetAnchorAsString(int anchor)
{
  switch (anchor)
    {
    case ANCHOR_N:
      return "n";
    case ANCHOR_NE:
      return "ne";
    case ANCHOR_E:
      return "e";
    case ANCHOR_SE:
      return "se";
    case ANCHOR_S:
      return "s";
    case ANCHOR_SW:
      return "sw";
    case ANCHOR_W:
      return "w";
    case ANCHOR_NW:
      return "nw";
    case ANCHOR_CENTER:
      return "center";
    default:
      return "";
    }
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetImageOption(int icon_index,
                                 const char *blend_color_option,
                                 const char *image_option)
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  this->SetImageOption(icon, blend_color_option, image_option);
  icon->Delete();
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetImageOption(vtkKWIcon* icon,
                                 const char *blend_color_option,
                                 const char *image_option)
{
  if (!icon)
    {
    return;
    }

  this->SetImageOption(icon->GetData(), 
                       icon->GetWidth(), 
                       icon->GetHeight(), 
                       icon->GetPixelSize(),
                       0,
                       blend_color_option, 
                       image_option);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetImageOption(const unsigned char* data, 
                                 int width, 
                                 int height,
                                 int pixel_size,
                                 unsigned long buffer_length,
                                 const char *blend_color_option,
                                 const char *image_option)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return;
    }

  if (!image_option || !*image_option)
    {
    image_option = "-image";
    }

  if (!this->HasConfigurationOption(image_option))
    {
    return;
    }

  ostrstream image_name;
  image_name << this->GetWidgetName() << "." << &image_option[1] << ends;

#if (TK_MAJOR_VERSION == 8) && (TK_MINOR_VERSION < 4)
  // This work-around is put here to "fix" what looks like a bug
  // in Tk. Without this, there seems to be some weird problems
  // with Tk picking some alpha values for some colors.
  this->Script("catch {destroy %s}", image_name.str());
#endif
  
  if (!vtkKWTkUtilities::UpdatePhoto(this->GetApplication(),
                                     image_name.str(),
                                     data, 
                                     width, height, pixel_size,
                                     buffer_length,
                                     this->GetWidgetName(),
                                     blend_color_option))
    {
    vtkWarningMacro("Error updating Tk photo " << image_name.str());
    image_name.rdbuf()->freeze(0);
    return;
    }

  this->SetImageOption(image_name.str(), image_option);

  image_name.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void vtkKWWidget::SetImageOption(const char *image_name,
                                 const char *image_option)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return;
    }

  if (!image_option || !*image_option)
    {
    image_option = "-image";
    }

  if (!this->HasConfigurationOption(image_option))
    {
    return;
    }

  if (!image_name)
    {
    image_name = "";
    }

  this->Script("%s configure %s {%s}", 
               this->GetWidgetName(), image_option, image_name);
}

//----------------------------------------------------------------------------
void vtkKWWidget::Grab()
{
  if (!this->IsCreated())
    {
    return;
    }

  this->Script("grab %s", this->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWWidget::ReleaseGrab()
{
  if (!this->IsCreated())
    {
    return;
    }

  this->Script("grab release %s", this->GetWidgetName());
}

//----------------------------------------------------------------------------
int vtkKWWidget::IsGrabbed()
{
  if (!this->IsCreated())
    {
    return 0;
    }

  const char *res = this->Script("grab status %s", this->GetWidgetName());
  return (!strcmp(res, "none") ? 0 : 1);
}

//----------------------------------------------------------------------------
void vtkKWWidget::ConfigureOptions(const char* opts)
{
  if (!this->IsCreated() || !opts)
    {
    return;
    }
  this->Script("%s configure %s", this->GetWidgetName(), opts);
}

//----------------------------------------------------------------------------
int vtkKWWidget::TakeScreenDump(const char* fname,
                                int top, int bottom, int left, int right)
{
  return vtkKWTkUtilities::TakeScreenDump(
    this, fname, top, bottom, left, right);
}

//----------------------------------------------------------------------------
void vtkKWWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "BalloonHelpJustification: " 
     << this->GetBalloonHelpJustification() << endl;
  os << indent << "BalloonHelpString: " 
     << (this->BalloonHelpString ? this->BalloonHelpString : "None") << endl;
  os << indent << "Children: " << this->Children << endl;
  os << indent << "Parent: " << this->GetParent() << endl;
  os << indent << "Enabled: " << (this->Enabled ? "On" : "Off") << endl;
  os << indent << "DragAndDropHelper: ";
  if (this->DragAndDropHelper)
    {
    os << this->DragAndDropHelper << endl;
    }
  else
    {
    os << "None" << endl;
    }
  os << indent << "IsCreated: " << (this->IsCreated() ? "Yes" : "No") << endl;
}

