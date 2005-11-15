/*=========================================================================

  Module:    $RCSfile: vtkKWMultiColumnList.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWApplication.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWFrame.h"
#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWIcon.h"
#include "vtkKWCheckButton.h"
#include "vtkKWComboBox.h"
#include "vtkKWRadioButton.h"

#include <vtksys/stl/string>
#include <vtksys/stl/vector>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

#include "Utilities/Tablelist/vtkKWTablelistInit.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMultiColumnList);
vtkCxxRevisionMacro(vtkKWMultiColumnList, "$Revision: 1.40 $");

//----------------------------------------------------------------------------
class vtkKWMultiColumnListInternals
{
public:
  
  vtksys_stl::vector<int> LastSelectionRowIndices;
  vtksys_stl::vector<int> LastSelectionColIndices;

  int EditedCellRowIndex;
  int EditedCellColumnIndex;
};

//----------------------------------------------------------------------------
vtkKWMultiColumnList::vtkKWMultiColumnList()
{
  this->EditStartCommand = NULL;
  this->EditEndCommand = NULL;
  this->CellUpdatedCommand = NULL;
  this->SelectionCommand = NULL;
  this->SelectionChangedCommand = NULL;
  this->PotentialCellColorsChangedCommand = NULL;
  this->ColumnSortedCommand = NULL;

  this->Internals = new vtkKWMultiColumnListInternals;
}

//----------------------------------------------------------------------------
vtkKWMultiColumnList::~vtkKWMultiColumnList()
{
  if (this->EditStartCommand)
    {
    delete [] this->EditStartCommand;
    this->EditStartCommand = NULL;
    }
  if (this->EditEndCommand)
    {
    delete [] this->EditEndCommand;
    this->EditEndCommand = NULL;
    }
  if (this->CellUpdatedCommand)
    {
    delete [] this->CellUpdatedCommand;
    this->CellUpdatedCommand = NULL;
    }
  if (this->SelectionCommand)
    {
    delete [] this->SelectionCommand;
    this->SelectionCommand = NULL;
    }
  if (this->SelectionChangedCommand)
    {
    delete [] this->SelectionChangedCommand;
    this->SelectionChangedCommand = NULL;
    }
  if (this->PotentialCellColorsChangedCommand)
    {
    delete [] this->PotentialCellColorsChangedCommand;
    this->PotentialCellColorsChangedCommand = NULL;
    }
  if (this->ColumnSortedCommand)
    {
    delete [] this->ColumnSortedCommand;
    this->ColumnSortedCommand = NULL;
    }
  delete this->Internals;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::Create(vtkKWApplication *app)
{
  // Use Tablelist class:
  // http://www.nemethi.de/

  vtkKWTablelistInit::Initialize(app ? app->GetMainInterp() : NULL);

  // Call the superclass to set the appropriate flags then create manually

  if (!this->Superclass::CreateSpecificTkWidget(app, "tablelist::tablelist"))
    {
    vtkErrorMacro("Failed creating widget " << this->GetClassName());
    return;
    }

  this->SetBackgroundColor(0.98, 0.98, 0.98);
  this->SetStripeBackgroundColor(0.878, 0.909, 0.941);

#ifdef _WIN32
  this->SetSelectionBackgroundColor(0.0392157, 0.141176, 0.415686);
#else
  this->SetSelectionBackgroundColor(0.988, 1.0, 0.725);
#endif

  this->SetColumnSeparatorsVisibility(1);
  this->SetSortArrowVisibility(0);
  this->SetHighlightThickness(0);
  this->SetSelectionModeToSingle();
  this->SetLabelCommand(NULL, "tablelist::sortByColumn");
  this->SetReliefToSunken();
  this->SetBorderWidth(2);
  this->SetRowSpacing(2);
  this->ExportSelectionOff();

  this->SetConfigurationOption("-activestyle", "none");

  char *command = NULL;
  this->SetObjectMethodCommand(&command, this, "EditStartCallback");
  this->SetConfigurationOption("-editstartcommand", command);
  delete [] command;

  command = NULL;
  this->SetObjectMethodCommand(&command, this, "EditEndCallback");
  this->SetConfigurationOption("-editendcommand", command);
  delete [] command;

  this->AddBinding("<<TablelistSelect>>", this, "SelectionCallback");
  this->AddBinding("<<TablelistCellUpdated>>", this, "CellUpdatedCallback");
  this->AddBinding("<<TablelistColumnSorted>>", this, "ColumnSortedCallback");
  
  // Update enable state

  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetWidth(int width)
{
  this->SetConfigurationOptionAsInt("-width", width);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetWidth()
{
  return this->GetConfigurationOptionAsInt("-width");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetHeight(int height)
{
  this->SetConfigurationOptionAsInt("-height", height);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetHeight()
{
  return this->GetConfigurationOptionAsInt("-height");
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::InsertColumn(int col_index, const char *title)
{
  if (this->IsCreated())
    {
    int nb_columns = this->GetNumberOfColumns();
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script(
      "%s insertcolumns %d 0 {%s}", 
      this->GetWidgetName(), col_index, title ? title : "");
    this->SetState(old_state);
    if (this->GetNumberOfColumns() != nb_columns)
      {
      this->NumberOfColumnsChanged();
      }
    return col_index;
    }
  return -1;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::AddColumn(const char *title)
{
  return this->InsertColumn(this->GetNumberOfColumns(), title);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::NumberOfColumnsChanged()
{
  // Changing the number of columns can potentially affect the selection
  // Check for that

  this->HasSelectionChanged();
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetNumberOfColumns()
{
  if (this->IsCreated())
    {
    const char *val = this->Script("%s columncount", this->GetWidgetName());
    if (val && *val)
      {
      return atoi(val);
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SeeColumn(int col_index)
{
  if (this->IsCreated())
    {
    this->Script("%s seecolumn %d", this->GetWidgetName(), col_index);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::DeleteColumn(int col_index)
{
  if (this->IsCreated())
    {
    int nb_columns = this->GetNumberOfColumns();
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s deletecolumns %d %d", 
                 this->GetWidgetName(), col_index, col_index);
    this->SetState(old_state);
    if (this->GetNumberOfColumns() != nb_columns)
      {
      this->NumberOfColumnsChanged();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::DeleteAllColumns()
{
  if (this->IsCreated())
    {
    int nb_columns = this->GetNumberOfColumns();
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s deletecolumns 0 end", this->GetWidgetName());
    this->SetState(old_state);
    if (this->GetNumberOfColumns() != nb_columns)
      {
      this->NumberOfColumnsChanged();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetMovableColumns(int arg)
{
  this->SetConfigurationOptionAsInt("-movablecolumns", arg ? 1 : 0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetMovableColumns()
{
  return this->GetConfigurationOptionAsInt("-movablecolumns");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetRowSpacing(int arg)
{
  this->SetConfigurationOptionAsInt("-spacing", arg);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetRowSpacing()
{
  return this->GetConfigurationOptionAsInt("-spacing");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetResizableColumns(int arg)
{
  this->SetConfigurationOptionAsInt("-resizablecolumns", arg ? 1 : 0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetResizableColumns()
{
  return this->GetConfigurationOptionAsInt("-resizablecolumns");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnSeparatorsVisibility(int arg)
{
  this->SetConfigurationOptionAsInt("-showseparators", arg ? 1 : 0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnSeparatorsVisibility()
{
  return this->GetConfigurationOptionAsInt("-showseparators");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnLabelsVisibility(int arg)
{
  this->SetConfigurationOptionAsInt("-showlabels", arg ? 1 : 0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnLabelsVisibility()
{
  return this->GetConfigurationOptionAsInt("-showlabels");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetColumnLabelBackgroundColor(
  double *r, double *g, double *b)
{
  vtkKWTkUtilities::GetOptionColor(this, "-labelbackground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetColumnLabelBackgroundColor()
{
  static double rgb[3];
  this->GetColumnLabelBackgroundColor(rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnLabelBackgroundColor(double r, double g, double b)
{
  vtkKWTkUtilities::SetOptionColor(this, "-labelbackground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetColumnLabelForegroundColor(double *r, double *g, double *b)
{
  vtkKWTkUtilities::GetOptionColor(this, "-labelforeground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetColumnLabelForegroundColor()
{
  static double rgb[3];
  this->GetColumnLabelForegroundColor(rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnLabelForegroundColor(double r, double g, double b)
{
  vtkKWTkUtilities::SetOptionColor(this, "-labelforeground", r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnName(int col_index, const char *name)
{
  this->SetColumnConfigurationOptionAsText(col_index, "-name", name);
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetColumnName(int col_index)
{
  return this->GetColumnConfigurationOptionAsText(col_index, "-name");
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnIndexWithName(const char *name)
{
  if (this->IsCreated() && name)
    {
    int fail = atoi(
      this->Script("catch {%s columnindex {%s}} %s_foo", 
                   this->GetWidgetName(), name, this->GetTclName()));
    if (!fail)
      {
      return atoi(this->Script("set %s_foo", this->GetTclName()));
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnTitle(int col_index, const char *title)
{
  this->SetColumnConfigurationOptionAsText(col_index, "-title", title);
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetColumnTitle(int col_index)
{
  return this->GetColumnConfigurationOptionAsText(col_index, "-title");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnWidth(int col_index, int width)
{
  this->SetColumnConfigurationOptionAsInt(col_index, "-width", width);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnWidth(int col_index)
{
  return this->GetColumnConfigurationOptionAsInt(col_index, "-width");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnMaximumWidth(int col_index, int width)
{
  this->SetColumnConfigurationOptionAsInt(col_index, "-maxwidth", width);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnMaximumWidth(int col_index)
{
  return this->GetColumnConfigurationOptionAsInt(col_index, "-maxwidth");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnStretchable(int col_index, int flag)
{
  this->SetColumnConfigurationOptionAsInt(col_index, "-stretchable",flag?1:0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnStretchable(int col_index)
{
  return this->GetColumnConfigurationOptionAsInt(col_index, "-stretchable");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetStretchableColumns(int arg)
{
  this->SetConfigurationOption("-stretch", arg ? "all" : "");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnEditWindow(int col_index, int arg)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *arg_opt;
  switch (arg)
    {
    case vtkKWMultiColumnList::ColumnEditWindowEntry:
      arg_opt = "entry";
      break;
    case vtkKWMultiColumnList::ColumnEditWindowCheckButton:
      arg_opt = "checkbutton";
      break;
    case vtkKWMultiColumnList::ColumnEditWindowSpinBox:
      arg_opt = "spinbox";
      break;
    default:
      arg_opt = "entry";
      break;
    }
  this->SetColumnConfigurationOption(col_index, "-editwindow", arg_opt);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnEditWindow(int col_index)
{
  if (this->IsCreated())
    {
    const char *val = 
      this->GetColumnConfigurationOption(col_index, "-editwindow");
    if (val && *val)
      {
      if (!strcmp(val, "entry"))
        {
        return vtkKWMultiColumnList::ColumnEditWindowEntry;
        }
      if (!strcmp(val, "checkbutton"))
        {
        return vtkKWMultiColumnList::ColumnEditWindowCheckButton;
        }
      if (!strcmp(val, "spinbox"))
        {
        return vtkKWMultiColumnList::ColumnEditWindowSpinBox;
        }
      }
    }

  return vtkKWMultiColumnList::ColumnEditWindowUnknown;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnAlignment(int col_index, int align)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *alignment_opt;
  switch (align)
    {
    case vtkKWMultiColumnList::ColumnAlignmentLeft:
      alignment_opt = "left";
      break;
    case vtkKWMultiColumnList::ColumnAlignmentRight:
      alignment_opt = "right";
      break;
    case vtkKWMultiColumnList::ColumnAlignmentCenter:
      alignment_opt = "center";
      break;
    default:
      alignment_opt = "left";
      break;
    }
  this->SetColumnConfigurationOption(col_index, "-align", alignment_opt);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnAlignment(int col_index)
{
  if (this->IsCreated())
    {
    const char *val = this->GetColumnConfigurationOption(col_index, "-align");
    if (val && *val)
      {
      if (!strcmp(val, "left"))
        {
        return vtkKWMultiColumnList::ColumnAlignmentLeft;
        }
      if (!strcmp(val, "right"))
        {
        return vtkKWMultiColumnList::ColumnAlignmentRight;
        }
      if (!strcmp(val, "center"))
        {
        return vtkKWMultiColumnList::ColumnAlignmentCenter;
        }
      }
    }

  return vtkKWMultiColumnList::ColumnAlignmentUnknown;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnLabelAlignment(int col_index, int align)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *alignment_opt;
  switch (align)
    {
    case vtkKWMultiColumnList::ColumnAlignmentLeft:
      alignment_opt = "left";
      break;
    case vtkKWMultiColumnList::ColumnAlignmentRight:
      alignment_opt = "right";
      break;
    case vtkKWMultiColumnList::ColumnAlignmentCenter:
      alignment_opt = "center";
      break;
    default:
      alignment_opt = "left";
      break;
    }
  this->SetColumnConfigurationOption(col_index, "-labelalign", alignment_opt);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnLabelAlignment(int col_index)
{
  if (this->IsCreated())
    {
    const char *val = 
      this->GetColumnConfigurationOption(col_index, "-labelalign");
    if (val && *val)
      {
      if (!strcmp(val, "left"))
        {
        return vtkKWMultiColumnList::ColumnAlignmentLeft;
        }
      if (!strcmp(val, "right"))
        {
        return vtkKWMultiColumnList::ColumnAlignmentRight;
        }
      if (!strcmp(val, "center"))
        {
        return vtkKWMultiColumnList::ColumnAlignmentCenter;
        }
      }
    }

  return vtkKWMultiColumnList::ColumnAlignmentUnknown;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnResizable(int col_index, int flag)
{
  this->SetColumnConfigurationOptionAsInt(col_index, "-resizable", flag ?1:0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnResizable(int col_index)
{
  return this->GetColumnConfigurationOptionAsInt(col_index, "-resizable");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnEditable(int col_index, int flag)
{
  this->SetColumnConfigurationOptionAsInt(col_index, "-editable", flag ?1:0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnEditable(int col_index)
{
  return this->GetColumnConfigurationOptionAsInt(col_index, "-editable");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnVisibility(int col_index, int flag)
{
  this->SetColumnConfigurationOptionAsInt(col_index, "-hide", flag ?0:1);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnVisibility(int col_index)
{
  return this->GetColumnConfigurationOptionAsInt(col_index, "-hide") ? 0 : 1;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetColumnBackgroundColor(
  int col_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetColumnConfigurationOption(col_index, "-bg"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetColumnBackgroundColor(int col_index)
{
  static double rgb[3];
  this->GetColumnBackgroundColor(col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnBackgroundColor(
  int col_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetColumnConfigurationOption(col_index, "-bg", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetColumnForegroundColor(
  int col_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetColumnConfigurationOption(col_index, "-fg"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetColumnForegroundColor(int col_index)
{
  static double rgb[3];
  this->GetColumnForegroundColor(col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnForegroundColor(
  int col_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetColumnConfigurationOption(col_index, "-fg", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnLabelImage(
  int col_index, const char *image_name)
{
  this->SetColumnConfigurationOption(col_index, "-labelimage", image_name);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnLabelImageToPredefinedIcon(
  int col_index, int icon_index)
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  this->SetColumnLabelImageToIcon(col_index, icon);
  icon->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnLabelImageToIcon(
  int col_index, vtkKWIcon* icon)
{
  if (icon)
    {
    this->SetColumnLabelImageToPixels(
      col_index, 
      icon->GetData(), 
      icon->GetWidth(), icon->GetHeight(), icon->GetPixelSize());
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnLabelImageToPixels(
  int col_index,
  const unsigned char* pixels, 
  int width, 
  int height,
  int pixel_size,
  unsigned long buffer_length)
{
  static int col_label_img_counter = 0;

  if (!this->IsCreated())
    {
    return;
    }

  // Use the prev pic, or create a new one

  vtksys_stl::string image_name(
    this->GetColumnConfigurationOption(col_index, "-labelimage"));
  if (!image_name.size())
    {
    char buffer[30];
    sprintf(buffer, ".col_label_img_%d", col_label_img_counter++);
    image_name = this->GetWidgetName();
    image_name += buffer;
    }

  if (!vtkKWTkUtilities::UpdatePhoto(this->GetApplication(),
                                     image_name.c_str(),
                                     pixels, 
                                     width, height, pixel_size,
                                     buffer_length))
    {
    vtkWarningMacro(
      << "Error updating Tk photo " << image_name.c_str());
    return;
    }

  this->SetColumnLabelImage(col_index, image_name.c_str());
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SortByColumn(int col_index, int order)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *sort_opt;
  switch (order)
    {
    case vtkKWMultiColumnList::SortByIncreasingOrder:
      sort_opt = "-increasing";
      break;
    case vtkKWMultiColumnList::SortByDecreasingOrder:
      sort_opt = "-decreasing";
      break;
    default:
      sort_opt = "-increasing";
      break;
    }

  this->Script(
    "%s sortbycolumn %d %s", this->GetWidgetName(), col_index,  sort_opt);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnSortMode(int col_index, int mode)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *sortmode_opt;
  switch (mode)
    {
    case vtkKWMultiColumnList::SortModeAscii:
      sortmode_opt = "ascii";
      break;
    case vtkKWMultiColumnList::SortModeDictionary:
      sortmode_opt = "dictionary";
      break;
    case vtkKWMultiColumnList::SortModeInteger:
      sortmode_opt = "integer";
      break;
    case vtkKWMultiColumnList::SortModeReal:
      sortmode_opt = "real";
      break;
    default:
      sortmode_opt = "ascii";
      break;
    }
  this->SetColumnConfigurationOption(col_index, "-sortmode", sortmode_opt);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnSortMode(int col_index)
{
  if (this->IsCreated())
    {
    const char *val = 
      this->GetColumnConfigurationOption(col_index, "-sortmode");
    if (val && *val)
      {
      if (!strcmp(val, "ascii"))
        {
        return vtkKWMultiColumnList::SortModeAscii;
        }
      if (!strcmp(val, "dictionary"))
        {
        return vtkKWMultiColumnList::SortModeDictionary;
        }
      if (!strcmp(val, "integer"))
        {
        return vtkKWMultiColumnList::SortModeInteger;
        }
      if (!strcmp(val, "real"))
        {
        return vtkKWMultiColumnList::SortModeReal;
        }
      }
    }

  return vtkKWMultiColumnList::SortModeUnknown;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetSortArrowVisibility(int arg)
{
  this->SetConfigurationOptionAsInt("-showarrow", arg ? 1 : 0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetSortArrowVisibility()
{
  return this->GetConfigurationOptionAsInt("-showarrow");
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::SetColumnConfigurationOption(
  int col_index, const char *option, const char *value)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return 0;
    }

  if (!option || !value)
    {
    vtkWarningMacro("Wrong option or value !");
    return 0;
    }

  const char *res = 
    this->Script("%s columnconfigure %d %s {%s}", 
                 this->GetWidgetName(), col_index, option, value);

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
      << widget_name.c_str() << ") at column: " << col_index 
      << " with option: [" << option 
      << "] and value [" << value << "] => " << err_msg.c_str());
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::HasColumnConfigurationOption(
  int col_index, const char *option)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return 0;
    }

  return (this->GetApplication() && 
          !this->GetApplication()->EvaluateBooleanExpression(
            "catch {%s columncget %d %s}",
            this->GetWidgetName(), col_index, option));
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetColumnConfigurationOption(
  int col_index, const char* option)
{
  if (!this->HasColumnConfigurationOption(col_index, option))
    {
    return NULL;
    }

  return this->Script(
    "%s columncget %d %s", this->GetWidgetName(), col_index, option);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::SetColumnConfigurationOptionAsInt(
  int col_index, const char *option, int value)
{
  char buffer[20];
  sprintf(buffer, "%d", value);
  return this->SetColumnConfigurationOption(col_index, option, buffer);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetColumnConfigurationOptionAsInt(
  int col_index, const char* option)
{
  if (!this->HasColumnConfigurationOption(col_index, option))
    {
    return 0;
    }

  return atoi(
    this->Script("%s columncget %d %s", 
                 this->GetWidgetName(), col_index, option));
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnConfigurationOptionAsText(
  int col_index, const char *option, const char *value)
{
  if (!option || !this->IsCreated())
    {
    return;
    }

  const char *val = this->ConvertInternalStringToTclString(
    value, vtkKWCoreWidget::ConvertStringEscapeInterpretable);
  this->Script("%s columnconfigure %d %s \"%s\"", 
               this->GetWidgetName(), col_index, option, val ? val : "");
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetColumnConfigurationOptionAsText(
  int col_index, const char *option)
{
  if (!option || !this->IsCreated())
    {
    return "";
    }

  return this->ConvertTclStringToInternalString(
    this->GetColumnConfigurationOption(col_index, option));
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnFormatCommand(int col_index, 
                                                  vtkObject* object, 
                                                  const char *method)
{
  if (this->IsCreated())
    {
    char *command = NULL;
    this->SetObjectMethodCommand(&command, object, method);
    this->SetColumnConfigurationOption(col_index, "-formatcommand", command);
    delete [] command;
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnFormatCommandToEmptyOutput(int col_index)
{
  this->SetColumnFormatCommand(col_index, NULL, "tablelist::emptyStr");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetMovableRows(int arg)
{
  this->SetConfigurationOptionAsInt("-movablerows", arg ? 1 : 0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetMovableRows()
{
  return this->GetConfigurationOptionAsInt("-movablerows");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InsertRow(int row_index)
{
  if (this->IsCreated())
    {
    int nb_cols = this->GetNumberOfColumns();
    if (nb_cols > 0)
      {
      vtksys_stl::string item;
      for (int i = 0; i < nb_cols; i++)
        {
        item += "\"\" ";
        }
      int nb_rows = this->GetNumberOfRows();
      int old_state = this->GetState();
      if (this->GetState() != vtkKWTkOptions::StateNormal)
        {
        this->SetStateToNormal();
        }
      this->Script("%s insert %d {%s}", 
                   this->GetWidgetName(), row_index, item.c_str());
      this->SetState(old_state);
      if (this->GetNumberOfRows() != nb_rows)
        {
        this->NumberOfRowsChanged();
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::NumberOfRowsChanged()
{
  // Changing the number of columns can potentially affect the selection
  // Check for that

  this->HasSelectionChanged();

  // Trigger this because inserting/removing rows can change the background
  // color of a row (given the stripes, or the specific row colors, etc.)

  this->InvokePotentialCellColorsChangedCommand();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::AddRow()
{
  this->InsertRow(this->GetNumberOfRows());
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetNumberOfRows()
{
  if (this->IsCreated())
    {
    const char *val = this->Script("%s size", this->GetWidgetName());
    if (val && *val)
      {
      return atoi(val);
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SeeRow(int row_index)
{
  if (this->IsCreated())
    {
    this->Script("%s see %d", this->GetWidgetName(), row_index);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::DeleteRow(int row_index)
{
  if (this->IsCreated())
    {
    int nb_rows = this->GetNumberOfRows();
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s delete %d %d", 
                 this->GetWidgetName(), row_index, row_index);
    this->SetState(old_state);
    if (this->GetNumberOfRows() != nb_rows)
      {
      this->NumberOfRowsChanged();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::DeleteAllRows()
{
  if (this->IsCreated())
    {
    int nb_rows = this->GetNumberOfRows();
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s delete 0 end", this->GetWidgetName());
    this->SetState(old_state);
    if (this->GetNumberOfRows() != nb_rows)
      {
      this->NumberOfRowsChanged();
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetRowBackgroundColor(
  int row_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetRowConfigurationOption(row_index, "-bg"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetRowBackgroundColor(int row_index)
{
  static double rgb[3];
  this->GetRowBackgroundColor(row_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetRowBackgroundColor(
  int row_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetRowConfigurationOption(row_index, "-bg", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetRowForegroundColor(
  int row_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetRowConfigurationOption(row_index, "-fg"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetRowForegroundColor(int row_index)
{
  static double rgb[3];
  this->GetRowForegroundColor(row_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetRowForegroundColor(
  int row_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetRowConfigurationOption(row_index, "-fg", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetStripeBackgroundColor(
  double *r, double *g, double *b)
{
  vtkKWTkUtilities::GetOptionColor(this, "-stripebackground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetStripeBackgroundColor()
{
  static double rgb[3];
  this->GetStripeBackgroundColor(rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetStripeBackgroundColor(double r, double g, double b)
{
  vtkKWTkUtilities::SetOptionColor(this, "-stripebackground", r, g, b);
  this->InvokePotentialCellColorsChangedCommand();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetStripeForegroundColor(double *r, double *g, double *b)
{
  vtkKWTkUtilities::GetOptionColor(this, "-stripeforeground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetStripeForegroundColor()
{
  static double rgb[3];
  this->GetStripeForegroundColor(rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetStripeForegroundColor(double r, double g, double b)
{
  vtkKWTkUtilities::SetOptionColor(this, "-stripeforeground", r, g, b);
  this->InvokePotentialCellColorsChangedCommand();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetStripeHeight(int height)
{
  this->SetConfigurationOptionAsInt("-stripeheight", height);
  // Trigger this method since the stripe will be re-organized, hence
  // the background color of a cell will change
  this->InvokePotentialCellColorsChangedCommand();
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetStripeHeight()
{
  return this->GetConfigurationOptionAsInt("-stripeheight");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetRowSelectable(int row_index, int flag)
{
  this->SetRowConfigurationOptionAsInt(row_index, "-selectable", flag ?1:0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetRowSelectable(int row_index)
{
  return this->GetRowConfigurationOptionAsInt(row_index, "-selectable");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::ActivateRow(int row_index)
{
  if (this->IsCreated())
    {
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s activate %d", this->GetWidgetName(), row_index);
    this->SetState(old_state);
    }
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::SetRowConfigurationOption(
  int row_index, const char *option, const char *value)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return 0;
    }

  if (!option || !value)
    {
    vtkWarningMacro("Wrong option or value !");
    return 0;
    }

  const char *res = 
    this->Script("%s rowconfigure %d %s {%s}", 
                 this->GetWidgetName(), row_index, option, value);

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
      << widget_name.c_str() << ") at row: " << row_index 
      << " with option: [" << option 
      << "] and value [" << value << "] => " << err_msg.c_str());
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::HasRowConfigurationOption(
  int row_index, const char *option)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return 0;
    }

  return (this->GetApplication() && 
          !this->GetApplication()->EvaluateBooleanExpression(
            "catch {%s rowcget %d %s}",
            this->GetWidgetName(), row_index, option));
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetRowConfigurationOption(
  int row_index, const char* option)
{
  if (!this->HasRowConfigurationOption(row_index, option))
    {
    return NULL;
    }

  return this->Script(
    "%s rowcget %d %s", this->GetWidgetName(), row_index, option);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::SetRowConfigurationOptionAsInt(
  int row_index, const char *option, int value)
{
  char buffer[20];
  sprintf(buffer, "%d", value);
  return this->SetRowConfigurationOption(row_index, option, buffer);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetRowConfigurationOptionAsInt(
  int row_index, const char* option)
{
  if (!this->HasRowConfigurationOption(row_index, option))
    {
    return 0;
    }

  return atoi(
    this->Script("%s rowcget %d %s", 
                 this->GetWidgetName(), row_index, option));
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InsertCellText(
  int row_index, int col_index, const char *text)
{
  if (this->IsCreated() && text)
    {
    while (row_index > this->GetNumberOfRows() - 1)
      {
      this->AddRow();
      }
    this->SetCellText(row_index, col_index, text);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InsertCellTextAsInt(
  int row_index, int col_index, int value)
{
  char tmp[1024];
  sprintf(tmp, "%d", value);
  this->InsertCellText(row_index, col_index, tmp);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InsertCellTextAsDouble(
  int row_index, int col_index, double value)
{
  char tmp[1024];
  sprintf(tmp, "%.5g", value);
  this->InsertCellText(row_index, col_index, tmp);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InsertCellTextAsFormattedDouble(
  int row_index, int col_index, double value, int size)
{
  char format[1024];
  sprintf(format, "%%.%dg", size);
  char tmp[1024];
  sprintf(tmp, format, value);
  this->InsertCellText(row_index, col_index, tmp);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellText(
  int row_index, int col_index, const char *text)
{
  int old_state = this->GetState();
  if (this->GetState() != vtkKWTkOptions::StateNormal)
    {
    this->SetStateToNormal();
    }
  this->SetCellConfigurationOptionAsText(row_index, col_index, "-text", text);
  this->SetState(old_state);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellTextAsInt(
  int row_index, int col_index, int value)
{
  char tmp[1024];
  sprintf(tmp, "%d", value);
  this->SetCellText(row_index, col_index, tmp);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellTextAsDouble(
  int row_index, int col_index, double value)
{
  char tmp[1024];
  sprintf(tmp, "%.5g", value);
  this->SetCellText(row_index, col_index, tmp);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellTextAsFormattedDouble(
  int row_index, int col_index, double value, int size)
{
  char format[1024];
  sprintf(format, "%%.%dg", size);
  char tmp[1024];
  sprintf(tmp, format, value);
  this->SetCellText(row_index, col_index, tmp);
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetCellText(int row_index, int col_index)
{
  return this->GetCellConfigurationOptionAsText(row_index, col_index, "-text");
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetCellTextAsInt(int row_index, int col_index)
{
  return atoi(this->GetCellConfigurationOption(row_index, col_index, "-text"));
}

//----------------------------------------------------------------------------
double vtkKWMultiColumnList::GetCellTextAsDouble(int row_index, int col_index)
{
  return atof(this->GetCellConfigurationOption(row_index, col_index, "-text"));
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InsertRowText(int row_index, const char *text)
{
  int nb_cols = this->GetNumberOfColumns();
  for (int i = 0; i < nb_cols; i++)
    {
    this->InsertCellText(row_index, i, text);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InsertColumnText(int col_index, const char *text)
{
  int nb_rows = this->GetNumberOfRows();
  for (int i = 0; i < nb_rows; i++)
    {
    this->InsertCellText(i, col_index, text);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::FindAndInsertCellText(
    int look_for_col_index, const char *look_for_text, 
    int col_index, const char *text)
{
  int row_index = 
    this->FindCellTextInColumn(look_for_col_index, look_for_text);
  
  if (row_index < 0)
    {
    row_index = this->GetNumberOfRows();
    this->InsertRow(row_index);
    this->InsertCellText(row_index, look_for_col_index, look_for_text);
    }

  this->InsertCellText(row_index, col_index, text);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::ActivateCell(int row_index, int col_index)
{
  if (this->IsCreated())
    {
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s activate %d,%d", 
                 this->GetWidgetName(), row_index, col_index);
    this->SetState(old_state);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SeeCell(int row_index, int col_index)
{
  if (this->IsCreated())
    {
    this->Script("%s seecell %d,%d", 
                 this->GetWidgetName(), row_index, col_index);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetCellBackgroundColor(
  int row_index, int col_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetCellConfigurationOption(row_index, col_index, "-bg"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetCellBackgroundColor(
  int row_index, int col_index)
{
  static double rgb[3];
  this->GetCellBackgroundColor(row_index, col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellBackgroundColor(
  int row_index, int col_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetCellConfigurationOption(row_index, col_index, "-bg", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetCellForegroundColor(
  int row_index, int col_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetCellConfigurationOption(row_index, col_index, "-fg"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetCellForegroundColor(int row_index, int col_index)
{
  static double rgb[3];
  this->GetCellForegroundColor(row_index, col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellForegroundColor(
  int row_index, int col_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetCellConfigurationOption(row_index, col_index, "-fg", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetCellCurrentBackgroundColor(
  int row_index, int col_index, double *r, double *g, double *b)
{
  const char *bgcolor;

  // If disabled, everything is background

  if (!this->GetEnabled())
    {
    this->GetBackgroundColor(r, g, b);
    return;
    }

  // Selection has priority

  if (this->IsCellSelected(row_index, col_index))
    {
    // Cell selection color has priority, then row, column

    bgcolor = this->GetCellConfigurationOption(
      row_index, col_index, "-selectbackground");
    if (bgcolor && *bgcolor)
      {
      this->GetCellSelectionBackgroundColor(row_index, col_index, r, g, b);
      return;
      }

    bgcolor = this->GetRowConfigurationOption(row_index, "-selectbackground");
    if (bgcolor && *bgcolor)
      {
      this->GetRowSelectionBackgroundColor(row_index, r, g, b);
      return;
      }

    bgcolor = this->GetColumnConfigurationOption(
      col_index, "-selectbackground");
    if (bgcolor && *bgcolor)
      {
      this->GetColumnSelectionBackgroundColor(col_index, r, g, b);
      return;
      }

    this->GetSelectionBackgroundColor(r, g, b);
    return;
    }

  // Cell color has priority

  bgcolor =  this->GetCellConfigurationOption(row_index, col_index, "-bg");
  if (bgcolor && *bgcolor)
    {
    this->GetCellBackgroundColor(row_index, col_index, r, g, b);
    return;
    }
  
  // Then row color

  bgcolor = this->GetRowConfigurationOption(row_index, "-bg");
  if (bgcolor && *bgcolor)
    {
    this->GetRowBackgroundColor(row_index, r, g, b);
    return;
    }

  // Then stripe color, if within a stripe

  bgcolor = this->GetConfigurationOption("-stripebackground");
  if (bgcolor && *bgcolor)
    {
    int stripeh = this->GetStripeHeight();
    if ((row_index / stripeh) & 1)
      {
      this->GetStripeBackgroundColor(r, g, b);
      return ;
      }
    }
  
  // Then column color

  bgcolor = this->GetColumnConfigurationOption(col_index, "-bg");
  if (bgcolor && *bgcolor)
    {
    this->GetColumnBackgroundColor(col_index, r, g, b);
    return;
    }

  // Then background color

  this->GetBackgroundColor(r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetCellCurrentBackgroundColor(
  int row_index, int col_index)
{
  static double rgb[3];
  this->GetCellCurrentBackgroundColor(
    row_index, col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetCellCurrentForegroundColor(
  int row_index, int col_index, double *r, double *g, double *b)
{
  const char *fgcolor;

  // If disabled, everything is disabledforeground

  if (!this->GetEnabled())
    {
    vtkKWTkUtilities::GetOptionColor(this, "-disabledforeground", r, g, b);
    return;
    }

  // Selection has priority

  if (this->IsCellSelected(row_index, col_index))
    {
    // Cell selection color has priority, then row, column

    fgcolor = this->GetCellConfigurationOption(
      row_index, col_index, "-selectforeground");
    if (fgcolor && *fgcolor)
      {
      this->GetCellSelectionForegroundColor(row_index, col_index, r, g, b);
      return;
      }

    fgcolor = this->GetRowConfigurationOption(row_index, "-selectforeground");
    if (fgcolor && *fgcolor)
      {
      this->GetRowSelectionForegroundColor(row_index, r, g, b);
      return;
      }

    fgcolor = this->GetColumnConfigurationOption(
      col_index, "-selectforeground");
    if (fgcolor && *fgcolor)
      {
      this->GetColumnSelectionForegroundColor(col_index, r, g, b);
      return;
      }

    this->GetSelectionForegroundColor(r, g, b);
    return;
    }

  // Cell color has priority

  fgcolor =  this->GetCellConfigurationOption(row_index, col_index, "-fg");
  if (fgcolor && *fgcolor)
    {
    this->GetCellForegroundColor(row_index, col_index, r, g, b);
    return;
    }
  
  // Then row color

  fgcolor = this->GetRowConfigurationOption(row_index, "-fg");
  if (fgcolor && *fgcolor)
    {
    this->GetRowForegroundColor(row_index, r, g, b);
    return;
    }

  // Then stripe color, if within a stripe

  fgcolor = this->GetConfigurationOption("-stripeforeground");
  if (fgcolor && *fgcolor)
    {
    int stripeh = this->GetStripeHeight();
    if ((row_index / stripeh) & 1)
      {
      this->GetStripeForegroundColor(r, g, b);
      return ;
      }
    }
  
  // Then column color

  fgcolor = this->GetColumnConfigurationOption(col_index, "-fg");
  if (fgcolor && *fgcolor)
    {
    this->GetColumnForegroundColor(col_index, r, g, b);
    return;
    }

  // Then foreground color

  this->GetForegroundColor(r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetCellCurrentForegroundColor(
  int row_index, int col_index)
{
  static double rgb[3];
  this->GetCellCurrentForegroundColor(
    row_index, col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellEditable(
  int row_index, int col_index, int flag)
{
  this->SetCellConfigurationOptionAsInt(
    row_index, col_index, "-editable", flag ? 1 : 0);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetCellEditable(
  int row_index, int col_index)
{
  return this->GetCellConfigurationOptionAsInt(
    row_index, col_index, "-editable");
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetCellImage(
  int row_index, int col_index)
{
  return this->GetCellConfigurationOption(
    row_index, col_index, "-image");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellImage(
  int row_index, int col_index, const char *image_name)
{
  int old_state = this->GetState();
  if (this->GetState() != vtkKWTkOptions::StateNormal)
    {
    this->SetStateToNormal();
    }
  this->SetCellConfigurationOption(
    row_index, col_index, "-image", image_name);
  this->SetState(old_state);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellImageToPredefinedIcon(
  int row_index, int col_index, int icon_index)
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  this->SetCellImageToIcon(row_index, col_index, icon);
  icon->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellImageToIcon(
  int row_index, int col_index, vtkKWIcon* icon)
{
  if (icon)
    {
    this->SetCellImageToPixels(
      row_index, col_index, 
      icon->GetData(), 
      icon->GetWidth(), icon->GetHeight(), icon->GetPixelSize());
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellImageToPixels(
  int row_index, int col_index,
  const unsigned char* pixels, 
  int width, 
  int height,
  int pixel_size,
  unsigned long buffer_length)
{
  static int cell_img_counter = 0;

  if (!this->IsCreated())
    {
    return;
    }

  // Use the prev pic, or create a new one

  vtksys_stl::string image_name(
    this->GetCellConfigurationOption(row_index, col_index, "-image"));
  if (!image_name.size())
    {
    char buffer[30];
    sprintf(buffer, ".cell_img_%d", cell_img_counter++);
    image_name = this->GetWidgetName();
    image_name += buffer;
    }

  if (!vtkKWTkUtilities::UpdatePhoto(this->GetApplication(),
                                     image_name.c_str(),
                                     pixels, 
                                     width, height, pixel_size,
                                     buffer_length))
    {
    vtkWarningMacro(
      << "Error updating Tk photo " << image_name.c_str());
    return;
    }

  this->SetCellImage(row_index, col_index, image_name.c_str());
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellEditWindow(
  int row_index, int col_index, int arg)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *arg_opt;
  switch (arg)
    {
    case vtkKWMultiColumnList::CellEditWindowEntry:
      arg_opt = "entry";
      break;
    case vtkKWMultiColumnList::CellEditWindowCheckButton:
      arg_opt = "checkbutton";
      break;
    case vtkKWMultiColumnList::CellEditWindowSpinBox:
      arg_opt = "spinbox";
      break;
    default:
      arg_opt = "entry";
      break;
    }
  this->SetCellConfigurationOption(
    row_index, col_index, "-editwindow", arg_opt);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetCellEditWindow(int row_index, int col_index)
{
  if (this->IsCreated())
    {
    const char *val = 
      this->GetCellConfigurationOption(row_index, col_index, "-editwindow");
    if (val && *val)
      {
      if (!strcmp(val, "entry"))
        {
        return vtkKWMultiColumnList::CellEditWindowEntry;
        }
      if (!strcmp(val, "checkbutton"))
        {
        return vtkKWMultiColumnList::CellEditWindowCheckButton;
        }
      if (!strcmp(val, "spinbox"))
        {
        return vtkKWMultiColumnList::CellEditWindowSpinBox;
        }
      }
    }

  return vtkKWMultiColumnList::CellEditWindowUnknown;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellWindowCommand(int row_index, 
                                                int col_index, 
                                                vtkObject* object, 
                                                const char *method)
{
  if (this->IsCreated())
    {
    char *command = NULL;
    this->SetObjectMethodCommand(&command, object, method);
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->SetCellConfigurationOption(
      row_index, col_index, "-window", command);
    this->SetState(old_state);
    delete [] command;
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellWindowDestroyCommand(int row_index, 
                                                       int col_index, 
                                                       vtkObject* object, 
                                                       const char *method)
{
  if (this->IsCreated())
    {
    char *command = NULL;
    this->SetObjectMethodCommand(&command, object, method);
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->SetCellConfigurationOption(
      row_index, col_index, "-windowdestroy", command);
    this->SetState(old_state);
    delete [] command;
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellWindowDestroyCommandToRemoveChild(
  int row_index, 
  int col_index)
{
  this->SetCellWindowDestroyCommand(
    row_index, col_index, this, "CellWindowDestroyRemoveChildCallback");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::CellWindowDestroyRemoveChildCallback(
  const char *, int, int, const char *widget)
{
  vtkKWWidget *child = this->GetChildWidgetWithName(widget);
  if (child)
    {
    child->SetParent(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::RefreshCellWithWindowCommand(int row_index, 
                                                        int col_index)
{
  const char *command = 
    this->GetCellConfigurationOption(row_index, col_index, "-window");
  if (command && *command)
    {
    vtksys_stl::string command_str(command);
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->SetCellConfigurationOption(
      row_index, col_index, "-window", "");
    this->SetCellConfigurationOption(
      row_index, col_index, "-window", command_str.c_str());
    this->SetState(old_state);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::RefreshAllCellsWithWindowCommand()
{
  int nb_rows = this->GetNumberOfRows();
  int nb_cols = this->GetNumberOfColumns();
  for (int row = 0; row < nb_rows; row++)
    {
    for (int col = 0; col < nb_cols; col++)
      {
      this->RefreshCellWithWindowCommand(row, col);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::RefreshColorsOfCellWithWindowCommand(
  int row_index, 
  int col_index)
{
  const char *command = 
    this->GetCellConfigurationOption(row_index, col_index, "-window");
  if (command && *command)
    {
    const char *child_name = 
      this->GetCellWindowWidgetName(row_index, col_index);
    if (child_name && *child_name)
      {
      vtkKWCoreWidget *child = vtkKWCoreWidget::SafeDownCast(
        this->GetChildWidgetWithName(child_name));
      if (child)
        {
        int is_frame = vtkKWFrame::SafeDownCast(child) ? 1 : 0;
        int is_check = (vtkKWCheckButton::SafeDownCast(child) || 
                        vtkKWRadioButton::SafeDownCast(child)) ? 1 : 0;
        double br, bg, bb, fr, fg, fb;
        this->GetCellCurrentBackgroundColor(
          row_index, col_index, &br, &bg, &bb);
        this->GetCellCurrentForegroundColor(
          row_index, col_index, &fr, &fg, &fb);
        int nb_grand_children = child->GetNumberOfChildren();

        // If it is a frame and it has no children, let's assume that
        // frame is used for a specific purpose other than being just
        // a container, and don't modify its color (maybe a color button ?)
        // unless the widget is disabled in that case do it anyhow
        if (!(is_frame && nb_grand_children == 0) || !this->GetEnabled())
          {
          child->SetBackgroundColor(br, bg, bb);
          }
        // If it a frame, no foreground color option. If it is a check
        // or radio button, do not change its foreground color since it
        // controls the color of the 'tick' mark
        if (!is_frame && !is_check)
          {
          child->SetForegroundColor(fr, fg, fb);
          }
        for (int i = 0; i < nb_grand_children; i++)
          {
          vtkKWCoreWidget *grand_child = vtkKWCoreWidget::SafeDownCast(
            child->GetNthChild(i));
          if (grand_child)
            {
            is_frame = vtkKWFrame::SafeDownCast(grand_child) ? 1 : 0;
            is_check = (vtkKWCheckButton::SafeDownCast(child) || 
                        vtkKWRadioButton::SafeDownCast(child)) ? 1 : 0;
            grand_child->SetBackgroundColor(br, bg, bb);
            if (!is_frame && !is_check)
              {
              grand_child->SetForegroundColor(fr, fg, fb);
              }
            }
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::RefreshColorsOfAllCellsWithWindowCommand()
{
  int nb_rows = this->GetNumberOfRows();
  int nb_cols = this->GetNumberOfColumns();
  for (int row = 0; row < nb_rows; row++)
    {
    for (int col = 0; col < nb_cols; col++)
      {
      this->RefreshColorsOfCellWithWindowCommand(row, col);
      }
    }
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetCellWindowWidgetName(int row_index, 
                                                   int col_index)
{
  if (this->IsCreated())
    {
    return this->Script("%s windowpath %d,%d", 
                        this->GetWidgetName(), row_index, col_index);
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellWindowCommandToCheckButton(
  int row_index, int col_index)
{
  this->SetCellWindowCommand(row_index, col_index, NULL, NULL);
  this->SetCellWindowCommand(
    row_index, col_index, this, "CellWindowCommandToCheckButtonCallback");
  this->SetCellWindowDestroyCommandToRemoveChild(row_index, col_index);
  this->SetCellEditable(row_index, col_index, 0);
}

//---------------------------------------------------------------------------
void vtkKWMultiColumnList::CellWindowCommandToCheckButtonCallback(
  const char *, int row, int col, const char *widget)
{
  vtkKWCheckButton *child = vtkKWCheckButton::New();
  child->SetWidgetName(widget);
  child->SetParent(this);
  child->Create(this->GetApplication());
  child->SetHighlightThickness(0);
  child->SetBorderWidth(0);
  child->SetPadX(0);
  child->SetPadY(0);
  child->SetBackgroundColor(this->GetCellCurrentBackgroundColor(row, col));
  child->SetSelectedState(this->GetCellTextAsInt(row, col));
  child->SetEnabled(this->GetEnabled()); 
  char command[256];
  sprintf(command, 
          "CellWindowCommandToCheckButtonSelectCallback %s %d %d", 
          child->GetTclName(), row, col);
  child->SetCommand(this, command);
  child->Delete();
}

//---------------------------------------------------------------------------
void vtkKWMultiColumnList::CellWindowCommandToCheckButtonSelectCallback(
  vtkKWWidget *widget, int row, int col)
{
  vtkKWCheckButton *cb = vtkKWCheckButton::SafeDownCast(widget);
  if (widget)
    {
    // Make sure we are dealing with the right one
    // Sometimes when a column is sorted, not *all* cells with a user-defined
    // window are re-created. In our case, our user-defined checkbutton
    // has its row,col location coded in its callback. Yet, sorting the
    // column might have moved the checkbutton around, without re-creating
    // it, i.e. without updating its callback. Let's check if this is the
    // case, and look for the right location if not matching.

    if (strcmp(widget->GetWidgetName(), 
               this->GetCellWindowWidgetName(row, col)))
      {
      for (row = 0; row < this->GetNumberOfRows(); row++)
        {
        if (!strcmp(widget->GetWidgetName(), 
                    this->GetCellWindowWidgetName(row, col)))
          {
          break;
          }
        }
      if (row == this->GetNumberOfRows())
        {
        return;
        }
      }

    char cb_state[10];
    sprintf(cb_state, "%d", cb->GetSelectedState());
    int validated = atoi(this->InvokeEditEndCommand(row, col, cb_state));
    cb->SetSelectedState(validated);
    int cell_content = this->GetCellTextAsInt(row, col);
    if (cell_content != validated)
      {
      this->SetCellTextAsInt(row, col, validated);
      sprintf(cb_state, "%d", this->GetCellTextAsInt(row, col));
      this->InvokeCellUpdatedCommand(row, col, cb_state);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellWindowCommandToColorButton(
  int row_index, int col_index)
{
  this->SetCellWindowCommand(row_index, col_index, NULL, NULL);
  this->SetCellWindowCommand(
    row_index, col_index, this, "CellWindowCommandToColorButtonCallback");
  this->SetCellWindowDestroyCommandToRemoveChild(row_index, col_index);
}

//---------------------------------------------------------------------------
void vtkKWMultiColumnList::CellWindowCommandToColorButtonCallback(
  const char *, int row, int col, const char *widget)
{
  vtkKWFrame *child = vtkKWFrame::New();
  child->SetWidgetName(widget);
  child->SetParent(this);
  child->Create(this->GetApplication());
  child->SetBorderWidth(1);
  child->SetReliefToSolid();
  child->SetWidth(16);
  child->SetHeight(16);
  child->SetPadX(0);
  child->SetPadY(0);
  double r, g, b;
  if (!this->GetEnabled() ||
      sscanf(this->GetCellText(row, col), "%lg %lg %lg", &r, &g, &b) != 3)
    {
    this->GetCellCurrentBackgroundColor(row, col, &r, &g, &b);
    }
  child->SetBackgroundColor(r, g, b);
  child->SetEnabled(this->GetEnabled()); 
  this->AddBindingsToWidget(child);
  child->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellWindowCommandToReadOnlyComboBox(
  int row_index, int col_index)
{
  this->SetCellWindowCommand(row_index, col_index, NULL, NULL);
  this->SetCellWindowCommand(
    row_index, col_index, this, "CellWindowCommandToReadOnlyComboBoxCallback");
  this->SetCellWindowDestroyCommandToRemoveChild(row_index, col_index);
  this->SetCellEditable(row_index, col_index, 0);
}

//---------------------------------------------------------------------------
void vtkKWMultiColumnList::CellWindowCommandToReadOnlyComboBoxCallback(
  const char *, int row, int col, const char *widget)
{
  vtkKWComboBox *child = vtkKWComboBox::New();
  child->SetWidgetName(widget);
  child->SetParent(this);
  child->Create(this->GetApplication());
  child->SetHighlightThickness(0);
  child->SetBorderWidth(0);
  // Set the default based on the current column width.. 
  // child->SetWidth( this->GetColumnWidth(col)-2 );
  child->SetWidth( 20 );
  child->SetBackgroundColor(this->GetCellCurrentBackgroundColor(row, col));
  child->SetEnabled(this->GetEnabled()); 
  child->ReadOnlyOn();
  child->Delete();
}

//---------------------------------------------------------------------------
void vtkKWMultiColumnList::SetNthEntryInReadOnlyComboBox( 
                int i, const char *value, int row, int col)
{
  const char *child_name = this->GetCellWindowWidgetName(row, col);
  vtkKWComboBox *child = vtkKWComboBox::SafeDownCast(
                  this->GetChildWidgetWithName(child_name));

  // If it is a combo box
  if ( child )
    {
    // If we have (i+1) entries, remove that entry and add a fresh one
    if ( i < child->GetNumberOfValues() ) 
      {
      child->ReplaceNthValue( i, value );

      // Set the title of the combo box to be the same as the first entry.
      // The main title will be present because of the MulticolumnList anyway
      if ( i == 0 ) 
        {
        child->SetValue( value );    
        }
      }
    else
      {
      child->AddValue( value );

      // Set the title of the combo box to be the same as the first entry.
      if ( child->GetNumberOfValues() == 1 )
        {
        child->SetValue( value );    
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkKWMultiColumnList::DeleteNthEntryInReadOnlyComboBox( 
                                     int i, int row, int col)
{
  const char *child_name = this->GetCellWindowWidgetName(row, col);
  vtkKWComboBox *child = vtkKWComboBox::SafeDownCast(
                  this->GetChildWidgetWithName(child_name));

  // If it is a combo box
  if ( child )
    {
    // If we have (i+1) entries, remove that entry 
    if ( i < child->GetNumberOfValues() ) 
      {
      child->DeleteValue( i );
      }
    }
}

//---------------------------------------------------------------------------
void vtkKWMultiColumnList::DeleteAllEntriesInReadOnlyComboBox( int row, int col)
{
  const char *child_name = this->GetCellWindowWidgetName(row, col);
  vtkKWComboBox *child = vtkKWComboBox::SafeDownCast(
                  this->GetChildWidgetWithName(child_name));

  // If it is a combo box
  if ( child )
    {
    child->DeleteAllValues();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::AddBindingsToWidgetName(const char *widget_name)
{
  if (!this->IsCreated() || !widget_name || !*widget_name)
    {
    return;
    }

  this->Script("bindtags %s [lreplace [bindtags %s] 1 1 TablelistBody]",
               widget_name, widget_name);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::AddBindingsToWidget(vtkKWWidget *widget)
{
  if (!widget || !widget->IsCreated())
    {
    return;
    }

  this->AddBindingsToWidgetName(widget->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::AddBindingsToWidgetAndChildren(vtkKWWidget *widget)
{
  this->AddBindingsToWidget(widget);
  int nb_children = widget->GetNumberOfChildren();
  for (int i = 0; i < nb_children; i++)
    {
    this->AddBindingsToWidget(widget->GetNthChild(i));
    }
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::FindCellText(
  const char *text, int *row_index, int *col_index)
{
  if (this->IsCreated() && 
      text && row_index && col_index)
    {
    int nb_cols = this->GetNumberOfColumns();
    int nb_rows = this->GetNumberOfRows();

    for (int j = 0; j < nb_rows; j++)
      {
      for (int i = 0; i < nb_cols; i++)
        {
        const char *cell_text = this->GetCellText(j, i);
        if (cell_text && !strcmp(cell_text, text))
          {
          *row_index = j;
          *col_index = i;
          return 1;
          }
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int* vtkKWMultiColumnList::FindCellText(const char *text)
{
  static int pos[2];
  if (this->FindCellText(text, pos, pos + 1))
    {
    return pos;
    }
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::FindCellTextInColumn(
  int col_index, const char *text)
{
  if (this->IsCreated() && text)
    {
    int nb_rows = this->GetNumberOfRows();
    for (int j = 0; j < nb_rows; j++)
      {
      const char *cell_text = this->GetCellText(j, col_index);
      if (cell_text && !strcmp(cell_text, text))
        {
        return j;
        }
      }
    }

  return -1;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::FindCellTextAsIntInColumn(
  int col_index, int value)
{
  if (this->IsCreated() && col_index >= 0)
    {
    int nb_rows = this->GetNumberOfRows();
    for (int j = 0; j < nb_rows; j++)
      {
      if (value == this->GetCellTextAsInt(j, col_index))
        {
        return j;
        }
      }
    }

  return -1;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::EditCell(int row_index, int col_index)
{
  if (this->IsCreated())
    {
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s editcell %d,%d", 
                 this->GetWidgetName(), row_index, col_index);
    this->SetState(old_state);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::CancelEditing()
{
  if (this->IsCreated())
    {
    this->Script("%s cancelediting",  this->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::RejectInput()
{
  if (this->IsCreated())
    {
    this->Script("%s rejectinput",  this->GetWidgetName());
    }
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::SetCellConfigurationOption(
  int row_index, int col_index, const char *option, const char *value)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return 0;
    }

  if (!option || !value)
    {
    vtkWarningMacro("Wrong option or value !");
    return 0;
    }

  const char *res = 
    this->Script("%s cellconfigure %d,%d %s {%s}", 
                 this->GetWidgetName(), row_index, col_index, option, value);

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
      << widget_name.c_str() << ") at cell: " << row_index << "," << col_index
      << " with option: [" << option 
      << "] and value [" << value << "] => " << err_msg.c_str());
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::HasCellConfigurationOption(
  int row_index, int col_index, const char *option)
{
  if (!this->IsCreated())
    {
    vtkWarningMacro("Widget is not created yet !");
    return 0;
    }

  return (this->GetApplication() && 
          !this->GetApplication()->EvaluateBooleanExpression(
            "catch {%s cellcget %d,%d %s}",
            this->GetWidgetName(), row_index, col_index, option));
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetCellConfigurationOption(
  int row_index, int col_index, const char* option)
{
  if (!this->HasCellConfigurationOption(row_index, col_index, option))
    {
    return NULL;
    }

  return this->Script(
    "%s cellcget %d,%d %s", this->GetWidgetName(), row_index,col_index,option);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::SetCellConfigurationOptionAsInt(
  int row_index, int col_index, const char *option, int value)
{
  char buffer[20];
  sprintf(buffer, "%d", value);
  return 
    this->SetCellConfigurationOption(row_index, col_index, option, buffer);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetCellConfigurationOptionAsInt(
  int row_index, int col_index, const char* option)
{
  if (!this->HasCellConfigurationOption(row_index, col_index, option))
    {
    return 0;
    }

  return atoi(
    this->Script("%s cellcget %d,%d %s", 
                 this->GetWidgetName(), row_index, col_index, option));
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellConfigurationOptionAsText(
  int row_index, int col_index, const char *option, const char *value)
{
  if (!option || !this->IsCreated())
    {
    return;
    }

  const char *val = this->ConvertInternalStringToTclString(
    value, vtkKWCoreWidget::ConvertStringEscapeInterpretable);
  this->Script("%s cellconfigure %d,%d %s \"%s\"", 
               this->GetWidgetName(), 
               row_index, col_index, option, val ? val : "");
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::GetCellConfigurationOptionAsText(
  int row_index, int col_index, const char *option)
{
  if (!option || !this->IsCreated())
    {
    return "";
    }

  return this->ConvertTclStringToInternalString(
    this->GetCellConfigurationOption(row_index, col_index, option));
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetBackgroundColor(double r, double g, double b)
{
  this->Superclass::SetBackgroundColor(r, g, b);
  this->InvokePotentialCellColorsChangedCommand();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetForegroundColor(double r, double g, double b)
{
  this->Superclass::SetForegroundColor(r, g, b);
  this->InvokePotentialCellColorsChangedCommand();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetSelectionBackgroundColor(
  double *r, double *g, double *b)
{
  vtkKWTkUtilities::GetOptionColor(this, "-selectbackground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetSelectionBackgroundColor()
{
  static double rgb[3];
  this->GetSelectionBackgroundColor(rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetSelectionBackgroundColor(double r, double g, double b)
{
  vtkKWTkUtilities::SetOptionColor(this, "-selectbackground", r, g, b);
  this->InvokePotentialCellColorsChangedCommand();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetSelectionForegroundColor(double *r, double *g, double *b)
{
  vtkKWTkUtilities::GetOptionColor(this, "-selectforeground", r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetSelectionForegroundColor()
{
  static double rgb[3];
  this->GetSelectionForegroundColor(rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetSelectionForegroundColor(double r, double g, double b)
{
  vtkKWTkUtilities::SetOptionColor(this, "-selectforeground", r, g, b);
  this->InvokePotentialCellColorsChangedCommand();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetColumnSelectionBackgroundColor(
  int col_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetColumnConfigurationOption(col_index, "-selectbackground"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetColumnSelectionBackgroundColor(int col_index)
{
  static double rgb[3];
  this->GetColumnSelectionBackgroundColor(col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnSelectionBackgroundColor(
  int col_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetColumnConfigurationOption(col_index, "-selectbackground", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetColumnSelectionForegroundColor(
  int col_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetColumnConfigurationOption(col_index, "-selectforeground"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetColumnSelectionForegroundColor(int col_index)
{
  static double rgb[3];
  this->GetColumnSelectionForegroundColor(col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnSelectionForegroundColor(
  int col_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetColumnConfigurationOption(col_index, "-selectforeground", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetRowSelectionBackgroundColor(
  int row_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetRowConfigurationOption(row_index, "-selectbackground"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetRowSelectionBackgroundColor(int row_index)
{
  static double rgb[3];
  this->GetRowSelectionBackgroundColor(row_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetRowSelectionBackgroundColor(
  int row_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetRowConfigurationOption(row_index, "-selectbackground", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetRowSelectionForegroundColor(
  int row_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetRowConfigurationOption(row_index, "-selectforeground"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetRowSelectionForegroundColor(int row_index)
{
  static double rgb[3];
  this->GetRowSelectionForegroundColor(row_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetRowSelectionForegroundColor(
  int row_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetRowConfigurationOption(row_index, "-selectforeground", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetCellSelectionBackgroundColor(
  int row_index, int col_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetCellConfigurationOption(
      row_index, col_index, "-selectbackground"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetCellSelectionBackgroundColor(
  int row_index, int col_index)
{
  static double rgb[3];
  this->GetCellSelectionBackgroundColor(
    row_index, col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellSelectionBackgroundColor(
  int row_index, int col_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetCellConfigurationOption(
      row_index, col_index, "-selectbackground", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::GetCellSelectionForegroundColor(
  int row_index, int col_index, double *r, double *g, double *b)
{
  vtksys_stl::string color(
    this->GetCellConfigurationOption(
      row_index, col_index, "-selectforeground"));
  vtkKWTkUtilities::GetRGBColor(this, color.c_str(), r, g, b);
}

//----------------------------------------------------------------------------
double* vtkKWMultiColumnList::GetCellSelectionForegroundColor(
  int row_index, int col_index)
{
  static double rgb[3];
  this->GetCellSelectionForegroundColor(row_index, col_index, rgb, rgb + 1, rgb + 2);
  return rgb;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellSelectionForegroundColor(
  int row_index, int col_index, double r, double g, double b)
{
  if (r >= 0.0 && r <= 1.0 && g >= 0.0 && g <= 1.0 && b >= 0.0 && b <= 1.0)
    {
    char color[10];
    sprintf(color, "#%02x%02x%02x", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
    this->SetCellConfigurationOption(
      row_index, col_index, "-selectforeground", color);
    this->InvokePotentialCellColorsChangedCommand();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetSelectionMode(int relief)
{
  this->SetConfigurationOption(
    "-selectmode", vtkKWTkOptions::GetSelectionModeAsTkOptionValue(relief));
  this->HasSelectionChanged();
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetSelectionMode()
{
  return vtkKWTkOptions::GetSelectionModeFromTkOptionValue(
    this->GetConfigurationOption("-selectmode"));
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetSelectionType(int type)
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *type_opt;
  switch (type)
    {
    case vtkKWMultiColumnList::SelectionTypeRow:
      type_opt = "row";
      break;
    case vtkKWMultiColumnList::SelectionTypeCell:
      type_opt = "cell";
      break;
    default:
      type_opt = "row";
      break;
    }
  this->SetConfigurationOption("-selecttype", type_opt);
  this->HasSelectionChanged();
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetSelectionType()
{
  if (this->IsCreated())
    {
    const char *val = this->GetConfigurationOption("-selecttype");
    if (val && *val)
      {
      if (!strcmp(val, "row"))
        {
        return vtkKWMultiColumnList::SelectionTypeRow;
        }
      if (!strcmp(val, "cell"))
        {
        return vtkKWMultiColumnList::SelectionTypeCell;
        }
      }
    }

  return vtkKWMultiColumnList::SelectionTypeUnknown;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SelectSingleRow(int row_index)
{
  this->ClearSelection();
  this->SelectRow(row_index);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SelectRow(int row_index)
{
  if (this->IsCreated())
    {
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s selection set %d %d", 
                 this->GetWidgetName(), row_index, row_index);
    this->SetState(old_state);
    this->SelectionCallback();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::DeselectRow(int row_index)
{
  if (this->IsCreated())
    {
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s selection clear %d %d", 
                 this->GetWidgetName(), row_index, row_index);
    this->SetState(old_state);
    this->SelectionCallback();
    }
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::IsRowSelected(int row_index)
{
  if (this->IsCreated())
    {
    return atoi(this->Script("%s selection includes %d", 
                             this->GetWidgetName(), row_index));
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetNumberOfSelectedRows()
{
  if (this->IsCreated())
    {
    return atoi(
      this->Script("llength [%s curselection]", this->GetWidgetName()));
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetSelectedRows(int *indices)
{
  if (!this->IsCreated())
    {
    return 0;
    }

  vtksys_stl::string curselection(
    this->Script("%s curselection", this->GetWidgetName()));

  vtksys_stl::vector<vtksys_stl::string> split_elems;
  vtksys::SystemTools::Split(curselection.c_str(), split_elems, ' ');
  
  vtksys_stl::vector<vtksys_stl::string>::iterator it = split_elems.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator end = split_elems.end();
  int index = 0;
  for (; it != end; index++, it++)
    {
    indices[index] = atoi((*it).c_str());
    }

  return index;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetIndexOfFirstSelectedRow()
{
  if (this->IsCreated())
    {
    const char *sel = this->Script("lindex [%s curselection] 0", 
                                   this->GetWidgetName());
    if (sel && *sel)
      {
      return atoi(sel);
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SelectSingleCell(int row_index, int col_index)
{
  this->ClearSelection();
  this->SelectCell(row_index, col_index);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SelectCell(int row_index, int col_index)
{
  if (this->IsCreated())
    {
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s cellselection set %d,%d %d,%d", 
                 this->GetWidgetName(), 
                 row_index, col_index, row_index, col_index);
    this->SetState(old_state);
    this->SelectionCallback();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::DeselectCell(int row_index, int col_index)
{
  if (this->IsCreated())
    {
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s cellselection clear %d,%d %d,%d", 
                 this->GetWidgetName(), 
                 row_index, col_index, row_index, col_index);
    this->SetState(old_state);
    this->SelectionCallback();
    }
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::IsCellSelected(int row_index, int col_index)
{
  if (this->IsCreated())
    {
    return atoi(this->Script("%s cellselection includes %d,%d", 
                             this->GetWidgetName(), row_index, col_index));
    }
    return 0;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetNumberOfSelectedCells()
{
  if (this->IsCreated())
    {
    return atoi(
      this->Script("llength [%s curcellselection]", this->GetWidgetName()));
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetSelectedCells(int *row_indices, int *col_indices)
{
  if (!this->IsCreated())
    {
    return 0;
    }

  vtksys_stl::string curselection(
    this->Script("%s curcellselection", this->GetWidgetName()));

  vtksys_stl::vector<vtksys_stl::string> split_elems;
  vtksys::SystemTools::Split(curselection.c_str(), split_elems, ' ');
  
  vtksys_stl::vector<vtksys_stl::string>::iterator it = split_elems.begin();
  vtksys_stl::vector<vtksys_stl::string>::iterator end = split_elems.end();
  int index = 0, row, col;
  for (; it != end; it++)
    {
    if (sscanf((*it).c_str(), "%d,%d", &row, &col) == 2)
      {
      row_indices[index] = row;
      col_indices[index] = col;
      index++;
      }
    }

  return index;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::ClearSelection()
{
  if (this->IsCreated())
    {
    int old_state = this->GetState();
    if (this->GetState() != vtkKWTkOptions::StateNormal)
      {
      this->SetStateToNormal();
      }
    this->Script("%s selection clear 0 end", this->GetWidgetName());
    this->SetState(old_state);
    this->SelectionCallback();
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetExportSelection(int arg)
{
  this->SetConfigurationOptionAsInt("-exportselection", arg);
}

//----------------------------------------------------------------------------
int vtkKWMultiColumnList::GetExportSelection()
{
  return this->GetConfigurationOptionAsInt("-exportselection");
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetEditStartCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->EditStartCommand, object, method);
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::InvokeEditStartCommand(
  int row, int col, const char *text)
{
  if (this->EditStartCommand && *this->EditStartCommand && this->IsCreated())
    {
    return this->Script("%s %d %d {%s}", 
                        this->EditStartCommand, row, col, text);
    }
  return text;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetEditEndCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->EditEndCommand, object, method);
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::InvokeEditEndCommand(
  int row, int col, const char *text)
{
  if (this->EditEndCommand && *this->EditEndCommand && this->IsCreated())
    {
    return this->Script("%s %d %d {%s}", 
                        this->EditEndCommand, row, col, text);
    }
  return text;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetCellUpdatedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->CellUpdatedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InvokeCellUpdatedCommand(
  int row, int col, const char *text)
{
  if (this->CellUpdatedCommand && *this->CellUpdatedCommand && 
      this->IsCreated())
    {
    this->Script("%s %d %d {%s}", 
                 this->CellUpdatedCommand, row, col, text);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetSelectionCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->SelectionCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InvokeSelectionCommand()
{
  if (this->SelectionCommand && 
      *this->SelectionCommand && 
      this->IsCreated())
    {
    this->Script("eval %s", this->SelectionCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetSelectionChangedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->SelectionChangedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InvokeSelectionChangedCommand()
{
  if (this->SelectionChangedCommand && 
      *this->SelectionChangedCommand && 
      this->IsCreated())
    {
    this->Script("eval %s", this->SelectionChangedCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetPotentialCellColorsChangedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->PotentialCellColorsChangedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InvokePotentialCellColorsChangedCommand()
{
  if (this->PotentialCellColorsChangedCommand && 
      *this->PotentialCellColorsChangedCommand && 
      this->IsCreated())
    {
    this->Script("eval %s", this->PotentialCellColorsChangedCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetColumnSortedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->ColumnSortedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::InvokeColumnSortedCommand()
{
  if (this->ColumnSortedCommand && *this->ColumnSortedCommand && 
      this->IsCreated())
    {
    this->Script("eval %s", this->ColumnSortedCommand);
    }
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::EditStartCallback(
  const char *, int row, int col, const char *text)
{
  // Save the position of the cell that is being edited, so that
  // CellUpdatedCallback can find and propagate which one it was

  if (this->Internals)
    {
    this->Internals->EditedCellRowIndex = row;
    this->Internals->EditedCellColumnIndex = col;
    }

  // Check if the cell is a user-defined widget that we have control
  // of (as set by SetCellWindowCommandToColorButton for example):

  const char *command = 
    this->GetCellConfigurationOption(row, col, "-window");
  if (command && *command)
    {
    char *color_button_command = NULL;
    this->SetObjectMethodCommand(
      &color_button_command, this, "CellWindowCommandToColorButtonCallback");
    int res = !strcmp(command, color_button_command);
    delete [] color_button_command;
    if (res)
      {
      this->CancelEditing();
      vtksys_stl::string cell_contents(this->GetCellText(row, col));
      vtksys_stl::string start_contents(
        this->InvokeEditStartCommand(row, col, cell_contents.c_str()));
      double r, g, b, out_r, out_g, out_b;
      if (sscanf(start_contents.c_str(), "%lg %lg %lg", &r, &g, &b) == 3 &&
          vtkKWTkUtilities::QueryUserForColor(
            this->GetApplication(), this->GetWidgetName(), "Pick Color", 
            r, g, b, &out_r, &out_g, &out_b))
        {
        char buffer[256];
        sprintf(buffer, "%g %g %g", out_r, out_g, out_b);
        vtksys_stl::string validated_contents(
          this->InvokeEditEndCommand(row, col, buffer));
        if (strcmp(validated_contents.c_str(), cell_contents.c_str()) &&
            sscanf(validated_contents.c_str(), "%lg %lg %lg", &r, &g, &b) == 3)
          {
          this->SetCellText(row, col, validated_contents.c_str());
          const char *child_name = this->GetCellWindowWidgetName(row, col);
          if (child_name && *child_name)
            {
            vtkKWFrame *child = vtkKWFrame::SafeDownCast(
              this->GetChildWidgetWithName(child_name));
            if (child)
              {
              child->SetBackgroundColor(r, g, b);
              }
            }
          this->InvokeCellUpdatedCommand(
            row, col, validated_contents.c_str());
          }
        }
      return NULL;
      }
    }

  return this->InvokeEditStartCommand(row, col, text);
}

//----------------------------------------------------------------------------
const char* vtkKWMultiColumnList::EditEndCallback(
  const char *, int row, int col, const char *text)
{
  return this->InvokeEditEndCommand(row, col, text);
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SelectionCallback()
{
  this->InvokeSelectionCommand();
  this->HasSelectionChanged();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::CellUpdatedCallback()
{
  int row = 0, col = 0;
  if (this->Internals)
    {
    row = this->Internals->EditedCellRowIndex;
    col = this->Internals->EditedCellColumnIndex;
    }

  this->InvokeCellUpdatedCommand(row, col, this->GetCellText(row, col));
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::ColumnSortedCallback()
{
  this->InvokePotentialCellColorsChangedCommand();
  this->InvokeColumnSortedCommand();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::HasSelectionChanged()
{
  // Retrieve the selected cells

  int max_nb_cells = this->GetNumberOfRows() * this->GetNumberOfColumns();
  int *row_indices = new int [max_nb_cells];
  int *col_indices = new int [max_nb_cells];
  int nb_of_selected_cells = this->GetSelectedCells(row_indices, col_indices);

  // Compare it to the last time we saved the selection

  int selection_has_changed = 0;
  int prev_nb_of_selected_cells = 
    this->Internals->LastSelectionRowIndices.size();
  if (nb_of_selected_cells != prev_nb_of_selected_cells)
    {
    selection_has_changed = 1;
    }
  else
    {
    for (int i = 0; i < nb_of_selected_cells; i++)
      {
      if (row_indices[i] != this->Internals->LastSelectionRowIndices[i] ||
          col_indices[i] != this->Internals->LastSelectionColIndices[i])
        {
        selection_has_changed = 1;
        break;
        }
      }
    }

  // Changes, let's save the selection

  if (selection_has_changed)
    {
    if (nb_of_selected_cells != prev_nb_of_selected_cells)
      {
      this->Internals->LastSelectionRowIndices.resize(nb_of_selected_cells);
      this->Internals->LastSelectionColIndices.resize(nb_of_selected_cells);
      }
    vtksys_stl::copy(row_indices, 
                     row_indices + nb_of_selected_cells, 
                     this->Internals->LastSelectionRowIndices.begin());
    vtksys_stl::copy(col_indices, 
                     col_indices + nb_of_selected_cells, 
                     this->Internals->LastSelectionColIndices.begin());
  
    this->InvokeSelectionChangedCommand();
    this->InvokePotentialCellColorsChangedCommand();
    }

  delete [] row_indices;
  delete [] col_indices;
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetLabelCommand(vtkObject* object, 
                                           const char *method)
{
  if (this->IsCreated())
    {
    char *command = NULL;
    this->SetObjectMethodCommand(&command, object, method);
    this->SetConfigurationOption("-labelcommand", command);
    delete [] command;
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::SetSortCommand(vtkObject* object, 
                                          const char *method)
{
  if (this->IsCreated())
    {
    char *command = NULL;
    this->SetObjectMethodCommand(&command, object, method);
    this->SetConfigurationOption("-sortcommand", command);
    delete [] command;
    }
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->SetState(this->GetEnabled());

  this->RefreshAllCellsWithWindowCommand();
}

//----------------------------------------------------------------------------
void vtkKWMultiColumnList::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
