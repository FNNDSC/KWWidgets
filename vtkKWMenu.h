/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWMenu.h,v $
  Language:  C++
  Date:      $Date: 2003-04-03 20:18:38 $
  Version:   $Revision: 1.29 $

Copyright (c) 2000-2001 Kitware Inc. 469 Clifton Corporate Parkway,
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
// .NAME vtkKWMenu - a menu widget
// .SECTION Description
// This class is the Menu abstraction for the
// Kitware toolkit. It provides a c++ interface to
// the TK menu widgets used by the Kitware toolkit.

#ifndef __vtkKWMenu_h
#define __vtkKWMenu_h

#include "vtkKWWidget.h"

//BTX
template <class key, class data> 
class vtkArrayMap;
//ETX

class VTK_EXPORT vtkKWMenu : public vtkKWWidget
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
  char* CreateCheckButtonVariable(vtkKWObject* Object, const char* varname);
  int   GetCheckButtonValue(vtkKWObject* Object, const char* varname);
  void  CheckCheckButton(vtkKWObject *Object, const char *varname, int id);
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
  // Returns the integer index of the menu item by string
  int GetIndex(const char* item);

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
  const char* GetItemOption(int position, const char *option);
  const char* GetItemOption(const char *item, const char *option);

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
  // Configure the item at given index.
  void ConfigureItem(int index, const char*);

//BTX
  enum { Normal = 0, Active, Disabled, Unknown };
//ETX

//BTX
  // Description:
  // The following two methods allows one to store and restore
  // the state of a menu. This can be used to, for example, to
  // store the state, disable all the entries and then restore
  // the menu to the previous. Note that only the state of menu
  // entries with labels are stored.
  void StoreMenuState(vtkArrayMap<const char*, int>* state);
  void RestoreMenuState(vtkArrayMap<const char*, int>* state);
//ETX

protected:
  
  vtkKWMenu();
  ~vtkKWMenu();

  int TearOff;
  
private:
  vtkKWMenu(const vtkKWMenu&); // Not implemented
  void operator=(const vtkKWMenu&); // Not implemented
};


#endif


