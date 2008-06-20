/*=========================================================================

  Module:    vtkKWClipboardHelper.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
#include <Carbon/Carbon.h>
#define Cursor X11Cursor 
#endif
#endif // __APPLE__

#include "vtkWindows.h"

#include "vtkKWClipboardHelper.h"

#include "vtkDebugLeaks.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"

#ifdef __APPLE__
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
#undef Cursor
#endif
#endif // __APPLE__

vtkStandardNewMacro( vtkKWClipboardHelper );
vtkCxxRevisionMacro(vtkKWClipboardHelper, "1.0");

//----------------------------------------------------------------------------
vtkKWClipboardHelper::vtkKWClipboardHelper()
{
}

//----------------------------------------------------------------------------
vtkKWClipboardHelper::~vtkKWClipboardHelper()
{
}

//----------------------------------------------------------------------------
int vtkKWClipboardHelper::CopyTextToClipboard(const char* text)
{
  if(!text || !(*text))
  {
  return 0;
  }

#ifdef _WIN32 
    if (::OpenClipboard(NULL))
      {
      EmptyClipboard();
      HANDLE hDIB = 
        ::GlobalAlloc(GMEM_DDESHARE, strlen(text) + 1);
      char *ptr = (char*)::GlobalLock(hDIB);
      strcpy(ptr, text);
      SetClipboardData(CF_TEXT, hDIB);
      ::GlobalUnlock(hDIB);
      CloseClipboard();
      return 1;
      }
#endif

// For MacOsX - 10.4+     
#ifdef __APPLE__
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4

    PasteboardRef pb = NULL;
    if(noErr == PasteboardCreate(kPasteboardClipboard,&pb))
      {
      CFDataRef filedata = CFDataCreate(kCFAllocatorDefault, 
        (UInt8*)text,strlen(text));
      PasteboardClear(pb);
      PasteboardSyncFlags syncFlags = PasteboardSynchronize(pb);
      PasteboardPutItemFlavor(pb,(PasteboardItemID)1,
        CFSTR("public.utf8-plain-text"),filedata,0);

      CFRelease(filedata);
      CFRelease(pb);
      return 1;
      }
#endif
#endif

  return 0;
}
//----------------------------------------------------------------------------
int vtkKWClipboardHelper::CopyImageToClipboard(vtkImageData* iData)
{
  if(!iData)
    {
    return 0;
    }

  int *extent = iData->GetExtent();
  if (extent[0] > extent[1] && extent[2] > extent[3] && extent[4] > extent[5])
    {
    return 0;
    }
 
  unsigned char *ptr = (unsigned char *)(iData->GetScalarPointer());
  extent = iData->GetWholeExtent();
  int size[2];
  size[0] = extent[1] - extent[0] + 1;
  size[1] = extent[3] - extent[2] + 1;

  // Save to clipboard

#ifdef _WIN32

  //if (::OpenClipboard(
  //      (HWND)first_rwwidget->GetRenderWindow()->GetGenericWindowId()))
  if (::OpenClipboard(NULL))
    {
    int data_width = ((size[0] * 3 + 3) / 4) * 4;
    int src_width = size[0] * 3;
  
    EmptyClipboard();

    DWORD dwLen = sizeof(BITMAPINFOHEADER) + data_width * size[1];
    HANDLE hDIB = ::GlobalAlloc(GHND, dwLen);
    LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER) ::GlobalLock(hDIB);
    
    lpbi->biSize = sizeof(BITMAPINFOHEADER);
    lpbi->biWidth = size[0];
    lpbi->biHeight = size[1];
    lpbi->biPlanes = 1;
    lpbi->biBitCount = 24;
    lpbi->biCompression = BI_RGB;
    lpbi->biClrUsed = 0;
    lpbi->biClrImportant = 0;
    lpbi->biSizeImage = data_width * size[1];
    
    // Copy the data to the clipboard

    unsigned char *dest = (unsigned char *)lpbi + lpbi->biSize;

    int i,j;
    for (i = 0; i < size[1]; i++)
      {
      for (j = 0; j < size[0]; j++)
        {
        *dest++ = ptr[2];
        *dest++ = ptr[1];
        *dest++ = *ptr;
        ptr += 3;
        }
      dest = dest + (data_width - src_width);
      }
    
    SetClipboardData (CF_DIB, hDIB);
    ::GlobalUnlock(hDIB);
    CloseClipboard();
    return 1;
    }           
#endif

// For MacOsX - 10.4+
#ifdef __APPLE__
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4

  void* bitmapData;
  int bytePRow = size[0]*4;
  int byteCount = bytePRow*size[1];

  bitmapData = malloc(byteCount);
  unsigned char* newDest = (unsigned char*)bitmapData;

  int i, j;
  unsigned char* ptrOrig = ptr;
  int origBPR = 3*size[0];
  for(i=size[1]-1;i>=0;i--)
    {
    ptr = ptrOrig + i*origBPR; 
    for(int j=0;j<size[0];j++)
      {
      *newDest++ = *ptr;
      *newDest++ = ptr[1];
      *newDest++ = ptr[2];
      *newDest++ = 1;
      ptr += 3;
      }
    }

  CGColorSpaceRef colorSpace= CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB); 
  CGContextRef pBitmapRef = CGBitmapContextCreate (
    bitmapData, size[0], size[1], 8, bytePRow, 
    colorSpace, kCGImageAlphaNoneSkipLast);

  CFRelease(colorSpace);
  CGImageRef image = CGBitmapContextCreateImage(pBitmapRef);

  //Copy our image into data
  CFMutableDataRef      data1 = CFDataCreateMutable(kCFAllocatorDefault,0);

  CGImageDestinationRef dest1 = CGImageDestinationCreateWithData(
    (CFMutableDataRef)data1,CFSTR("com.apple.pict"),1,NULL);
  CGImageDestinationAddImage(dest1,image,NULL);
  CGImageDestinationFinalize(dest1);

  CGImageRelease(image);
  CFRelease(dest1);

  CFRange range;
  range.location = 0;
  range.length = 512;//PICT header length

  // The clipboard REQIRES only the image data, not the header.
  CFDataDeleteBytes(data1, range);

  CGImageRef image2 = CGBitmapContextCreateImage(pBitmapRef);
  CFMutableDataRef data2 = CFDataCreateMutable(kCFAllocatorDefault,0);
  CGImageDestinationRef dest2 =
    CGImageDestinationCreateWithData(data2,CFSTR("public.png"),1,NULL);

  CGImageDestinationAddImage(dest2,image2,NULL);
  CGImageDestinationFinalize(dest2);

  CFRelease(dest2);
  CGImageRelease(image2);
  CFRelease(pBitmapRef);

  OSStatus err = noErr;

  PasteboardRef pb = NULL;
  err = PasteboardCreate(kPasteboardClipboard,&pb);
  if(err == noErr)
    {
    PasteboardClear(pb);
    PasteboardSyncFlags syncFlags = PasteboardSynchronize(pb);
    PasteboardPutItemFlavor(pb,(PasteboardItemID)1,CFSTR("com.apple.pict"),data1,0);     
    PasteboardPutItemFlavor(pb,(PasteboardItemID)2, CFSTR("public.png"),data2, 0);
    }

  CFRelease(data1);
  CFRelease(data2);

  if(pb)
     {
     CFRelease(pb);
     }

  free(bitmapData);
  if(err == noErr)
    {
    return 1;
    }
#endif
#endif

  return 0;
}

//----------------------------------------------------------------------------
void vtkKWClipboardHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
