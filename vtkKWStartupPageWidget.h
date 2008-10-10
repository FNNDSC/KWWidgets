/*=========================================================================

  Module:    vtkKWStartupPageWidget.h,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWStartupPageWidget - an HSV color selector
// Open file
// Double click
// Drag and drop
// Recent files
// .SECTION Description
// A widget that allows the user choose a HSV color interactively

#ifndef __vtkKWStartupPageWidget_h
#define __vtkKWStartupPageWidget_h

#include "vtkKWCompositeWidget.h"

class vtkKWCanvas;
class vtkKWLabel;
class vtkKWIcon;
class vtkKWStartupPageWidgetInternals;
class vtkKWMostRecentFilesManager;

class KWWidgets_EXPORT vtkKWStartupPageWidget : public vtkKWCompositeWidget
{
public:
  static vtkKWStartupPageWidget* New();
  vtkTypeRevisionMacro(vtkKWStartupPageWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the gradient colors (in RGB space)
  vtkGetVector3Macro(GradientColor1, double);
  virtual void SetGradientColor1(double r, double g, double b);
  virtual void SetGradientColor1(double rgb[3])
    { this->SetGradientColor1(rgb[0], rgb[1], rgb[2]); };
  vtkGetVector3Macro(GradientColor2, double);
  virtual void SetGradientColor2(double r, double g, double b);
  virtual void SetGradientColor2(double rgb[3])
    { this->SetGradientColor2(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Set/Get the text color (in RGB space)
  vtkGetVector3Macro(TextColor, double);
  virtual void SetTextColor(double r, double g, double b);
  virtual void SetTextColor(double rgb[3])
    { this->SetTextColor(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Set/Get the text color (in RGB space) when it is selected (hovered on)
  vtkGetVector3Macro(SelectedTextColor, double);
  virtual void SetSelectedTextColor(double r, double g, double b);
  virtual void SetSelectedTextColor(double rgb[3])
    { this->SetSelectedTextColor(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Set/Get the hint color (in RGB space)
  vtkGetVector3Macro(HintColor, double);
  virtual void SetHintColor(double r, double g, double b);
  virtual void SetHintColor(double rgb[3])
    { this->SetHintColor(rgb[0], rgb[1], rgb[2]); };

  // Description:
  // Set/Get if the Open section is supported/shown.
  virtual void SetSupportOpen(int);
  vtkGetMacro(SupportOpen, int);
  vtkBooleanMacro(SupportOpen, int);

  // Description:
  // Set/Get the Open section icon using an icon, or the index to a 
  // predefined icon found in vtkKWIcon.
  // Note that the Set method does *not* keep a reference to the icon
  // passed as parameter: it copies the whole icon contents internally.
  vtkGetObjectMacro(OpenIcon, vtkKWIcon);
  virtual void SetOpenIcon(vtkKWIcon*);
  virtual void SetOpenIconToPredefinedIcon(int icon_index);

  // Description:
  // Set/Get if the Double Click section is supported/shown.
  virtual void SetSupportDoubleClick(int);
  vtkGetMacro(SupportDoubleClick, int);
  vtkBooleanMacro(SupportDoubleClick, int);
  
  // Description:
  // Set/Get the Double Click section icon using an icon, or the index to a 
  // predefined icon found in vtkKWIcon.
  // Note that the Set method does *not* keep a reference to the icon
  // passed as parameter: it copies the whole icon contents internally.
  vtkGetObjectMacro(DoubleClickIcon, vtkKWIcon);
  virtual void SetDoubleClickIcon(vtkKWIcon*);
  virtual void SetDoubleClickIconToPredefinedIcon(int icon_index);

  // Description:
  // Set/Get if the Drag & Drop section is supported/shown.
  virtual void SetSupportDrop(int);
  vtkGetMacro(SupportDrop, int);
  vtkBooleanMacro(SupportDrop, int);
  
  // Description:
  // Set/Get the Drag & Drop section icon using an icon, or the index to a 
  // predefined icon found in vtkKWIcon.
  // Note that the Set method does *not* keep a reference to the icon
  // passed as parameter: it copies the whole icon contents internally.
  vtkGetObjectMacro(DropIcon, vtkKWIcon);
  virtual void SetDropIcon(vtkKWIcon*);
  virtual void SetDropIconToPredefinedIcon(int icon_index);

  // Description:
  // Set/Get if the Most Recent Files section is supported/shown.
  virtual void SetSupportMostRecentFiles(int);
  vtkGetMacro(SupportMostRecentFiles, int);
  vtkBooleanMacro(SupportMostRecentFiles, int);
  
  // Description:
  // Set/Get the Most Recent Files section icon using an icon, or the index to 
  // a predefined icon found in vtkKWIcon.
  // Note that the Set method does *not* keep a reference to the icon
  // passed as parameter: it copies the whole icon contents internally.
  vtkGetObjectMacro(MostRecentFilesIcon, vtkKWIcon);
  virtual void SetMostRecentFilesIcon(vtkKWIcon*);
  virtual void SetMostRecentFilesIconToPredefinedIcon(int icon_index);

  // Description:
  // Set/Get the most recent files manager this page should listen to.
  vtkGetObjectMacro(MostRecentFilesManager, vtkKWMostRecentFilesManager);
  virtual void SetMostRecentFilesManager(vtkKWMostRecentFilesManager *mgr);

  // Description:
  // Specifies commands to associate with the widget. 
  // 'OpenCommand' is invoked when the user click on the Open section.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetOpenCommand(
    vtkObject *object, const char *method);

  // Description:
  // Specifies commands to associate with the widget. 
  // 'DropCommand' is invoked when the user drop a file on the widget.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the command:
  // - filename(s): list of filenames
  virtual void SetDropCommand(
    vtkObject *object, const char *method);

  // Description:
  // Specifies commands to associate with the widget. 
  // 'DoubleClickCommand' is invoked when the user double-click anywhere
  // in the page widget.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetDoubleClickCommand(
    vtkObject *object, const char *method);

  // Description:
  // Access to the canvas and internal elements
  vtkGetObjectMacro(StartupPageCanvas, vtkKWCanvas);

  // Description:
  // Update the whole UI depending on the value of the Ivars
  virtual void Update();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Callbacks. Internal, do not use.
  virtual void ConfigureCallback();
  virtual void RedrawCallback();
  virtual void HighlightSectionCallback(const char *tag, int flag);
  virtual void OpenCallback();
  virtual void DoubleClickCallback();

  // Description:
  // Add all the default observers needed by that object, or remove
  // all the observers that were added through AddCallbackCommandObserver.
  // Subclasses can override these methods to add/remove their own default
  // observers, but should call the superclass too.
  virtual void AddCallbackCommandObservers();
  virtual void RemoveCallbackCommandObservers();

protected:
  vtkKWStartupPageWidget();
  ~vtkKWStartupPageWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  double GradientColor1[3];
  double GradientColor2[3];
  double TextColor[3];
  double SelectedTextColor[3];
  double HintColor[3];

  int    SupportOpen;
  int    SupportDoubleClick;
  int    SupportDrop;
  int    SupportMostRecentFiles;

  vtkKWIcon *OpenIcon;
  vtkKWIcon *DoubleClickIcon;
  vtkKWIcon *DropIcon;
  vtkKWIcon *MostRecentFilesIcon;

  // Recent files manager

  vtkKWMostRecentFilesManager *MostRecentFilesManager;

  // Commands

  char *OpenCommand;
  char *DropCommand;
  char *DoubleClickCommand;

  // GUI

  vtkKWCanvas *StartupPageCanvas;

  // Description:
  // Bind/Unbind all components.
  virtual void Bind();
  virtual void UnBind();

  // Description:
  // Redraw or update canvas elements
  virtual void Redraw();
  virtual void ScheduleRedraw();

  // Description:
  // Look for a tag in a canvas. 
  virtual int CanvasHasTag(const char *canvas, const char *tag);
  virtual void CanvasDeleteTag(const char *canvas, const char *tag);

  // Description:
  // Update bindings, fonts, colors, icons
  virtual void UpdateInternalCanvasBindings();
  virtual void UpdateInternalCanvasColors();
  virtual void UpdateInternalCanvasFonts();
  virtual void UpdateInternalCanvasIcons();

  // Description:
  // Draw section
  virtual void AddSectionToCanvas(
    ostream &tk_cmd, 
    int x, int y, 
    vtkKWIcon *icon,
    const char *text, const char *text_font, 
    const char *hint, const char *hint_font, 
    const char *tag);
  virtual void AddMostRecentFilesSectionToCanvas(
    ostream &tk_cmd, 
    int x, int y);

  // Description:
  // Invoke the commands
  virtual void InvokeOpenCommand();
  virtual void InvokeDoubleClickCommand();

  // PIMPL Encapsulation for STL containers
  //BTX
  vtkKWStartupPageWidgetInternals *Internals;
  //ETX

  // Description:
  // Processes the events that are passed through CallbackCommand (or others).
  // Subclasses can oberride this method to process their own events, but
  // should call the superclass too.
  virtual void ProcessCallbackCommandEvents(
    vtkObject *caller, unsigned long event, void *calldata);
  
private:
  vtkKWStartupPageWidget(const vtkKWStartupPageWidget&); // Not implemented
  void operator=(const vtkKWStartupPageWidget&); // Not implemented
};

#endif

