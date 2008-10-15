/*=========================================================================

  Module:    vtkKWStartupPageWidget.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWStartupPageWidget.h"

#include "vtkKWApplication.h"
#include "vtkKWCanvas.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWInternationalization.h"
#include "vtkKWMostRecentFilesManager.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWIcon.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/ios/sstream>
#include <vtksys/stl/string>

vtkCxxRevisionMacro(vtkKWStartupPageWidget, "1.21");
vtkStandardNewMacro(vtkKWStartupPageWidget);

#define VTK_KW_SPW_OPEN_TAG        "open_section"
#define VTK_KW_SPW_DOUBLE_TAG      "double_section"
#define VTK_KW_SPW_DROP_TAG        "drop_section"
#define VTK_KW_SPW_MRF_TAG         "mrf_section"
#define VTK_KW_SPW_MRF_PATTERN_TAG "mrf_item_%d"
#define VTK_KW_SPW_GRADIENT_TAG    "gradient"

//----------------------------------------------------------------------------
class vtkKWStartupPageWidgetInternals
{
public:
  vtksys_stl::string ScheduleRedrawTimerId;

  int LastRedrawSize[2];

  vtksys_stl::string BaseFont;

  char TextFont[1024];
  char HexTextColor[20];
  char HexSelectedTextColor[20];
  char HexTextShadowColor[20];

  char MostRecentFilesFont[1024];

  char HintFont[1024];
  char HexHintColor[20];
  char HexHintShadowColor[20];

  vtkKWStartupPageWidgetInternals() 
    {
      this->LastRedrawSize[0] = 0;
      this->LastRedrawSize[1] = 0;
    }
};

//----------------------------------------------------------------------------
vtkKWStartupPageWidget::vtkKWStartupPageWidget()
{
  this->Internals = new vtkKWStartupPageWidgetInternals;

  this->GradientColor1[0]    = 21.0 / 255.0;
  this->GradientColor1[1]    = 27.0 / 255.0;
  this->GradientColor1[2]    = 26.0 / 255.0;

  this->GradientColor2[0]    = 70.0 / 255.0;
  this->GradientColor2[1]    = 81.0 / 255.0;
  this->GradientColor2[2]    = 80.0 / 255.0;

  this->TextColor[0]         = 255.0 / 255.0;
  this->TextColor[1]         = 255.0 / 255.0;
  this->TextColor[2]         = 255.0 / 255.0;

  this->SelectedTextColor[0] = 255.0 / 255.0;
  this->SelectedTextColor[1] = 249.0 / 255.0;
  this->SelectedTextColor[2] =  85.0 / 255.0;

  this->HintColor[0]         = 140.0 / 255.0;
  this->HintColor[1]         = 140.0 / 255.0;
  this->HintColor[2]         = 140.0 / 255.0;

  this->TextSize           = 14;
  this->MostRecentFileSize = 10;
#ifdef _WIN32
  this->HintSize           = 9;
#else
  this->HintSize           = 10;
#endif

  this->SupportDoubleClick     = 1;
  this->SupportDrop            = 1;
  this->SupportMostRecentFiles = 1;
  this->SupportOpen            = 1;

  this->AddShadowToHint = 1;
  this->MaximumNumberOfMostRecentFiles = 5;

  this->OpenIcon            = NULL;
  this->DoubleClickIcon     = NULL;
  this->DropIcon            = NULL;
  this->MostRecentFilesIcon = NULL;
  this->MostRecentFileIcon  = NULL;

  this->OpenCommand            = NULL;
  this->DropCommand            = NULL;
  this->DoubleClickCommand     = NULL;

  this->StartupPageCanvas      = vtkKWCanvas::New();

  this->MostRecentFilesManager = NULL;
}

//----------------------------------------------------------------------------
vtkKWStartupPageWidget::~vtkKWStartupPageWidget()
{
  delete this->Internals;
  this->Internals = NULL;

  // Commands

  if (this->OpenCommand)
    {
    delete [] this->OpenCommand;
    this->OpenCommand = NULL;
    }

  if (this->DropCommand)
    {
    delete [] this->DropCommand;
    this->DropCommand = NULL;
    }

  if (this->DoubleClickCommand)
    {
    delete [] this->DoubleClickCommand;
    this->DoubleClickCommand = NULL;
    }

  // GUI

  if (this->StartupPageCanvas)
    {
    this->StartupPageCanvas->Delete();
    this->StartupPageCanvas = NULL;
    }

  if (this->OpenIcon)
    {
    this->OpenIcon->Delete();
    this->OpenIcon = NULL;
    }

  if (this->DoubleClickIcon)
    {
    this->DoubleClickIcon->Delete();
    this->DoubleClickIcon = NULL;
    }

  if (this->DropIcon)
    {
    this->DropIcon->Delete();
    this->DropIcon = NULL;
    }

  if (this->MostRecentFilesIcon)
    {
    this->MostRecentFilesIcon->Delete();
    this->MostRecentFilesIcon = NULL;
    }

  if (this->MostRecentFileIcon)
    {
    this->MostRecentFileIcon->Delete();
    this->MostRecentFileIcon = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // Create the Hue/Sat canvas

  this->StartupPageCanvas->SetParent(this);
  this->StartupPageCanvas->Create();
  this->StartupPageCanvas->SetHeight(0);
  this->StartupPageCanvas->SetWidth(0);
  this->StartupPageCanvas->SetBorderWidth(0);
  this->StartupPageCanvas->SetBackgroundColor(this->GradientColor2);

  this->Script("pack %s -fill both -expand 1", 
               this->StartupPageCanvas->GetWidgetName());

  // Update

  this->AddCallbackCommandObservers();

  this->SetDropFileBinding(NULL, this->DropCommand);

  this->UpdateInternalCanvasBindings();
  this->UpdateInternalCanvasColors();
  this->UpdateInternalCanvasFonts();
  this->UpdateInternalCanvasIcons();

  this->Update();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::Update()
{
  this->UpdateEnableState();

  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::AddCallbackCommandObservers()
{
  this->Superclass::AddCallbackCommandObservers();

  if (this->MostRecentFilesManager)
    {
    this->AddCallbackCommandObserver(
      this->MostRecentFilesManager, 
      vtkKWMostRecentFilesManager::MenuHasChangedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::RemoveCallbackCommandObservers()
{
  this->Superclass::RemoveCallbackCommandObservers();

  if (this->MostRecentFilesManager)
    {
    this->RemoveCallbackCommandObserver(
      this->MostRecentFilesManager, 
      vtkKWMostRecentFilesManager::MenuHasChangedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::UpdateInternalCanvasBindings()
{
  this->StartupPageCanvas->SetBinding(
    "<Double-1>", this, "DoubleClickCallback");

  this->StartupPageCanvas->SetBinding(
    "<Configure>", this, "ConfigureCallback");
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::UpdateInternalCanvasFonts()
{
  Tcl_Interp *interp = this->GetApplication()->GetMainInterp();

  // This is very clumsy but I don't know how to get the default font
  // that is used for labels...

  if (!this->Internals->BaseFont.size() && this->IsCreated())
    {
    vtkKWLabel *temp = vtkKWLabel::New();
    temp->SetParent(this);
    temp->Create();
    this->Internals->BaseFont = temp->GetFont();
    temp->Delete();
    }

  char base_font_bold[1024];
  vtkKWTkUtilities::ChangeFontWeightToBold(
    interp, this->Internals->BaseFont.c_str(), base_font_bold);
  
  vtkKWTkUtilities::ChangeFontSize(
    interp, base_font_bold, this->TextSize, 
    this->Internals->TextFont);
  
  vtkKWTkUtilities::ChangeFontSize(
    interp, this->Internals->BaseFont.c_str(), this->MostRecentFileSize, 
    this->Internals->MostRecentFilesFont);

  vtkKWTkUtilities::ChangeFontSize(
    interp, this->Internals->BaseFont.c_str(), this->HintSize, 
    this->Internals->HintFont);
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::UpdateInternalCanvasColors()
{
  double average, shadow;

  // Text

  sprintf(this->Internals->HexTextColor, "#%02x%02x%02x", 
          (int)(this->TextColor[0] * 255.0),
          (int)(this->TextColor[1] * 255.0),
          (int)(this->TextColor[2] * 255.0));

  sprintf(this->Internals->HexSelectedTextColor, "#%02x%02x%02x", 
          (int)(this->SelectedTextColor[0] * 255.0),
          (int)(this->SelectedTextColor[1] * 255.0),
          (int)(this->SelectedTextColor[2] * 255.0));

  average = 
    (this->TextColor[0] + this->TextColor[1] + this->TextColor[2]) / 3.0;
  shadow = average > 0.5 ? 0.0 : 1.0;

  sprintf(this->Internals->HexTextShadowColor, "#%02x%02x%02x", 
          (int)(shadow * 255.0),
          (int)(shadow * 255.0),
          (int)(shadow * 255.0));

  // Hint

  sprintf(this->Internals->HexHintColor, "#%02x%02x%02x", 
          (int)(this->HintColor[0] * 255.0),
          (int)(this->HintColor[1] * 255.0),
          (int)(this->HintColor[2] * 255.0));

  average = 
    (this->HintColor[0] + this->HintColor[1] + this->HintColor[2]) / 3.0;
  shadow = average > 0.3 ? 0.0 : 1.0;

  sprintf(this->Internals->HexHintShadowColor, "#%02x%02x%02x", 
          (int)(shadow * 255.0),
          (int)(shadow * 255.0),
          (int)(shadow * 255.0));
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::UpdateInternalCanvasIcons()
{
  if (!this->OpenIcon)
    {
    this->OpenIcon = vtkKWIcon::New();
    this->OpenIcon->SetImage(
      vtkKWIcon::IconNuvola48x48FilesystemsFolderBlue);
    }

  if (!this->DoubleClickIcon)
    {
    this->DoubleClickIcon = vtkKWIcon::New();
    this->DoubleClickIcon->SetImage(
      vtkKWIcon::IconNuvola48x48DevicesMouse);
    }

  if (!this->DropIcon)
    {
    this->DropIcon = vtkKWIcon::New();
    this->DropIcon->SetImage(
      vtkKWIcon::IconNuvola48x48AppsDownloadManager);
    }

  if (!this->MostRecentFilesIcon)
    {
    this->MostRecentFilesIcon = vtkKWIcon::New();
    this->MostRecentFilesIcon->SetImage(
      vtkKWIcon::IconNuvola48x48ActionsHistory);
    }

  if (!this->MostRecentFileIcon)
    {
    this->MostRecentFileIcon = vtkKWIcon::New();
    this->MostRecentFileIcon->SetImage(
      vtkKWIcon::IconNuvola22x22FilesystemsFolderBlue);
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetSupportOpen(int arg)
{
  if (this->SupportOpen == arg)
    {
    return;
    }

  this->SupportOpen = arg;

  this->Modified();

  this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_OPEN_TAG);
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetOpenIcon(vtkKWIcon *arg)
{
  if (arg)
    {
    if (!this->OpenIcon)
      {
      this->OpenIcon = vtkKWIcon::New();
      }
    this->OpenIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_OPEN_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetOpenIconToPredefinedIcon(int arg)
{
  if (arg)
    {
    if (!this->OpenIcon)
      {
      this->OpenIcon = vtkKWIcon::New();
      }
    this->OpenIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_OPEN_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetSupportDoubleClick(int arg)
{
  if (this->SupportDoubleClick == arg)
    {
    return;
    }

  this->SupportDoubleClick = arg;

  this->Modified();

  this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_DOUBLE_TAG);
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetDoubleClickIcon(vtkKWIcon *arg)
{
  if (arg)
    {
    if (!this->DoubleClickIcon)
      {
      this->DoubleClickIcon = vtkKWIcon::New();
      }
    this->DoubleClickIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_DOUBLE_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetDoubleClickIconToPredefinedIcon(int arg)
{
  if (arg)
    {
    if (!this->DoubleClickIcon)
      {
      this->DoubleClickIcon = vtkKWIcon::New();
      }
    this->DoubleClickIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_DOUBLE_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetSupportDrop(int arg)
{
  if (this->SupportDrop == arg)
    {
    return;
    }

  this->SupportDrop = arg;

  this->Modified();

  this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_DROP_TAG);
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetDropIcon(vtkKWIcon *arg)
{
  if (arg)
    {
    if (!this->DropIcon)
      {
      this->DropIcon = vtkKWIcon::New();
      }
    this->DropIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_DROP_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetDropIconToPredefinedIcon(int arg)
{
  if (arg)
    {
    if (!this->DropIcon)
      {
      this->DropIcon = vtkKWIcon::New();
      }
    this->DropIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_DROP_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetSupportMostRecentFiles(int arg)
{
  if (this->SupportMostRecentFiles == arg)
    {
    return;
    }

  this->SupportMostRecentFiles = arg;

  this->Modified();

  this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_MRF_TAG);
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetMostRecentFilesIcon(vtkKWIcon *arg)
{
  if (arg)
    {
    if (!this->MostRecentFilesIcon)
      {
      this->MostRecentFilesIcon = vtkKWIcon::New();
      }
    this->MostRecentFilesIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_MRF_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetMostRecentFilesIconToPredefinedIcon(int arg)
{
  if (arg)
    {
    if (!this->MostRecentFilesIcon)
      {
      this->MostRecentFilesIcon = vtkKWIcon::New();
      }
    this->MostRecentFilesIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_MRF_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetMostRecentFileIcon(vtkKWIcon *arg)
{
  if (arg)
    {
    if (!this->MostRecentFileIcon)
      {
      this->MostRecentFileIcon = vtkKWIcon::New();
      }
    this->MostRecentFileIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_MRF_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetMostRecentFileIconToPredefinedIcon(int arg)
{
  if (arg)
    {
    if (!this->MostRecentFileIcon)
      {
      this->MostRecentFileIcon = vtkKWIcon::New();
      }
    this->MostRecentFileIcon->SetImage(arg);

    this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_MRF_TAG);
    this->Redraw();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetMostRecentFilesManager(
  vtkKWMostRecentFilesManager *arg)
{
  if (this->MostRecentFilesManager == arg)
    {
    return;
    }

  this->MostRecentFilesManager = arg;
  this->Modified();

  this->AddCallbackCommandObservers();

  this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_MRF_TAG);
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetMaximumNumberOfMostRecentFiles(int _arg)
{
  if (this->MaximumNumberOfMostRecentFiles == _arg || _arg < 0)
    {
    return;
    }

  this->MaximumNumberOfMostRecentFiles = _arg;
  this->Modified();

  this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_MRF_TAG);
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::Redraw()
{
  if (!this->StartupPageCanvas || !this->StartupPageCanvas->IsAlive())
    {
    return;
    }

  int canv_width, canv_height;
  if (!vtkKWTkUtilities::GetWidgetSize(
        this->StartupPageCanvas, &canv_width, &canv_height))
    {
    return;
    }

  // Too small

  if (canv_width <= 5 || canv_height <= 5)
    {
    return;
    }

  // Same size?

#if 0
  if (this->Internals->LastRedrawSize[0] == canv_width &&
      this->Internals->LastRedrawSize[1] == canv_height)
    {
    return;
    }

  this->Internals->LastRedrawSize[0] = canv_width;
  this->Internals->LastRedrawSize[1] = canv_height;
#endif

  int center_x = canv_width / 2;
  int x = (int)(0.10 * canv_width);
  int y = (int)(0.10 * canv_height);
  int interspace = (int)(0.12 * canv_height);

  vtksys_ios::ostringstream tk_cmd;

  const char *canv = this->StartupPageCanvas->GetWidgetName();

  // Gradient

  if (!this->StartupPageCanvas->HasTag(VTK_KW_SPW_GRADIENT_TAG))
    {
    this->StartupPageCanvas->AddHorizontalRGBGradient(
      this->GradientColor1[0],this->GradientColor1[1],this->GradientColor1[2], 
      this->GradientColor2[0],this->GradientColor2[1],this->GradientColor2[2], 
      0, 0, canv_width - 1, canv_height - 1, 
      VTK_KW_SPW_GRADIENT_TAG);
    tk_cmd << canv << " lower " << VTK_KW_SPW_GRADIENT_TAG << " all" << endl;
    }

  // Open file

  if (this->SupportOpen)
    {
    this->AddSectionToCanvas(
      tk_cmd, 
      x, y, 
      this->OpenIcon,
      ks_("Startup Web Page|Open File"), 
      this->Internals->TextFont,
      ks_("...by clicking on this button or by using the \"File -> Open File\" menu."), 
      this->Internals->HintFont,
      this, "OpenCallback",
      VTK_KW_SPW_OPEN_TAG);
    y += interspace;
    }

  // Double Click

  if (this->SupportDoubleClick)
    {
    this->AddSectionToCanvas(
      tk_cmd, 
      x, y, 
      this->DoubleClickIcon,
      ks_("Startup Web Page|Double Click"), 
      this->Internals->TextFont,
      ks_("...anywhere in this area to open a file."), 
      this->Internals->HintFont,
      NULL, NULL,
      VTK_KW_SPW_DOUBLE_TAG);
    y += interspace;
    }

  // Drag & Drop

  if (this->SupportDrop)
    {
    this->AddSectionToCanvas(
      tk_cmd, 
      x, y, 
      this->DropIcon,
      ks_("Startup Web Page|Drag & Drop"), 
      this->Internals->TextFont,
      ks_("...any file in this area to open it."), 
      this->Internals->HintFont,
      NULL, NULL,
      VTK_KW_SPW_DROP_TAG);
    y += interspace;
    }

  // Most Recent Files

  if (this->SupportMostRecentFiles)
    {
    this->AddMostRecentFilesSectionToCanvas(
      tk_cmd,
      x, y);
    }
  
  this->Script(tk_cmd.str().c_str());
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::AddMostRecentFilesSectionToCanvas(
  ostream &tk_cmd, 
  int x, int y)
{
  if (this->MostRecentFilesManager)
    {
    int nb_files = this->MostRecentFilesManager->GetNumberOfFiles();
    if (nb_files > this->MaximumNumberOfMostRecentFiles)
      {
      nb_files = this->MaximumNumberOfMostRecentFiles;
      }

    if (nb_files)
      {
      this->AddSectionToCanvas(
        tk_cmd, 
        x, y, 
        this->MostRecentFilesIcon,
        ks_("Startup Web Page|Open Recent File"), 
        this->Internals->TextFont,
        ks_("...by selecting a file below  or by using the \"File -> Open Recent File\" menu."), 
        this->Internals->HintFont,
        NULL, NULL,
        VTK_KW_SPW_MRF_TAG);
      x += this->GetHorizontalIncrementFromIcon(this->MostRecentFilesIcon) + 20;
      y += 50;
      }
    
    char tag[100];
    int i;
    for (i = 0; i < nb_files; i++)
      {
      const char *filename = this->MostRecentFilesManager->GetNthFileName(i);
      const char *label = this->MostRecentFilesManager->GetNthLabel(i);
      vtkObject *target_object = 
        this->MostRecentFilesManager->GetNthTargetObject(i);
      if (!target_object)
        {
        target_object = 
          this->MostRecentFilesManager->GetDefaultTargetObject();
        }
      const char *target_command = 
        this->MostRecentFilesManager->GetNthTargetCommand(i);
      if (!target_command || !*target_command)
        {
        target_command = 
          this->MostRecentFilesManager->GetDefaultTargetCommand();
        }

      vtksys_stl::string filename_name;
      if (!label || !*label)
        {
        filename_name = vtksys::SystemTools::GetFilenameName(filename);
        label = filename_name.c_str();
        }
      
      vtksys_stl::string filename_path(
        vtksys::SystemTools::CropString(
          vtksys::SystemTools::GetFilenamePath(filename), 60));
      
      vtksys_stl::string cmd(target_command);
      cmd += " {";
      cmd += filename;
      cmd += '}';

      sprintf(tag, VTK_KW_SPW_MRF_PATTERN_TAG, i);
      this->AddSectionToCanvas(
        tk_cmd, 
        x, y, 
        this->MostRecentFileIcon,
        label, 
        this->Internals->MostRecentFilesFont,
        filename_path.c_str(), 
        this->Internals->HintFont,
        target_object, cmd.c_str(),
        tag, VTK_KW_SPW_MRF_TAG);
      y += 40;
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::AddSectionToCanvas(
  ostream &tk_cmd, 
  int x, int y, 
  vtkKWIcon *icon,
  const char *text, const char *text_font, 
  const char *hint, const char *hint_font,
  vtkObject *object, const char *method,
  const char *tag, const char *extra_tag)
{
  const char *canv = this->StartupPageCanvas->GetWidgetName();

  vtksys_stl::string tags(tag);
  if (extra_tag)
    {
    tags += ' ';
    tags += extra_tag;
    }

  // Text

  int has_tag = this->StartupPageCanvas->HasTag(tag);

  if (!has_tag || !this->StartupPageCanvas->HasTag("text"))
    {
    tk_cmd 
      << canv << " create text 0 0 -anchor sw"
      << " -fill " << this->Internals->HexTextShadowColor
      << " -text {" << text << "}"
      << " -font {" << text_font << "}"
      << " -tags {" << tag << "textshadow text " << tags.c_str()<<"}" 
      << endl;
    tk_cmd 
      << canv << " create text 0 0 -anchor sw"
      << " -fill " << this->Internals->HexTextColor
      << " -text {" << text << "}"
      << " -font {" << text_font << "}"
      << " -tags {" << tag << "text text " << tags.c_str() << "}" 
      << endl;
    if (this->AddShadowToHint)
      {
      tk_cmd 
        << canv << " create text 0 0 -anchor nw"
        << " -fill " << this->Internals->HexHintShadowColor
        << " -text {" << hint << "}"
        << " -font {" << hint_font << "}"
        << " -tags {" << tag << "hintshadow text " << tags.c_str()<<"}" 
        << endl;
      }
    tk_cmd 
      << canv << " create text 0 0 -anchor nw"
      << " -fill " << this->Internals->HexHintColor
      << " -text {" << hint << "}"
      << " -font {" << hint_font << "}"
      << " -tags {" << tag << "hint text " << tags.c_str() << "}" 
      << endl;
    }

  // Icon

  if (icon && (!has_tag || !this->StartupPageCanvas->HasTag("icon")))
    {
    vtksys_stl::string img_name(canv);
    img_name += tag;
    img_name += "icon";
    if (vtkKWTkUtilities::UpdatePhotoFromIcon(
          this->GetApplication(), img_name.c_str(), icon))
      {
      tk_cmd 
        << canv << " create image 0 0 -anchor center"
        << " -image " << img_name.c_str()
        << " -tags {" << tag << "icon icon " << tags.c_str() << "}" 
        << endl;
      }

    // Create highlight icon, if needed

    if (method)
      {
      double hsv[3];
      vtkMath::RGBToHSV(this->SelectedTextColor, hsv);
      vtkKWIcon *hue_icon = vtkKWIcon::New();
      hue_icon->SetImage(icon);
      hue_icon->SetHue(hsv[0]);
      img_name += "highlight";
      vtkKWTkUtilities::UpdatePhotoFromIcon(
        this->GetApplication(), img_name.c_str(), hue_icon);
      hue_icon->Delete();
      }
    }

  // Bindings

  if (method && !has_tag)
    {
    vtksys_stl::string command("HighlightSectionCallback ");
    command += tag;
    
    vtksys_stl::string highlight_command(command);
    highlight_command += " 1";
    this->StartupPageCanvas->SetCanvasBinding(
        tag, "<Enter>", this, highlight_command.c_str());
    
    vtksys_stl::string no_highlight_command(command);
    no_highlight_command += " 0";
    this->StartupPageCanvas->SetCanvasBinding(
      tag, "<Leave>", this, no_highlight_command.c_str());
    
    this->StartupPageCanvas->SetCanvasBinding(
      tag, "<ButtonPress-1>", object, method);
    }

  // Move items

  if (icon)
    {
    tk_cmd << canv << " coords " << tag << "icon " 
           << x << " " << y - 4 << endl;
    x += this->GetHorizontalIncrementFromIcon(icon);
    }

  tk_cmd << canv << " coords " << tag << "text " 
         << x << " " << y << endl
         << canv << " coords " << tag << "textshadow " 
         << x + 2 << " " << y + 2 << endl
         << canv << " coords " << tag << "hint " 
         << x << " " << y << endl;
  if (this->AddShadowToHint)
    {
    tk_cmd << canv << " coords " << tag << "hintshadow " 
           << x + 2 << " " << y + 2 << endl;
    }
}

//----------------------------------------------------------------------------
int vtkKWStartupPageWidget::GetHorizontalIncrementFromIcon(vtkKWIcon *icon)
{
  return icon ? ((int)(1.0 * icon->GetWidth()) + 4) : 0;
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::HighlightSectionCallback(
  const char *tag, int flag)
{
  const char *canv = this->StartupPageCanvas->GetWidgetName();

  vtksys_ios::ostringstream tk_cmd;

  tk_cmd << canv << " itemconfigure " << tag << "text -fill "
         << (flag ? this->Internals->HexSelectedTextColor : this->Internals->HexTextColor) << endl;
  
  tk_cmd << canv << " itemconfigure " << tag << "icon -image "
         << canv << tag << (flag ? "iconhighlight" : "icon") << endl;

  this->Script(tk_cmd.str().c_str());
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::OpenCallback()
{
  this->InvokeOpenCommand();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::DoubleClickCallback()
{
  this->InvokeDoubleClickCommand();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::RedrawCallback()
{
  if (!this->GetApplication() || 
      this->GetApplication()->GetInExit() ||
      !this->IsAlive())
    {
    return;
    }

  this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_GRADIENT_TAG);
  this->Redraw();
  this->Internals->ScheduleRedrawTimerId = "";
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::ScheduleRedraw()
{
  // Already scheduled

  if (this->Internals->ScheduleRedrawTimerId.size())
    {
    return;
    }

  this->Internals->ScheduleRedrawTimerId =
    this->Script(
      "after idle {catch {%s RedrawCallback}}", this->GetTclName());
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::Bind()
{
  if (!this->IsCreated())
    {
    return;
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::UnBind()
{
  if (!this->IsCreated())
    {
    return;
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::ConfigureCallback()
{
  this->ScheduleRedraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetGradientColor1(
  double r, double g, double b)
{
  if ((r < 0.0 || r > 1.0 ||
       g < 0.0 || g > 1.0 ||
       b < 0.0 || b > 1.0) ||
      (r == this->GradientColor1[0] &&
       g == this->GradientColor1[1] &&
       b == this->GradientColor1[2]))
    {
    return;
    }

  this->GradientColor1[0] = r;
  this->GradientColor1[1] = g;
  this->GradientColor1[2] = b;

  this->Modified();

  this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_GRADIENT_TAG);
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetGradientColor2(
  double r, double g, double b)
{
  if ((r < 0.0 || r > 1.0 ||
       g < 0.0 || g > 1.0 ||
       b < 0.0 || b > 1.0) ||
      (r == this->GradientColor2[0] &&
       g == this->GradientColor2[1] &&
       b == this->GradientColor2[2]))
    {
    return;
    }

  this->GradientColor2[0] = r;
  this->GradientColor2[1] = g;
  this->GradientColor2[2] = b;

  this->Modified();

  this->StartupPageCanvas->SetBackgroundColor(this->GradientColor2);
  this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_GRADIENT_TAG);
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetTextColor(
  double r, double g, double b)
{
  if ((r < 0.0 || r > 1.0 ||
       g < 0.0 || g > 1.0 ||
       b < 0.0 || b > 1.0) ||
      (r == this->TextColor[0] &&
       g == this->TextColor[1] &&
       b == this->TextColor[2]))
    {
    return;
    }

  this->TextColor[0] = r;
  this->TextColor[1] = g;
  this->TextColor[2] = b;

  this->Modified();

  this->UpdateInternalCanvasColors();
  this->StartupPageCanvas->DeleteTag("text");
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetTextSize(int arg)
{
  if (arg == this->TextSize)
    {
    return;
    }

  this->TextSize = arg;

  this->Modified();

  this->UpdateInternalCanvasFonts();
  this->StartupPageCanvas->DeleteTag("text");
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetSelectedTextColor(
  double r, double g, double b)
{
  if ((r < 0.0 || r > 1.0 ||
       g < 0.0 || g > 1.0 ||
       b < 0.0 || b > 1.0) ||
      (r == this->SelectedTextColor[0] &&
       g == this->SelectedTextColor[1] &&
       b == this->SelectedTextColor[2]))
    {
    return;
    }

  this->SelectedTextColor[0] = r;
  this->SelectedTextColor[1] = g;
  this->SelectedTextColor[2] = b;

  this->Modified();

  this->UpdateInternalCanvasColors();
  this->StartupPageCanvas->DeleteTag("text");
  this->StartupPageCanvas->DeleteTag("icon");
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetHintColor(
  double r, double g, double b)
{
  if ((r < 0.0 || r > 1.0 ||
       g < 0.0 || g > 1.0 ||
       b < 0.0 || b > 1.0) ||
      (r == this->HintColor[0] &&
       g == this->HintColor[1] &&
       b == this->HintColor[2]))
    {
    return;
    }

  this->HintColor[0] = r;
  this->HintColor[1] = g;
  this->HintColor[2] = b;

  this->Modified();

  this->UpdateInternalCanvasColors();
  this->StartupPageCanvas->DeleteTag("text");
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetHintSize(int arg)
{
  if (arg == this->HintSize)
    {
    return;
    }

  this->HintSize = arg;

  this->Modified();

  this->UpdateInternalCanvasFonts();
  this->StartupPageCanvas->DeleteTag("text");
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetAddShadowToHint(int arg)
{
  if (this->AddShadowToHint == arg)
    {
    return;
    }

  this->AddShadowToHint = arg;

  this->Modified();

  this->StartupPageCanvas->DeleteTag("text");
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetMostRecentFileSize(int arg)
{
  if (arg == this->MostRecentFileSize)
    {
    return;
    }

  this->MostRecentFileSize = arg;

  this->Modified();

  this->UpdateInternalCanvasFonts();
  this->StartupPageCanvas->DeleteTag("text");
  this->Redraw();
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetOpenCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->OpenCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::InvokeOpenCommand()
{
  this->InvokeObjectMethodCommand(this->OpenCommand);
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetDropCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->DropCommand, object, method);

  if (this->IsCreated())
    {
    this->SetDropFileBinding(NULL, this->DropCommand);
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::SetDoubleClickCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(
    &this->DoubleClickCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::InvokeDoubleClickCommand()
{
  this->InvokeObjectMethodCommand(this->DoubleClickCommand);
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->StartupPageCanvas);

  if (this->GetEnabled())
    {
    this->Bind();
    }
  else
    {
    this->UnBind();
    }
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::ProcessCallbackCommandEvents(vtkObject *caller,
                                                          unsigned long event,
                                                          void *calldata)
{
  if (caller == this->MostRecentFilesManager)
    {
    switch (event)
      {
      case vtkKWMostRecentFilesManager::MenuHasChangedEvent:
        this->StartupPageCanvas->DeleteTag(VTK_KW_SPW_MRF_TAG);
        this->Redraw();
        break;
      }
    }

  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}

//----------------------------------------------------------------------------
void vtkKWStartupPageWidget::PrintSelf(
  ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "SupportOpen: "
     << (this->SupportOpen ? "On" : "Off") << endl;
  os << indent << "SupportDoubleClick: "<< this->SupportDoubleClick << endl;
  os << indent << "SupportDrop: "<< this->SupportDrop << endl;
  os << indent << "AddShadowToHint: "
     << this->AddShadowToHint << endl;
  os << indent << "MaximumNumberOfMostRecentFiles: " 
     << this->MaximumNumberOfMostRecentFiles << endl;
  os << indent << "GradientColor1: (" 
     << this->GradientColor1[0] << ", "
     << this->GradientColor1[1] << ", "
     << this->GradientColor1[2] << ") " << endl;
  os << indent << "GradientColor2: (" 
     << this->GradientColor2[0] << ", "
     << this->GradientColor2[1] << ", "
     << this->GradientColor2[2] << ") " << endl;
  os << indent << "TextSize: " << this->TextSize << endl;
  os << indent << "HintSize: " << this->HintSize << endl;
  os << indent << "MostRecentFileSize: " << this->MostRecentFileSize << endl;
  os << indent << "TextColor: (" 
     << this->TextColor[0] << ", "
     << this->TextColor[1] << ", "
     << this->TextColor[2] << ") " << endl;
  os << indent << "SelectedTextColor: (" 
     << this->SelectedTextColor[0] << ", "
     << this->SelectedTextColor[1] << ", "
     << this->SelectedTextColor[2] << ") " << endl;
  os << indent << "HintColor: (" 
     << this->HintColor[0] << ", "
     << this->HintColor[1] << ", "
     << this->HintColor[2] << ") " << endl;
  os << indent << "StartupPageCanvas: ";
  if (this->StartupPageCanvas)
    {
    os << endl;
    this->StartupPageCanvas->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << "None" << endl;
    }
}

