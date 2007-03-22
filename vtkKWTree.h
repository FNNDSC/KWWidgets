/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of Kitware nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific
   prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkKWTree - tree widget
// .SECTION Description
// A simple tree widget

#ifndef __vtkKWTree_h
#define __vtkKWTree_h

#include "vtkKWCoreWidget.h"

class vtkKWTreeInternals;

class KWWidgets_EXPORT vtkKWTree : public vtkKWCoreWidget
{
public:
  static vtkKWTree* New();
  vtkTypeRevisionMacro(vtkKWTree,vtkKWCoreWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add a new node identified by 'node' at the end of the children list of 
  // 'parent'. If parent is NULL, or an empty string or 'root', insert at the
  // root of the tree automatically. Note that the 'node' parameter is the
  // string identifier to be used later on to refer to the node. The label
  // of the node itself is provided by the 'text' parameter.
  // A new node is selectable and closed by default.
  virtual void AddNode(const char *parent, const char *node, const char *text);
  
  // Description:
  // Delete a node identified by parameter 'node', which is the
  // unique string identifier.
  virtual void DeleteNode(const char *node);

  // Description:
  // Set/Get the label of the node.
  // The return value of GetNodeText is a pointer to a temporary buffer
  // which contents should be copied *right away*.
  virtual const char* GetNodeText(const char *node);
  virtual void SetNodeText(const char *node, const char *text);

  // Description:
  // Set/Get if node can be selected.
  virtual int GetNodeSelectableFlag(const char *node);
  virtual void SetNodeSelectableFlag(const char *node, int flag);

  // Description:
  // Specifies the font to use when drawing a specific node. 
  // You can use predefined font names (e.g. 'system'), or you can specify
  // a set of font attributes with a platform-independent name, for example,
  // 'times 12 bold'. In this example, the font is specified with a three
  // element list: the first element is the font family, the second is the
  // size, the third is a list of style parameters (normal, bold, roman, 
  // italic, underline, overstrike). Example: 'times 12 {bold italic}'.
  // The Times, Courier and Helvetica font families are guaranteed to exist
  // and will be matched to the corresponding (closest) font on your system.
  // If you are familiar with the X font names specification, you can also
  // describe the font that way (say, '*times-medium-r-*-*-12*').
  // The return value of GetNodeFont is a pointer to a temporary buffer
  // which contents should be copied *right away*.
  virtual const char* GetNodeFont(const char *node);
  virtual void SetNodeFont(const char *node, const char *font);

  // Description:
  // Convenience methods to set the font weight/slant for a specific node.
  // For more control, check the SetNodeFont method.
  virtual void SetNodeFontWeightToBold(const char *node);
  virtual void SetNodeFontWeightToNormal(const char *node);
  virtual void SetNodeFontSlantToItalic(const char *node);
  virtual void SetNodeFontSlantToRoman(const char *node);

  // Description:
  // Set/Get the user data associated to the node. This can be used
  // to bury additional data in the node.
  // The return value of GetNodeUserData is a pointer to a temporary buffer
  // which contents should be copied *right away*.
  virtual const char* GetNodeUserData(const char *node);
  virtual void SetNodeUserData(const char *node, const char *data);
  virtual int GetNodeUserDataAsInt(const char *node);
  virtual void SetNodeUserDataAsInt(const char *node, int);

  // Description:
  // Query if given node exists in the tree
  virtual int HasNode(const char *node);

  // Description:
  // Delete all nodes
  virtual void DeleteAllNodes();
  
  // Description:
  // Delete all child nodes of the given node
  virtual void DeleteNodeChildren(const char *node);
  
  // Description:
  // Get node's children as a space separated list of nodes
  // The return value of GetNodeChildren is a pointer to a temporary buffer
  // which contents should be copied *right away*.
  virtual const char* GetNodeChildren(const char *node);

  // Description:
  // Get node's parent
  // The return value of GetNodeParent is a pointer to a temporary buffer
  // which contents should be copied *right away*.
  virtual const char* GetNodeParent(const char *node);

  // Description:
  // Return if a node is the ancestor of another node.
  virtual int IsNodeAncestor(const char *ancestor, const char *node);

  // Description:
  // Mode a node to the children list of parent at position index. Parent
  // can not be a descendant of node.
  virtual void MoveNode(const char *node, const char *new_parent, int pos);

  // Description:
  // Find a node with a specific user data associated, starting the
  // search among the children of a specific node (set it to NULL or 'root'
  // to search the whole tree).
  // Return node name if found, NULL otherwise.
  // The return value of both methods is a pointer to a temporary buffer
  // which contents should be copied *right away*.
  virtual const char* FindNodeWithUserData(
    const char *parent, const char *data);
  virtual const char* FindNodeWithUserDataAsInt(
    const char *parent, int data);

  // Description:
  // Set/Get the one of several styles for manipulating the selection. 
  // Valid constants can be found in vtkKWOptions::SelectionModeType.
  virtual void SetSelectionMode(int);
  vtkGetMacro(SelectionMode, int);
  virtual void SetSelectionModeToSingle();
  virtual void SetSelectionModeToMultiple();

  // Description:
  // Select a node (this adds the node to the selection), deselect a node.
  // Note that this selecting more than one node is likely not to work if
  // the SelectionMode is not Multiple.
  virtual void SelectNode(const char *node);
  virtual void DeselectNode(const char *node);

  // Description:
  // Select a single node (any other selection is cleared first)
  virtual void SelectSingleNode(const char *node);

  // Description:
  // Clear the selection
  virtual void ClearSelection();

  // Description:
  // Return the selection as a list of space separated selected nodes
  // The return value of GetSelection is a pointer to a temporary buffer
  // which contents should be copied *right away*.
  virtual const char* GetSelection();

  // Description:
  // Return if a node is selected
  virtual int HasSelection();

  // Description:
  // Select/Deselect all the node's children.
  // Note that this selecting more than one node is likely not to work if
  // the SelectionMode is not Multiple.
  virtual void SelectNodeChildren(const char *node);
  virtual void DeselectNodeChildren(const char *node);

  // Description:
  // Arrange the tree to see a given node
  virtual void SeeNode(const char *node);

  // Description:
  // Open/close a node.
  virtual void OpenNode(const char *node);
  virtual void CloseNode(const char *node);
  virtual int IsNodeOpen(const char *node);

  // Description:
  // Open/close the first node of the tree.
  virtual void OpenFirstNode();
  virtual void CloseFirstNode();

  // Description:
  // Open/close a tree, i.e. a node and all its children.
  virtual void OpenTree(const char *node);
  virtual void CloseTree(const char *node);

  // Description:
  // Specifies wether or not a node can be reparented interactively using
  // drag and drop.
  // Check the NodeParentChangedCommand callback and NodeParentChangedEvent
  // to be notified about the change.
  vtkBooleanMacro(EnableReparenting, int);
  vtkGetMacro(EnableReparenting, int);
  virtual void SetEnableReparenting(int);

  // Description:
  // Set/Get the width/height.
  virtual void SetWidth(int);
  virtual int GetWidth();
  virtual void SetHeight(int);
  virtual int GetHeight();

  // Description:
  // Specifies an image to display at the left of the label of a node.
  // The SetImageToPredefinedIcon method accepts an index to one of the
  // predefined icon listed in vtkKWIcon.
  // The SetImageToPixels method sets the image using pixel data. It expects
  // a pointer to the pixels and the structure of the image, i.e. its width, 
  // height and the pixel_size (how many bytes per pixel, say 3 for RGB, or
  // 1 for grayscale). If buffer_length = 0, it is computed automatically
  // from the previous parameters. If it is not, it will most likely indicate
  // that the buffer has been encoded using base64 and/or zlib.
  // If pixel_size > 3 (i.e. RGBA), the image is blend the with background
  // color of the widget.
  virtual void SetNodeImageToIcon(const char *node, vtkKWIcon *icon);
  virtual void SetNodeImageToPredefinedIcon(const char *node, int icon_index);
  virtual void SetNodeImageToPixels(const char *node,
    const unsigned char *pixels, int width, int height, int pixel_size,
    unsigned long buffer_length = 0);
  
  // Description:
  // Set/Get the distance between image or window and text of a node.
  virtual void SetNodePadX(const char *node, int);
  virtual int GetNodePadX(const char *node);

  // Description:
  // Specifies wether or not the tree should be redrawn when entering idle. 
  // Set it to false if you call update while modifying the tree
  vtkBooleanMacro(RedrawOnIdle, int);
  virtual void SetRedrawOnIdle(int);
  virtual int GetRedrawOnIdle();

  // Description:
  // Specifies wether or not lines should be drawn between nodes.
  vtkBooleanMacro(LinesVisibility, int);
  virtual void SetLinesVisibility(int);
  virtual int GetLinesVisibility();

  // Description:
  // If true, the selection box will be drawn across the entire tree from
  // left-to-right instead of just around the item text.
  vtkBooleanMacro(SelectionFill, int);
  virtual void SetSelectionFill(int);
  virtual int GetSelectionFill();

  // Description:
  // Set/Get the background color of the widget.
  virtual void GetBackgroundColor(double *r, double *g, double *b);
  virtual double* GetBackgroundColor();
  virtual void SetBackgroundColor(double r, double g, double b);
  virtual void SetBackgroundColor(double rgb[3])
    { this->SetBackgroundColor(rgb[0], rgb[1], rgb[2]); };
  
  // Description:
  // Set/Get the highlight thickness, a non-negative value indicating the
  // width of the highlight rectangle to draw around the outside of the
  // widget when it has the input focus.
  virtual void SetHighlightThickness(int);
  virtual int GetHighlightThickness();
  
  // Description:
  // Set/Get the border width, a non-negative value indicating the width of
  // the 3-D border to draw around the outside of the widget (if such a border
  // is being drawn; the Relief option typically determines this).
  virtual void SetBorderWidth(int);
  virtual int GetBorderWidth();
  
  // Description:
  // Set/Get the 3-D effect desired for the widget. 
  // The value indicates how the interior of the widget should appear
  // relative to its exterior. 
  // Valid constants can be found in vtkKWOptions::ReliefType.
  virtual void SetRelief(int);
  virtual int GetRelief();
  virtual void SetReliefToRaised();
  virtual void SetReliefToSunken();
  virtual void SetReliefToFlat();
  virtual void SetReliefToRidge();
  virtual void SetReliefToSolid();
  virtual void SetReliefToGroove();

  // Description:
  // Set/Get the distance between image or window and text of the nodes.
  virtual void SetPadX(int);
  virtual int GetPadX();

  // Description:
  // Set/Get the horizontal indentation between a node and its children.
  virtual void SetDeltaX(int);
  virtual int GetDeltaX();

  // Description:
  // Set/Get the vertical size of the nodes.
  virtual void SetDeltaY(int);
  virtual int GetDeltaY();

  // Description:
  // Set/Get the selection foreground and background color
  virtual void GetSelectionBackgroundColor(double *r, double *g, double *b);
  virtual double* GetSelectionBackgroundColor();
  virtual void SetSelectionBackgroundColor(double r, double g, double b);
  virtual void SetSelectionBackgroundColor(double rgb[3])
    { this->SetSelectionBackgroundColor(rgb[0], rgb[1], rgb[2]); };
  virtual void GetSelectionForegroundColor(double *r, double *g, double *b);
  virtual double* GetSelectionForegroundColor();
  virtual void SetSelectionForegroundColor(double r, double g, double b);
  virtual void SetSelectionForegroundColor(double rgb[3])
    { this->SetSelectionForegroundColor(rgb[0], rgb[1], rgb[2]); };
  
  // Description:
  // Specifies a command to associate with the widget. This command is 
  // typically invoked when the user opens a node.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the command:
  // - path to the opened node: const char*
  virtual void SetOpenCommand(vtkObject *object, const char *method);

  // Description:
  // Specifies a command to associate with the widget. This command is 
  // typically invoked when the user closes a node.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the command:
  // - path to the closed node: const char*
  virtual void SetCloseCommand(vtkObject *object, const char *method);
  
  // Description:
  // Set/add/remove a binding to a widget, i.e. the command that is invoked
  // whenever the 'event' is triggered on the widget.
  // SetBinding will replace any old bindings, whereas AddBinding will
  // add the binding to the list of bindings already defined for that event.
  // RemoveBinding can remove a specific binding or all bindings for an event.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetBinding(
    const char *event, vtkObject *object, const char *method);
  virtual void SetBinding(
    const char *event, const char *command);
  virtual void AddBinding(
    const char *event, vtkObject *object, const char *method);
  virtual void AddBinding(
    const char *event, const char *command);
  virtual void RemoveBinding(const char *event);
  virtual void RemoveBinding(
    const char *event, vtkObject *object, const char *method);

