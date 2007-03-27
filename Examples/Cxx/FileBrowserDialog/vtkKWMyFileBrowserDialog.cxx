#include "vtkKWMyFileBrowserDialog.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWTextWithScrollbarsWithLabel.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWText.h"
#include "vtkKWLabel.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWFileListTable.h"
#include "vtkKWFileBrowserWidget.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMyFileBrowserDialog );
vtkCxxRevisionMacro(vtkKWMyFileBrowserDialog, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkKWMyFileBrowserDialog::vtkKWMyFileBrowserDialog()
{
  this->PreviewFrameVisibility = 1;

  this->ContentText = NULL;
}

//----------------------------------------------------------------------------
vtkKWMyFileBrowserDialog::~vtkKWMyFileBrowserDialog()
{
  if (this->ContentText)
    {
    this->ContentText->Delete();
    this->ContentText = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkKWMyFileBrowserDialog::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();
  
  // Create the preview content

  this->ContentText = vtkKWTextWithScrollbarsWithLabel::New();
  this->ContentText->SetParent(this->GetPreviewFrame());
  this->ContentText->Create();
  this->ContentText->SetBorderWidth(2);
  this->ContentText->SetReliefToGroove();
  this->ContentText->SetPadX(2);
  this->ContentText->SetPadY(2);
  this->ContentText->ExpandWidgetOn();

  this->Script("pack %s -side top -expand y -fill both", 
               this->ContentText->GetWidgetName());

  this->ContentText->SetLabelText("Preview text files:");
  this->ContentText->SetLabelPositionToTop();
  vtkKWTkUtilities::ChangeFontWeightToBold(this->ContentText->GetLabel());

  this->Script("pack %s -side top -expand y -fill both", 
               this->ContentText->GetWidget()->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWMyFileBrowserDialog::ProcessFileSelectionChangedEvent(
  const char* fullname)
{
  this->Superclass::ProcessFileSelectionChangedEvent(fullname);

  this->UpdatePreview();
}

//----------------------------------------------------------------------------
void vtkKWMyFileBrowserDialog::UpdatePreview()
{
  vtkKWFileListTable *table = this->GetFileBrowserWidget()->GetFileListTable();
  const char *fullname = table->GetSelectedFileName();

  vtkKWText *text = this->ContentText->GetWidget()->GetWidget();
  text->SetText("");

  // If no file, or directory, or it's a binary file, don't do anything

  if (!fullname || !*fullname || 
      vtksys::SystemTools::FileIsDirectory(fullname) ||
      (vtksys::SystemTools::DetectFileType(fullname) != 
       vtksys::SystemTools::FileTypeText))
    {
    return;
    } 

  // Load the first few bytes and update the preview

  FILE *fp = fopen(fullname, "r");
  if (!fp)
    {
    return;
    }

  const size_t fsize = 512;

  char buffer[fsize + 1];
  buffer[0] = '\0';
  size_t count = fread(buffer, 1, fsize, fp);
  if (count)
    {
    buffer[count] = '\0';
    text->SetText(buffer);
    }
  fclose(fp);
}

