/*=========================================================================

  Module:    $RCSfile: vtkKWBWidgetsInit.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWBWidgetsInit.h"

#include "vtkObjectFactory.h"
#include "vtkKWTkUtilities.h"

#include "vtkTk.h"

#include "Utilities/BWidgets/vtkKWBWidgetsTclLibrary.h"
 
//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWBWidgetsInit );
vtkCxxRevisionMacro(vtkKWBWidgetsInit, "$Revision: 1.3 $");

int vtkKWBWidgetsInit::Initialized = 0;

//----------------------------------------------------------------------------
void vtkKWBWidgetsInit::Initialize(Tcl_Interp* interp)
{
  if (vtkKWBWidgetsInit::Initialized)
    {
    return;
    }

  if (!interp)
    {
    vtkGenericWarningMacro(
      "An interpreter is needed to initialize the BWidgets library.");
    return;
    }

  vtkKWBWidgetsInit::Initialized = 1;

  // Create the images required for tree.tcl

  if (!vtkKWTkUtilities::UpdatePhoto(
        interp, 
        "bwminus", 
        image_bwminus, 
        image_bwminus_width, image_bwminus_height, 
        image_bwminus_pixel_size, image_bwminus_length) ||
      !vtkKWTkUtilities::UpdatePhoto(
        interp, 
        "bwplus", 
        image_bwplus, 
        image_bwplus_width, image_bwplus_height, 
        image_bwplus_pixel_size, image_bwplus_length) ||
      !vtkKWTkUtilities::UpdatePhoto(
        interp, 
        "bwdragfile", 
        image_bwdragfile, 
        image_bwdragfile_width, image_bwdragfile_height, 
        image_bwdragfile_pixel_size, image_bwdragfile_length) ||
      !vtkKWTkUtilities::UpdatePhoto(
        interp, 
        "bwdragicon", 
        image_bwdragicon, 
        image_bwdragicon_width, image_bwdragicon_height, 
        image_bwdragicon_pixel_size, image_bwdragicon_length)
    )
    {
    vtkGenericWarningMacro("Can not initialize BWidgets resources.");
    return;
    }

  // Evaluate the library
  
  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_utils_tcl, 
                         file_utils_tcl_length,
                         file_utils_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_widget_tcl, 
                         file_widget_tcl_length,
                         file_widget_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_dragsite_tcl, 
                         file_dragsite_tcl_length,
                         file_dragsite_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_dropsite_tcl, 
                         file_dropsite_tcl_length,
                         file_dropsite_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_dynhelp_tcl, 
                         file_dynhelp_tcl_length,
                         file_dynhelp_tcl_decoded_length);

  // Combobox

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_arrow_tcl, 
                         file_arrow_tcl_length,
                         file_arrow_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_entry_tcl, 
                         file_entry_tcl_length,
                         file_entry_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_listbox_tcl, 
                         file_listbox_tcl_length,
                         file_listbox_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_combobox_tcl, 
                         file_combobox_tcl_length,
                         file_combobox_tcl_decoded_length);

  // ScrolledWindow/Frame

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_scrollframe_tcl, 
                         file_scrollframe_tcl_length,
                         file_scrollframe_tcl_decoded_length);

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_scrollw_tcl, 
                         file_scrollw_tcl_length,
                         file_scrollw_tcl_decoded_length);

  // Tree

  vtkKWTkUtilities::EvaluateEncodedString(interp, 
                         file_tree_tcl, 
                         file_tree_tcl_length,
                         file_tree_tcl_decoded_length);
}

//----------------------------------------------------------------------------
void vtkKWBWidgetsInit::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