  // Description:
  // Specifies a command to execute whenever the event sequence given 
  // by 'event' occurs on the label of a node. 
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the command:
  // - path to the node for which the event occured: const char*
  virtual void SetBindText(
    const char *event, vtkObject *object, const char *method);

  // Description:
  // Set the command for single/double/right click on a node. This, 
  // in turn, just calls SetBindText.
  // WARNING: SetSingleClickOnNodeCommand will override the default behaviour
  // that selects a node when it is clicked on. It is therefore up to the
  // user to select the node if needed.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the command:
  // - path to the node for which the event occured: const char*
  virtual void SetDoubleClickOnNodeCommand(
    vtkObject *object, const char *method);
  virtual void SetSingleClickOnNodeCommand(
    vtkObject *object, const char *method);
  virtual void SetRightClickOnNodeCommand(
    vtkObject *object, const char *method);
  
  // Description:
  // Specifies a command to associate with the widget. This command is 
  // typically invoked when the selection changes.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetSelectionChangedCommand(
    vtkObject *object, const char *method);
  
  // Description:
  // Specifies a command to associate with the widget. This command is 
  // typically invoked when a node is reparented interactively.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  // The following parameters are also passed to the command:
  // - path to the node that was reparented: const char*
  // - path to the node's new parent: const char*
  // - path to the node's previous parent: const char*
  virtual void SetNodeParentChangedCommand(
    vtkObject *object, const char *method);
  
  // Description:
  // Events. The SelectionChangedEvent is triggered when the selection is
  // changed (i.e. one or more node(s) are selected/deselected).
  // It is  similar in concept as the 'SelectionChangedCommand' but can be
  // used by multiple listeners/observers at a time.
  // The RightClickOnNodeEvent event is triggered when right-clicking on
  // a specific node.
  // The following parameters are also passed as client data for 
  // RightClickOnNodeEvent:
  // - path to the node: const char*
  // The following parameters are also passed as client data for 
  // NodeParentChangedEvent:
  // - path to the node that was reparented: const char*
  // - path to the node's new parent: const char*
  // - path to the node's previous parent: const char*
  //BTX
  enum
  {
    SelectionChangedEvent = 10000,
    RightClickOnNodeEvent,
    NodeParentChangedEvent
  };
  //ETX

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Set focus to this widget.
  virtual void Focus();
  virtual int HasFocus();

  // Description:
  // Callbacks. Internal, do not use.
  virtual void SelectionCallback();
  virtual void RightClickOnNodeCallback(const char *node);
  virtual void KeyNavigationCallback(const char* key);
  virtual void DropOverNodeCallback(const char*, const char*, const char*, const char*, const char*, const char*);

protected:
  vtkKWTree();
  ~vtkKWTree();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  int SelectionMode;
  int EnableReparenting;

  char *SelectionChangedCommand;
  virtual void InvokeSelectionChangedCommand();
  
  char *NodeParentChangedCommand;
  virtual void InvokeNodeParentChangedCommand(
    const char *node, const char *new_parent, const char *previous_parent);

  virtual void UpdateDragAndDrop();

  // PIMPL Encapsulation for STL containers
  //BTX
  vtkKWTreeInternals *Internals;
  //ETX

private:
  vtkKWTree(const vtkKWTree&); // Not implemented
  void operator=(const vtkKWTree&); // Not implemented
};

#endif
