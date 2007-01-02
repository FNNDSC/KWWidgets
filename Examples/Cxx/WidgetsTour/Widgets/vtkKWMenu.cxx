#include "vtkKWMenu.h"
#include "vtkKWMessage.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

#include <vtksys/stl/string>

class vtkKWMenuItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWMenuItem::Create(vtkKWWidget *parent, vtkKWWindow *win)
{
  vtkKWApplication *app = parent->GetApplication();

  int index;
  size_t i;
  const char* days[] = 
    {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
  const char* colors[] = 
    {"Re&d", "Gree&n", "&Blue"};

  // -----------------------------------------------------------------------
  
  // Let's create a label to explain what's going on

  vtkKWMessage *message1 = vtkKWMessage::New();
  message1->SetParent(parent);
  message1->Create();
  message1->SetText("This example creates a new menu entry called 'Test Menu' in the window menu bar and populate it with various menu entries. Check the source code below for more details.");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    message1->GetWidgetName());
  
  // -----------------------------------------------------------------------

  // Let's create a new entry in the main menu bar of our window
  // (vtkKWWindow is a subclass of vtkKWTopLevel, which has a pointer
  // to the menu bar)

  vtkKWMenu *menu1 = vtkKWMenu::New();
  menu1->SetParent(win->GetMenu());
  menu1->Create();

  win->GetMenu()->AddCascade("Test Menu", menu1);
  
  // -----------------------------------------------------------------------

  // Let's add a few commands in the menu

  index = menu1->AddCommand(
    "&Left Justification", message1, "SetJustificationToLeft");
  menu1->SetItemAccelerator(index, "F8");
  menu1->SetBindingForItemAccelerator(index, menu1->GetParentTopLevel());
  menu1->SetItemHelpString(index, "Set the message justification to left.");

  index = menu1->AddCommand(
    "&Right Justification", message1, "SetJustificationToRight");

  index = menu1->AddCommand(
    "&Center Justification", message1, "SetJustificationToCenter");
  // disable entry for demonstration purposes
  menu1->SetItemState(index, 0); 

  // -----------------------------------------------------------------------

  // Let's add a cascade menu
  
  menu1->AddSeparator();

  vtkKWMenu *submenu1 = vtkKWMenu::New();
  submenu1->SetParent(menu1);
  submenu1->Create();

  menu1->AddCascade("Days", submenu1);
  
  // Populate this menu with radiobuttons.
  // This is as simple as it gets since by default all radiobuttons
  // share the same group.

  for (i = 0; i < sizeof(days) / sizeof(days[0]); i++)
    {
    submenu1->AddRadioButton(days[i]);
    }

  submenu1->SelectItem("Monday");

  // -----------------------------------------------------------------------

  // Let's add a more complex cascade menu
  
  menu1->AddSeparator();

  vtkKWMenu *submenu2 = vtkKWMenu::New();
  submenu2->SetParent(menu1);
  submenu2->Create();

  menu1->AddCascade("Colors and Widths", submenu2);
  
  // Populate this menu with radiobuttons
  // This time, we need to assign each items the proper group.
  // There are several ways to do so, you can for example pick
  // an arbitrary group name...
  // No command here, just a simple radiobutton. You can use
  // SetItemCommand later on...
  
  for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++)
    {
    index = submenu2->AddRadioButton(colors[i]);
    submenu2->SetItemGroupName(index, "Colors");
    }

  submenu2->SelectItem("Red");

  submenu2->AddSeparator();

  // ... or you can use the group name that is assigned to any radiobutton
  // when it is created and make sure all other share the same. This can be
  // done with PutItemInGroup too. We also show that an arbitrary value can
  // be assigned to each entry: a string, or here, an integer. By default, 
  // this value is the label itself.

  int index_g = submenu2->AddRadioButton(
    "&Tiny Width", message1, "SetWidth 50");
  submenu2->SetItemSelectedValueAsInt(index_g, 50);

  vtksys_stl::string group_name = submenu2->GetItemGroupName(index_g);

  index = submenu2->AddRadioButton(
    "&Small Width", message1, "SetWidth 100");
  submenu2->SetItemSelectedValueAsInt(index, 100);
  submenu2->SetItemGroupName(index, group_name.c_str());

  index = submenu2->AddRadioButton(
    "&Medium Width", message1, "SetWidth 150");
  submenu2->SetItemSelectedValueAsInt(index, 150);
  submenu2->PutItemInGroup(index, index_g);
  
  // Now instead of selecting by label, or even index, we use the selected
  // value we assigned for each entry in that group. The goal here is
  // to provide an easy way to map an internal variable (say, 'width')
  // to a choice in the menu.
  
  int width = 150;
  message1->SetWidth(width);
  submenu2->SelectItemInGroupWithSelectedValueAsInt(group_name.c_str(), width);

  // The callbacks above hardcodes the width in each method to keep this
  // example short, but it could/should have been a unique 'SetWidthFromMenu'
  // callback that would have retrieved the width that was associated to each
  // entry like this:

  int selected_width = submenu2->GetItemSelectedValueAsInt(
    submenu2->GetIndexOfSelectedItemInGroup(group_name.c_str()));
  message1->SetWidth(selected_width);

  // -----------------------------------------------------------------------

  // Let's add a some checkbuttons

  menu1->AddSeparator();
  
  index = menu1->AddCheckButton("Italic");
  // Better use markers in label, but hey 
  menu1->SetItemUnderline(index, 1);

  // A callback command can be associated to the checkbutton, inside which
  // the selected state can be queried

  int is_selected = menu1->GetItemSelectedState("Italic");

  // If it is easier to remember, you can use a group name too for
  // that single checkbutton (don't put any other item in it). You don't even
  // have to make one up, it is assigned one by default that can be
  // retrieved using the entry's index (or label, but hey, if you remember
  // the label at this point, you probably do not need a group :)
  // The selected value of a checkbutton is 1 by default.

  index = menu1->AddCheckButton("I want some bold");
  menu1->SetItemGroupName(index, "Bold");
  menu1->SelectItemInGroupWithSelectedValueAsInt("Bold", 1);

  is_selected = menu1->GetItemSelectedState(
    menu1->GetIndexOfSelectedItemInGroup("Bold"));

  (void)is_selected;
  message1->Delete();
  submenu1->Delete();
  submenu2->Delete();
  menu1->Delete();
}

int vtkKWMenuItem::GetType()
{
  return KWWidgetsTourItem::TypeCore;
}

KWWidgetsTourItem* vtkKWMenuEntryPoint()
{
  return new vtkKWMenuItem();
}
