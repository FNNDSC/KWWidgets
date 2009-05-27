/*=========================================================================

  Module:    vtkKWKeyBindingsManager.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWKeyBindingsManager.h"

#ifdef _MSC_VER
// Let us get rid of "decorated name length exceeded, name was truncated"
// http://msdn.microsoft.com/en-us/library/074af4b6(VS.80).aspx
// VS6 support might get discontinued anyway...
#pragma warning( disable : 4503 )
#endif 

#include "vtkKWApplication.h"
#include "vtkKWEventMap.h"

#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include <vtksys/stl/string>
#include <vtksys/stl/vector>
#include <vtksys/stl/map>
#include <vtksys/SystemTools.hxx>

vtkCxxRevisionMacro(vtkKWKeyBindingsManager, "1.15");
vtkStandardNewMacro(vtkKWKeyBindingsManager );

//----------------------------------------------------------------------------
class vtkKWKeyBindingsManagerInternals
{
public:

  class KeyBinding
  {
  public:
    vtkObject *Target;
    vtksys_stl::string Binding;
    vtkObject *CallbackObject;
    vtksys_stl::string CallbackCommand;
    vtksys_stl::string Context;
    vtksys_stl::string Description;

    void Update(vtkObject *target, 
                const char *binding, 
                vtkObject *callback_object, 
                const char *callback_command,
                const char *context,
                const char *description);
  };
  
  // Vector of keybindings (has all the fields per keybinding)
  // In a struct to avoid Compiler Warning (level 1) C4503
  struct EntriesContainer
  {
    vtksys_stl::vector<KeyBinding> Container;
  };
  typedef vtksys_stl::vector<KeyBinding>::iterator EntriesIterator;

  // Faster access:
  // container[target][binding][callback object] = vector of key bindings

  // Faster access: map of callback object to keybindings that have that
  // same callback object

  typedef vtksys_stl::map<vtkObject*, EntriesContainer> ObjectToEntriesContainer;
  typedef vtksys_stl::map<vtkObject*, EntriesContainer>::iterator ObjectToEntriesIterator;

  // Faster access: map of binding (i.e. event) to (map of callback object to
  // keybindings)

  typedef vtksys_stl::map<vtksys_stl::string, ObjectToEntriesContainer> BindingToObjectContainer;
  typedef vtksys_stl::map<vtksys_stl::string, ObjectToEntriesContainer>::iterator BindingToObjectIterator;

  // Faster access: map of target to binding  (i.e. event) to
  // (map of callback object to keybindings)

  typedef vtksys_stl::map<vtkObject*, BindingToObjectContainer> ObjectToBindingContainer;
  typedef vtksys_stl::map<vtkObject*, BindingToObjectContainer>::iterator ObjectToBindingIterator;

  ObjectToBindingContainer ObjectToBindings;

  // String to string, cache the pretty objects

  typedef vtksys_stl::map<vtksys_stl::string, vtksys_stl::string> StringToStringContainer;
  typedef vtksys_stl::map<vtksys_stl::string, vtksys_stl::string>::iterator StringToStringIterator;

  StringToStringContainer BindingToPrettyBinding;
  StringToStringContainer ContextToPrettyContext;
};

//----------------------------------------------------------------------------
void vtkKWKeyBindingsManagerInternals::KeyBinding::Update(
  vtkObject *target, 
  const char *binding, 
  vtkObject *callback_object, 
  const char *callback_command,
  const char *context,
  const char *description)
{
  this->Target = target;
  if (binding && *binding)
    {
    this->Binding = binding;
    }
  this->CallbackObject =  callback_object;
  if (callback_command && *callback_command)
    {
    this->CallbackCommand = callback_command;
    }
  if (context && *context)
    {
    this->Context = context;
    }
  if (description && *description)
    {
    this->Description = description;
    }
}

//----------------------------------------------------------------------------
vtkKWKeyBindingsManager::vtkKWKeyBindingsManager()
{
  this->Internals = new vtkKWKeyBindingsManagerInternals;
}

//----------------------------------------------------------------------------
vtkKWKeyBindingsManager::~vtkKWKeyBindingsManager()
{
  if (this->Internals)
    {
    delete this->Internals;
    }
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsManager::AddKeyBinding(
  vtkObject *target, 
  const char *binding, 
  vtkObject *callback_object, 
  const char *callback_command,
  const char *context,
  const char *description)
{
  if (!target || !binding)
    {
    return;
    }

  vtkKWKeyBindingsManagerInternals::EntriesContainer &keybindings = 
    this->Internals->ObjectToBindings[target][binding][callback_object];
  keybindings.Container.push_back(
    vtkKWKeyBindingsManagerInternals::KeyBinding());
  keybindings.Container.back().Update(
    target, binding, callback_object, callback_command, context, description);

  // When an object goes away, remove all the key bindings associated to it.

  this->AddCallbackCommandObserver(target, vtkCommand::DeleteEvent);
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsManager::SetKeyBinding(
  vtkObject *target, 
  const char *binding, 
  vtkObject *callback_object, 
  const char *callback_command,
  const char *context,
  const char *description)
{
  if (!target || !binding)
    {
    return;
    }
  
  vtkKWKeyBindingsManagerInternals::ObjectToEntriesContainer
    &callback_objects = this->Internals->ObjectToBindings[target][binding];
  callback_objects.clear();

  vtkKWKeyBindingsManagerInternals::EntriesContainer &keybindings = 
    callback_objects[callback_object];
  keybindings.Container.push_back(
    vtkKWKeyBindingsManagerInternals::KeyBinding());
  keybindings.Container.back().Update(
    target, binding, callback_object, callback_command, context, description);
  this->AddCallbackCommandObserver(target, vtkCommand::DeleteEvent);
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsManager::RemoveKeyBinding(
  vtkObject *target, 
  const char *binding, 
  vtkObject *callback_object, 
  const char *callback_command)
{
  if (!target)
    {
    return;
    }
  
  vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
    this->Internals->ObjectToBindings.find(target);
  if (it != this->Internals->ObjectToBindings.end())
    {
    // Erase all bindings

    if (!binding)
      {
      this->Internals->ObjectToBindings.erase(it);
      }
    else
      {
      vtkKWKeyBindingsManagerInternals::BindingToObjectIterator 
        it2 = (it->second).find(binding);
      if (it2 != (it->second).end())
        {
        // Check if we are trying to remove a specific key binding
      
        if (callback_object || callback_command)
          {
          vtkKWKeyBindingsManagerInternals::ObjectToEntriesIterator
            it3 = (it2->second).find(callback_object);
          if (it3 != (it2->second).end())
            {
            vtkKWKeyBindingsManagerInternals::EntriesIterator
              it4 = (it3->second).Container.begin();
            vtkKWKeyBindingsManagerInternals::EntriesIterator
              end4 = (it3->second).Container.end();
            for (; it4 != end4; ++it4)
              {
              if ((!it4->CallbackCommand.size() && !callback_command) ||
                  (callback_command && 
                   !strcmp(callback_command, it4->CallbackCommand.c_str())))
                {
                (it3->second).Container.erase(it4);
                if (!(it3->second).Container.size())
                  {
                  (it2->second).erase(it3);
                  if (!(it2->second).size())
                    {
                    (it->second).erase(it2);
                    }
                  }
                return;
                }
              }
            }
          }

        // Otherwise erase all key bindings for this specific binding

        else
          {
          (it->second).erase(it2);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
int vtkKWKeyBindingsManager::GetNumberOfTargets()
{
  return this->Internals ? (int)this->Internals->ObjectToBindings.size() : 0;
}

//----------------------------------------------------------------------------
vtkObject* vtkKWKeyBindingsManager::GetNthTarget(int idx)
{
  if (idx < 0 || idx >= this->GetNumberOfTargets())
    {
    return NULL;
    }

  vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
    this->Internals->ObjectToBindings.begin();
  while (idx)
    {
    --idx;
    ++it;
    }
  return (it->first);
}

//----------------------------------------------------------------------------
int vtkKWKeyBindingsManager::GetNumberOfBindings(vtkObject *target)
{
  if (target)
    {
    vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
      this->Internals->ObjectToBindings.find(target);
    if (it != this->Internals->ObjectToBindings.end())
      {
      return (int)(it->second).size();
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
const char* vtkKWKeyBindingsManager::GetNthBinding(vtkObject *target, int idx)
{
  if (!target || idx < 0 || idx >= this->GetNumberOfBindings(target))
    {
    return NULL;
    }

  vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
    this->Internals->ObjectToBindings.find(target);
  if (it != this->Internals->ObjectToBindings.end())
    {
    vtkKWKeyBindingsManagerInternals::BindingToObjectIterator 
      it2 = (it->second).begin();
    while (idx)
      {
      --idx;
      ++it2;
      }
    return (it2->first).c_str();
    }

  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWKeyBindingsManager::GetNumberOfCallbackObjects(
  vtkObject *target, const char *binding)
{
  if (target && binding)
    {
    vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
      this->Internals->ObjectToBindings.find(target);
    if (it != this->Internals->ObjectToBindings.end())
      {
      vtkKWKeyBindingsManagerInternals::BindingToObjectIterator 
        it2 = (it->second).find(binding);
      if (it2 != (it->second).end())
        {
        return (int)(it2->second).size();
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkObject* vtkKWKeyBindingsManager::GetNthCallbackObject(
  vtkObject *target, const char *binding, int idx)
{
  if (!target || 
      idx < 0 || idx >= this->GetNumberOfCallbackObjects(target, binding))
    {
    return NULL;
    }

  vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
    this->Internals->ObjectToBindings.find(target);
  if (it != this->Internals->ObjectToBindings.end())
    {
    vtkKWKeyBindingsManagerInternals::BindingToObjectIterator 
      it2 = (it->second).find(binding);
    if (it2 != (it->second).end())
      {
      vtkKWKeyBindingsManagerInternals::ObjectToEntriesIterator
        it3 = (it2->second).begin();
      while (idx)
        {
        --idx;
        ++it3;
        }
      return (it3->first);
      }
    }
  
  return NULL;
}

//----------------------------------------------------------------------------
int vtkKWKeyBindingsManager::GetNumberOfKeyBindings(
  vtkObject *target, const char *binding, vtkObject *callback_object)
{
  if (target && binding)
    {
    vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
      this->Internals->ObjectToBindings.find(target);
    if (it != this->Internals->ObjectToBindings.end())
      {
      vtkKWKeyBindingsManagerInternals::BindingToObjectIterator 
        it2 = (it->second).find(binding);
      if (it2 != (it->second).end())
        {
        vtkKWKeyBindingsManagerInternals::ObjectToEntriesIterator
          it3 = (it2->second).find(callback_object);
        if (it3 != (it2->second).end())
          {
          return (int)(it3->second).Container.size();
          }
        }
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
const char* vtkKWKeyBindingsManager::GetNthCallbackCommand(
  vtkObject *target, const char *binding, vtkObject *callback_object, int idx)
{
  if (!target || 
      idx < 0 || 
      idx >= this->GetNumberOfKeyBindings(target, binding, callback_object))
    {
    return NULL;
    }

  vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
    this->Internals->ObjectToBindings.find(target);
  if (it != this->Internals->ObjectToBindings.end())
    {
    vtkKWKeyBindingsManagerInternals::BindingToObjectIterator 
      it2 = (it->second).find(binding);
    if (it2 != (it->second).end())
      {
      vtkKWKeyBindingsManagerInternals::ObjectToEntriesIterator
        it3 = (it2->second).find(callback_object);
      return (it3->second).Container[idx].CallbackCommand.c_str();
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
const char* vtkKWKeyBindingsManager::GetNthContext(
  vtkObject *target, const char *binding, vtkObject *callback_object, int idx)
{
  if (!target || 
      idx < 0 || 
      idx >= this->GetNumberOfKeyBindings(target, binding, callback_object))
    {
    return NULL;
    }

  vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
    this->Internals->ObjectToBindings.find(target);
  if (it != this->Internals->ObjectToBindings.end())
    {
    vtkKWKeyBindingsManagerInternals::BindingToObjectIterator 
      it2 = (it->second).find(binding);
    if (it2 != (it->second).end())
      {
      vtkKWKeyBindingsManagerInternals::ObjectToEntriesIterator
        it3 = (it2->second).find(callback_object);
      return (it3->second).Container[idx].Context.c_str();
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
const char* vtkKWKeyBindingsManager::GetNthDescription(
  vtkObject *target, const char *binding, vtkObject *callback_object, int idx)
{
  if (!target || 
      idx < 0 || 
      idx >= this->GetNumberOfKeyBindings(target, binding, callback_object))
    {
    return NULL;
    }

  vtkKWKeyBindingsManagerInternals::ObjectToBindingIterator it = 
    this->Internals->ObjectToBindings.find(target);
  if (it != this->Internals->ObjectToBindings.end())
    {
    vtkKWKeyBindingsManagerInternals::BindingToObjectIterator 
      it2 = (it->second).find(binding);
    if (it2 != (it->second).end())
      {
      vtkKWKeyBindingsManagerInternals::ObjectToEntriesIterator
        it3 = (it2->second).find(callback_object);
      return (it3->second).Container[idx].Description.c_str();
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
const char* vtkKWKeyBindingsManager::GetPrettyBinding(const char *binding)
{
  vtkKWKeyBindingsManagerInternals::StringToStringIterator it = 
    this->Internals->BindingToPrettyBinding.find(binding);
  if (it != this->Internals->BindingToPrettyBinding.end())
    {
    return (it->second).c_str();
    }

  const char *end = binding + strlen(binding);
  const char *begin = binding;
  
  // Remove leading < and trailing >

  if (*begin == '<')
    {
    ++begin;
    if (end > begin && *(end - 1) == '>')
      {
      --end;
      }
    }

  // Remove Key- and KeyPress- prefixes

  const char *key = "Key-";
  const char *keypress = "KeyPress-";
  if (vtksys::SystemTools::StringStartsWith(begin, key))
    {
    begin += strlen(key);
    }
  else if (vtksys::SystemTools::StringStartsWith(begin, keypress))
    {
    begin += strlen(keypress);
    }

  // Assign to string in cache container

  vtksys_stl::string &pretty = 
    this->Internals->BindingToPrettyBinding[binding];
  pretty.append(begin, end);

  // Replace Prior with Page Up and Next with Page Down

  vtksys::SystemTools::ReplaceString(pretty, "Prior", "Page Up");
  vtksys::SystemTools::ReplaceString(pretty, "Next", "Page Down");

  // Replace Control-, Shift- and Alt-

  vtksys::SystemTools::ReplaceString(pretty, "Control-", "Control+");
  vtksys::SystemTools::ReplaceString(pretty, "Shift-", "Shift+");
  vtksys::SystemTools::ReplaceString(pretty, "Alt-", "Alt+");

  // Uppercase the letter if it's a single letter, or something like
  // Control+t

  size_t pretty_size = pretty.size();
  if (pretty_size == 1 || 
      (pretty_size >= 2 && pretty[pretty_size - 2] == '+'))
    {
    pretty[pretty_size - 1] = 
      static_cast<vtksys_stl::string::value_type>(
        toupper(pretty[pretty_size - 1]));
    }

  return pretty.c_str();
}

//----------------------------------------------------------------------------
const char* vtkKWKeyBindingsManager::GetPrettyContext(const char *context)
{
  vtkKWKeyBindingsManagerInternals::StringToStringIterator it = 
    this->Internals->ContextToPrettyContext.find(context);
  if (it != this->Internals->ContextToPrettyContext.end())
    {
    return (it->second).c_str();
    }

  const char *end = context + strlen(context);
  const char *begin = context;

  int is_single_word = (strchr(begin, ' ') == NULL);
  if (is_single_word)
    {
    // Remove vtk prefix
    
    const char *vtk = "vtk";
    if (vtksys::SystemTools::StringStartsWith(begin, vtk))
      {
      begin += strlen(vtk);
      }
    
    // Pass any toolkit prefix, say KW, or OSA, anything that is all uppercase
    // Not super optimized, right?
    
    const char *endm1 = end - 1;
    while (begin < endm1 && isupper(*begin) && isupper(*(begin + 1)))
      {
      ++begin;
      }
    }
    
  // Assign to string in cache container

  vtksys_stl::string &pretty = 
    this->Internals->ContextToPrettyContext[context];
  pretty.append(begin, end);

  // Add space between capitalized words

  if (is_single_word)
    {
    pretty = vtksys::SystemTools::AddSpaceBetweenCapitalizedWords(pretty);
    }
  else
    {
    pretty = vtksys::SystemTools::CapitalizedWords(pretty);
    }

  return pretty.c_str();
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsManager::SetKeyBindingsFromEventMap(
  vtkKWEventMap *map)
{
  if (!map)
    {
    return;
    }

  vtksys_stl::string binding;
  int i;

  // Keys

  int nb_key_events = map->GetNumberOfKeyEvents();
  for (i = 0; i < nb_key_events; ++i)
    {
    vtkKWEventMap::KeyEvent *key_event = map->GetKeyEvent(i);
    if (key_event)
      {
      binding = '<';
      if (key_event->Modifier & vtkKWEventMap::ControlShiftModifier)
        {
        binding += "Control-Shift-";
        }
      else if (key_event->Modifier & vtkKWEventMap::ControlModifier)
        {
        binding += "Control-";
        }
      else if (key_event->Modifier & vtkKWEventMap::ShiftModifier)
        {
        binding += "Shift-";
        }
      else
        {
        binding += "KeyPress-";
        }
      binding += (char)key_event->Key;
      binding += '>';
      this->SetKeyBinding(
        map, binding.c_str(), NULL, key_event->Action, 
        key_event->Context, key_event->Description);
      }
    }
  
  // Keysyms

  int nb_keysym_events = map->GetNumberOfKeySymEvents();
  for (i = 0; i < nb_keysym_events; ++i)
    {
    vtkKWEventMap::KeySymEvent *keysym_event = map->GetKeySymEvent(i);
    if (keysym_event)
      {
      binding = '<';
      if (keysym_event->Modifier & vtkKWEventMap::ControlShiftModifier)
        {
        binding += "Control-Shift-";
        }
      else if (keysym_event->Modifier & vtkKWEventMap::ControlModifier)
        {
        binding += "Control-";
        }
      else if (keysym_event->Modifier & vtkKWEventMap::ShiftModifier)
        {
        binding += "Shift-";
        }
      binding += keysym_event->KeySym;
      binding += '>';
      this->SetKeyBinding(
        map, binding.c_str(), NULL, keysym_event->Action, 
        keysym_event->Context, keysym_event->Description);
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsManager::ProcessCallbackCommandEvents(vtkObject *caller,
                                                           unsigned long event,
                                                           void *calldata)
{
  // When an object goes away, remove all the key bindings associated to it.

  if (event == vtkCommand::DeleteEvent)
    {
    this->RemoveKeyBinding(caller);
    }
}

//----------------------------------------------------------------------------
void vtkKWKeyBindingsManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


