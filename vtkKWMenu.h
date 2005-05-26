/*=========================================================================

  Module:    $RCSfile: vtkKWMenu.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMenu - a menu widget
// .SECTION Description
// This class is the Menu abstraction for the
// Kitware toolkit. It provides a c++ interface to
// the TK menu widgets used by the Kitware toolkit.

#ifndef __vtkKWMenu_h
#define __vtkKWMenu_h

#include "vtkKWWidget.h"

class KWWIDGETS_EXPORT vtkKWMenu : public vtkKWWidget
{
public:
  static vtkKWMenu* New();
  vtkTypeRevisionMacro(vtkKWMenu,vtkKWWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
 
  virtual void Create(vtkKWApplication* app, const char* args);
  
  // Description: 
  // Append/Insert a separator to the menu.
  void AddSeparator();
  void InsertSeparator(int position);
  
  // Description: 
  // Append/Insert a sub menu to the current menu.
  void AddCascade(const char* label, vtkKWMenu*, 
                  int underline, const char* help = 0);
  void InsertCascade(int position, const char* label,  vtkKWMenu*, 
                     int underline, const char* help = 0  );
  int GetCascadeIndex(vtkKWMenu *);

  // Description:
  // Set cascade menu for menu entry.
  void SetCascade(int index, vtkKWMenu*);
  void SetCascade(const char* item, vtkKWMenu*);
  void SetCascade(int index, const char*);
  void SetCascade(const char* item, const char*);

  // Description:
  // Copy the radio button variable logic.

  // Description: 
  // Append/Insert a CheckButton menu item to the current menu.
  char* CreateCheckButtonVariable(vtkKWObject* Object, const char* name);
  int   GetCheckButtonValue(vtkKWObject* Object, const char* name);
  void  CheckCheckButton(vtkKWObject *Object, const char *name, int val);
  void AddCheckButton(const char* label, const char* ButtonVar, 
                      vtkKWObject* Object, 
                      const char* MethodAndArgString , const char* help = 0);
  void AddCheckButton(const char* label, const char* ButtonVar, 
                      vtkKWObject* Object, 
                      const char* MethodAndArgString , int underline,
                      const char* help = 0);
  void InsertCheckButton(int position, 
                         const char* label, const char* ButtonVar, 
                         vtkKWObject* Object, 
                         const char* MethodAndArgString , const char* help = 0);
  void InsertCheckButton(int position, 
                         const char* label, const char* ButtonVar, 
                         vtkKWObject* Object, 
                         const char* MethodAndArgString , 
                         int underline, const char* help = 0);

  // Description: 
  // Append/Insert a standard menu item and command to the current menu.
  void AddCommand(const char* label, vtkKWObject* Object,
                  const char* MethodAndArgString , const char* help = 0);
  void AddCommand(const char* label, vtkKWObject* Object,
                  const char* MethodAndArgString , int underline, 
                  const char* help = 0);
  void InsertCommand(int position, const char* label, vtkKWObject* Object,
                     const char* MethodAndArgString , const char* help = 0);
  void InsertCommand(int position, const char* label, vtkKWObject* Object,
                     const char* MethodAndArgString , 
                     int underline, const char* help = 0);

  // Description:
  // Set command of the menu entry with a given index.
  void SetEntryCommand(int index, vtkKWObject* object, 
                       const char* MethodAndArgString);
  void SetEntryCommand(const char* item, vtkKWObject* object, 
                       const char* method);
  void SetEntryCommand(int item, const char* method);
  void SetEntryCommand(const char* item, const char* method);

  // Description: 
  // Append a radio menu item and command to the current menu.
  // The radio group is specified by the buttonVar value.
  char* CreateRadioButtonVariable(vtkKWObject* Object, const char* varname);
  int   GetRadioButtonValue(vtkKWObject* Object, const char* varname);
  void  CheckRadioButton(vtkKWObject *Object, const char *varname, int id);
  int   GetCheckedRadioButtonItem(vtkKWObject *Object, const char *varname);
  void AddRadioButton(int value, const char* label, const char* buttonVar, 
                      vtkKWObject* Called, 
                      const char* MethodAndArgString, const char* help = 0);
  void AddRadioButton(int value, const char* label, const char* buttonVar, 
                      vtkKWObject* Called, 
                      const char* MethodAndArgString, int underline,  
                      const char* help = 0);
  void AddRadioButtonImage(int value, const char* imgname, 
                           const char* buttonVar, vtkKWObject* Called, 
                          const char* MethodAndArgString, const char* help = 0);
  void  InsertRadioButton(int position, int value, const char* label, 
                          const char* buttonVar, vtkKWObject* Called, 
                          const char* MethodAndArgString, const char* help = 0);
  void  InsertRadioButton(int position, int value, const char* label, 
                          const char* buttonVar, vtkKWObject* Called, 
                          const char* MethodAndArgString, 
                          int underline, const char* help = 0);

  // Description: 
  // Add a generic menu item (defined by addtype)
  void AddGeneric(const char* addtype, const char* label, vtkKWObject* Object,
                  const char* MethodAndArgString, const char* extra, 
                  const char* help);
  void InsertGeneric(int position, const char* addtype, const char* label, 
                     vtkKWObject* Object,
                     const char* MethodAndArgString, const char* extra, 
                     const char* help);

  // Description:
  // Call the menu item callback at the given index
  void Invoke(int position);
  void Invoke(const char* item);

  // Description:
  // Delete the menu item at the given position.
  // Be careful, there is a bug in tk, that will break other items
  // in the menu below the one being deleted, unless a new item is added.
  void DeleteMenuItem(int position);
  void DeleteMenuItem(const char* item);
  void DeleteAllMenuItems();
  
  // Description:
  // Returns the integer index of the menu item by string, or by the
  // command (object/method) pair associated to it.
  int GetIndex(const char* item);
  int GetIndexOfCommand(vtkKWObject* Object, const char* MethodAndArgString);

  // Description:
  // Get the command for the entry at index. This is what is returned by
  // Script, so you should make a copy if you want to use it in Tcl.
  const char* GetItemCommand(int idx);

  // Description:
  // Copies the label of the item at the given position
  // to the given string ( with the given length ). Returns VTK_OK
  // if there is label, VTK_ERROR otherwise.
  // The second version returns a pointer to the result of the
  // Tcl interpreter last evaluation (be careful).
  int GetItemLabel(int position, char* label, int maxlen);
  const char* GetItemLabel(int position);

  // Description:
  // Get the option of an entry
  int HasItemOption(int position, const char *option);
  const char* GetItemOption(int position, const char *option);
  const char* GetItemOption(const char *item, const char *option);

  // Description:
  // Set the compound image of an entry
  void SetItemCompoundImage(int position, const char *imagename);
  void SetItemCompoundImage(const char *item, const char *imagename);

  // Description:
  // Checks if an item is in the menu
  int HasItem(const char* item);

  // Description:
  // Returns the number of items
  int GetNumberOfItems();
  
  // Description:
  // Call back for active menu item doc line help
  void DisplayHelp(const char*);
  
  // Description:
  // Option to make this menu a tearoff menu.  By dafault this value is off.
  void SetTearOff(int val);
  vtkGetMacro(TearOff, int);
  vtkBooleanMacro(TearOff, int);

  // Description:
  // Set/Get state of the menu entry with a given index or name.
  void SetState(int index, int state);
  void SetState(const char* item, int state);
  int  GetState(int index);
  int  GetState(const char* item);

  // Description:
  // Convenience method to set the state of all entries.
  void SetState(int state);

  // Description:
  // Configure the item at given index.
  void ConfigureItem(int index, const char*);

  //BTX
  enum { Normal = 0, Active, Disabled, Unknown };
  //ETX

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  
  vtkKWMenu();
  ~vtkKWMenu();

  int TearOff;
  
private:
  vtkKWMenu(const vtkKWMenu&); // Not implemented
  void operator=(const vtkKWMenu&); // Not implemented
};


#endif



