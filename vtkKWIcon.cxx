/*=========================================================================

  Module:    $RCSfile: vtkKWIcon.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWIcon.h"

#include "vtkObjectFactory.h"
#include "vtkKWResourceUtilities.h"
#include "vtkColorTransferFunction.h"
#include "vtkImageClip.h"
#include "vtkImageResample.h"
#include "vtkImageData.h"
#include "vtkImagePermute.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPointData.h"
#include "vtkMath.h"

#include "Resources/vtkKWIconResources.h"
#include "Resources/vtkKWNuvolaIconResources.h"
#include "Resources/vtkKWCrystalProjectIconResources.h"
#include "Resources/vtkKWSilkIconResources.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWIcon );
vtkCxxRevisionMacro(vtkKWIcon, "$Revision: 1.70 $");

//----------------------------------------------------------------------------
vtkKWIcon::vtkKWIcon()
{
  this->Data         = 0;
  this->Width        = 0;
  this->Height       = 0;
  this->PixelSize    = 0;
}

//----------------------------------------------------------------------------
vtkKWIcon::~vtkKWIcon()
{
  this->SetData(NULL, 0, 0, 0);
}

//----------------------------------------------------------------------------
void vtkKWIcon::DeepCopy(vtkKWIcon* icon)
{
  this->SetImage(icon);
}

//----------------------------------------------------------------------------
void vtkKWIcon::SetImage(vtkKWIcon* icon)
{
  if (icon)
    {
    this->SetData(icon->GetData(), 
                  icon->GetWidth(), 
                  icon->GetHeight(), 
                  icon->GetPixelSize());
    }
  else
    {
    this->SetData(NULL, 0, 0, 0);
    }
}

//----------------------------------------------------------------------------
void vtkKWIcon::SetImage(vtkImageData* image)
{
  if (!image)
    {
    this->SetData(NULL, 0, 0, 0);
    return;
    }

  // First, let's make sure we are processing the image as it
  // is by clipping its UpdateExtent. By doing so, we prevent 
  // our permute filter the process the image's *whole* extent.

  vtkImageClip *clip = vtkImageClip::New();
  clip->SetInput(image);
  clip->SetOutputWholeExtent(image->GetUpdateExtent());

  // Permute, as a convenience (in case we were given a XZ or YZ slice)

  vtkImageData *input = NULL;

  clip->Update();
  int clip_dims[3];
  clip->GetOutput()->GetDimensions(clip_dims);

  vtkImagePermute *permute = NULL;
  if (clip_dims[2] != 1)
    {
    permute = vtkImagePermute::New();
    permute->SetInput(clip->GetOutput());
    if (clip_dims[0] == 1)
      {
      permute->SetFilteredAxes(1, 2, 0);
      }
    else
      {
      permute->SetFilteredAxes(0, 2, 1);
      }
    input = permute->GetOutput();
    }
  else
    {
    input = clip->GetOutput();
    }
  input->Update();

  int *input_dims = input->GetDimensions();
  this->SetData((const unsigned char*)input->GetScalarPointer(),
                input_dims[0], input_dims[1], 
                image->GetNumberOfScalarComponents(), 
                vtkKWIcon::ImageOptionFlipVertical);

  clip->Delete();
  if (permute)
    {
    permute->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkKWIcon::SetImage(const unsigned char *data, 
                         int width, 
                         int height, 
                         int pixel_size, 
                         unsigned long buffer_length,
                         int options)
{
  if (this->Data == data &&
      this->Width == width &&
      this->Height == height &&
      this->PixelSize == pixel_size &&
      !options)
    {
    return;
    }

  unsigned long nb_of_raw_bytes = width * height * pixel_size;
  if (!buffer_length)
    {
    buffer_length = nb_of_raw_bytes;
    }

  // Is the data encoded (zlib and/or base64) ?

  unsigned char *decoded_data = NULL;
  if (buffer_length && buffer_length != nb_of_raw_bytes)
    {
    if (!vtkKWResourceUtilities::DecodeBuffer(
          data, buffer_length, &decoded_data, nb_of_raw_bytes))
      {
      vtkErrorMacro("Error while decoding icon pixels");
      return;
      }
    data = decoded_data;
    }

  if (data)
    {
    this->SetData(data, width, height, pixel_size, options);
    }

  if (decoded_data)
    {
    delete [] decoded_data;
    }
}


//----------------------------------------------------------------------------
void vtkKWIcon::SetData(const unsigned char *data, 
                        int width, int height, 
                        int pixel_size,
                        int options)
{
  if (this->Data == data &&
      this->Width == width &&
      this->Height == height &&
      this->PixelSize == pixel_size &&
      !options)
    {
    return;
    }

  unsigned long stride = width * pixel_size;
  unsigned long buffer_length = stride * height;
  if (data && buffer_length > 0)
    {
    this->Width  = width;
    this->Height = height;
    this->PixelSize = pixel_size;
    unsigned char *new_data = new unsigned char [buffer_length];
    if (options & vtkKWIcon::ImageOptionFlipVertical)
      {
      const unsigned char *src = data + buffer_length - stride;
      unsigned char *dest = new_data;
      unsigned char *dest_end = dest + buffer_length;
      while (dest < dest_end)
        {
        memcpy(dest, src, stride);
        dest += stride;
        src -= stride;
        }
      }
    else
      {
      memcpy(new_data, data, buffer_length);
      }
    delete [] this->Data;
    this->Data = new_data;
    }
  else
    {
    delete [] this->Data;
    this->Data         = NULL;
    this->Width        = 0;
    this->Height       = 0;
    this->PixelSize    = 0;
    }
}

//----------------------------------------------------------------------------
void vtkKWIcon::SetImage(int image)
{
  this->SetData(NULL, 0, 0, 0);

  if (image == vtkKWIcon::IconNoIcon)
    {
    return;
    }
  
  switch (image)
    {
    case vtkKWIcon::IconAngleTool:
      this->SetImage(
        image_angle_tool, 
        image_angle_tool_width, image_angle_tool_height,
        image_angle_tool_pixel_size, 
        image_angle_tool_length);
      break;

    case vtkKWIcon::IconBiDimensionalTool:
      this->SetImage(
        image_bidimensional_tool, 
        image_bidimensional_tool_width, image_bidimensional_tool_height,
        image_bidimensional_tool_pixel_size, 
        image_bidimensional_tool_length);
      break;

    case vtkKWIcon::IconBoundingBox:
      this->SetImage(
        image_bounding_box, 
        image_bounding_box_width, image_bounding_box_height,
        image_bounding_box_pixel_size, 
        image_bounding_box_length);
      break;

    case vtkKWIcon::IconCalculator:
      this->SetImage(
        image_calculator, 
        image_calculator_width, image_calculator_height,
        image_calculator_pixel_size, 
        image_calculator_length);
      break;

    case vtkKWIcon::IconCamera:
      this->SetImage(
        image_camera, 
        image_camera_width, image_camera_height,
        image_camera_pixel_size, 
        image_camera_length);
      break;

    case vtkKWIcon::IconCameraMini:
      this->SetImage(
        image_camera_mini, 
        image_camera_mini_width, image_camera_mini_height,
        image_camera_mini_pixel_size, 
        image_camera_mini_length);
      break;

    case vtkKWIcon::IconColorBarAnnotation:
      this->SetImage(
        image_color_bar_annotation, 
        image_color_bar_annotation_width, image_color_bar_annotation_height,
        image_color_bar_annotation_pixel_size, 
        image_color_bar_annotation_length);
      break;

    case vtkKWIcon::IconColorSquares:
      this->SetImage(
        image_color_squares, 
        image_color_squares_width, image_color_squares_height,
        image_color_squares_pixel_size, 
        image_color_squares_length);
      break;

    case vtkKWIcon::IconColorAlphaSquares:
      this->SetImage(
        image_color_alpha_squares, 
        image_color_alpha_squares_width, image_color_alpha_squares_height,
        image_color_alpha_squares_pixel_size, 
        image_color_alpha_squares_length);
      break;

    case vtkKWIcon::IconColumns:
      this->SetImage(
        image_columns, 
        image_columns_width, image_columns_height,
        image_columns_pixel_size, 
        image_columns_length);
      break;

    case vtkKWIcon::IconCompress:
      this->SetImage(
        image_compress, 
        image_compress_width, image_compress_height,
        image_compress_pixel_size, 
        image_compress_length);
      break;

    case vtkKWIcon::IconConnection:
      this->SetImage(
        image_connection, 
        image_connection_width, image_connection_height,
        image_connection_pixel_size, 
        image_connection_length);
      break;

    case vtkKWIcon::IconContourTool:
      this->SetImage(
        image_contour_tool, 
        image_contour_tool_width, image_contour_tool_height,
        image_contour_tool_pixel_size, 
        image_contour_tool_length);
      break;

    case vtkKWIcon::IconContourSegment:
      this->SetImage(
        image_contour_segment, 
        image_contour_segment_width, image_contour_segment_height,
        image_contour_segment_pixel_size, 
        image_contour_segment_length);
      break;

    case vtkKWIcon::IconContrast:
      this->SetImage(
        image_contrast, 
        image_contrast_width, image_contrast_height,
        image_contrast_pixel_size, 
        image_contrast_length);
      break;

    case vtkKWIcon::IconCornerAnnotation:
      this->SetImage(
        image_corner_annotation, 
        image_corner_annotation_width, image_corner_annotation_height,
        image_corner_annotation_pixel_size, 
        image_corner_annotation_length);
      break;

    case vtkKWIcon::IconCropTool:
      this->SetImage(
        image_crop_tool, 
        image_crop_tool_width, image_crop_tool_height,
        image_crop_tool_pixel_size, 
        image_crop_tool_length);
      break;

    case vtkKWIcon::IconDistanceTool:
      this->SetImage(
        image_distance_tool, 
        image_distance_tool_width, image_distance_tool_height,
        image_distance_tool_pixel_size, 
        image_distance_tool_length);
      break;

    case vtkKWIcon::IconDocument:
      this->SetImage(
        image_document, 
        image_document_width, image_document_height,
        image_document_pixel_size, 
        image_document_length);
      break;

    case vtkKWIcon::IconDocumentRGBColor:
      this->SetImage(
        image_document_rgb_color, 
        image_document_rgb_color_width, image_document_rgb_color_height,
        image_document_rgb_color_pixel_size, 
        image_document_rgb_color_length);
      break;

    case vtkKWIcon::IconDocumentWindowLevel:
      this->SetImage(
        image_document_window_level, 
        image_document_window_level_width, image_document_window_level_height,
        image_document_window_level_pixel_size, 
        image_document_window_level_length);
      break;

    case vtkKWIcon::IconDocumentVolumeProperty:
      this->SetImage(
        image_document_volume_property, 
        image_document_volume_property_width, 
        image_document_volume_property_height,
        image_document_volume_property_pixel_size, 
        image_document_volume_property_length);
      break;

    case vtkKWIcon::IconEditContrast:
      this->SetImage(
        image_edit_contrast, 
        image_edit_contrast_width, image_edit_contrast_height,
        image_edit_contrast_pixel_size, 
        image_edit_contrast_length);
      break;

    case vtkKWIcon::IconEditVolumeProperties:
      this->SetImage(
        image_edit_volume_properties, 
        image_edit_volume_properties_width, image_edit_volume_properties_height,
        image_edit_volume_properties_pixel_size, 
        image_edit_volume_properties_length);
      break;

    case vtkKWIcon::IconEmpty16x16:
      this->SetImage(
        image_empty_16x16, 
        image_empty_16x16_width, image_empty_16x16_height,
        image_empty_16x16_pixel_size, 
        image_empty_16x16_length);
      break;

    case vtkKWIcon::IconEmpty1x1:
      this->SetImage(
        image_empty_1x1, 
        image_empty_1x1_width, image_empty_1x1_height,
        image_empty_1x1_pixel_size, 
        image_empty_1x1_length);
      break;

    case vtkKWIcon::IconError:
      this->SetImage(
        image_error, 
        image_error_width, image_error_height,
        image_error_pixel_size, 
        image_error_length);
      break;

    case vtkKWIcon::IconExpand:
      this->SetImage(
        image_expand, 
        image_expand_width, image_expand_height,
        image_expand_pixel_size, 
        image_expand_length);
      break;

    case vtkKWIcon::IconExpandMini:
      this->SetImage(
        image_expand_mini, 
        image_expand_mini_width, image_expand_mini_height,
        image_expand_mini_pixel_size, 
        image_expand_mini_length);
      break;

    case vtkKWIcon::IconExpandLeftMini:
      this->SetImage(
        image_expand_left_mini, 
        image_expand_left_mini_width, image_expand_left_mini_height,
        image_expand_left_mini_pixel_size, 
        image_expand_left_mini_length);
      break;

    case vtkKWIcon::IconExpandRightMini:
      this->SetImage(
        image_expand_right_mini, 
        image_expand_right_mini_width, image_expand_right_mini_height,
        image_expand_right_mini_pixel_size, 
        image_expand_right_mini_length);
      break;

    case vtkKWIcon::IconExpandUpMini:
      this->SetImage(
        image_expand_up_mini, 
        image_expand_up_mini_width, image_expand_up_mini_height,
        image_expand_up_mini_pixel_size, 
        image_expand_up_mini_length);
      break;

    case vtkKWIcon::IconEye:
      this->SetImage(
        image_eye, 
        image_eye_width, image_eye_height,
        image_eye_pixel_size, 
        image_eye_length);
      break;

    case vtkKWIcon::IconFileOpen:
      this->SetImage(
        image_file_open, 
        image_file_open_width, image_file_open_height,
        image_file_open_pixel_size, 
        image_file_open_length);
      break;

    case vtkKWIcon::IconFloppy:
      this->SetImage(
        image_floppy, 
        image_floppy_width, image_floppy_height,
        image_floppy_pixel_size, 
        image_floppy_length);
      break;

    case vtkKWIcon::IconFolder:
      this->SetImage(
        image_folder, 
        image_folder_width, image_folder_height,
        image_folder_pixel_size, 
        image_folder_length);
      break;

    case vtkKWIcon::IconFolderOpen:
      this->SetImage(
        image_folder_open, 
        image_folder_open_width, image_folder_open_height,
        image_folder_open_pixel_size, 
        image_folder_open_length);
      break;

    case vtkKWIcon::IconGrayscaleSquares:
      this->SetImage(
        image_grayscale_squares, 
        image_grayscale_squares_width, image_grayscale_squares_height,
        image_grayscale_squares_pixel_size, 
        image_grayscale_squares_length);
      break;

    case vtkKWIcon::IconGridLinear:
      this->SetImage(
        image_grid_linear, 
        image_grid_linear_width, image_grid_linear_height,
        image_grid_linear_pixel_size, 
        image_grid_linear_length);
      break;      

    case vtkKWIcon::IconGridLog:
      this->SetImage(
        image_grid_log, 
        image_grid_log_width, image_grid_log_height,
        image_grid_log_pixel_size, 
        image_grid_log_length);
      break;      

    case vtkKWIcon::IconHeaderAnnotation:
      this->SetImage(
        image_header_annotation, 
        image_header_annotation_width, image_header_annotation_height,
        image_header_annotation_pixel_size, 
        image_header_annotation_length);
      break;

    case vtkKWIcon::IconHelpBubble:
      this->SetImage(
        image_helpbubble, 
        image_helpbubble_width, image_helpbubble_height,
        image_helpbubble_pixel_size, 
        image_helpbubble_length);
      break;      

    case vtkKWIcon::IconHSVDiagram:
      this->SetImage(
        image_hsv_diagram, 
        image_hsv_diagram_width, image_hsv_diagram_height,
        image_hsv_diagram_pixel_size, 
        image_hsv_diagram_length);
      break;      

    case vtkKWIcon::IconInfoMini:
      this->SetImage(
        image_info_mini, 
        image_info_mini_width, image_info_mini_height,
        image_info_mini_pixel_size, 
        image_info_mini_length);
      break;

    case vtkKWIcon::IconInterpolationBilinear:
      this->SetImage(
        image_interpolation_bilinear, 
        image_interpolation_bilinear_width, image_interpolation_bilinear_height,
        image_interpolation_bilinear_pixel_size, 
        image_interpolation_bilinear_length);
      break;

    case vtkKWIcon::IconInterpolationNearest:
      this->SetImage(
        image_interpolation_nearest, 
        image_interpolation_nearest_width, image_interpolation_nearest_height,
        image_interpolation_nearest_pixel_size, 
        image_interpolation_nearest_length);
      break;

    case vtkKWIcon::IconLock:
      this->SetImage(
        image_lock, 
        image_lock_width, image_lock_height,
        image_lock_pixel_size, 
        image_lock_length);
      break;

    case vtkKWIcon::IconMagGlass:
      this->SetImage(
        image_mag_glass,
        image_mag_glass_width, image_mag_glass_height,
        image_mag_glass_pixel_size,
        image_mag_glass_length);
      break;

    case vtkKWIcon::IconMinus:
      this->SetImage(
        image_minus, 
        image_minus_width, image_minus_height,
        image_minus_pixel_size, 
        image_minus_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue1:
      this->SetImage(
        image_small_counter_blue_1, 
        image_small_counter_blue_1_width, image_small_counter_blue_1_height,
        image_small_counter_blue_1_pixel_size, 
        image_small_counter_blue_1_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue2:
      this->SetImage(
        image_small_counter_blue_2, 
        image_small_counter_blue_2_width, image_small_counter_blue_2_height,
        image_small_counter_blue_2_pixel_size, 
        image_small_counter_blue_2_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue3:
      this->SetImage(
        image_small_counter_blue_3, 
        image_small_counter_blue_3_width, image_small_counter_blue_3_height,
        image_small_counter_blue_3_pixel_size, 
        image_small_counter_blue_3_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue4:
      this->SetImage(
        image_small_counter_blue_4, 
        image_small_counter_blue_4_width, image_small_counter_blue_4_height,
        image_small_counter_blue_4_pixel_size, 
        image_small_counter_blue_4_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue5:
      this->SetImage(
        image_small_counter_blue_5, 
        image_small_counter_blue_5_width, image_small_counter_blue_5_height,
        image_small_counter_blue_5_pixel_size, 
        image_small_counter_blue_5_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue6:
      this->SetImage(
        image_small_counter_blue_6, 
        image_small_counter_blue_6_width, image_small_counter_blue_6_height,
        image_small_counter_blue_6_pixel_size, 
        image_small_counter_blue_6_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue7:
      this->SetImage(
        image_small_counter_blue_7, 
        image_small_counter_blue_7_width, image_small_counter_blue_7_height,
        image_small_counter_blue_7_pixel_size, 
        image_small_counter_blue_7_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue8:
      this->SetImage(
        image_small_counter_blue_8, 
        image_small_counter_blue_8_width, image_small_counter_blue_8_height,
        image_small_counter_blue_8_pixel_size, 
        image_small_counter_blue_8_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue9:
      this->SetImage(
        image_small_counter_blue_9, 
        image_small_counter_blue_9_width, image_small_counter_blue_9_height,
        image_small_counter_blue_9_pixel_size, 
        image_small_counter_blue_9_length);
      break;      

    case vtkKWIcon::IconSmallCounterBlue9Plus:
      this->SetImage(
        image_small_counter_blue_9plus, 
        image_small_counter_blue_9plus_width, image_small_counter_blue_9plus_height,
        image_small_counter_blue_9plus_pixel_size, 
        image_small_counter_blue_9plus_length);
      break;      

    case vtkKWIcon::IconMove:
      this->SetImage(
        image_move, 
        image_move_width, image_move_height,
        image_move_pixel_size, 
        image_move_length);
      break;      

    case vtkKWIcon::IconMoveH:
      this->SetImage(
        image_move_h, 
        image_move_h_width, image_move_h_height,
        image_move_h_pixel_size, 
        image_move_h_length);
      break;      

    case vtkKWIcon::IconMoveV:
      this->SetImage(
        image_move_v, 
        image_move_v_width, image_move_v_height,
        image_move_v_pixel_size, 
        image_move_v_length);
      break;      

    case vtkKWIcon::IconObliqueProbe:
      this->SetImage(
        image_oblique_probe, 
        image_oblique_probe_width, 
        image_oblique_probe_height,
        image_oblique_probe_pixel_size, 
        image_oblique_probe_length);
      break;

    case vtkKWIcon::IconOrientationCubeAnnotation:
      this->SetImage(
        image_orientation_cube_annotation, 
        image_orientation_cube_annotation_width, 
        image_orientation_cube_annotation_height,
        image_orientation_cube_annotation_pixel_size, 
        image_orientation_cube_annotation_length);
      break;

    case vtkKWIcon::IconPanHand:
      this->SetImage(
        image_pan_hand, 
        image_pan_hand_width, image_pan_hand_height,
        image_pan_hand_pixel_size, 
        image_pan_hand_length);
      break;      

    case vtkKWIcon::IconParallelProjection:
      this->SetImage(
        image_parallel_projection, 
        image_parallel_projection_width, image_parallel_projection_height,
        image_parallel_projection_pixel_size, 
        image_parallel_projection_length);
      break;      

    case vtkKWIcon::IconPerspectiveProjection:
      this->SetImage(
        image_perspective_projection, 
        image_perspective_projection_width, image_perspective_projection_height,
        image_perspective_projection_pixel_size, 
        image_perspective_projection_length);
      break;      

    case vtkKWIcon::IconPlus:
      this->SetImage(
        image_plus, 
        image_plus_width, image_plus_height,
        image_plus_pixel_size, 
        image_plus_length);
      break;      

    case vtkKWIcon::IconPointFinger:
      this->SetImage(
        image_point_finger, 
        image_point_finger_width, image_point_finger_height,
        image_point_finger_pixel_size, 
        image_point_finger_length);
      break;      

    case vtkKWIcon::IconPresetAdd:
      this->SetImage(
        image_preset_add, 
        image_preset_add_width, image_preset_add_height,
        image_preset_add_pixel_size, 
        image_preset_add_length);
      break;      

    case vtkKWIcon::IconPresetApply:
      this->SetImage(
        image_preset_apply, 
        image_preset_apply_width, image_preset_apply_height,
        image_preset_apply_pixel_size, 
        image_preset_apply_length);
      break;      

    case vtkKWIcon::IconPresetDelete:
      this->SetImage(
        image_preset_delete, 
        image_preset_delete_width, image_preset_delete_height,
        image_preset_delete_pixel_size, 
        image_preset_delete_length);
      break;      

    case vtkKWIcon::IconPresetEmail:
      this->SetImage(
        image_preset_email, 
        image_preset_email_width, image_preset_email_height,
        image_preset_email_pixel_size, 
        image_preset_email_length);
      break;      

    case vtkKWIcon::IconPresetLoad:
      this->SetImage(
        image_preset_load, 
        image_preset_load_width, image_preset_load_height,
        image_preset_load_pixel_size, 
        image_preset_load_length);
      break;      

    case vtkKWIcon::IconPresetLocate:
      this->SetImage(
        image_preset_locate, 
        image_preset_locate_width, image_preset_locate_height,
        image_preset_locate_pixel_size, 
        image_preset_locate_length);
      break;      

    case vtkKWIcon::IconPresetNext:
      this->SetImage(
        image_preset_next, 
        image_preset_next_width, image_preset_next_height,
        image_preset_next_pixel_size, 
        image_preset_next_length);
      break;      

    case vtkKWIcon::IconPresetPrevious:
      this->SetImage(
        image_preset_previous, 
        image_preset_previous_width, image_preset_previous_height,
        image_preset_previous_pixel_size, 
        image_preset_previous_length);
      break;      

    case vtkKWIcon::IconPresetRewind:
      this->SetImage(
        image_preset_rewind, 
        image_preset_rewind_width, image_preset_rewind_height,
        image_preset_rewind_pixel_size, 
        image_preset_rewind_length);
      break;      

    case vtkKWIcon::IconPresetUpdate:
      this->SetImage(
        image_preset_update, 
        image_preset_update_width, image_preset_update_height,
        image_preset_update_pixel_size, 
        image_preset_update_length);
      break;      

    case vtkKWIcon::IconPresetFilter:
      this->SetImage(
        image_preset_filter, 
        image_preset_filter_width, image_preset_filter_height,
        image_preset_filter_pixel_size, 
        image_preset_filter_length);
      break;      

    case vtkKWIcon::IconQuestion:
      this->SetImage(
        image_question, 
        image_question_width, image_question_height,
        image_question_pixel_size, 
        image_question_length);
      break;

    case vtkKWIcon::IconReload:
      this->SetImage(
        image_reload, 
        image_reload_width, image_reload_height,
        image_reload_pixel_size, 
        image_reload_length);
      break;

    case vtkKWIcon::IconResetCamera:
      this->SetImage(
        image_reset_camera, 
        image_reset_camera_width, image_reset_camera_height,
        image_reset_camera_pixel_size, 
        image_reset_camera_length);
      break;

    case vtkKWIcon::IconResetContrast:
      this->SetImage(
        image_reset_contrast, 
        image_reset_contrast_width, image_reset_contrast_height,
        image_reset_contrast_pixel_size, 
        image_reset_contrast_length);
      break;

    case vtkKWIcon::IconRotate:
      this->SetImage(
        image_rotate, 
        image_rotate_width, image_rotate_height,
        image_rotate_pixel_size, 
        image_rotate_length);
      break;

    case vtkKWIcon::IconRows:
      this->SetImage(
        image_rows, 
        image_rows_width, image_rows_height,
        image_rows_pixel_size, 
        image_rows_length);
      break;

    case vtkKWIcon::IconScaleBarAnnotation:
      this->SetImage(
        image_scale_bar_annotation, 
        image_scale_bar_annotation_width, image_scale_bar_annotation_height,
        image_scale_bar_annotation_pixel_size, 
        image_scale_bar_annotation_length);
      break;

    case vtkKWIcon::IconSideAnnotation:
      this->SetImage(
        image_side_annotation, 
        image_side_annotation_width, image_side_annotation_height,
        image_side_annotation_pixel_size, 
        image_side_annotation_length);
      break;

    case vtkKWIcon::IconSpinDown:
      this->SetImage(
        image_spin_down, 
        image_spin_down_width, image_spin_down_height,
        image_spin_down_pixel_size, 
        image_spin_down_length);
      break;

    case vtkKWIcon::IconSpinLeft:
      this->SetImage(
        image_spin_left, 
        image_spin_left_width, image_spin_left_height,
        image_spin_left_pixel_size, 
        image_spin_left_length);
      break;

    case vtkKWIcon::IconSpinRight:
      this->SetImage(
        image_spin_right, 
        image_spin_right_width, image_spin_right_height,
        image_spin_right_pixel_size, 
        image_spin_right_length);
      break;

    case vtkKWIcon::IconSpinUp:
      this->SetImage(
        image_spin_up, 
        image_spin_up_width, image_spin_up_height,
        image_spin_up_pixel_size, 
        image_spin_up_length);
      break;

    case vtkKWIcon::IconShrink:
      this->SetImage(
        image_shrink, 
        image_shrink_width, image_shrink_height,
        image_shrink_pixel_size, 
        image_shrink_length);
      break;

    case vtkKWIcon::IconErrorMini:
      this->SetImage(
        image_error_mini, 
        image_error_mini_width, image_error_mini_height,
        image_error_mini_pixel_size, 
        image_error_mini_length);
      break;

    case vtkKWIcon::IconErrorRedMini:
      this->SetImage(
        image_error_red_mini, 
        image_error_red_mini_width, image_error_red_mini_height,
        image_error_red_mini_pixel_size, 
        image_error_red_mini_length);
      break;

    case vtkKWIcon::IconSeedTool:
      this->SetImage(
        image_seed_tool, 
        image_seed_tool_width, image_seed_tool_height,
        image_seed_tool_pixel_size, 
        image_seed_tool_length);
      break;
      
    case vtkKWIcon::IconStopwatch:
      this->SetImage(
        image_stopwatch, 
        image_stopwatch_width, image_stopwatch_height,
        image_stopwatch_pixel_size, 
        image_stopwatch_length);
      break;
      
    case vtkKWIcon::IconStandardView:
      this->SetImage(
        image_standard_view, 
        image_standard_view_width, image_standard_view_height,
        image_standard_view_pixel_size, 
        image_standard_view_length);
      break;
      
    case vtkKWIcon::IconTime:
      this->SetImage(
        image_time, 
        image_time_width, image_time_height,
        image_time_pixel_size, 
        image_time_length);
      break;
      
    case vtkKWIcon::IconTransportBeginning:
      this->SetImage(
        image_transport_beginning, 
        image_transport_beginning_width, image_transport_beginning_height,
        image_transport_beginning_pixel_size, 
        image_transport_beginning_length);
      break;
      
    case vtkKWIcon::IconTransportEnd:
      this->SetImage(
        image_transport_end, 
        image_transport_end_width, image_transport_end_height,
        image_transport_end_pixel_size, 
        image_transport_end_length);
      break;
      
    case vtkKWIcon::IconTransportFastForward:
      this->SetImage(
        image_transport_fast_forward, 
        image_transport_fast_forward_width, 
        image_transport_fast_forward_height,
        image_transport_fast_forward_pixel_size, 
        image_transport_fast_forward_length);
      break;
      
    case vtkKWIcon::IconTransportFastForwardToKey:
      this->SetImage(
        image_transport_fast_forward_to_key, 
        image_transport_fast_forward_to_key_width, 
        image_transport_fast_forward_to_key_height,
        image_transport_fast_forward_to_key_pixel_size, 
        image_transport_fast_forward_to_key_length);
      break;
      
    case vtkKWIcon::IconTransportLoop:
      this->SetImage(
        image_transport_loop, 
        image_transport_loop_width, 
        image_transport_loop_height,
        image_transport_loop_pixel_size, 
        image_transport_loop_length);
      break;
      
    case vtkKWIcon::IconTransportPause:
      this->SetImage(
        image_transport_pause, 
        image_transport_pause_width, 
        image_transport_pause_height,
        image_transport_pause_pixel_size, 
        image_transport_pause_length);
      break;
      
    case vtkKWIcon::IconTransportPlay:
      this->SetImage(
        image_transport_play, 
        image_transport_play_width, 
        image_transport_play_height,
        image_transport_play_pixel_size, 
        image_transport_play_length);
      break;
      
    case vtkKWIcon::IconTransportPlayBackward:
      this->SetImage(
        image_transport_play_backward, 
        image_transport_play_backward_width, 
        image_transport_play_backward_height,
        image_transport_play_backward_pixel_size, 
        image_transport_play_backward_length);
      break;
      
    case vtkKWIcon::IconTransportPlayToKey:
      this->SetImage(
        image_transport_play_to_key, 
        image_transport_play_to_key_width, 
        image_transport_play_to_key_height,
        image_transport_play_to_key_pixel_size, 
        image_transport_play_to_key_length);
      break;
      
    case vtkKWIcon::IconTransportRewind:
      this->SetImage(
        image_transport_rewind, 
        image_transport_rewind_width, 
        image_transport_rewind_height,
        image_transport_rewind_pixel_size, 
        image_transport_rewind_length);
      break;
      
    case vtkKWIcon::IconTransportRewindToKey:
      this->SetImage(
        image_transport_rewind_to_key, 
        image_transport_rewind_to_key_width, 
        image_transport_rewind_to_key_height,
        image_transport_rewind_to_key_pixel_size, 
        image_transport_rewind_to_key_length);
      break;
      
    case vtkKWIcon::IconTransportStop:
      this->SetImage(
        image_transport_stop, 
        image_transport_stop_width, 
        image_transport_stop_height,
        image_transport_stop_pixel_size, 
        image_transport_stop_length);
      break;
      
    case vtkKWIcon::IconTrashcan:
      this->SetImage(
        image_trashcan, 
        image_trashcan_width, image_trashcan_height,
        image_trashcan_pixel_size, 
        image_trashcan_length);
      break;
      
    case vtkKWIcon::IconTreeClose:
      this->SetImage(
        image_tree_close, 
        image_tree_close_width, image_tree_close_height,
        image_tree_close_pixel_size, 
        image_tree_close_length);
      break;
      
    case vtkKWIcon::IconTreeOpen:
      this->SetImage(
        image_tree_open, 
        image_tree_open_width, image_tree_open_height,
        image_tree_open_pixel_size, 
        image_tree_open_length);
      break;
      
    case vtkKWIcon::IconWarning:
      this->SetImage(
        image_warning, 
        image_warning_width, image_warning_height,
        image_warning_pixel_size, 
        image_warning_length);
      break;

    case vtkKWIcon::IconWarningMini:
      this->SetImage(
        image_warning_mini, 
        image_warning_mini_width, image_warning_mini_height,
        image_warning_mini_pixel_size, 
        image_warning_mini_length);
      break;

    case vtkKWIcon::IconWindowLevel:
      this->SetImage(
        image_window_level, 
        image_window_level_width, image_window_level_height,
        image_window_level_pixel_size, 
        image_window_level_length);
      break;

    case vtkKWIcon::IconBrowserUp:
      this->SetImage(
        image_browserup, 
        image_browserup_width, image_browserup_height,
        image_browserup_pixel_size, 
        image_browserup_length);
      break;

    case vtkKWIcon::IconBrowserBack:
      this->SetImage(
        image_browserback, 
        image_browserback_width, image_browserback_height,
        image_browserback_pixel_size, 
        image_browserback_length);
      break;

    case vtkKWIcon::IconBrowserForward:
      this->SetImage(
        image_browserforward, 
        image_browserforward_width, image_browserforward_height,
        image_browserforward_pixel_size, 
        image_browserforward_length);
      break;

    case vtkKWIcon::IconBugMini:
      this->SetImage(
        image_bug_mini, 
        image_bug_mini_width, image_bug_mini_height,
        image_bug_mini_pixel_size, 
        image_bug_mini_length);
      break;

    case vtkKWIcon::IconFileDelete:
      this->SetImage(
        image_filedelete, 
        image_filedelete_width, image_filedelete_height,
        image_filedelete_pixel_size, 
        image_filedelete_length);
      break;

    case vtkKWIcon::IconFolderNew:
      this->SetImage(
        image_foldernew, 
        image_foldernew_width, image_foldernew_height,
        image_foldernew_pixel_size, 
        image_foldernew_length);
      break;

    case vtkKWIcon::IconCdRom:
      this->SetImage(
        image_cdrom, 
        image_cdrom_width, image_cdrom_height,
        image_cdrom_pixel_size, 
        image_cdrom_length);
      break;

    case vtkKWIcon::IconHardDrive:
      this->SetImage(
        image_harddisk, 
        image_harddisk_width, image_harddisk_height,
        image_harddisk_pixel_size, 
        image_harddisk_length);
      break;

    case vtkKWIcon::IconFloppyDrive:
      this->SetImage(
        image_floppydrive, 
        image_floppydrive_width, image_floppydrive_height,
        image_floppydrive_pixel_size, 
        image_floppydrive_length);
      break;

    case vtkKWIcon::IconNetDrive:
      this->SetImage(
        image_netdrive, 
        image_netdrive_width, image_netdrive_height,
        image_netdrive_pixel_size, 
        image_netdrive_length);
      break;

    case vtkKWIcon::IconFavorites:
      this->SetImage(
        image_favorites, 
        image_favorites_width, image_favorites_height,
        image_favorites_pixel_size, 
        image_favorites_length);
      break;

    case vtkKWIcon::IconFolder32:
      this->SetImage(
        image_folder32, 
        image_folder32_width, image_folder32_height,
        image_folder32_pixel_size, 
        image_folder32_length);
      break;

    case vtkKWIcon::IconFolderXP:
      this->SetImage(
        image_folderxp, 
        image_folderxp_width, image_folderxp_height,
        image_folderxp_pixel_size, 
        image_folderxp_length);
      break;

    case vtkKWIcon::IconTestTube:
      this->SetImage(
        image_testtube, 
        image_testtube_width, image_testtube_height,
        image_testtube_pixel_size, 
        image_testtube_length);
      break;

      // -------------------------------------------------------------------

    case IconCrystalProject16x16Actions14LayerDeletelayer:
      this->SetImage(
        image_CrystalProject_16x16_actions_14_layer_deletelayer, 
        image_CrystalProject_16x16_actions_14_layer_deletelayer_width, 
        image_CrystalProject_16x16_actions_14_layer_deletelayer_height,
        image_CrystalProject_16x16_actions_14_layer_deletelayer_pixel_size, 
        image_CrystalProject_16x16_actions_14_layer_deletelayer_length);
      break;

    case IconCrystalProject16x16Actions14LayerNovisible:
      this->SetImage(
        image_CrystalProject_16x16_actions_14_layer_novisible, 
        image_CrystalProject_16x16_actions_14_layer_novisible_width, 
        image_CrystalProject_16x16_actions_14_layer_novisible_height,
        image_CrystalProject_16x16_actions_14_layer_novisible_pixel_size, 
        image_CrystalProject_16x16_actions_14_layer_novisible_length);
      break;

    case IconCrystalProject16x16Actions14LayerVisible:
      this->SetImage(
        image_CrystalProject_16x16_actions_14_layer_visible, 
        image_CrystalProject_16x16_actions_14_layer_visible_width, 
        image_CrystalProject_16x16_actions_14_layer_visible_height,
        image_CrystalProject_16x16_actions_14_layer_visible_pixel_size, 
        image_CrystalProject_16x16_actions_14_layer_visible_length);
      break;

    case IconCrystalProject16x16ActionsJabberOnline:
      this->SetImage(
        image_CrystalProject_16x16_actions_jabber_online, 
        image_CrystalProject_16x16_actions_jabber_online_width, 
        image_CrystalProject_16x16_actions_jabber_online_height,
        image_CrystalProject_16x16_actions_jabber_online_pixel_size, 
        image_CrystalProject_16x16_actions_jabber_online_length);
      break;

    case IconCrystalProject16x16ActionsRotate:
      this->SetImage(
        image_CrystalProject_16x16_actions_rotate, 
        image_CrystalProject_16x16_actions_rotate_width, 
        image_CrystalProject_16x16_actions_rotate_height,
        image_CrystalProject_16x16_actions_rotate_pixel_size, 
        image_CrystalProject_16x16_actions_rotate_length);
      break;

    case IconCrystalProject16x16AppsCookie:
      this->SetImage(
        image_CrystalProject_16x16_apps_cookie, 
        image_CrystalProject_16x16_apps_cookie_width, 
        image_CrystalProject_16x16_apps_cookie_height,
        image_CrystalProject_16x16_apps_cookie_pixel_size, 
        image_CrystalProject_16x16_apps_cookie_length);
      break;

    case IconCrystalProject16x16AppsTutorials:
      this->SetImage(
        image_CrystalProject_16x16_apps_tutorials, 
        image_CrystalProject_16x16_apps_tutorials_width, 
        image_CrystalProject_16x16_apps_tutorials_height,
        image_CrystalProject_16x16_apps_tutorials_pixel_size, 
        image_CrystalProject_16x16_apps_tutorials_length);
      break;

      // -------------------------------------------------------------------

    case IconNuvola16x16ActionsConfigure:
      this->SetImage(
        image_Nuvola_16x16_actions_configure, 
        image_Nuvola_16x16_actions_configure_width, 
        image_Nuvola_16x16_actions_configure_height,
        image_Nuvola_16x16_actions_configure_pixel_size, 
        image_Nuvola_16x16_actions_configure_length);
      break;

    case IconNuvola16x16ActionsDecrypted:
      this->SetImage(
        image_Nuvola_16x16_actions_decrypted, 
        image_Nuvola_16x16_actions_decrypted_width, 
        image_Nuvola_16x16_actions_decrypted_height,
        image_Nuvola_16x16_actions_decrypted_pixel_size, 
        image_Nuvola_16x16_actions_decrypted_length);
      break;

    case IconNuvola16x16ActionsEncrypted:
      this->SetImage(
        image_Nuvola_16x16_actions_encrypted, 
        image_Nuvola_16x16_actions_encrypted_width, 
        image_Nuvola_16x16_actions_encrypted_height,
        image_Nuvola_16x16_actions_encrypted_pixel_size, 
        image_Nuvola_16x16_actions_encrypted_length);
      break;

    case IconNuvola16x16ActionsExit:
      this->SetImage(
        image_Nuvola_16x16_actions_exit, 
        image_Nuvola_16x16_actions_exit_width, 
        image_Nuvola_16x16_actions_exit_height,
        image_Nuvola_16x16_actions_exit_pixel_size, 
        image_Nuvola_16x16_actions_exit_length);
      break;

    case IconNuvola16x16ActionsFileClose:
      this->SetImage(
        image_Nuvola_16x16_actions_fileclose, 
        image_Nuvola_16x16_actions_fileclose_width, 
        image_Nuvola_16x16_actions_fileclose_height,
        image_Nuvola_16x16_actions_fileclose_pixel_size, 
        image_Nuvola_16x16_actions_fileclose_length);
      break;

    case IconNuvola16x16ActionsFileOpen:
      this->SetImage(
        image_Nuvola_16x16_actions_fileopen, 
        image_Nuvola_16x16_actions_fileopen_width, 
        image_Nuvola_16x16_actions_fileopen_height,
        image_Nuvola_16x16_actions_fileopen_pixel_size, 
        image_Nuvola_16x16_actions_fileopen_length);
      break;

    case IconNuvola16x16ActionsFilePrint:
      this->SetImage(
        image_Nuvola_16x16_actions_fileprint, 
        image_Nuvola_16x16_actions_fileprint_width, 
        image_Nuvola_16x16_actions_fileprint_height,
        image_Nuvola_16x16_actions_fileprint_pixel_size, 
        image_Nuvola_16x16_actions_fileprint_length);
      break;

    case IconNuvola16x16ActionsFileSave:
      this->SetImage(
        image_Nuvola_16x16_actions_filesave, 
        image_Nuvola_16x16_actions_filesave_width, 
        image_Nuvola_16x16_actions_filesave_height,
        image_Nuvola_16x16_actions_filesave_pixel_size, 
        image_Nuvola_16x16_actions_filesave_length);
      break;

    case IconNuvola16x16ActionsHelp:
      this->SetImage(
        image_Nuvola_16x16_actions_help, 
        image_Nuvola_16x16_actions_help_width, 
        image_Nuvola_16x16_actions_help_height,
        image_Nuvola_16x16_actions_help_pixel_size, 
        image_Nuvola_16x16_actions_help_length);
      break;

    case IconNuvola16x16ActionsMailGeneric:
      this->SetImage(
        image_Nuvola_16x16_actions_mail_generic, 
        image_Nuvola_16x16_actions_mail_generic_width, 
        image_Nuvola_16x16_actions_mail_generic_height,
        image_Nuvola_16x16_actions_mail_generic_pixel_size, 
        image_Nuvola_16x16_actions_mail_generic_length);
      break;

    case IconNuvola16x16ActionsMessage:
      this->SetImage(
        image_Nuvola_16x16_actions_message, 
        image_Nuvola_16x16_actions_message_width, 
        image_Nuvola_16x16_actions_message_height,
        image_Nuvola_16x16_actions_message_pixel_size, 
        image_Nuvola_16x16_actions_message_length);
      break;

    case IconNuvola16x16ActionsMessageBoxCritical:
      this->SetImage(
        image_Nuvola_16x16_actions_messagebox_critical, 
        image_Nuvola_16x16_actions_messagebox_critical_width, 
        image_Nuvola_16x16_actions_messagebox_critical_height,
        image_Nuvola_16x16_actions_messagebox_critical_pixel_size, 
        image_Nuvola_16x16_actions_messagebox_critical_length);
      break;

    case IconNuvola16x16ActionsMessageBoxInfo:
      this->SetImage(
        image_Nuvola_16x16_actions_messagebox_info, 
        image_Nuvola_16x16_actions_messagebox_info_width, 
        image_Nuvola_16x16_actions_messagebox_info_height,
        image_Nuvola_16x16_actions_messagebox_info_pixel_size, 
        image_Nuvola_16x16_actions_messagebox_info_length);
      break;

    case IconNuvola16x16ActionsMessageBoxWarning:
      this->SetImage(
        image_Nuvola_16x16_actions_messagebox_warning, 
        image_Nuvola_16x16_actions_messagebox_warning_width, 
        image_Nuvola_16x16_actions_messagebox_warning_height,
        image_Nuvola_16x16_actions_messagebox_warning_pixel_size, 
        image_Nuvola_16x16_actions_messagebox_warning_length);
      break;

    case IconNuvola16x16ActionsNo:
      this->SetImage(
        image_Nuvola_16x16_actions_no, 
        image_Nuvola_16x16_actions_no_width, 
        image_Nuvola_16x16_actions_no_height,
        image_Nuvola_16x16_actions_no_pixel_size, 
        image_Nuvola_16x16_actions_no_length);
      break;

    case IconNuvola16x16ActionsRotate:
      this->SetImage(
        image_Nuvola_16x16_actions_rotate, 
        image_Nuvola_16x16_actions_rotate_width, 
        image_Nuvola_16x16_actions_rotate_height,
        image_Nuvola_16x16_actions_rotate_pixel_size, 
        image_Nuvola_16x16_actions_rotate_length);
      break;

    case IconNuvola16x16ActionsStop:
      this->SetImage(
        image_Nuvola_16x16_actions_stop, 
        image_Nuvola_16x16_actions_stop_width, 
        image_Nuvola_16x16_actions_stop_height,
        image_Nuvola_16x16_actions_stop_pixel_size, 
        image_Nuvola_16x16_actions_stop_length);
      break;

    case IconNuvola16x16ActionsThumbnail:
      this->SetImage(
        image_Nuvola_16x16_actions_thumbnail, 
        image_Nuvola_16x16_actions_thumbnail_width, 
        image_Nuvola_16x16_actions_thumbnail_height,
        image_Nuvola_16x16_actions_thumbnail_pixel_size, 
        image_Nuvola_16x16_actions_thumbnail_length);
      break;

    case IconNuvola16x16ActionsViewMag:
      this->SetImage(
        image_Nuvola_16x16_actions_viewmag, 
        image_Nuvola_16x16_actions_viewmag_width, 
        image_Nuvola_16x16_actions_viewmag_height,
        image_Nuvola_16x16_actions_viewmag_pixel_size, 
        image_Nuvola_16x16_actions_viewmag_length);
      break;

    case IconNuvola16x16ActionsViewBottom:
      this->SetImage(
        image_Nuvola_16x16_actions_view_bottom, 
        image_Nuvola_16x16_actions_view_bottom_width, 
        image_Nuvola_16x16_actions_view_bottom_height,
        image_Nuvola_16x16_actions_view_bottom_pixel_size, 
        image_Nuvola_16x16_actions_view_bottom_length);
      break;

    case IconNuvola16x16ActionsViewFullScreen:
      this->SetImage(
        image_Nuvola_16x16_actions_view_fullscreen, 
        image_Nuvola_16x16_actions_view_fullscreen_width, 
        image_Nuvola_16x16_actions_view_fullscreen_height,
        image_Nuvola_16x16_actions_view_fullscreen_pixel_size, 
        image_Nuvola_16x16_actions_view_fullscreen_length);
      break;

    case IconNuvola16x16ActionsViewRight:
      this->SetImage(
        image_Nuvola_16x16_actions_view_right, 
        image_Nuvola_16x16_actions_view_right_width, 
        image_Nuvola_16x16_actions_view_right_height,
        image_Nuvola_16x16_actions_view_right_pixel_size, 
        image_Nuvola_16x16_actions_view_right_length);
      break;

    case IconNuvola16x16ActionsColorPicker:
      this->SetImage(
        image_Nuvola_16x16_actions_colorpicker, 
        image_Nuvola_16x16_actions_colorpicker_width, 
        image_Nuvola_16x16_actions_colorpicker_height,
        image_Nuvola_16x16_actions_colorpicker_pixel_size, 
        image_Nuvola_16x16_actions_colorpicker_length);
      break;

    case IconNuvola16x16ActionsEditCopy:
      this->SetImage(
        image_Nuvola_16x16_actions_editcopy, 
        image_Nuvola_16x16_actions_editcopy_width, 
        image_Nuvola_16x16_actions_editcopy_height,
        image_Nuvola_16x16_actions_editcopy_pixel_size, 
        image_Nuvola_16x16_actions_editcopy_length);
      break;

    case IconNuvola16x16ActionsEditCut:
      this->SetImage(
        image_Nuvola_16x16_actions_editcut, 
        image_Nuvola_16x16_actions_editcut_width, 
        image_Nuvola_16x16_actions_editcut_height,
        image_Nuvola_16x16_actions_editcut_pixel_size, 
        image_Nuvola_16x16_actions_editcut_length);
      break;

    case IconNuvola16x16ActionsEditPaste:
      this->SetImage(
        image_Nuvola_16x16_actions_editpaste, 
        image_Nuvola_16x16_actions_editpaste_width, 
        image_Nuvola_16x16_actions_editpaste_height,
        image_Nuvola_16x16_actions_editpaste_pixel_size, 
        image_Nuvola_16x16_actions_editpaste_length);
      break;

    case IconNuvola16x16ActionsHistory:
      this->SetImage(
        image_Nuvola_16x16_actions_history, 
        image_Nuvola_16x16_actions_history_width, 
        image_Nuvola_16x16_actions_history_height,
        image_Nuvola_16x16_actions_history_pixel_size, 
        image_Nuvola_16x16_actions_history_length);
      break;

    case IconNuvola16x16ActionsKAlarm:
      this->SetImage(
        image_Nuvola_16x16_actions_kalarm, 
        image_Nuvola_16x16_actions_kalarm_width, 
        image_Nuvola_16x16_actions_kalarm_height,
        image_Nuvola_16x16_actions_kalarm_pixel_size, 
        image_Nuvola_16x16_actions_kalarm_length);
      break;

    case IconNuvola16x16ActionsLedBlue:
      this->SetImage(
        image_Nuvola_16x16_actions_ledblue, 
        image_Nuvola_16x16_actions_ledblue_width, 
        image_Nuvola_16x16_actions_ledblue_height,
        image_Nuvola_16x16_actions_ledblue_pixel_size, 
        image_Nuvola_16x16_actions_ledblue_length);
      break;

    case IconNuvola16x16ActionsLedGreen:
      this->SetImage(
        image_Nuvola_16x16_actions_ledgreen, 
        image_Nuvola_16x16_actions_ledgreen_width, 
        image_Nuvola_16x16_actions_ledgreen_height,
        image_Nuvola_16x16_actions_ledgreen_pixel_size, 
        image_Nuvola_16x16_actions_ledgreen_length);
      break;

    case IconNuvola16x16ActionsLedLightBlue:
      this->SetImage(
        image_Nuvola_16x16_actions_ledlightblue, 
        image_Nuvola_16x16_actions_ledlightblue_width, 
        image_Nuvola_16x16_actions_ledlightblue_height,
        image_Nuvola_16x16_actions_ledlightblue_pixel_size, 
        image_Nuvola_16x16_actions_ledlightblue_length);
      break;

    case IconNuvola16x16ActionsLedLightGreen:
      this->SetImage(
        image_Nuvola_16x16_actions_ledlightgreen, 
        image_Nuvola_16x16_actions_ledlightgreen_width, 
        image_Nuvola_16x16_actions_ledlightgreen_height,
        image_Nuvola_16x16_actions_ledlightgreen_pixel_size, 
        image_Nuvola_16x16_actions_ledlightgreen_length);
      break;

    case IconNuvola16x16ActionsLedOrange:
      this->SetImage(
        image_Nuvola_16x16_actions_ledorange, 
        image_Nuvola_16x16_actions_ledorange_width, 
        image_Nuvola_16x16_actions_ledorange_height,
        image_Nuvola_16x16_actions_ledorange_pixel_size, 
        image_Nuvola_16x16_actions_ledorange_length);
      break;

    case IconNuvola16x16ActionsLedPurple:
      this->SetImage(
        image_Nuvola_16x16_actions_ledpurple, 
        image_Nuvola_16x16_actions_ledpurple_width, 
        image_Nuvola_16x16_actions_ledpurple_height,
        image_Nuvola_16x16_actions_ledpurple_pixel_size, 
        image_Nuvola_16x16_actions_ledpurple_length);
      break;

    case IconNuvola16x16ActionsLedRed:
      this->SetImage(
        image_Nuvola_16x16_actions_ledred, 
        image_Nuvola_16x16_actions_ledred_width, 
        image_Nuvola_16x16_actions_ledred_height,
        image_Nuvola_16x16_actions_ledred_pixel_size, 
        image_Nuvola_16x16_actions_ledred_length);
      break;

    case IconNuvola16x16ActionsLedYellow:
      this->SetImage(
        image_Nuvola_16x16_actions_ledyellow, 
        image_Nuvola_16x16_actions_ledyellow_width, 
        image_Nuvola_16x16_actions_ledyellow_height,
        image_Nuvola_16x16_actions_ledyellow_pixel_size, 
        image_Nuvola_16x16_actions_ledyellow_length);
      break;

    case IconNuvola16x16ActionsViewLeftRight:
      this->SetImage(
        image_Nuvola_16x16_actions_view_left_right, 
        image_Nuvola_16x16_actions_view_left_right_width, 
        image_Nuvola_16x16_actions_view_left_right_height,
        image_Nuvola_16x16_actions_view_left_right_pixel_size, 
        image_Nuvola_16x16_actions_view_left_right_length);
      break;

    case IconNuvola16x16ActionsViewTopBottom:
      this->SetImage(
        image_Nuvola_16x16_actions_view_top_bottom, 
        image_Nuvola_16x16_actions_view_top_bottom_width, 
        image_Nuvola_16x16_actions_view_top_bottom_height,
        image_Nuvola_16x16_actions_view_top_bottom_pixel_size, 
        image_Nuvola_16x16_actions_view_top_bottom_length);
      break;

    case IconNuvola16x16ActionsWindowNew:
      this->SetImage(
        image_Nuvola_16x16_actions_window_new, 
        image_Nuvola_16x16_actions_window_new_width, 
        image_Nuvola_16x16_actions_window_new_height,
        image_Nuvola_16x16_actions_window_new_pixel_size, 
        image_Nuvola_16x16_actions_window_new_length);
      break;

    case IconNuvola16x16AppsArk:
      this->SetImage(
        image_Nuvola_16x16_apps_ark, 
        image_Nuvola_16x16_apps_ark_width, 
        image_Nuvola_16x16_apps_ark_height,
        image_Nuvola_16x16_apps_ark_pixel_size, 
        image_Nuvola_16x16_apps_ark_length);
      break;

    case IconNuvola16x16AppsBug:
      this->SetImage(
        image_Nuvola_16x16_apps_bug, 
        image_Nuvola_16x16_apps_bug_width, 
        image_Nuvola_16x16_apps_bug_height,
        image_Nuvola_16x16_apps_bug_pixel_size, 
        image_Nuvola_16x16_apps_bug_length);
      break;

    case IconNuvola16x16AppsEmail:
      this->SetImage(
        image_Nuvola_16x16_apps_email, 
        image_Nuvola_16x16_apps_email_width, 
        image_Nuvola_16x16_apps_email_height,
        image_Nuvola_16x16_apps_email_pixel_size, 
        image_Nuvola_16x16_apps_email_length);
      break;

    case IconNuvola16x16AppsError:
      this->SetImage(
        image_Nuvola_16x16_apps_error, 
        image_Nuvola_16x16_apps_error_width, 
        image_Nuvola_16x16_apps_error_height,
        image_Nuvola_16x16_apps_error_pixel_size, 
        image_Nuvola_16x16_apps_error_length);
      break;

    case IconNuvola16x16AppsKeyboard:
      this->SetImage(
        image_Nuvola_16x16_apps_keyboard, 
        image_Nuvola_16x16_apps_keyboard_width, 
        image_Nuvola_16x16_apps_keyboard_height,
        image_Nuvola_16x16_apps_keyboard_pixel_size, 
        image_Nuvola_16x16_apps_keyboard_length);
      break;

    case IconNuvola16x16AppsKTimer:
      this->SetImage(
        image_Nuvola_16x16_apps_ktimer, 
        image_Nuvola_16x16_apps_ktimer_width, 
        image_Nuvola_16x16_apps_ktimer_height,
        image_Nuvola_16x16_apps_ktimer_pixel_size, 
        image_Nuvola_16x16_apps_ktimer_length);
      break;

    case IconNuvola16x16DevicesCamera:
      this->SetImage(
        image_Nuvola_16x16_devices_camera, 
        image_Nuvola_16x16_devices_camera_width, 
        image_Nuvola_16x16_devices_camera_height,
        image_Nuvola_16x16_devices_camera_pixel_size, 
        image_Nuvola_16x16_devices_camera_length);
      break;

    case IconNuvola16x16FilesystemsFolderInbox:
      this->SetImage(
        image_Nuvola_16x16_filesystems_folder_inbox, 
        image_Nuvola_16x16_filesystems_folder_inbox_width, 
        image_Nuvola_16x16_filesystems_folder_inbox_height,
        image_Nuvola_16x16_filesystems_folder_inbox_pixel_size, 
        image_Nuvola_16x16_filesystems_folder_inbox_length);
      break;

    case IconNuvola16x16FilesystemsFolderOutbox:
      this->SetImage(
        image_Nuvola_16x16_filesystems_folder_outbox, 
        image_Nuvola_16x16_filesystems_folder_outbox_width, 
        image_Nuvola_16x16_filesystems_folder_outbox_height,
        image_Nuvola_16x16_filesystems_folder_outbox_pixel_size, 
        image_Nuvola_16x16_filesystems_folder_outbox_length);
      break;

    case IconNuvola16x16FilesystemsTrashcanEmpty:
      this->SetImage(
        image_Nuvola_16x16_filesystems_trashcan_empty, 
        image_Nuvola_16x16_filesystems_trashcan_empty_width, 
        image_Nuvola_16x16_filesystems_trashcan_empty_height,
        image_Nuvola_16x16_filesystems_trashcan_empty_pixel_size, 
        image_Nuvola_16x16_filesystems_trashcan_empty_length);
      break;

    case IconNuvola16x16FilesystemsTrashcanFull:
      this->SetImage(
        image_Nuvola_16x16_filesystems_trashcan_full, 
        image_Nuvola_16x16_filesystems_trashcan_full_width, 
        image_Nuvola_16x16_filesystems_trashcan_full_height,
        image_Nuvola_16x16_filesystems_trashcan_full_pixel_size, 
        image_Nuvola_16x16_filesystems_trashcan_full_length);
      break;

      // -------------------------------------------------------------------

    case IconNuvola22x22ActionsHelp:
      this->SetImage(
        image_Nuvola_22x22_actions_help, 
        image_Nuvola_22x22_actions_help_width, 
        image_Nuvola_22x22_actions_help_height,
        image_Nuvola_22x22_actions_help_pixel_size, 
        image_Nuvola_22x22_actions_help_length);
      break;

    case IconNuvola22x22ActionsMessageBoxInfo:
      this->SetImage(
        image_Nuvola_22x22_actions_messagebox_info, 
        image_Nuvola_22x22_actions_messagebox_info_width, 
        image_Nuvola_22x22_actions_messagebox_info_height,
        image_Nuvola_22x22_actions_messagebox_info_pixel_size, 
        image_Nuvola_22x22_actions_messagebox_info_length);
      break;

    case IconNuvola22x22ActionsMisc:
      this->SetImage(
        image_Nuvola_22x22_actions_misc, 
        image_Nuvola_22x22_actions_misc_width, 
        image_Nuvola_22x22_actions_misc_height,
        image_Nuvola_22x22_actions_misc_pixel_size, 
        image_Nuvola_22x22_actions_misc_length);
      break;

    case IconNuvola22x22ActionsWizard:
      this->SetImage(
        image_Nuvola_22x22_actions_wizard, 
        image_Nuvola_22x22_actions_wizard_width, 
        image_Nuvola_22x22_actions_wizard_height,
        image_Nuvola_22x22_actions_wizard_pixel_size, 
        image_Nuvola_22x22_actions_wizard_length);
      break;

    case IconNuvola22x22AppsDesigner:
      this->SetImage(
        image_Nuvola_22x22_apps_designer, 
        image_Nuvola_22x22_apps_designer_width, 
        image_Nuvola_22x22_apps_designer_height,
        image_Nuvola_22x22_apps_designer_pixel_size, 
        image_Nuvola_22x22_apps_designer_length);
      break;

    case IconNuvola22x22DevicesCamera:
      this->SetImage(
        image_Nuvola_22x22_devices_camera, 
        image_Nuvola_22x22_devices_camera_width, 
        image_Nuvola_22x22_devices_camera_height,
        image_Nuvola_22x22_devices_camera_pixel_size, 
        image_Nuvola_22x22_devices_camera_length);
      break;

    case IconNuvola22x22FilesystemsFolderBlue:
      this->SetImage(
        image_Nuvola_22x22_filesystems_folder_blue, 
        image_Nuvola_22x22_filesystems_folder_blue_width, 
        image_Nuvola_22x22_filesystems_folder_blue_height,
        image_Nuvola_22x22_filesystems_folder_blue_pixel_size, 
        image_Nuvola_22x22_filesystems_folder_blue_length);
      break;

      // -------------------------------------------------------------------

    case IconNuvola48x48ActionsMessageBoxCritical:
      this->SetImage(
        image_Nuvola_48x48_actions_messagebox_critical, 
        image_Nuvola_48x48_actions_messagebox_critical_width, 
        image_Nuvola_48x48_actions_messagebox_critical_height,
        image_Nuvola_48x48_actions_messagebox_critical_pixel_size, 
        image_Nuvola_48x48_actions_messagebox_critical_length);
      break;

    case IconNuvola48x48ActionsMessageBoxInfo:
      this->SetImage(
        image_Nuvola_48x48_actions_messagebox_info, 
        image_Nuvola_48x48_actions_messagebox_info_width, 
        image_Nuvola_48x48_actions_messagebox_info_height,
        image_Nuvola_48x48_actions_messagebox_info_pixel_size, 
        image_Nuvola_48x48_actions_messagebox_info_length);
      break;

    case IconNuvola48x48ActionsMessageBoxWarning:
      this->SetImage(
        image_Nuvola_48x48_actions_messagebox_warning, 
        image_Nuvola_48x48_actions_messagebox_warning_width, 
        image_Nuvola_48x48_actions_messagebox_warning_height,
        image_Nuvola_48x48_actions_messagebox_warning_pixel_size, 
        image_Nuvola_48x48_actions_messagebox_warning_length);
      break;

    case vtkKWIcon::IconNuvola48x48ActionsHelp:
      this->SetImage(
        image_Nuvola_48x48_actions_help, 
        image_Nuvola_48x48_actions_help_width, 
        image_Nuvola_48x48_actions_help_height,
        image_Nuvola_48x48_actions_help_pixel_size, 
        image_Nuvola_48x48_actions_help_length);
      break;

    case vtkKWIcon::IconNuvola48x48ActionsHistory:
      this->SetImage(
        image_Nuvola_48x48_actions_history, 
        image_Nuvola_48x48_actions_history_width, 
        image_Nuvola_48x48_actions_history_height,
        image_Nuvola_48x48_actions_history_pixel_size, 
        image_Nuvola_48x48_actions_history_length);
      break;

    case vtkKWIcon::IconNuvola48x48AppsDownloadManager:
      this->SetImage(
        image_Nuvola_48x48_apps_download_manager, 
        image_Nuvola_48x48_apps_download_manager_width, 
        image_Nuvola_48x48_apps_download_manager_height,
        image_Nuvola_48x48_apps_download_manager_pixel_size, 
        image_Nuvola_48x48_apps_download_manager_length);
      break;

    case vtkKWIcon::IconNuvola48x48DevicesMouse:
      this->SetImage(
        image_Nuvola_48x48_devices_mouse, 
        image_Nuvola_48x48_devices_mouse_width, 
        image_Nuvola_48x48_devices_mouse_height,
        image_Nuvola_48x48_devices_mouse_pixel_size, 
        image_Nuvola_48x48_devices_mouse_length);
      break;

    case IconNuvola48x48FilesystemsFolderBlue:
      this->SetImage(
        image_Nuvola_48x48_filesystems_folder_blue, 
        image_Nuvola_48x48_filesystems_folder_blue_width, 
        image_Nuvola_48x48_filesystems_folder_blue_height,
        image_Nuvola_48x48_filesystems_folder_blue_pixel_size, 
        image_Nuvola_48x48_filesystems_folder_blue_length);
      break;

      // -------------------------------------------------------------------

    case IconSilkApplicationXpTerminal:
      this->SetImage(
        image_Silk_application_xp_terminal, 
        image_Silk_application_xp_terminal_width, image_Silk_application_xp_terminal_height,
        image_Silk_application_xp_terminal_pixel_size, 
        image_Silk_application_xp_terminal_length);
      break;

    case IconSilkArrowRefresh:
      this->SetImage(
        image_Silk_arrow_refresh, 
        image_Silk_arrow_refresh_width, image_Silk_arrow_refresh_height,
        image_Silk_arrow_refresh_pixel_size, 
        image_Silk_arrow_refresh_length);
      break;

    case IconSilkAttach:
      this->SetImage(
        image_Silk_attach, 
        image_Silk_attach_width, image_Silk_attach_height,
        image_Silk_attach_pixel_size, 
        image_Silk_attach_length);
      break;

    case IconSilkBinClosed:
      this->SetImage(
        image_Silk_bin_closed, 
        image_Silk_bin_closed_width, image_Silk_bin_closed_height,
        image_Silk_bin_closed_pixel_size, 
        image_Silk_bin_closed_length);
      break;

    case IconSilkBug:
      this->SetImage(
        image_Silk_bug, 
        image_Silk_bug_width, image_Silk_bug_height,
        image_Silk_bug_pixel_size, 
        image_Silk_bug_length);
      break;

    case IconSilkBulletToggleMinus:
      this->SetImage(
        image_Silk_bullet_toggle_minus, 
        image_Silk_bullet_toggle_minus_width, image_Silk_bullet_toggle_minus_height,
        image_Silk_bullet_toggle_minus_pixel_size, 
        image_Silk_bullet_toggle_minus_length);
      break;

    case IconSilkBulletTogglePlus:
      this->SetImage(
        image_Silk_bullet_toggle_plus, 
        image_Silk_bullet_toggle_plus_width, image_Silk_bullet_toggle_plus_height,
        image_Silk_bullet_toggle_plus_pixel_size, 
        image_Silk_bullet_toggle_plus_length);
      break;

    case IconSilkCamera:
      this->SetImage(
        image_Silk_camera, 
        image_Silk_camera_width, image_Silk_camera_height,
        image_Silk_camera_pixel_size, 
        image_Silk_camera_length);
      break;

    case IconSilkChartCurve:
      this->SetImage(
        image_Silk_chart_curve, 
        image_Silk_chart_curve_width, image_Silk_chart_curve_height,
        image_Silk_chart_curve_pixel_size, 
        image_Silk_chart_curve_length);
      break;

    case IconSilkChartLine:
      this->SetImage(
        image_Silk_chart_line, 
        image_Silk_chart_line_width, image_Silk_chart_line_height,
        image_Silk_chart_line_pixel_size, 
        image_Silk_chart_line_length);
      break;

    case IconSilkChartOrganisation:
      this->SetImage(
        image_Silk_chart_organisation, 
        image_Silk_chart_organisation_width, image_Silk_chart_organisation_height,
        image_Silk_chart_organisation_pixel_size, 
        image_Silk_chart_organisation_length);
      break;

    case IconSilkChartPie:
      this->SetImage(
        image_Silk_chart_pie, 
        image_Silk_chart_pie_width, image_Silk_chart_pie_height,
        image_Silk_chart_pie_pixel_size, 
        image_Silk_chart_pie_length);
      break;

    case IconSilkClock:
      this->SetImage(
        image_Silk_clock, 
        image_Silk_clock_width, image_Silk_clock_height,
        image_Silk_clock_pixel_size, 
        image_Silk_clock_length);
      break;

    case IconSilkCog:
      this->SetImage(
        image_Silk_cog, 
        image_Silk_cog_width, image_Silk_cog_height,
        image_Silk_cog_pixel_size, 
        image_Silk_cog_length);
      break;

    case IconSilkColorSwatch:
      this->SetImage(
        image_Silk_color_swatch, 
        image_Silk_color_swatch_width, image_Silk_color_swatch_height,
        image_Silk_color_swatch_pixel_size, 
        image_Silk_color_swatch_length);
      break;

    case IconSilkColorWheel:
      this->SetImage(
        image_Silk_color_wheel, 
        image_Silk_color_wheel_width, image_Silk_color_wheel_height,
        image_Silk_color_wheel_pixel_size, 
        image_Silk_color_wheel_length);
      break;

    case IconSilkCompress:
      this->SetImage(
        image_Silk_compress, 
        image_Silk_compress_width, image_Silk_compress_height,
        image_Silk_compress_pixel_size, 
        image_Silk_compress_length);
      break;

    case IconSilkDate:
      this->SetImage(
        image_Silk_date, 
        image_Silk_date_width, image_Silk_date_height,
        image_Silk_date_pixel_size, 
        image_Silk_date_length);
      break;

    case IconSilkDisk:
      this->SetImage(
        image_Silk_disk, 
        image_Silk_disk_width, image_Silk_disk_height,
        image_Silk_disk_pixel_size, 
        image_Silk_disk_length);
      break;

    case IconSilkDoorIn:
      this->SetImage(
        image_Silk_door_in, 
        image_Silk_door_in_width, image_Silk_door_in_height,
        image_Silk_door_in_pixel_size, 
        image_Silk_door_in_length);
      break;

    case IconSilkDoorOut:
      this->SetImage(
        image_Silk_door_out, 
        image_Silk_door_out_width, image_Silk_door_out_height,
        image_Silk_door_out_pixel_size, 
        image_Silk_door_out_length);
      break;

    case IconSilkEmail:
      this->SetImage(
        image_Silk_email, 
        image_Silk_email_width, image_Silk_email_height,
        image_Silk_email_pixel_size, 
        image_Silk_email_length);
      break;

    case IconSilkExclamation:
      this->SetImage(
        image_Silk_exclamation, 
        image_Silk_exclamation_width, image_Silk_exclamation_height,
        image_Silk_exclamation_pixel_size, 
        image_Silk_exclamation_length);
      break;

    case IconSilkEye:
      this->SetImage(
        image_Silk_eye, 
        image_Silk_eye_width, image_Silk_eye_height,
        image_Silk_eye_pixel_size, 
        image_Silk_eye_length);
      break;

    case IconSilkFilm:
      this->SetImage(
        image_Silk_film, 
        image_Silk_film_width, image_Silk_film_height,
        image_Silk_film_pixel_size, 
        image_Silk_film_length);
      break;

    case IconSilkHelp:
      this->SetImage(
        image_Silk_help, 
        image_Silk_help_width, image_Silk_help_height,
        image_Silk_help_pixel_size, 
        image_Silk_help_length);
      break;

    case IconSilkHourglass:
      this->SetImage(
        image_Silk_hourglass, 
        image_Silk_hourglass_width, image_Silk_hourglass_height,
        image_Silk_hourglass_pixel_size, 
        image_Silk_hourglass_length);
      break;

    case IconSilkInformation:
      this->SetImage(
        image_Silk_information, 
        image_Silk_information_width, image_Silk_information_height,
        image_Silk_information_pixel_size, 
        image_Silk_information_length);
      break;

    case IconSilkKey:
      this->SetImage(
        image_Silk_key, 
        image_Silk_key_width, image_Silk_key_height,
        image_Silk_key_pixel_size, 
        image_Silk_key_length);
      break;

    case IconSilkLink:
      this->SetImage(
        image_Silk_link, 
        image_Silk_link_width, image_Silk_link_height,
        image_Silk_link_pixel_size, 
        image_Silk_link_length);
      break;

    case IconSilkLock:
      this->SetImage(
        image_Silk_lock, 
        image_Silk_lock_width, image_Silk_lock_height,
        image_Silk_lock_pixel_size, 
        image_Silk_lock_length);
      break;

    case IconSilkMagnifier:
      this->SetImage(
        image_Silk_magnifier, 
        image_Silk_magnifier_width, image_Silk_magnifier_height,
        image_Silk_magnifier_pixel_size, 
        image_Silk_magnifier_length);
      break;

    case IconSilkPaintbrush:
      this->SetImage(
        image_Silk_paintbrush, 
        image_Silk_paintbrush_width, image_Silk_paintbrush_height,
        image_Silk_paintbrush_pixel_size, 
        image_Silk_paintbrush_length);
      break;

    case IconSilkPaintCan:
      this->SetImage(
        image_Silk_paintcan, 
        image_Silk_paintcan_width, image_Silk_paintcan_height,
        image_Silk_paintcan_pixel_size, 
        image_Silk_paintcan_length);
      break;

    case IconSilkPill:
      this->SetImage(
        image_Silk_pill, 
        image_Silk_pill_width, image_Silk_pill_height,
        image_Silk_pill_pixel_size, 
        image_Silk_pill_length);
      break;

    case IconSilkPlugin:
      this->SetImage(
        image_Silk_plugin, 
        image_Silk_plugin_width, image_Silk_plugin_height,
        image_Silk_plugin_pixel_size, 
        image_Silk_plugin_length);
      break;

    case IconSilkStar:
      this->SetImage(
        image_Silk_star, 
        image_Silk_star_width, image_Silk_star_height,
        image_Silk_star_pixel_size, 
        image_Silk_star_length);
      break;

    case IconSilkStop:
      this->SetImage(
        image_Silk_stop, 
        image_Silk_stop_width, image_Silk_stop_height,
        image_Silk_stop_pixel_size, 
        image_Silk_stop_length);
      break;

    case IconSilkThumbDown:
      this->SetImage(
        image_Silk_thumb_down, 
        image_Silk_thumb_down_width, image_Silk_thumb_down_height,
        image_Silk_thumb_down_pixel_size, 
        image_Silk_thumb_down_length);
      break;

    case IconSilkThumbUp:
      this->SetImage(
        image_Silk_thumb_up, 
        image_Silk_thumb_up_width, image_Silk_thumb_up_height,
        image_Silk_thumb_up_pixel_size, 
        image_Silk_thumb_up_length);
      break;

    case IconSilkTick:
      this->SetImage(
        image_Silk_tick, 
        image_Silk_tick_width, image_Silk_tick_height,
        image_Silk_tick_pixel_size, 
        image_Silk_tick_length);
      break;

    case IconSilkTime:
      this->SetImage(
        image_Silk_time, 
        image_Silk_time_width, image_Silk_time_height,
        image_Silk_time_pixel_size, 
        image_Silk_time_length);
      break;

    case IconSilkWrench:
      this->SetImage(
        image_Silk_wrench, 
        image_Silk_wrench_width, image_Silk_wrench_height,
        image_Silk_wrench_pixel_size, 
        image_Silk_wrench_length);
      break;

    case IconSilkZoom:
      this->SetImage(
        image_Silk_zoom, 
        image_Silk_zoom_width, image_Silk_zoom_height,
        image_Silk_zoom_pixel_size, 
        image_Silk_zoom_length);
      break;

    case IconSilkZoomIn:
      this->SetImage(
        image_Silk_zoom_in, 
        image_Silk_zoom_in_width, image_Silk_zoom_in_height,
        image_Silk_zoom_in_pixel_size, 
        image_Silk_zoom_in_length);
      break;

    case IconSilkZoomOut:
      this->SetImage(
        image_Silk_zoom_out, 
        image_Silk_zoom_out_width, image_Silk_zoom_out_height,
        image_Silk_zoom_out_pixel_size, 
        image_Silk_zoom_out_length);
      break;

    case IconSilkAccept:
      this->SetImage(
        image_Silk_accept, 
        image_Silk_accept_width, image_Silk_accept_height,
        image_Silk_accept_pixel_size, 
        image_Silk_accept_length);
      break;

    case IconSilkAdd:
      this->SetImage(
        image_Silk_add, 
        image_Silk_add_width, image_Silk_add_height,
        image_Silk_add_pixel_size, 
        image_Silk_add_length);
      break;

    case IconSilkCancel:
      this->SetImage(
        image_Silk_cancel, 
        image_Silk_cancel_width, image_Silk_cancel_height,
        image_Silk_cancel_pixel_size, 
        image_Silk_cancel_length);
      break;

    case IconSilkCross:
      this->SetImage(
        image_Silk_cross, 
        image_Silk_cross_width, image_Silk_cross_height,
        image_Silk_cross_pixel_size, 
        image_Silk_cross_length);
      break;

    case IconSilkCut:
      this->SetImage(
        image_Silk_cut, 
        image_Silk_cut_width, image_Silk_cut_height,
        image_Silk_cut_pixel_size, 
        image_Silk_cut_length);
      break;

    case IconSilkDelete:
      this->SetImage(
        image_Silk_delete, 
        image_Silk_delete_width, image_Silk_delete_height,
        image_Silk_delete_pixel_size, 
        image_Silk_delete_length);
      break;

    case IconSilkPastePlain:
      this->SetImage(
        image_Silk_paste_plain, 
        image_Silk_paste_plain_width, image_Silk_paste_plain_height,
        image_Silk_paste_plain_pixel_size, 
        image_Silk_paste_plain_length);
      break;

    case IconSilkTelephone:
      this->SetImage(
        image_Silk_telephone, 
        image_Silk_telephone_width, image_Silk_telephone_height,
        image_Silk_telephone_pixel_size, 
        image_Silk_telephone_length);
      break;

    case IconSilkTextAllCaps:
      this->SetImage(
        image_Silk_text_allcaps, 
        image_Silk_text_allcaps_width, image_Silk_text_allcaps_height,
        image_Silk_text_allcaps_pixel_size, 
        image_Silk_text_allcaps_length);
      break;

    case IconSilkTextBold:
      this->SetImage(
        image_Silk_text_bold, 
        image_Silk_text_bold_width, image_Silk_text_bold_height,
        image_Silk_text_bold_pixel_size, 
        image_Silk_text_bold_length);
      break;

    case IconSilkTextItalic:
      this->SetImage(
        image_Silk_text_italic, 
        image_Silk_text_italic_width, image_Silk_text_italic_height,
        image_Silk_text_italic_pixel_size, 
        image_Silk_text_italic_length);
      break;

    case IconSilkTextUnderline:
      this->SetImage(
        image_Silk_text_underline, 
        image_Silk_text_underline_width, image_Silk_text_underline_height,
        image_Silk_text_underline_pixel_size, 
        image_Silk_text_underline_length);
      break;

    }
}

//----------------------------------------------------------------------------
const unsigned char* vtkKWIcon::GetData()
{
  return this->Data;
}

//----------------------------------------------------------------------------
void vtkKWIcon::Fade(double factor)
{
  if (!this->Data || 
      this->Width == 0 || 
      this->Height == 0 || 
      this->PixelSize != 4)
    {
    return;
    }

  unsigned long data_length = this->Width * this->Height * this->PixelSize;
  unsigned char *data_ptr = this->Data;
  const unsigned char *data_ptr_end = this->Data + data_length;

  data_ptr += 3;
  while (data_ptr < data_ptr_end)
    {
    *data_ptr = (unsigned char)((double)(*data_ptr) * factor);
    data_ptr += this->PixelSize;
    }
}

//----------------------------------------------------------------------------
void vtkKWIcon::SetHue(double hue)
{
  if (!this->Data || 
      this->Width == 0 || this->Height == 0 || 
      this->PixelSize < 3)
    {
    return;
    }

  unsigned long data_length = this->Width * this->Height * this->PixelSize;
  unsigned char *data_ptr = this->Data;
  const unsigned char *data_ptr_end = this->Data + data_length;

  double h, s, v, r, g, b;

  while (data_ptr < data_ptr_end)
    {
    r = (double)data_ptr[0] / 255.0;
    g = (double)data_ptr[1] / 255.0;
    b = (double)data_ptr[2] / 255.0;
    vtkMath::RGBToHSV(r, g, b, &h, &s, &v);
    h = hue;
    vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
    data_ptr[0] = (int)(r * 255.0);
    data_ptr[1] = (int)(g * 255.0);
    data_ptr[2] = (int)(b * 255.0);
    data_ptr += this->PixelSize;
    }
}

//----------------------------------------------------------------------------
void vtkKWIcon::Flatten(double r, double g, double b)
{
  if (!this->Data || 
      this->Width == 0 || 
      this->Height == 0 || 
      this->PixelSize != 4)
    {
    return;
    }

  unsigned long data_length = this->Width * this->Height * this->PixelSize;
  unsigned char *data_ptr = this->Data;
  const unsigned char *data_ptr_end = this->Data + data_length;

  unsigned long new_data_length = this->Width * this->Height * 3;
  unsigned char *new_data = new unsigned char [new_data_length];
  unsigned char *new_data_ptr = new_data;

  unsigned char rc = (unsigned char)(r * 255.0);
  unsigned char gc = (unsigned char)(g * 255.0);
  unsigned char bc = (unsigned char)(b * 255.0);

  while (data_ptr < data_ptr_end)
    {
    double alpha = static_cast<double>(*(data_ptr + 3)) / 255.0;
    *new_data_ptr++ = 
      static_cast<unsigned char>(rc * (1 - alpha) + *data_ptr++ * alpha);
    *new_data_ptr++ = 
      static_cast<unsigned char>(gc * (1 - alpha) + *data_ptr++ * alpha);
    *new_data_ptr++ = 
      static_cast<unsigned char>(bc * (1 - alpha) + *data_ptr++ * alpha);
    data_ptr++;
    }

  this->PixelSize = 3;
  delete [] this->Data;
  this->Data = new_data;
}

//----------------------------------------------------------------------------
int vtkKWIcon::Compose(vtkKWIcon *icon)
{
  if (!icon || 
      icon->GetWidth() != this->GetWidth() ||
      icon->GetHeight() != this->GetHeight() ||
      icon->GetPixelSize() != this->GetPixelSize() ||
      this->GetPixelSize() != 4)
    {
    vtkErrorMacro("Can not compose against a dissimilar icon!");
    return 0;
    }

  int width = this->GetWidth();
  int height = this->GetHeight();
  int pixel_size = this->GetPixelSize();
  size_t buffer_size = (size_t)width * (size_t)height * (size_t)pixel_size;

  const unsigned char* img_ptr = this->GetData();
  const unsigned char* img_ptr_end = img_ptr + buffer_size;
  const unsigned char* icon_img_ptr = icon->GetData();

  unsigned char* blended_img_ptr = new unsigned char [buffer_size];
  unsigned char* ptr = blended_img_ptr;

  while (img_ptr < img_ptr_end)
    {
    int icon_img_alpha_char = static_cast<int>(*(icon_img_ptr + 3));
    double icon_img_alpha = static_cast<double>(icon_img_alpha_char) / 255.0;

    *ptr++ = static_cast<unsigned char>
      (*img_ptr++ * (1 - icon_img_alpha) + *icon_img_ptr++ * icon_img_alpha);
    *ptr++ = static_cast<unsigned char>
      (*img_ptr++ * (1 - icon_img_alpha) + *icon_img_ptr++ * icon_img_alpha);
    *ptr++ = static_cast<unsigned char>
      (*img_ptr++ * (1 - icon_img_alpha) + *icon_img_ptr++ * icon_img_alpha);

    icon_img_alpha_char += *img_ptr++;
    icon_img_ptr++;

    *ptr++ = static_cast<unsigned char>
      (icon_img_alpha_char > 255 ? 255 : icon_img_alpha_char); 
    }

  delete [] this->Data;
  this->Data = blended_img_ptr;

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWIcon::Compose(int icon_index)
{
  vtkKWIcon *icon = vtkKWIcon::New();
  icon->SetImage(icon_index);
  int res = this->Compose(icon);
  icon->Delete();
  return res;
}

//----------------------------------------------------------------------------
int vtkKWIcon::TrimTop()
{
  if (this->GetPixelSize() != 4)
    {
    vtkErrorMacro("Can not trim if not RGBA!");
    return 0;
    }

  int width = this->GetWidth();
  int height = this->GetHeight();
  int pixel_size = this->GetPixelSize();
  size_t row_size = (size_t)width * pixel_size;
  size_t buffer_size = (size_t)width * (size_t)height * (size_t)pixel_size;

  const unsigned char *ptr_start = this->GetData();
  const unsigned char *ptr_end = ptr_start + buffer_size;
  const unsigned char *ptr = ptr_start;

  while (ptr < ptr_end)
    {
    // Find if there is a non-transparent pixel in that row
    const unsigned char *row_ptr = ptr;
    const unsigned char *row_ptr_end = row_ptr + row_size;
    row_ptr += 3;
    while (row_ptr < row_ptr_end)
      {
      if (*row_ptr) // Found one
        {
        // Shift everything up, set the bottom part to 0
        size_t preserve_size = ptr_end - ptr;
        memmove((void*)ptr_start, (void*)ptr, preserve_size);
        memset((void*)(ptr_start+preserve_size), 0, buffer_size-preserve_size);
        return 1;
        }
      row_ptr += pixel_size;
      }
    // Next row...
    ptr = row_ptr_end;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWIcon::TrimRight()
{
  if (this->GetPixelSize() != 4)
    {
    vtkErrorMacro("Can not trim if not RGBA!");
    return 0;
    }

  int width = this->GetWidth();
  int height = this->GetHeight();
  int pixel_size = this->GetPixelSize();
  size_t row_size = (size_t)width * pixel_size;
  size_t buffer_size = (size_t)width * (size_t)height * (size_t)pixel_size;

  const unsigned char *ptr_start = this->GetData();
  const unsigned char *ptr_end = ptr_start + buffer_size;
  const unsigned char *last_col_ptr = ptr_start + row_size - pixel_size;
  const unsigned char *ptr = last_col_ptr;

  while (ptr >= ptr_start)
    {
    // Find if there is a non-transparent pixel in that column
    const unsigned char *col_ptr = ptr;
    col_ptr += 3;
    while (col_ptr < ptr_end)
      {
      if (*col_ptr)
        {
        // Shift everything to the right, set the left part to 0
        size_t empty_size = last_col_ptr - ptr;
        size_t preserve_size = row_size - empty_size;
        ptr = ptr_start;
        while (ptr < ptr_end)
          {
          memmove((void*)(ptr + empty_size),(void*)ptr, preserve_size);
          memset((void*)ptr, 0, empty_size);
          ptr += row_size;
          }
        return 1;
        }
      col_ptr += row_size;
      }
    ptr -= pixel_size;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWIcon::SetImageToGradient(vtkColorTransferFunction *ctf, 
                                   int width, int height,
                                   int options)
{
  if (!ctf || width < 4 || height < 4)
    {
    return 0;
    }

  int draw_w_border = 
    (options & vtkKWIcon::ImageOptionDrawDoubleBorder) ? 1 : 0;
  int draw_b_border = 
    (draw_w_border || options & vtkKWIcon::ImageOptionDrawBorder) ? 1 : 0;
  int draw_vertically = (options & vtkKWIcon::ImageOptionDrawVertically);

  // Get a ramp (leave 2 pixels for the black frame border if needed)

  int ramp_width = width - draw_b_border * 2 - draw_w_border * 2;
  int bytes_in_ramp = ramp_width * 3;
  const unsigned char *rgb_table = 
    ctf->GetTable(ctf->GetRange()[0], ctf->GetRange()[1], ramp_width);

  // Allocate a buffer for the whole image

  int bytes_in_row = width * 3;
  int buffer_length = height * bytes_in_row;

  unsigned char *buffer = new unsigned char [buffer_length];
  unsigned char *ptr = buffer;

  // Fill the first row for the border if needed, it will be used for the
  // last one too

  unsigned char *first_row = ptr;
  if (draw_b_border)
    {
    memset(first_row, 0, bytes_in_row);
    ptr += bytes_in_row;
    }

  // Fill the second row for the 2nd border if needed, it will be used for the
  // last - 1 one too

  unsigned char *second_row = ptr;
  if (draw_w_border)
    {
    memset(ptr, 0, 3);
    ptr += 3;
    memset(ptr, 255, 3 + bytes_in_ramp + 3);
    ptr += 3 + bytes_in_ramp + 3;
    memset(ptr, 0, 3);
    ptr += 3;
    }

  // Now draw the ramp, depending on the direction

  if (draw_vertically)
    {
    int j;
    for (j = draw_b_border + draw_w_border; 
         j < height - draw_b_border - draw_w_border; 
         j++)
      {
      if (draw_b_border)
        {
        memset(ptr, 0, 3);
        ptr += 3;
        }
      if (draw_w_border)
        {
        memset(ptr, 255, 3);
        ptr += 3;
        }
      unsigned char *ptr_end = ptr + bytes_in_ramp;
      while (ptr < ptr_end)
        {
        memcpy(ptr, rgb_table, 3);
        ptr += 3;
        }
      if (draw_w_border)
        {
        memset(ptr, 255, 3);
        ptr += 3;
        }
      if (draw_b_border)
        {
        memset(ptr, 0, 3);
        ptr += 3;
        }
      rgb_table += 3;
      }
    }
  else
    {
    // Fill the ramp row, it will be used for all
    // others row

    unsigned char *ramp_row = ptr;
    if (draw_b_border)
      {
      memset(ptr, 0, 3);
      ptr += 3;
      }
    if (draw_w_border)
      {
      memset(ptr, 255, 3);
      ptr += 3;
      }

    memcpy(ptr, rgb_table, bytes_in_ramp);
    ptr += bytes_in_ramp;
    
    if (draw_w_border)
      {
      memset(ptr, 255, 3);
      ptr += 3;
      }
    if (draw_b_border)
      {
      memset(ptr, 0, 3);
      ptr += 3;
      }

    // Fill all remaining rows, using the ramp row
    
    int j;
    for (j = 1 + draw_b_border + draw_w_border; 
         j < height - draw_b_border - draw_w_border; 
         j++)
      {
      memcpy(ptr, ramp_row, bytes_in_row);
      ptr += bytes_in_row;
      }
    }

  // Fill the last rows (border) using the first ones

  if (draw_w_border)
    {
    memcpy(ptr, second_row, bytes_in_row);
    ptr += bytes_in_row;
    }

  if (draw_b_border)
    {
    memcpy(ptr, first_row, bytes_in_row);
    }

  this->Width  = width;
  this->Height = height;
  this->PixelSize = 3;
  delete [] this->Data;
  this->Data = buffer;

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWIcon::SetImageToRGBGradient(double r1, double g1, double b1, 
                                     double r2, double g2, double b2, 
                                     int width, int height,
                                     int options)
{
  vtkColorTransferFunction *ctf = vtkColorTransferFunction::New();
  ctf->SetColorSpaceToRGB();
  ctf->AddRGBPoint(0.0, r1, g1, b1);
  ctf->AddRGBPoint(1.0, r2, g2, b2);
  int res = this->SetImageToGradient(ctf, width, height, options);
  ctf->Delete();
  return res;
}

//----------------------------------------------------------------------------
int vtkKWIcon::SetImageToSolidRGBColor(double r, double g, double b, 
                                       int width, int height,
                                       int options)
{
  vtkColorTransferFunction *ctf = vtkColorTransferFunction::New();
  ctf->SetColorSpaceToRGB();
  ctf->AddRGBPoint(0.0, r, g, b);
  ctf->AddRGBPoint(1.0, r, g, b);
  int res = this->SetImageToGradient(ctf, width, height, options);
  ctf->Delete();
  return res;
}

//----------------------------------------------------------------------------
int vtkKWIcon::ResizeCanvas(
  int resized_width, int resized_height, int position)
{
  if (resized_width < 1 || resized_height < 1 || this->PixelSize < 3)
    {
    return 0;
    }

  // Allocate a buffer for resized image

  int resized_pixel_size = 4;
  size_t resized_bytes_in_row = 
    (size_t)resized_width * (size_t)resized_pixel_size;
  size_t resized_buffer_length = (size_t)resized_height * resized_bytes_in_row;
  unsigned char *resized_buffer = new unsigned char [resized_buffer_length];
  memset((void*)(resized_buffer), 0, resized_buffer_length);
  
  // Compute the position of the current image in the resized image

  int resized_x, resized_y;

  int intersection_x = 0;
  int intersection_y = 0;
  int intersection_width = this->Width;
  int intersection_height = this->Height;

  if (position & vtkKWIcon::PositionInCanvasEast)
    {
    if (resized_width < this->Width)
      {
      intersection_x = this->Width - resized_width;
      intersection_width = resized_width;
      resized_x = 0;
      }
    else
      {
      resized_x = resized_width - this->Width;
      }
    }
  else
    {
    if (position & vtkKWIcon::PositionInCanvasWest)
      {
      resized_x = 0;
      }
    else
      {
      if (resized_width < this->Width)
        {
        int half = (this->Width - resized_width) >> 1;
        intersection_x = half;
        intersection_width -= half;
        resized_x = 0;
        }
      else
        {
        resized_x = (resized_width - this->Width) >> 1;
        }
      }
    if (resized_x + intersection_width > resized_width)
      {
      intersection_width = resized_width - resized_x;
      }
    }

  if (position & vtkKWIcon::PositionInCanvasSouth)
    {
    if (resized_height < this->Height)
      {
      intersection_y = this->Height - resized_height;
      intersection_height = resized_height;
      resized_y = 0;
      }
    else
      {
      resized_y = resized_height - this->Height;
      }
    }
  else
    {
    if (position & vtkKWIcon::PositionInCanvasNorth)
      {
      resized_y = 0;
      }
    else
      {
      if (resized_height < this->Height)
        {
        int half = (this->Height - resized_height) >> 1;
        intersection_y = half;
        intersection_height -= half;
        resized_y = 0;
        }
      else
        {
        resized_y = (resized_height - this->Height) >> 1;
        }
      }
    if (resized_y + intersection_height > resized_height)
      {
      intersection_height = resized_height - resized_y;
      }
    }

  // Copy icon inside buffer

  unsigned char *resized_ptr = resized_buffer + 
    resized_y * resized_bytes_in_row + resized_x * resized_pixel_size;

  size_t intersection_bytes_in_row = 
    (size_t)intersection_width * (size_t)this->PixelSize;

  size_t bytes_in_row = (size_t)this->Width * (size_t)this->PixelSize;
  size_t next_row = bytes_in_row - intersection_bytes_in_row;

  size_t resized_next_row = resized_bytes_in_row - 
    (size_t)intersection_width * resized_pixel_size;

  unsigned char *ptr = this->Data + intersection_y * bytes_in_row + 
    intersection_x * this->PixelSize;
  while (intersection_height)
    {
    unsigned char *ptr_end = ptr + intersection_bytes_in_row;
    if (this->PixelSize == resized_pixel_size)
      {
      while (ptr < ptr_end)
        {
        *resized_ptr++ = *ptr++; // RGBA
        }
      ptr += next_row;
      resized_ptr += resized_next_row;
      --intersection_height;
      }
    else
      {
      while (ptr < ptr_end)
        {
        *resized_ptr++ = *ptr++; // R
        *resized_ptr++ = *ptr++; // G
        *resized_ptr++ = *ptr++; // B
        *resized_ptr++ = 0xFF;   // A
        }
      ptr += next_row;
      resized_ptr += resized_next_row;
      --intersection_height;
      }
    }

  this->Width  = resized_width;
  this->Height = resized_height;
  this->PixelSize = resized_pixel_size;
  delete [] this->Data;
  this->Data = resized_buffer;

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWIcon::ResampleCanvas(int resampled_width, int resampled_height)
{
  if (resampled_width < 0 || resampled_height < 0 || 
      (resampled_width == 0 && resampled_height == 0))
    {
    return 0;
    }

  if (resampled_width == this->Width &&
      resampled_height == this->Height)
    {
    return 1;
    }

  double factor_width, factor_height;

  if (resampled_width)
    {
    factor_width = (double)resampled_width / (double)this->Width;
    }
  if (resampled_height)
    {
    factor_height = (double)resampled_height / (double)this->Height;
    }
  if (!resampled_width)
    {
    factor_width = factor_height;
    }
  if (!resampled_height)
    {
    factor_height = factor_width;
    }

  vtkImageData *input = vtkImageData::New();
  input->SetDimensions(this->Width, this->Height, 1);
  input->SetScalarTypeToUnsignedChar();
  input->SetNumberOfScalarComponents(this->PixelSize);

  vtkUnsignedCharArray *array = vtkUnsignedCharArray::New();
  array->SetNumberOfComponents(this->PixelSize);
  array->SetArray(this->Data, this->Width * this->Height * this->PixelSize, 1);
  input->GetPointData()->SetScalars(array);

  vtkImageResample *resample = vtkImageResample::New();
  resample->SetInput(input);
  resample->SetInterpolationModeToCubic();
  resample->SetDimensionality(2);

  resample->SetAxisMagnificationFactor(0, factor_width);
  resample->SetAxisMagnificationFactor(1, factor_height);
  resample->UpdateWholeExtent();

  vtkImageData *resample_output = resample->GetOutput();
  int resample_output_dims[3];
  resample_output->GetDimensions(resample_output_dims);

  this->SetImage(
    (const unsigned char*)resample_output->GetScalarPointer(),
    resample_output_dims[0],
    resample_output_dims[1],
    resample_output->GetNumberOfScalarComponents());

  resample->Delete();
  input->Delete();
  array->Delete();

  return 1;
}

//----------------------------------------------------------------------------
int vtkKWIcon::FitCanvas(int fit_width, int fit_height)
{
  if (fit_width <= 0 || fit_height <= 0)
    {
    return 0;
    }

  if (this->Width > this->Height)
    {
    return this->ResampleCanvas(fit_width, 0);
    }
  return this->ResampleCanvas(0, fit_height);
}

//----------------------------------------------------------------------------
void vtkKWIcon::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Width:  " << this->GetWidth() << endl
     << indent << "Height: " << this->GetHeight() << endl
     << indent << "PixelSize: " << this->GetPixelSize() << endl;
}



