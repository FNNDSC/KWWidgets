/*=========================================================================

  Module:    $RCSfile: KWConvertImageToHeader.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWResourceUtilities.h"
#include "vtkOutputWindow.h"

#include <vtksys/CommandLineArguments.hxx>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
int unknown_argument_handler(const char *, void *) { return 0; };

//----------------------------------------------------------------------------
void display_usage(vtksys::CommandLineArguments &args)
{
  vtksys_stl::string exe_basename = 
    vtksys::SystemTools::GetFilenameName(args.GetArgv0());
  cerr << "Usage: " << exe_basename.c_str() << " [--update] [--zlib] [--base64] header.h image.png [image.png image.png...]" << endl;
  cerr << args.GetHelp();
}

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  vtkOutputWindow::GetInstance()->PromptUserOn();

  vtksys::CommandLineArguments args;

  int option_update = 0;
  int option_zlib = 0;
  int option_base64 = 0;

  args.Initialize(argc, argv);
  args.SetUnknownArgumentCallback(unknown_argument_handler);

  args.AddArgument(
    "--update", vtksys::CommandLineArguments::NO_ARGUMENT, 
    &option_update, 
    "Update header only if one of the image is more recent than the header.");

  args.AddArgument(
    "--zlib", vtksys::CommandLineArguments::NO_ARGUMENT, 
    &option_zlib, 
    "Compress the image buffer using zlib.");

  args.AddArgument(
    "--base64", vtksys::CommandLineArguments::NO_ARGUMENT, 
    &option_base64, 
    "Convert the image buffer to base64.");

  args.Parse();

  // Process the remaining args (note that rem_argv[0] is still the prog name)

  int res = 1;

  int rem_argc;
  char **rem_argv;
  args.GetRemainingArguments(&rem_argc, &rem_argv);

  if (rem_argc < 3)
    {
    cerr << "Invalid or missing arguments" << endl;
    display_usage(args);
    res = 0;
    }
  else
    {
    option_update *= 
      vtkKWResourceUtilities::ConvertImageToHeaderOptionUpdate;
    option_zlib *= 
      vtkKWResourceUtilities::ConvertImageToHeaderOptionZlib;
    option_base64 *=
      vtkKWResourceUtilities::ConvertImageToHeaderOptionBase64;
   
    cout << "- " << rem_argv[1] << endl;

    vtkKWResourceUtilities::ConvertImageToHeader(
      rem_argv[1], (const char **)&rem_argv[2], rem_argc - 2, 
      option_update | option_zlib | option_base64);
    }

  delete [] rem_argv;
  return res;
}
