#include "vtkKWEvent.h"
#include "vtkKWObject.h"

#include "vtkToolkits.h"

/* Let's check if VTK >= 5.2 is exporting its headers correctly */

#if VTK_MAJOR_VERSION > 5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 2)

// MetaIO

#ifdef VTK_USE_METAIO
#include "vtkmetaio/metaLine.h"
#endif

// DICOM Parser

#include "DICOMParser.h"

// Verdict

#include "verdict.h"

#endif // VTK_MAJOR_VERSION > 5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 2)

int main()
{
#if VTK_MAJOR_VERSION > 5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 2)

  // MetaIO

#ifdef VTK_USE_METAIO
  vtkmetaio::MetaLine line;
  (void)line;
#endif

  // DICOM Parser
  
  DICOMParser parser;
  (void)parser;

  // Verdict

  HexMetricVals hex_metric;
  (void)hex_metric;

#endif // VTK_MAJOR_VERSION > 5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 2)

  int res = 0;
  const char *event = vtkKWEvent::GetStringFromEventId(2001);
  if (!event || strcmp("MessageDialogInvokeEvent", event))
    {
    cout << "Problem with vtkKWEvent::GetStringFromEventId. Requested:"
         << 2001 << "(MessageDialogInvokeEvent) got: " 
         << event << endl;
    res = 1;
    }
  return res;
}
