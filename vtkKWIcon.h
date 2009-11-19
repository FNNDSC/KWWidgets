/*=========================================================================

  Module:    $RCSfile: vtkKWIcon.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWIcon - simple wrapper for icons
// .SECTION Description
// A simple icon wrapper. It can either be used with file KWIcons.h to 
// provide a unified interface for internal icons or a wrapper for 
// custom icons. The icons are defined with width, height, pixel_size, 
// and array of unsigned char values.

#ifndef __vtkKWIcon_h
#define __vtkKWIcon_h

#include "vtkObject.h"
#include "vtkKWWidgets.h" // Needed for export symbols directives

class vtkColorTransferFunction;
class vtkImageData;

class KWWidgets_EXPORT vtkKWIcon : public vtkObject
{
public:
  static vtkKWIcon* New();
  vtkTypeRevisionMacro(vtkKWIcon,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  void DeepCopy(vtkKWIcon *p);

  //BTX
  // Description:
  // Predefined icons.
  enum { 
    IconAngleTool                 = 14,
    IconBiDimensionalTool         = 34,
    IconBoundingBox               = 13,
    IconBrowserBack               = 71,
    IconBrowserForward            = 72,
    IconBrowserUp                 = 70,
    IconBugMini                   = 200,
    IconCalculator                = 79,
    IconCamera                    = 19,
    IconCameraMini                = 175,
    IconCdRom                     = 75,
    IconColorAlphaSquares         = 216,
    IconColorBarAnnotation        = 12,
    IconColorSquares              = 18,
    IconColumns                   = 220,
    IconCompress                  = 211,
    IconConnection                = 1,
    IconContourSegment            = 33,
    IconContourTool               = 16,
    IconContrast                  = 212,
    IconCornerAnnotation          = 11,
    IconCropTool                  = 22,
    IconDistanceTool              = 15,
    IconDocument                  = 9,
    IconDocumentRGBColor          = 209,
    IconDocumentVolumeProperty    = 177,
    IconDocumentWindowLevel       = 176,
    IconEditContrast              = 225,
    IconEditVolumeProperties      = 226,
    IconEmpty16x16                = 6,
    IconEmpty1x1                  = 10,
    IconError                     = 2, 
    IconErrorMini                 = 3,
    IconErrorRedMini              = 4,
    IconExpand                    = 5,
    IconExpandLeftMini            = 180,
    IconExpandMini                = 23,
    IconExpandRightMini           = 178,
    IconExpandUpMini              = 179,
    IconEye                       = 17,
    IconFavorites                 = 67,
    IconFileDelete                = 73,
    IconFileOpen                  = 8,
    IconFloppy                    = 7,
    IconFloppyDrive               = 77,
    IconFolder                    = 20,
    IconFolder32                  = 65,
    IconFolderNew                 = 74,
    IconFolderOpen                = 21,
    IconFolderXP                  = 66,
    IconGrayscaleSquares          = 218,
    IconGridLinear                = 40,
    IconGridLog                   = 41,
    IconHSVDiagram                = 207,
    IconHardDrive                 = 76,
    IconHeaderAnnotation          = 50,
    IconHelpBubble                = 60,
    IconInfoMini                  = 61,
    IconInterpolationBilinear     = 215,
    IconInterpolationNearest      = 217,
    IconLock                      = 62,
    IconMagGlass                  = 63,
    IconMinus                     = 64,
    IconMove                      = 80,
    IconMoveH                     = 81,
    IconMoveV                     = 82,
    IconNetDrive                  = 78,
    IconNoIcon                    = 0,
    IconObliqueProbe              = 133,
    IconOrientationCubeAnnotation = 83,
    IconPanHand                   = 84,
    IconParallelProjection        = 213,
    IconPerspectiveProjection     = 214,
    IconPlus                      = 100,
    IconPointFinger               = 90,
    IconPresetAdd                 = 94,
    IconPresetApply               = 95,
    IconPresetDelete              = 96,
    IconPresetEmail               = 97,
    IconPresetFilter              = 237,
    IconPresetLoad                = 210,
    IconPresetLocate              = 98,
    IconPresetNext                = 173,
    IconPresetPrevious            = 174,
    IconPresetUpdate              = 99,
    IconQuestion                  = 101,
    IconReload                    = 102,
    IconResetCamera               = 222,
    IconResetContrast             = 223,
    IconRotate                    = 105,
    IconRows                      = 219,
    IconScaleBarAnnotation        = 106,
    IconSeedTool                  = 107,
    IconShrink                    = 103,
    IconSideAnnotation            = 110,
    IconSmallCounterBlue1         = 227,
    IconSmallCounterBlue2         = 228,
    IconSmallCounterBlue3         = 229,
    IconSmallCounterBlue4         = 230,
    IconSmallCounterBlue5         = 231,
    IconSmallCounterBlue6         = 232,
    IconSmallCounterBlue7         = 233,
    IconSmallCounterBlue8         = 234,
    IconSmallCounterBlue9         = 235,
    IconSmallCounterBlue9Plus     = 236,
    IconSpinDown                  = 111,
    IconSpinLeft                  = 112,
    IconSpinRight                 = 113,
    IconSpinUp                    = 114,
    IconStandardView              = 224,
    IconStopwatch                 = 104,
    IconTestTube                  = 190, 
    IconTime                      = 208,
    IconTransportBeginning        = 120,
    IconTransportEnd              = 121,
    IconTransportFastForward      = 122,
    IconTransportFastForwardToKey = 123,
    IconTransportLoop             = 124,
    IconTransportPause            = 125,
    IconTransportPlay             = 126,
    IconTransportPlayBackward     = 201,
    IconTransportPlayToKey        = 127,
    IconTransportRewind           = 128,
    IconTransportRewindToKey      = 129,
    IconTransportStop             = 130,
    IconTrashcan                  = 131,
    IconTreeClose                 = 150,
    IconTreeOpen                  = 151,
    IconWarning                   = 170,
    IconWarningMini               = 171,
    IconWindowLevel               = 172,

    IconNuvola16x16ActionsColorPicker        = 1022,
    IconNuvola16x16ActionsConfigure          = 1000,
    IconNuvola16x16ActionsDecrypted          = 1001,
    IconNuvola16x16ActionsEditCopy           = 1023,
    IconNuvola16x16ActionsEditCut            = 1024,
    IconNuvola16x16ActionsEditPaste          = 1025,
    IconNuvola16x16ActionsEncrypted          = 1019,
    IconNuvola16x16ActionsExit               = 1002,
    IconNuvola16x16ActionsFileClose          = 1003,
    IconNuvola16x16ActionsFileOpen           = 1004,
    IconNuvola16x16ActionsFilePrint          = 1005,
    IconNuvola16x16ActionsFileSave           = 1006,
    IconNuvola16x16ActionsHelp               = 1007,
    IconNuvola16x16ActionsHistory            = 1026,
    IconNuvola16x16ActionsKAlarm             = 1027,
    IconNuvola16x16ActionsLedBlue            = 1031,
    IconNuvola16x16ActionsLedGreen           = 1032,
    IconNuvola16x16ActionsLedLightBlue       = 1033,
    IconNuvola16x16ActionsLedLightGreen      = 1034,
    IconNuvola16x16ActionsLedOrange          = 1035,
    IconNuvola16x16ActionsLedPurple          = 1036,
    IconNuvola16x16ActionsLedRed             = 1037,
    IconNuvola16x16ActionsLedYellow          = 1038,
    IconNuvola16x16ActionsMailGeneric        = 1008,
    IconNuvola16x16ActionsMessage            = 1009,
    IconNuvola16x16ActionsMessageBoxCritical = 1010,
    IconNuvola16x16ActionsMessageBoxInfo     = 1011,
    IconNuvola16x16ActionsMessageBoxWarning  = 1012,
    IconNuvola16x16ActionsNo                 = 1013,
    IconNuvola16x16ActionsRotate             = 1014,
    IconNuvola16x16ActionsStop               = 1015,
    IconNuvola16x16ActionsThumbnail          = 1016,
    IconNuvola16x16ActionsViewBottom         = 1018,
    IconNuvola16x16ActionsViewFullScreen     = 1020,
    IconNuvola16x16ActionsViewLeftRight      = 1028,
    IconNuvola16x16ActionsViewMag            = 1017,
    IconNuvola16x16ActionsViewRight          = 1021,
    IconNuvola16x16ActionsViewTopBottom      = 1029,
    IconNuvola16x16ActionsWindowNew          = 1030,
    IconNuvola16x16AppsArk                   = 1300,
    IconNuvola16x16AppsBug                   = 1301,
    IconNuvola16x16AppsEmail                 = 1302,
    IconNuvola16x16AppsError                 = 1303,
    IconNuvola16x16AppsKeyboard              = 1304,
    IconNuvola16x16AppsKTimer                = 1305,
    IconNuvola16x16DevicesCamera             = 1600,
    IconNuvola16x16FilesystemsFolderInbox    = 1903,
    IconNuvola16x16FilesystemsFolderOutbox   = 1904,
    IconNuvola16x16FilesystemsTrashcanEmpty  = 1901,
    IconNuvola16x16FilesystemsTrashcanFull   = 1902,

    IconNuvola22x22ActionsHelp               = 3000,
    IconNuvola22x22ActionsMessageBoxInfo     = 3001,
    IconNuvola22x22ActionsMisc               = 3002,
    IconNuvola22x22ActionsWizard             = 3003,
    IconNuvola22x22AppsDesigner              = 3300,
    IconNuvola22x22DevicesCamera             = 3600,
    IconNuvola22x22FilesystemsFolderBlue     = 3900,

    IconNuvola48x48ActionsMessageBoxCritical = 5001,
    IconNuvola48x48ActionsMessageBoxInfo     = 5002,
    IconNuvola48x48ActionsMessageBoxWarning  = 5003,
    IconNuvola48x48ActionsHistory            = 5000,
    IconNuvola48x48AppsDownloadManager       = 5300,
    IconNuvola48x48DevicesMouse              = 5600,
    IconNuvola48x48FilesystemsFolderBlue     = 5900,

    IconCrystalProject16x16Actions14LayerDeletelayer = 10000,
    IconCrystalProject16x16Actions14LayerNovisible   = 10001,
    IconCrystalProject16x16Actions14LayerVisible     = 10002,
    IconCrystalProject16x16ActionsJabberOnline       = 10003,
    IconCrystalProject16x16ActionsRotate             = 10004,
    IconCrystalProject16x16AppsCookie                = 10300,
    IconCrystalProject16x16AppsTutorials             = 10301,

    IconSilkAccept                = 20044,
    IconSilkAdd                   = 20045,
    IconSilkApplicationXpTerminal = 20000,
    IconSilkArrowRefresh          = 20001,
    IconSilkAttach                = 20002,
    IconSilkBinClosed             = 20003,
    IconSilkBug                   = 20004,
    IconSilkBulletToggleMinus     = 20056,
    IconSilkBulletTogglePlus      = 20057, //
    IconSilkCamera                = 20005,
    IconSilkCancel                = 20046,
    IconSilkChartCurve            = 20006,
    IconSilkChartLine             = 20007,
    IconSilkChartOrganisation     = 20008,
    IconSilkChartPie              = 20009,
    IconSilkClock                 = 20010,
    IconSilkCog                   = 20011,
    IconSilkColorSwatch           = 20012,
    IconSilkColorWheel            = 20013,
    IconSilkCompress              = 20014,
    IconSilkCross                 = 20047,
    IconSilkCut                   = 20048,
    IconSilkDate                  = 20015,
    IconSilkDelete                = 20049,
    IconSilkDisk                  = 20016,
    IconSilkDoorIn                = 20017,
    IconSilkDoorOut               = 20018,
    IconSilkEmail                 = 20019,
    IconSilkExclamation           = 20020,
    IconSilkEye                   = 20021,
    IconSilkFilm                  = 20022,
    IconSilkHelp                  = 20023,
    IconSilkHourglass             = 20024,
    IconSilkInformation           = 20025,
    IconSilkKey                   = 20026,
    IconSilkLink                  = 20027,
    IconSilkLock                  = 20028,
    IconSilkMagnifier             = 20029,
    IconSilkPaintCan              = 20031,
    IconSilkPaintbrush            = 20030,
    IconSilkPastePlain            = 20050,
    IconSilkPill                  = 20032,
    IconSilkPlugin                = 20033,
    IconSilkStar                  = 20034,
    IconSilkStop                  = 20035,
    IconSilkTelephone             = 20051,
    IconSilkTextAllCaps           = 20052,
    IconSilkTextBold              = 20053,
    IconSilkTextItalic            = 20054,
    IconSilkTextUnderline         = 20055,
    IconSilkThumbDown             = 20036,
    IconSilkThumbUp               = 20037,
    IconSilkTick                  = 20038,
    IconSilkTime                  = 20039,
    IconSilkWrench                = 20040,
    IconSilkZoom                  = 20041,
    IconSilkZoomIn                = 20042,
    IconSilkZoomOut               = 20043,

    LastIcon
  };
  //ETX

  // Description:
  // Set the icon image based on a predefined icon.
  virtual void SetImage(int predefined_icon_index);

  // Description:
  // Set the icon image based on an existing vtkKWIcon.
  virtual void SetImage(vtkKWIcon*);

  // Description:
  // Set the icon image based on an existing vtkImageData.
  // A XZ, YZ slice will be permuted into a XY slice automatically.
  virtual void SetImage(vtkImageData*);

  // Description:
  // Set the icon image from pixel data, eventually zlib and base64.
  // If 'buffer_length' is 0, compute it automatically by multiplying
  // 'pixel_size', 'width' and 'height' together.
  // If ImageOptionFlipVertical is set in 'option', flip the image vertically
  //BTX
  enum 
  { 
    ImageOptionFlipVertical = 1
  };
  //ETX
  void SetImage(const unsigned char* data, 
                int width, int height, 
                int pixel_size, 
                unsigned long buffer_length = 0,
                int options = 0);

  // Description:
  // Set the icon image to a color transfer function gradient, or a 2-colors
  // gradients, or a single color. Borders can be added automatically.
  // Return 1 on success, 0 otherwise
  //BTX
  enum 
  { 
    ImageOptionDrawBorder = 1,
    ImageOptionDrawDoubleBorder = 2,
    ImageOptionDrawVertically = 4
  };
  //ETX
  virtual int SetImageToGradient(vtkColorTransferFunction *ctf, 
                                  int width, int height,
                                  int options = 0);
  virtual int SetImageToRGBGradient(double r1, double g1, double b1, 
                                     double r2, double g2, double b2, 
                                     int width, int height,
                                     int options = 0);
  virtual int SetImageToSolidRGBColor(double r, double g, double b, 
                                       int width, int height,
                                       int options = 0);

  // Description:
  // Get the raw image data.
  virtual const unsigned char* GetData();

  // Description:
  // Get the width of the image.
  vtkGetMacro(Width, int);

  // Description:
  // Get the height of the image.
  vtkGetMacro(Height, int);
  
  // Description:
  // Get the pixel size of the image.
  vtkGetMacro(PixelSize, int);

  // Description:
  // Fade the icon by a factor.
  // If the icon has an alpha channel, this multiplies each element in
  // the alpha channel by 'factor'.
  virtual void Fade(double factor);

  // Description:
  // Change the hue to a new hue.
  virtual void SetHue(double hue);

  // Description:
  // Flatten the image against a color.
  // If the icon has an alpha channel, this blends the image against
  // a 'r', 'g', 'b' background and discards the alpha channel.
  virtual void Flatten(double r, double g, double b);
  virtual void Flatten(double rgb[3])
    { this->Flatten(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Compose an icon on top of this instance.
  // Supports only same size RGBA against same size RGBA at the moment.
  // Return 1 on success, 0 otherwise
  virtual int Compose(vtkKWIcon *icon);
  virtual int Compose(int icon_index);

  // Description:
  // Trip the top or right portion of an icon (i.e. look for any fully 
  // transparent area and shift the icon to the top or right).
  // Supports only RGBA at the moment.
  // Return 1 on success, 0 otherwise
  virtual int TrimTop();
  virtual int TrimRight();

  // Description:
  // Resize the canvas. The new canvas can be made larger, or smaller
  // (thus cropping the current image). The position indicates where the
  // original image should be placed in the new canvas.
  // Supports pixel size 3 or 4, will create an output with pixel size = 4.
  // Return 1 on success, 0 otherwise
  //BTX
  enum 
  { 
    PositionInCanvasCenter    = 0,
    PositionInCanvasNorthWest = 9,
    PositionInCanvasNorth     = 1,
    PositionInCanvasNorthEast = 3,
    PositionInCanvasEast      = 2,
    PositionInCanvasSouthEast = 6,
    PositionInCanvasSouth     = 4,
    PositionInCanvasSouthWest = 12,
    PositionInCanvasWest      = 8
  };
  //ETX
  virtual int ResizeCanvas(
    int resized_width, int resized_height, int position);

  // Description:
  // Resample the icon to a new size.
  // If the new 'width' is 0, it will be computed from the new 'height' by
  // keeping the aspect ratio of the icon (and vice-versa if height is 0).
  // Return 1 on success, 0 otherwise
  virtual int ResampleCanvas(int resampled_width, int resampled_height);

  // Description:
  // Fit (and resample) the icon to a new size. The aspect ratio of the
  // icon is kept so that the new icon is not larger in any dimension than
  // the new size.
  // Return 1 on success, 0 otherwise
  virtual int FitCanvas(int fit_width, int fit_height);

protected:
  vtkKWIcon();
  ~vtkKWIcon();

  // Description:
  // Set icon to the custom data.
  void SetData(const unsigned char* data, 
               int width, int height, 
               int pixel_size,
               int options = 0);

  unsigned char* Data;
  int Width;
  int Height;
  int PixelSize;

private:
  vtkKWIcon(const vtkKWIcon&); // Not implemented
  void operator=(const vtkKWIcon&); // Not implemented
};

#endif



