/*=========================================================================

  Module:    $RCSfile: vtkKWColorSwatchesWidget.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkKWColorSwatchesWidget.h"

#include "vtkMath.h"

#include "vtkKWApplication.h"
#include "vtkKWLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkKWInternationalization.h"
#include "vtkKWBalloonHelpManager.h"
#include "vtkKWComboBox.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameSet.h"

#include <vtksys/stl/list>
#include <vtksys/stl/string>
#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWColorSwatchesWidget);
vtkCxxRevisionMacro(vtkKWColorSwatchesWidget, "$Revision: 1.3 $");

//----------------------------------------------------------------------------
class vtkKWColorSwatchesWidgetInternals
{
public:
  struct SwatchNode
  {
    vtksys_stl::string Name;
    double RGB[3];
    char HexRGB[6];
  };

  typedef vtksys_stl::list<SwatchNode> SwatchContainer;
  typedef vtksys_stl::list<SwatchNode>::iterator SwatchIterator;

  struct SwatchCollectionNode
  {
    int Id;
    vtksys_stl::string Name;
    SwatchContainer Swatches;
  };

  typedef vtksys_stl::list<SwatchCollectionNode> SwatchCollectionContainer;
  typedef vtksys_stl::list<SwatchCollectionNode>::iterator SwatchCollectionIterator;

  SwatchCollectionContainer SwatchCollections;

  SwatchCollectionNode* GetCollectionByName(const char *name);
  SwatchCollectionNode* GetCollectionById(int id);

  vtkKWFrameSet *SwatchesFrameSet;
  vtkKWBalloonHelpManager *SwatchesBalloonHelpManager;
  vtkKWComboBox *CollectionComboBox;

  static int SwatchCollectionCounter;
  vtksys_stl::string SchedulePopulateCollectionsTimerId;
  vtksys_stl::string SchedulePopulateSwatchesTimerId;
};

int vtkKWColorSwatchesWidgetInternals::SwatchCollectionCounter = 0;

vtkKWColorSwatchesWidgetInternals::SwatchCollectionNode* 
vtkKWColorSwatchesWidgetInternals::GetCollectionByName(const char *name)
{
  if (name && *name)
    {
    SwatchCollectionIterator it = this->SwatchCollections.begin();
    SwatchCollectionIterator end = this->SwatchCollections.end();
    for (; it != end; ++it)
      {
      if (!it->Name.compare(name))
        {
        return &(*it);
        }
      }
    }
  return NULL;
}

vtkKWColorSwatchesWidgetInternals::SwatchCollectionNode* 
vtkKWColorSwatchesWidgetInternals::GetCollectionById(int id)
{
  if (id >= 0)
    {
    SwatchCollectionIterator it = this->SwatchCollections.begin();
    SwatchCollectionIterator end = this->SwatchCollections.end();
    for (; it != end; ++it)
      {
      if (it->Id == id)
        {
        return &(*it);
        }
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkKWColorSwatchesWidget::vtkKWColorSwatchesWidget()
{
  this->Internals = new vtkKWColorSwatchesWidgetInternals;
  this->SwatchSize = 16;
  this->SwatchSelectedCommand = NULL;
  this->Internals->SwatchesFrameSet = vtkKWFrameSet::New();
  this->Internals->SwatchesBalloonHelpManager = NULL;
  this->Internals->CollectionComboBox = NULL;
}

//----------------------------------------------------------------------------
vtkKWColorSwatchesWidget::~vtkKWColorSwatchesWidget()
{
  if (this->SwatchSelectedCommand)
    {
    delete [] this->SwatchSelectedCommand;
    this->SwatchSelectedCommand = NULL;
    }
  if (this->Internals->SwatchesFrameSet)
    {
    this->Internals->SwatchesFrameSet->Delete();
    this->Internals->SwatchesFrameSet = NULL;
    }
  if (this->Internals->SwatchesBalloonHelpManager)
    {
    this->Internals->SwatchesBalloonHelpManager->Delete();
    this->Internals->SwatchesBalloonHelpManager = NULL;
    }
  if (this->Internals->CollectionComboBox)
    {
    this->Internals->CollectionComboBox->Delete();
    this->Internals->CollectionComboBox = NULL;
    }

  delete this->Internals;
}

//----------------------------------------------------------------------------
int vtkKWColorSwatchesWidget::GetCollectionId(const char *name)
{
  vtkKWColorSwatchesWidgetInternals::SwatchCollectionNode *collection =
    this->Internals->GetCollectionByName(name);
  if (collection)
    {
    return collection->Id;
    }
  return -1;
}

//----------------------------------------------------------------------------
int vtkKWColorSwatchesWidget::AddCollection(const char *name)
{
  // Need a name

  if (!name || !*name)
    {
    return -1;
    }

  // Already exists? Return the id

  int id = this->GetCollectionId(name);
  if (id >= 0)
    {
    return id;
    }

  // Create a new one

  id = vtkKWColorSwatchesWidgetInternals::SwatchCollectionCounter++;
  vtkKWColorSwatchesWidgetInternals::SwatchCollectionNode temp;
  this->Internals->SwatchCollections.push_back(temp);
  vtkKWColorSwatchesWidgetInternals::SwatchCollectionNode &collection = 
    this->Internals->SwatchCollections.back();
  collection.Id = id;
  collection.Name = name;

  this->SchedulePopulateCollections();

  return id;
}

//----------------------------------------------------------------------------
int vtkKWColorSwatchesWidget::RemoveCollection(int id)
{
  vtkKWColorSwatchesWidgetInternals::SwatchCollectionIterator it = 
    this->Internals->SwatchCollections.begin();
  vtkKWColorSwatchesWidgetInternals::SwatchCollectionIterator end = 
    this->Internals->SwatchCollections.end();
  for (; it != end; ++it)
    {
    if ((*it).Id == id)
      {
      this->Internals->SwatchCollections.erase(it);
      this->SchedulePopulateCollections();
      return 1;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::RemoveAllCollections()
{
  this->Internals->SwatchCollections.clear();
  this->SchedulePopulateCollections();
}

//----------------------------------------------------------------------------
int vtkKWColorSwatchesWidget::AddRGBSwatch(
  int collection_id, const char *name, double rgb[3])
{
  return this->AddRGBSwatch(collection_id, name, rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------
int vtkKWColorSwatchesWidget::AddRGBSwatch(
  int collection_id, const char *name, double r, double g, double b)
{
  vtkKWColorSwatchesWidgetInternals::SwatchCollectionNode *collection =
    this->Internals->GetCollectionById(collection_id);
  if (!collection)
    {
    return 0;
    }

  vtkKWColorSwatchesWidgetInternals::SwatchNode temp;
  collection->Swatches.push_back(temp);
  vtkKWColorSwatchesWidgetInternals::SwatchNode &swatch = 
    collection->Swatches.back();
  if (name)
    {
    swatch.Name = name;
    }
  swatch.RGB[0] = r;
  swatch.RGB[1] = g;
  swatch.RGB[2] = b;
  swatch.HexRGB[0] = 0;

  this->SchedulePopulateSwatches();

  return 1;
}

// ---------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::CreateWidget()
{
  if (this->IsCreated())
    {
    vtkErrorMacro("The panel is already created.");
    return;
    }

  // Create the superclass instance (and set the application)

  this->Superclass::CreateWidget();

  // Collection combobox

  this->Internals->CollectionComboBox = vtkKWComboBox::New();
  this->Internals->CollectionComboBox->SetParent(this);
  this->Internals->CollectionComboBox->Create();
  this->Internals->CollectionComboBox->SetReadOnly(1);
  this->Internals->CollectionComboBox->SetWidth(15);
  this->Internals->CollectionComboBox->SetCommand(
    this, "CollectionSelectedCallback");

  this->Script("pack %s -side top -anchor nw -expand n -fill none",
               this->Internals->CollectionComboBox->GetWidgetName());

  // Swatches frames

  if (!this->Internals->SwatchesFrameSet)
    {
    this->Internals->SwatchesFrameSet = vtkKWFrameSet::New();
    }
  this->Internals->SwatchesFrameSet->SetParent(this);
  this->Internals->SwatchesFrameSet->Create();
  this->Internals->SwatchesFrameSet->PackHorizontallyOn();
  this->Internals->SwatchesFrameSet->SetMaximumNumberOfWidgetsInPackingDirection(13);
  this->Internals->SwatchesFrameSet->SetWidgetsPadX(2);
  this->Internals->SwatchesFrameSet->SetWidgetsPadY(
    this->Internals->SwatchesFrameSet->GetWidgetsPadX());

  this->Script("pack %s -side top -anchor nw -expand n -fill none",
               this->Internals->SwatchesFrameSet->GetWidgetName());

  this->SchedulePopulateCollections();
  this->SchedulePopulateSwatches();
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::SetMaximumNumberOfSwatchesPerRow(int arg)
{
  this->Internals->SwatchesFrameSet->SetMaximumNumberOfWidgetsInPackingDirection(arg);
}

//----------------------------------------------------------------------------
int vtkKWColorSwatchesWidget::GetMaximumNumberOfSwatchesPerRow()
{
  return this->Internals->SwatchesFrameSet->GetMaximumNumberOfWidgetsInPackingDirection();
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::SetSwatchesPadding(int arg)
{
  this->Internals->SwatchesFrameSet->SetWidgetsPadX(arg);
  this->Internals->SwatchesFrameSet->SetWidgetsPadY(arg);
}

//----------------------------------------------------------------------------
int vtkKWColorSwatchesWidget::GetSwatchesPadding()
{
  return this->Internals->SwatchesFrameSet->GetWidgetsPadX();
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::PopulateCollections()
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *temp = this->Internals->CollectionComboBox->GetValue();
  vtksys_stl::string old_sel(temp ? temp : "");

  this->Internals->CollectionComboBox->DeleteAllValues();

  vtkKWColorSwatchesWidgetInternals::SwatchCollectionIterator it = 
    this->Internals->SwatchCollections.begin();
  vtkKWColorSwatchesWidgetInternals::SwatchCollectionIterator end = 
    this->Internals->SwatchCollections.end();
  for (; it != end; ++it)
    {
    this->Internals->CollectionComboBox->AddValue((*it).Name.c_str());
    }

  const char *value = NULL;
  if (old_sel.size() && this->GetCollectionId(old_sel.c_str()) != -1)
    {
    value = old_sel.c_str();
    }
  else 
    {
    if (this->Internals->SwatchCollections.size())
      {
      value = this->Internals->SwatchCollections.front().Name.c_str();
      }
    this->SchedulePopulateSwatches();
    }
  this->Internals->CollectionComboBox->SetValue(value);
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::SchedulePopulateCollections()
{
  // Already scheduled

  if (this->Internals->SchedulePopulateCollectionsTimerId.size())
    {
    return;
    }

  this->Internals->SchedulePopulateCollectionsTimerId =
    this->Script("after idle {catch {%s PopulateCollectionsCallback}}", 
                 this->GetTclName());
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::PopulateCollectionsCallback()
{
  if (!this->GetApplication() || 
      this->GetApplication()->GetInExit() ||
      !this->IsAlive())
    {
    return;
    }

  this->PopulateCollections();
  this->Internals->SchedulePopulateCollectionsTimerId = "";
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::PopulateSwatches()
{
  if (!this->IsCreated())
    {
    return;
    }

  const char *current_collection_name = NULL;
  if (this->Internals->SwatchCollections.size())
    {
    current_collection_name = this->Internals->CollectionComboBox->GetValue();
    }

  vtkKWColorSwatchesWidgetInternals::SwatchCollectionNode *collection =
    this->Internals->GetCollectionByName(current_collection_name);

  int nb_swatches = collection ? (int)collection->Swatches.size() : 0;
  int nb_frames = this->Internals->SwatchesFrameSet->GetNumberOfWidgets();

  int max_entries = nb_swatches > nb_frames ? nb_swatches : nb_frames;
  if (!max_entries)
    {
    return;
    }

  // Create or update the frames

  int *frames_id = new int[max_entries];
  int *frames_id_ptr = frames_id;
  int *frames_visibility = new int[max_entries];
  int *frames_visibility_ptr = frames_visibility;

  char command[128];
  int i = 0;

  if (collection)
    {
    // Create the special balloon help manager, if not created already
    
    if (!this->Internals->SwatchesBalloonHelpManager)
      {
      this->Internals->SwatchesBalloonHelpManager = 
        vtkKWBalloonHelpManager::New();
      this->Internals->SwatchesBalloonHelpManager->SetApplication(
        this->GetApplication());
      this->Internals->SwatchesBalloonHelpManager->SetIgnoreIfNotEnabled(1);
      this->Internals->SwatchesBalloonHelpManager->SetDelay(10);
      }

    vtksys_ios::ostringstream tk_cmd;
    
    vtkKWColorSwatchesWidgetInternals::SwatchIterator it = 
      collection->Swatches.begin();
    vtkKWColorSwatchesWidgetInternals::SwatchIterator end = 
      collection->Swatches.end();
    for (; it != end; ++it, ++i)
      {
      vtkKWColorSwatchesWidgetInternals::SwatchNode &swatch = *it;
      vtkKWFrame *frame = this->Internals->SwatchesFrameSet->GetWidget(i);
      if (!frame)
        {
        frame = this->Internals->SwatchesFrameSet->AddWidget(i);
        }
      if (swatch.HexRGB[0] == 0)
        {
        // Cache the Hex RGB for speed
        sprintf(swatch.HexRGB, "%02x%02x%02x", 
                (int)(swatch.RGB[0] * 255.0),
                (int)(swatch.RGB[1] * 255.0),
                (int)(swatch.RGB[2] * 255.0));
        }
      tk_cmd << frame->GetWidgetName() 
             << " configure -bd 1 -relief solid -width " << this->SwatchSize 
             << " -height " << this->SwatchSize << " -bg #" << swatch.HexRGB 
             << endl; 
      sprintf(command, "SwatchSelectedCallback %f %f %f", 
              swatch.RGB[0], swatch.RGB[1], swatch.RGB[2]);
      frame->SetBinding("<Any-ButtonPress>", this, command);
      if (swatch.Name.size())
        {
        frame->SetBalloonHelpManager(
          this->Internals->SwatchesBalloonHelpManager);
        frame->SetBalloonHelpString(swatch.Name.c_str());
        }
      *frames_id_ptr++ = i;
      *frames_visibility_ptr++ = 1;
      }

    this->Script(tk_cmd.str().c_str());
    }

  // Hide the remaining frames

  for (; i < max_entries; ++i)
    {
    *frames_id_ptr++ = i;
    *frames_visibility_ptr++ = 0;
    }

  this->Internals->SwatchesFrameSet->SetWidgetsVisibility(
    max_entries, frames_id, frames_visibility);
  
  delete [] frames_id;
  delete [] frames_visibility;
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::SchedulePopulateSwatches()
{
  // Already scheduled

  if (this->Internals->SchedulePopulateSwatchesTimerId.size())
    {
    return;
    }

  this->Internals->SchedulePopulateSwatchesTimerId =
    this->Script("after idle {catch {%s PopulateSwatchesCallback}}", 
                 this->GetTclName());
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::PopulateSwatchesCallback()
{
  if (!this->GetApplication() || 
      this->GetApplication()->GetInExit() ||
      !this->IsAlive())
    {
    return;
    }

  this->PopulateSwatches();
  this->Internals->SchedulePopulateSwatchesTimerId = "";
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::SwatchSelectedCallback(
  double r, double g, double b)
{
  this->InvokeSwatchSelectedCommand(r, g, b);
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::CollectionSelectedCallback(const char*)
{
  this->SchedulePopulateSwatches();
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::SetSwatchSize(int arg)
{
  if (arg < 3)
    {
    arg = 3;
    }

  if (this->SwatchSize == arg)
    {
    return;
    }

  this->SwatchSize = arg;

  this->Modified();

  this->SchedulePopulateSwatches();
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::SetSwatchSelectedCommand(
  vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->SwatchSelectedCommand, object, method);
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::InvokeSwatchSelectedCommand(
  double r, double g, double b)
{
  if (this->SwatchSelectedCommand && *this->SwatchSelectedCommand && 
      this->GetApplication())
    {
    this->Script("%s %f %f %f", this->SwatchSelectedCommand, r, g, b);
    }
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::AddDefaultCollections()
{
  struct color_swatch
  {
    const char *Name;
    double R, G, B;
  };
  int i, id;
  char buffer[100];

  double r, g, b;
  double h, s, v;
  const int nb_generated_colors = 100;

  // -----------------------------------------------------------------------
  // Basic colors

  id = this->AddCollection("Basic Colors");

  color_swatch basic_colors[] =
  {
    {NULL, 1.00, 0.50, 0.50}, 
    {NULL, 1.00, 1.00, 0.50}, 
    {NULL, 0.50, 1.00, 0.50}, 
    {NULL, 0.00, 1.00, 0.50}, 
    {NULL, 0.50, 1.00, 1.00}, 
    {NULL, 0.00, 0.50, 1.00}, 
    {NULL, 1.00, 0.50, 0.75}, 
    {NULL, 1.00, 0.50, 1.00}, 
    {NULL, 1.00, 0.00, 0.00}, 
    {NULL, 1.00, 1.00, 0.00}, 
    {NULL, 0.50, 1.00, 0.00}, 
    {NULL, 0.00, 1.00, 0.25}, 
    {NULL, 0.00, 1.00, 1.00}, 
    {NULL, 0.00, 0.50, 0.75}, 
    {NULL, 0.50, 0.50, 0.75}, 
    {NULL, 1.00, 0.00, 1.00}, 
    {NULL, 0.50, 0.25, 0.25}, 
    {NULL, 1.00, 0.50, 0.25}, 
    {NULL, 0.00, 1.00, 0.00}, 
    {NULL, 0.00, 0.50, 0.50}, 
    {NULL, 0.00, 0.25, 0.50}, 
    {NULL, 0.50, 0.50, 1.00}, 
    {NULL, 0.50, 0.00, 0.25}, 
    {NULL, 1.00, 0.00, 0.50}, 
    {NULL, 0.50, 0.00, 0.00}, 
    {NULL, 1.00, 0.50, 0.00}, 
    {NULL, 0.00, 0.50, 0.00}, 
    {NULL, 0.00, 0.50, 0.25}, 
    {NULL, 0.00, 0.00, 1.00}, 
    {NULL, 0.00, 0.00, 0.63}, 
    {NULL, 0.50, 0.00, 0.50}, 
    {NULL, 0.50, 0.00, 1.00}, 
    {NULL, 0.25, 0.00, 0.00}, 
    {NULL, 0.50, 0.25, 0.00}, 
    {NULL, 0.00, 0.25, 0.00}, 
    {NULL, 0.00, 0.25, 0.25}, 
    {NULL, 0.00, 0.00, 0.50}, 
    {NULL, 0.00, 0.00, 0.25}, 
    {NULL, 0.25, 0.00, 0.25}, 
    {NULL, 0.25, 0.00, 0.50}, 
    {NULL, 0.00, 0.00, 0.00}, 
    {NULL, 0.50, 0.50, 0.00}, 
    {NULL, 0.50, 0.50, 0.25}, 
    {NULL, 0.50, 0.50, 0.50}, 
    {NULL, 0.25, 0.50, 0.50}, 
    {NULL, 0.75, 0.75, 0.75}, 
    {NULL, 0.25, 0.00, 0.25}, 
    {NULL, 1.00, 1.00, 1.00}, 
  };

  for (i = 0; i < sizeof(basic_colors) / sizeof(basic_colors[0]); ++i)
    {
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(basic_colors[i].R * 255.0),
            (int)(basic_colors[i].G * 255.0),
            (int)(basic_colors[i].B * 255.0),
            basic_colors[i].R,
            basic_colors[i].G,
            basic_colors[i].B);
    this->AddRGBSwatch(
      id, 
      buffer, 
      basic_colors[i].R, basic_colors[i].G, basic_colors[i].B);
    }

  // ===========================================
  // Rainbow (Full)

  id = this->AddCollection("Rainbow");
  for (h = 0.0; 
       h <= 1.0; 
       h += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, 1.0, 1.0, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Grayscale

  id = this->AddCollection("Grayscale");
  for (v = 0.0; 
       v <= 1.0; 
       v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(0.0, 0.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Iron

  id = this->AddCollection("Iron");
  for (h = 0.0; 
       h <= 0.15; 
       h += (0.15 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, 1.0, 1.0, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Ocean

  id = this->AddCollection("Ocean");
  for (h = 2.0/3.0; 
       h >= 0.5; h += (0.5 - 2.0/3.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, 1.0, 1.0, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Red

  id = this->AddCollection("Red");
  for (v = 0.0; 
       v <= 1.0; 
       v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(0, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Green

  id = this->AddCollection("Green");
  for (v = 0.0; 
       v <= 1.0; 
       v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(1.0/3.0, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Blue

  id = this->AddCollection("Blue");
  for (v = 0.0; 
       v <= 1.0; 
       v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(2.0/3.0, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Yellow

  id = this->AddCollection("Yellow");
  for (v = 0.0; 
       v <= 1.0; 
       v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(0.1666667, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Cyan

  id = this->AddCollection("Cyan");
  for (v = 0.0; 
       v <= 1.0; 
       v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(0.5, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Magenta

  id = this->AddCollection("Magenta");
  for (v = 0.0; 
       v <= 1.0; 
       v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(0.8333, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Warm Shade 1

  id = this->AddCollection("Warm Shade (1)");
  for (h = 0.1667, v = 0.0; 
       v <= 1.0; 
       h += (0.0 - 0.1667) / (double)nb_generated_colors, 
         v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Warm Shade 2

  id = this->AddCollection("Warm Shade (2)");
  for (h = 0.3333, v = 0.0; 
       v <= 1.0; 
       h += (0.1667 - 0.3333) / (double)nb_generated_colors, 
         v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Warm Shade 3

  id = this->AddCollection("Warm Shade (3)");
  for (h = 0.5, v = 0.0; 
       v <= 1.0; 
       h += (0.3333 - 0.5) / (double)nb_generated_colors, 
         v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Cool Shade 1

  id = this->AddCollection("Cool Shade (1)");
  for (h = 0.6667, v = 0.0; 
       v <= 1.0; 
       h += (0.5 - 0.6667) / (double)nb_generated_colors, 
         v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Cool Shade 2

  id = this->AddCollection("Cool Shade (2)");
  for (h = 0.8333, v = 0.0; 
       v <= 1.0; 
       h += (0.6667 - 0.8333) / (double)nb_generated_colors, 
         v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Cool Shade 3

  id = this->AddCollection("Cool Shade (3)");
  for (h = 1.0, v = 0.0; 
       v <= 1.0; 
       h += (0.8333 - 1.0) / (double)nb_generated_colors, 
         v += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, 1.0, v, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Warm Tint 1

  id = this->AddCollection("Warm Tint (1)");
  for (h = 0.1667, s = 0.0; 
       s <= 1.0; 
       h += (0.0 - 0.1667)  / (double)nb_generated_colors, 
         s += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, s, 1.0, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Warm Tint 2

  id = this->AddCollection("Warm Tint (2)");
  for (h = 0.3333, s = 0.0; 
       s <= 1.0; 
       h += (0.1667 - 0.3333) / (double)nb_generated_colors, 
         s += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, s, 1.0, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Warm Tint 3

  id = this->AddCollection("Warm Tint (3)");
  for (h = 0.5, s = 0.0; 
       s <= 1.0; 
       h += (0.3333 - 0.5) / (double)nb_generated_colors, 
         s += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, s, 1.0, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Cool Tint 1

  id = this->AddCollection("Cool Tint (1)");
  for (h = 0.6667, s = 0.0; 
       s <= 1.0; 
       h += (0.5 - 0.6667) / (double)nb_generated_colors, 
         s += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, s, 1.0, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Cool Tint 2

  id = this->AddCollection("Cool Tint (2)");
  for (h = 0.8333, s = 0.0; 
       s <= 1.0; 
       h += (0.6667 - 0.8333) / (double)nb_generated_colors, 
         s += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, s, 1.0, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================
  // Cool Tint 3

  id = this->AddCollection("Cool Tint (3)");
  for (h = 1.0, s = 0.0; 
       s <= 1.0; 
       h += (0.8333 - 1.0) / (double)nb_generated_colors, 
         s += (1.0 - 0.0) / (double)nb_generated_colors)
    {
    vtkMath::HSVToRGB(h, s, 1.0, &r, &g, &b);
    sprintf(buffer, 
            "#%02x%02x%02x (%.2f, %.2f, %.2f)", 
            (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0),
            r, g, b);
    this->AddRGBSwatch(id, buffer, r, g, b);
    }

  // ===========================================

  this->SchedulePopulateCollections();
  this->SchedulePopulateSwatches();
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->Internals->SwatchesFrameSet);
}

//----------------------------------------------------------------------------
void vtkKWColorSwatchesWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "SwatchSize: " << this->SwatchSize << endl;
}

