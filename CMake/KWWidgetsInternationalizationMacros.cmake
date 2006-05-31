# ---------------------------------------------------------------------------
# KWWidgets_CREATE_GETTEXT_TARGETS
# This macro can be used to create all the custom commands (and optionally
# targets) necessary to:
# - create a translation template file (pot) out of a set of sources
#   files where translatable strings were marked. This template file
#   will be re-generated each time its sources dependencies are modified.
#   This template file has to be stored out-of-source (say, in the build tree),
#   it is then up to the user to commit it back to the source tree if needed.
# - initialize translation files (po) for a set of locales, out of the 
#   translation template file (pot).
# - merge translation files (po) for a set of locales, out of a new or
#   re-generated translation template file (pot). Translations that were
#   in the po files are kept and new strings to translate found in the
#   pot file are copied to the po file for future translation.
#   Translation files are merged automatically each time the template file
#   is modified, either manually, or when it is re-generated from its sources
#   dependencies.
#   The translation files have to be stored out-of-source (say, in the build 
#   tree), it is then up to the user to commit them back to the source tree
#   if needed.
# - compile the translation files (po) for a set of locates into binary 
#   translation files (mo). Binary files are re-compiled each time the
#   translation file are modified, either manually, or when they have been
#   merge against a modified template file (as a result of manually editing
#   it or modifying its sources dependencies).
#   The binary files are generated from po files that are out-of-source
#   (say, in the build tree). The po files in the source tree are only
#   used to initialize the one in the build tree the first time they
#   are needed.
# - install the mo files.
#
# This macro accepts parameters as arg/value pairs or as a single arg if
# the arg is described as boolean (same as setting the arg to 1). The
# args can be specificied in any order and most of them are optionals.
#
# Required arguments:
# DOMAIN_NAME (string): the translation domain name, i.e. the name of the
#    application or library the translations are created for. 
#    Default to ${PROJECT_NAME} if not found.
# PO_DIR (path): absolute path to the directory where the translation  
#    files (po) are found. 
#    Default to "${CMAKE_CURRENT_SOURCE_DIR}/po" if not found.
# LOCALE_LIST (string): semicolon-separated list of locales to initialize, 
#    merge or compile translation files for (ex: "fr;zh_CN;en").
# MO_BUILD_DIR (path): absolute path to the directory in the *build* tree 
#    where the binary translation files (mo) should be saved.
#    Default "${EXECUTABLE_OUTPUT_PATH}/../locale" if EXECUTABLE_OUTPUT_PATH
#    is found, "${CMAKE_CURRENT_BINARY_DIR}/locale" otherwise.
#
# Optional arguments:
# SOURCES (list): list of source files the translation template file (pot)
#    will be (re)-generated from.
# POT_BUILD_DIR (path): absolute path to the directory in the *build* tree
#    where up-to-date translation template file (pot) should be stored. 
#    Default to "${CMAKE_CURRENT_BINARY_DIR}/po"  if not found.
# PO_BUILD_DIR (path): absolute path to the directory in the *build* tree
#    where up-to-date translation files (po) should be stored. 
#    Default to "${CMAKE_CURRENT_BINARY_DIR}/po"  if not found.
# PO_PREFIX (string): string that will be used to prefix the filename of
#    each translation file.
#    Default to the value of "${DOMAIN_NAME}_"
# MO_INSTALL_DIR (path): directory where the binary translation files (mo)
#    should be installed to.
# COPYRIGHT_HOLDER (string): copyright holder string that will be stored in
#    the translation template file (pot). 
#    Default to the empty string if not found.
# MSGID_BUGS_ADDRESS (string): report address for msgid bugs that will be stored in
#    the translation template file (pot). 
#    Default to the empty string if not found.
# DEFAULT_PO_ENCODING (string): default encoding to be used when initializing
#    new translation file (po) for each locale. This will not change the
#    encoding of existing translation file (po).
#    Default to "utf-8" (Unicode) if not found.
# EXTRA_GETTEXT_KEYWORDS (string): semicolon-separated list of extra keywords
#    that should be recognized as a call to the gettext() function.
# EXTRA_DGETTEXT_KEYWORDS (string): semicolon-separated list of extra keywords
#    that should be recognized as a call to the dgettext() function.
#
# Target arguments:
# By default, custom commands are created to create all the files, as well
# as *one* custom target that can be triggered to refresh all the files.
# This custom target can be added to the 'ALL' target, but is not by default
# as modifying any source file would trigger: re-generating the template
# file (pot), merging all translation files (po), and compiling them into
# binary files (mo).
# TARGET_BASENAME (string): basename of all targets (a suffix is added 
#    depending of each target).
#    Default to ${DOMAIN_NAME} if specified, ${PROJECT_NAME} otherwise.
# CREATE_POT_TARGET (boolean): create a target for the template file (pot),
#    using the '_pot' suffix. 
#    Default to 0.
# CREATE_PO_TARGET (boolean): create one unique target for all translation
#    files (po), using the '_po' suffix. Can be used to refresh all PO files.
#    Depends on the pot file.
#    Default to 0.
# CREATE_PO_LOCALE_TARGETS (boolean): create one target per locale 
#    translation file (po), using the '_po_locale' suffix (say '_po_fr'). Can
#    be used to refresh a single PO file. Depends on the pot file.
#    Default to 0.
# CREATE_MO_TARGET (boolean): create one unique target for all binary
#    translation files (mo), using the '_mo' suffix. Can be used to refresh all
#    MO files (i.e. everything in the translation pipeline). Depends on each
#    single po file. Can be added to the 'ALL' target using 
#    ADD_MO_TARGET_TO_ALL (CREATE_MO_TARGET will therefore be considered true).
#    Default to 1.
# CREATE_MO_LOCALE_TARGETS (boolean): create one target per locale binary
#    translation file (mo), using the '_mo_locale' suffix (say '_mo_fr'). Can
#    be used to refresh a single MO file. Depends on the same po file.
#    Default to 0.
# ADD_MO_TARGET_TO_ALL: add the unique MO target to the 'ALL' target. Doing
#    so is likely to trigger all translation targets each time a source
#    file is modified and compiled. This automatically creates the unique
#    target for all binary translation files (mo), just like if 
#    CREATE_MO_TARGET was true.
#    Default to 0.

CMAKE_MINIMUM_REQUIRED(VERSION 2.2.2) # We need FILE(RELATIVE_PATH...)

MACRO(KWWidgets_CREATE_GETTEXT_TARGETS)

  SET(notset_value             "__not_set__")

  # Provide some reasonable defaults

  SET(domain_name              ${PROJECT_NAME})
  SET(po_dir                   "${CMAKE_CURRENT_SOURCE_DIR}/po")
  SET(po_build_dir             "${CMAKE_CURRENT_BINARY_DIR}/po")
  SET(pot_build_dir            "${CMAKE_CURRENT_BINARY_DIR}/po")
  SET(locale_list              "")
  SET(default_po_encoding      "utf-8")
  SET(mo_install_dir           "")
  SET(copyright_holder         "")
  SET(msgid_bugs_address       "foo@bar.com")
  SET(sources                  )
  SET(po_prefix                ${notset_value})
  SET(extra_gettext_keywords   "")
  SET(extra_dgettext_keywords  "")

  SET(target_basename          ${notset_value})
  SET(create_pot_target        0)
  SET(create_po_target         0)
  SET(create_po_locale_targets 0)
  SET(create_mo_target         1)
  SET(create_mo_locale_targets 0)
  SET(add_mo_target_to_all     0)

  IF(EXECUTABLE_OUTPUT_PATH)
    GET_FILENAME_COMPONENT(
      mo_build_dir "${EXECUTABLE_OUTPUT_PATH}/../locale" ABSOLUTE)
  ELSE(EXECUTABLE_OUTPUT_PATH)
    SET(mo_build_dir "${CMAKE_CURRENT_BINARY_DIR}/locale")
  ENDIF(EXECUTABLE_OUTPUT_PATH)

  # Parse the arguments

  SET(valued_parameter_names "^(TARGET_BASENAME|DOMAIN_NAME|POT_BUILD_DIR|PO_DIR|PO_BUILD_DIR|DEFAULT_PO_ENCODING|MO_BUILD_DIR|MO_INSTALL_DIR|COPYRIGHT_HOLDER|MSGID_BUGS_ADDRESS|PO_PREFIX)$")
  SET(boolean_parameter_names "^(ADD_MO_TARGET_TO_ALL|CREATE_POT_TARGET|CREATE_PO_TARGET|CREATE_PO_LOCALE_TARGETS|CREATE_MO_TARGET|CREATE_MO_LOCALE_TARGETS)$")
  SET(list_parameter_names "^(SOURCES|LOCALE_LIST|EXTRA_GETTEXT_KEYWORDS|EXTRA_DGETTEXT_KEYWORDS)$")

  SET(next_arg_should_be_value 0)
  SET(prev_arg_was_boolean 0)
  SET(prev_arg_was_list 0)
  SET(unknown_parameters)
  
  STRING(REGEX REPLACE ";;" ";FOREACH_FIX;" parameter_list "${ARGV}")
  FOREACH(arg ${parameter_list})

    IF("${arg}" STREQUAL "FOREACH_FIX")
      SET(arg "")
    ENDIF("${arg}" STREQUAL "FOREACH_FIX")

    SET(matches_valued 0)
    IF("${arg}" MATCHES ${valued_parameter_names})
      SET(matches_valued 1)
    ENDIF("${arg}" MATCHES ${valued_parameter_names})

    SET(matches_boolean 0)
    IF("${arg}" MATCHES ${boolean_parameter_names})
      SET(matches_boolean 1)
    ENDIF("${arg}" MATCHES ${boolean_parameter_names})

    SET(matches_list 0)
    IF("${arg}" MATCHES ${list_parameter_names})
      SET(matches_list 1)
    ENDIF("${arg}" MATCHES ${list_parameter_names})
      
    IF(matches_valued OR matches_boolean OR matches_list)
      IF(prev_arg_was_boolean)
        STRING(TOLOWER ${prev_arg_name} prev_arg_name)
        SET(${prev_arg_name} 1)
      ELSE(prev_arg_was_boolean)
        IF(next_arg_should_be_value AND NOT prev_arg_was_list)
          MESSAGE(FATAL_ERROR 
            "Found ${arg} instead of value for ${prev_arg_name}")
        ENDIF(next_arg_should_be_value AND NOT prev_arg_was_list)
      ENDIF(prev_arg_was_boolean)
      SET(next_arg_should_be_value 1)
      SET(prev_arg_was_boolean ${matches_boolean})
      SET(prev_arg_was_list ${matches_list})
      SET(prev_arg_name ${arg})
    ELSE(matches_valued OR matches_boolean OR matches_list)
      IF(next_arg_should_be_value)
        IF(prev_arg_was_boolean)
          IF(NOT "${arg}" STREQUAL "1" AND NOT "${arg}" STREQUAL "0")
            MESSAGE(FATAL_ERROR 
              "Found ${arg} instead of 0 or 1 for ${prev_arg_name}")
          ENDIF(NOT "${arg}" STREQUAL "1" AND NOT "${arg}" STREQUAL "0")
        ENDIF(prev_arg_was_boolean)
        STRING(TOLOWER ${prev_arg_name} prev_arg_name)
        IF(prev_arg_was_list)
          SET(${prev_arg_name} ${${prev_arg_name}} ${arg})
        ELSE(prev_arg_was_list)
          SET(${prev_arg_name} ${arg})
          SET(next_arg_should_be_value 0)
        ENDIF(prev_arg_was_list)
      ELSE(next_arg_should_be_value)
        SET(unknown_parameters ${unknown_parameters} ${arg})
      ENDIF(next_arg_should_be_value)
      SET(prev_arg_was_boolean 0)
    ENDIF(matches_valued OR matches_boolean OR matches_list)

  ENDFOREACH(arg)

  IF(next_arg_should_be_value)
    IF(prev_arg_was_boolean)
      STRING(TOLOWER ${prev_arg_name} prev_arg_name)
      SET(${prev_arg_name} 1)
    ELSE(prev_arg_was_boolean)
      IF(prev_arg_was_list)
        STRING(TOLOWER ${prev_arg_name} prev_arg_name)
        SET(${prev_arg_name} ${${prev_arg_name}} ${arg})
      ELSE(prev_arg_was_list)
        MESSAGE(FATAL_ERROR "Missing value for ${prev_arg_name}")
      ENDIF(prev_arg_was_list)
    ENDIF(prev_arg_was_boolean)
  ENDIF(next_arg_should_be_value)
  IF(unknown_parameters)
    MESSAGE(FATAL_ERROR "Unknown parameter(s): ${unknown_parameters}")
  ENDIF(unknown_parameters)

  # Fix some defaults

  IF(${target_basename} STREQUAL ${notset_value})
    SET(target_basename ${domain_name})
  ENDIF(${target_basename} STREQUAL ${notset_value})

  IF(${po_prefix} STREQUAL ${notset_value})
    SET(po_prefix "${domain_name}_")
  ENDIF(${po_prefix} STREQUAL ${notset_value})

  # Create the targets

  IF(NOT "${sources}" STREQUAL "")
    KWWidgets_CREATE_POT_TARGET(
      "${domain_name}"
      "${pot_build_dir}"
      "${po_dir}"
      "${copyright_holder}"
      "${msgid_bugs_address}"
      "${sources}"
      "${target_basename}"
      "${create_pot_target}"
      "${extra_gettext_keywords}"
      "${extra_dgettext_keywords}"
      )
  ENDIF(NOT "${sources}" STREQUAL "")
  
  KWWidgets_CREATE_PO_TARGETS(
    "${domain_name}"
    "${pot_build_dir}"
    "${po_dir}"
    "${po_build_dir}"
    "${po_prefix}"
    "${locale_list}"
    "${default_po_encoding}"
    "${target_basename}"
    "${create_po_target}"
    "${create_po_locale_targets}"
    )

  KWWidgets_CREATE_MO_TARGETS(
    "${domain_name}"
    "${po_dir}"
    "${po_build_dir}"
    "${po_prefix}"
    "${locale_list}"
    "${mo_build_dir}"
    "${mo_install_dir}"
    "${target_basename}"
    "${create_mo_target}"
    "${create_mo_locale_targets}"
    "${add_mo_target_to_all}"
    )

ENDMACRO(KWWidgets_CREATE_GETTEXT_TARGETS)

# ---------------------------------------------------------------------------
# KWWidgets_GET_POT_FILENAME
# Given a translation domain and the location of a directory, return the
# filename to the domain template file (pot).
# 'varname': name of the var the template filename should be stored into
# 'domain_name': translation domain name (i.e. name of application or library)
# 'pot_build_dir': path in the build tree where the template should be stored
 
MACRO(KWWidgets_GET_POT_FILENAME varname domain_name pot_build_dir)

  SET(${varname} "${pot_build_dir}/${domain_name}.pot")

ENDMACRO(KWWidgets_GET_POT_FILENAME)

# ---------------------------------------------------------------------------
# KWWidgets_GET_PO_FILENAME
# Given a PO directory, a prefix and a locale, return the filename to the
# translation file (po) for that locale.
# 'varname': name of the var the translation filename should be stored into
# 'po_dir': path to the po directory where the PO file are stored
# 'po_prefix': string that is used to prefix each translation file.
# 'locale': a locale (say, "fr")
 
MACRO(KWWidgets_GET_PO_FILENAME varname po_dir po_prefix locale)

  SET(${varname} "${po_dir}/${po_prefix}${locale}.po")

ENDMACRO(KWWidgets_GET_PO_FILENAME)

# ---------------------------------------------------------------------------
# KWWidgets_GET_PO_SAFE_BUILD_DIR
# Given a PO directory, a PO build directory, returns either the
# PO build directory if it is different than the PO directory, or
# a directory in the build tree. This macro is used to get a safe place
# to write PO related files
# 'varname': name of the var the PO safe build dir should be stored into
# 'po_dir': path to the po directory where the PO file are stored
# 'po_build_dir': build path where up-to-date PO files should be stored
 
MACRO(KWWidgets_GET_PO_SAFE_BUILD_DIR varname po_dir po_build_dir)

  IF("${po_build_dir}" STREQUAL "${po_dir}")
    SET(${varname} "${CMAKE_CURRENT_BINARY_DIR}/po")
    #SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM 1)
  ELSE("${po_build_dir}" STREQUAL "${po_dir}")
    SET(${varname} "${po_build_dir}")
  ENDIF("${po_build_dir}" STREQUAL "${po_dir}")

ENDMACRO(KWWidgets_GET_PO_SAFE_BUILD_DIR)

# ---------------------------------------------------------------------------
# KWWidgets_GET_MO_FILENAME
# Given a translation domain, a MO build directory, and a locale, return the
# filename to the binary translation file (mo) for that locale and domain.
# 'varname': name of the var the translation filename should be stored into
# 'domain_name': translation domain name (i.e. name of application or library)
# 'mo_build_dir': directory where the binary MO files should be saved to
# 'locale': a locale (say, "fr")
 
MACRO(KWWidgets_GET_MO_FILENAME varname domain_name mo_build_dir locale)

  SET(${varname} "${mo_build_dir}/${locale}/LC_MESSAGES/${domain_name}.mo")

ENDMACRO(KWWidgets_GET_MO_FILENAME)

# ---------------------------------------------------------------------------
# KWWidgets_GET_ABSOLUTE_SOURCES
# Given a list of sources, return the corresponding absolute paths
# 'varname': name of the var the list of absolute paths should be stored into
# 'sources': list of source files

MACRO(KWWidgets_GET_ABSOLUTE_SOURCES varname sources)

  SET(${varname})
  FOREACH(file ${sources})
    GET_FILENAME_COMPONENT(abs_file ${file} ABSOLUTE)
    IF(NOT EXISTS ${abs_file})
      SET(abs_file "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    ENDIF(NOT EXISTS ${abs_file})
    SET(${varname} ${${varname}} ${abs_file})
  ENDFOREACH(file)

ENDMACRO(KWWidgets_GET_ABSOLUTE_SOURCES)

# ---------------------------------------------------------------------------
# KWWidgets_GET_RELATIVE_SOURCES
# Given a list of sources, return the corresponding relative paths to
# a directory.
# 'varname': name of the var the list of absolute paths should be stored into
# 'dir': path to the dir we want relative path from
# 'sources': list of *absolute* path to the source files

MACRO(KWWidgets_GET_RELATIVE_SOURCES varname dir sources)

  GET_FILENAME_COMPONENT(dir_abs ${dir} ABSOLUTE)

  SET(${varname})
  FOREACH(file ${sources})
    FILE(RELATIVE_PATH rel_file "${dir}" "${file}")
    SET(${varname} ${${varname}} ${rel_file})
  ENDFOREACH(file)

ENDMACRO(KWWidgets_GET_RELATIVE_SOURCES)

# ---------------------------------------------------------------------------
# KWWidgets_CREATE_POT_TARGET
# Given a domain name, the location of a PO directory, and a list of sources,
# create a custom command/target to generate a translation template file (pot)
# from the source files.
# 'domain_name': translation domain name (i.e. name of application or library)
# 'pot_build_dir': path in the build tree where the template should be stored
# 'po_dir': path to the po directory where the original PO files are found
# 'copyright_holder': optional copyright holder of the template file
# 'msgid_bugs_address': report address for msgid bugs in the template file
# 'sources': list of source files the template file will be generated from
# 'target_basename': basename of the template file target
# 'create_pot_target': if true, create pot target (on top of the command)
# 'extra_gettext_keywords': semicolon-separated list of extra gettext keywords
# 'extra_dgettext_keywords':semicolon-separated list of extra dgettext keywords

MACRO(KWWidgets_CREATE_POT_TARGET
    domain_name
    pot_build_dir
    po_dir
    copyright_holder
    msgid_bugs_address
    sources
    target_basename 
    create_pot_target
    extra_gettext_keywords
    extra_dgettext_keywords
    )

  KWWidgets_GET_POT_FILENAME(pot_build_file 
    "${domain_name}" "${pot_build_dir}")

  # We need the absolute path to each source file

  KWWidgets_GET_ABSOLUTE_SOURCES(abs_sources "${sources}")

  # Put the list on sources to internationalize in an internal cache var
  # so that sub-projects can use it to create their own translation

  SET(${domain_name}_INTERNATIONALIZED_SRCS_INTERNAL "${abs_sources}"
    CACHE INTERNAL "Sources that were internationalized for ${domain_name}")

  # Get relative sources to the PO files

  KWWidgets_GET_RELATIVE_SOURCES(rel_sources "${po_dir}" "${abs_sources}")

  # The extra keywords

  SET(keywords)
  FOREACH(keyword ${extra_gettext_keywords})
    SET(keywords ${keywords} 
      "--keyword=${keyword}" "--flag=${keyword}:1:pass-c-format")
  ENDFOREACH(keyword)
  FOREACH(keyword ${extra_dgettext_keywords})
    SET(keywords ${keywords} 
      "--keyword=${keyword}:2" "--flag=${keyword}:2:pass-c-format")
  ENDFOREACH(keyword)

  # Define a symbol in each source file that can be used by dgettext

  SET_SOURCE_FILES_PROPERTIES(${sources}
    COMPILE_FLAGS "-DGETTEXT_DOMAIN=\\\"${domain_name}\\\"")  

  FILE(MAKE_DIRECTORY ${pot_build_dir})

  # Output the list of sources to a file. This fill will be read
  # by xgettext (so that we do not have to pass it as a huge command
  # line argument below)

  KWWidgets_GET_PO_SAFE_BUILD_DIR(
    safe_build_dir "${po_dir}" "${pot_build_dir}")

  SET(files_from "${safe_build_dir}/${domain_name}_srcs.txt")

  STRING(REGEX REPLACE ";" "\n" contents "${rel_sources}")
  FILE(WRITE "${files_from}" "${contents}")
  #CONFIGURE_FILE(${KWWidgets_TEMPLATES_DIR}/KWWidgetsContents.in ${files_from})

  # We need a dummy file that will just say: this POT target is up to date as
  # far as its dependencies are concerned. This will prevent the POT
  # target to be triggered again and again because the sources are older
  # than the POT, but the POT does not really need to be changed, etc.

  KWWidgets_GET_POT_FILENAME(pot_uptodate_file 
    "${domain_name}" "${safe_build_dir}")
  SET(pot_uptodate_file "${pot_uptodate_file}.upd")

  # Extract strings to translate to template file (pot)

  IF(NOT "${GETTEXT_XGETTEXT_EXECUTABLE}" STREQUAL "")
    SET(options "--foreign-user")
    SET(keywords ${keyword}
      "--keyword=_" "--flag=_:1:pass-c-format"
      "--keyword=N_" "--flag=N_:1:pass-c-format"
      "--flag=autosprintf:1:c-format"
      "--keyword=kww_sgettext" "--flag=kww_sgettext:1:pass-c-format"
      "--keyword=kww_sdgettext:2" "--flag=kww_sdgettext:2:pass-c-format"
      "--keyword=k_" "--flag=k_:1:pass-c-format"
      "--keyword=ks_" "--flag=ks_:1:pass-c-format"
      "--keyword=s_" "--flag=s_:1:pass-c-format"
      "--flag=kww_printf:1:c-format" 
      "--flag=kww_sprintf:2:c-format" 
      "--flag=kww_fprintf:2:c-format")

    ADD_CUSTOM_COMMAND(
      OUTPUT "${pot_uptodate_file}"
      DEPENDS ${abs_sources}
      COMMAND ${CMAKE_COMMAND} 
      ARGS -E chdir "${po_dir}" ${CMAKE_COMMAND} -DCMAKE_BACKWARDS_COMPATIBILITY:STRING=${CMAKE_BACKWARDS_COMPATIBILITY} -Dpot_build_file:STRING=${pot_build_file} -Dpot_uptodate_file:STRING=${pot_uptodate_file} -Dpo_dir:STRING=${po_dir} -Doptions:STRING="${options}" -Dkeywords:STRING="${keywords}" -Dcopyright_holder:STRING="${copyright_holder}" -Dmsgid_bugs_address:STRING="${msgid_bugs_address}" -Dfiles_from:STRING=${files_from} -DGETTEXT_XGETTEXT_EXECUTABLE:STRING=${GETTEXT_XGETTEXT_EXECUTABLE} -P "${KWWidgets_CMAKE_DIR}/KWWidgetsGettextExtract.cmake")
    IF(create_pot_target)
      ADD_CUSTOM_TARGET(${target_basename}_pot DEPENDS ${pot_build_file})
    ENDIF(create_pot_target)
  ENDIF(NOT "${GETTEXT_XGETTEXT_EXECUTABLE}" STREQUAL "")

ENDMACRO(KWWidgets_CREATE_POT_TARGET)

# ---------------------------------------------------------------------------
# KWWidgets_CREATE_PO_TARGETS
# Given a domain name, the location of a PO build directory, and a list of
# locales create multiple custom commands/targets to initialize and/or merge 
# the translation files (po) for each locale. Each translation file 
# 'po_build_dir'/locale.po (say, 'po_build_dir'/fr.po) is either initialized
# from or merged against the translation template file in the 'pot_build_dir' 
# directory for the same domain (say, 'pot_build_dir'/'domain_name'.pot). 
# The default encoding of each newly initialized PO file can be specified too.
# 'domain_name': translation domain name (i.e. name of application or library)
# 'pot_build_dir': path in the build tree where the template should be stored
# 'po_dir': path to where the original PO file are found
# 'po_build_dir': build path where up-to-date PO files should be stored
# 'po_prefix': string that will be used to prefix each translation file.
# 'locale_list': semicolon-separated list of locale to generate targets for.
# 'default_po_encoding': default encoding for new initialized PO files.
# 'target_basename': basename of the PO targets
# 'create_po_target': create one unique target for all locale PO files
# 'create_po_locale_targets': create one target per locale PO file

MACRO(KWWidgets_CREATE_PO_TARGETS
    domain_name
    pot_build_dir
    po_dir
    po_build_dir
    po_prefix
    locale_list
    default_po_encoding
    target_basename
    create_po_target
    create_po_locale_targets
    )

  KWWidgets_GET_POT_FILENAME(pot_build_file 
    "${domain_name}" "${pot_build_dir}")

  FILE(MAKE_DIRECTORY ${po_build_dir})

  # We need dummy files that will just say: this PO target is up to date as 
  # far as its dependencies are concerned. This will prevent the PO
  # targets to be triggered again and again because the POT file is older
  # than the PO, but the PO does not really need to be changed, etc.

  KWWidgets_GET_PO_SAFE_BUILD_DIR(safe_build_dir "${po_dir}" "${po_build_dir}")

  KWWidgets_GET_POT_FILENAME(pot_uptodate_file 
    "${domain_name}" "${safe_build_dir}")
  SET(pot_uptodate_file "${pot_uptodate_file}.upd")

  SET(po_build_files)

  FOREACH(locale ${locale_list})
    KWWidgets_GET_PO_FILENAME(po_file 
      "${po_dir}" "${po_prefix}" "${locale}")
    KWWidgets_GET_PO_FILENAME(po_build_file 
      "${po_build_dir}" "${po_prefix}" "${locale}")
    KWWidgets_GET_PO_FILENAME(po_uptodate_file 
      "${safe_build_dir}" "${po_prefix}" "${locale}")
    SET(po_uptodate_file "${po_uptodate_file}.upd")
    SET(po_uptodate_files ${po_uptodate_files} ${po_uptodate_file})
    SET(depends "${pot_uptodate_file}")
    IF(EXISTS "${po_file}")
      SET(depends ${depends} "${po_file}")
    ENDIF(EXISTS "${po_file}")
    ADD_CUSTOM_COMMAND(
      OUTPUT "${po_uptodate_file}"
      DEPENDS ${depends}
      COMMAND ${CMAKE_COMMAND} 
      ARGS -DCMAKE_BACKWARDS_COMPATIBILITY:STRING=${CMAKE_BACKWARDS_COMPATIBILITY} -Dpo_file:STRING=${po_file} -Dpo_build_file:STRING=${po_build_file} -Dpo_uptodate_file:STRING=${po_uptodate_file} -Ddefault_po_encoding:STRING=${default_po_encoding} -Dpot_build_file:STRING=${pot_build_file} -Dlocale:STRING=${locale} -DGETTEXT_MSGINIT_EXECUTABLE:STRING=${GETTEXT_MSGINIT_EXECUTABLE} -DGETTEXT_MSGCONV_EXECUTABLE:STRING=${GETTEXT_MSGCONV_EXECUTABLE} -DGETTEXT_MSGMERGE_EXECUTABLE:STRING=${GETTEXT_MSGMERGE_EXECUTABLE} -DGETTEXT_MSGCAT_EXECUTABLE:STRING=${GETTEXT_MSGCAT_EXECUTABLE} -P "${KWWidgets_CMAKE_DIR}/KWWidgetsGettextInitOrMerge.cmake"
      )
    IF(create_po_locale_targets)
      ADD_CUSTOM_TARGET(
        ${target_basename}_po_${locale} DEPENDS ${po_uptodate_file})
    ENDIF(create_po_locale_targets)
  ENDFOREACH(locale ${locale_list})

  IF(create_po_target)
    ADD_CUSTOM_TARGET(${target_basename}_po DEPENDS ${po_uptodate_files})
  ENDIF(create_po_target)

ENDMACRO(KWWidgets_CREATE_PO_TARGETS)

# ---------------------------------------------------------------------------
# KWWidgets_CREATE_MO_TARGETS
# Given a domain name, the location of a PO directory, a list of locales, the
# location of a MO build and install dir, create multiple custom 
# commands/targets to compile the translation files (po) for each locale into
# a binary translation files (mo). Each translation file is found in the
# PO directory as 'locale.po' (say, fr.po) and compiled into a binary 
# translation file in 'mo_build_dir'/locale/LC_MESSAGES/'domain_name'.mo 
# (say, 'mo_build_dir'/fr/LC_MESSAGES/'domain_name'.mo).
# 'domain_name': translation domain name (i.e. name of application or library)
# 'po_dir': path to where the original PO file are found
# 'po_build_dir': build path to where up-to-date PO files are stored
# 'po_prefix': string that is used to prefix each translation file.
# 'locale_list': semicolon-separated list of locale to generate targets for.
# 'mo_build_dir': directory where the binary MO files should be saved to
# 'mo_install_dir': directory where the binary MO files should be installed to
# 'target_basename': basename of the MO targets
# 'create_mo_target': create one unique target for all locale MO files
# 'create_mo_locale_targets': create one target per locale MO file
# 'add_mo_target_to_all': if true, add the unique MO target to the 'ALL' target

MACRO(KWWidgets_CREATE_MO_TARGETS
    domain_name
    po_dir
    po_build_dir
    po_prefix
    locale_list
    mo_build_dir
    mo_install_dir
    target_basename
    create_mo_target
    create_mo_locale_targets
    add_mo_target_to_all
    )

  SET(mo_files)

  KWWidgets_GET_PO_SAFE_BUILD_DIR(safe_build_dir "${po_dir}" "${po_build_dir}")

  IF(NOT "${GETTEXT_MSGFMT_EXECUTABLE}" STREQUAL "")

    FOREACH(locale ${locale_list})
      KWWidgets_GET_PO_FILENAME(po_build_file 
        "${po_build_dir}" "${po_prefix}" "${locale}")
      KWWidgets_GET_PO_FILENAME(po_uptodate_file 
        "${safe_build_dir}" "${po_prefix}" "${locale}")
      SET(po_uptodate_file "${po_uptodate_file}.upd")
      KWWidgets_GET_MO_FILENAME(mo_file 
        "${domain_name}" "${mo_build_dir}" "${locale}")
      GET_FILENAME_COMPONENT(mo_dir "${mo_file}" PATH)
      FILE(MAKE_DIRECTORY ${mo_dir})
      SET(mo_files ${mo_files} ${mo_file})
      # --check-accelerators 
      ADD_CUSTOM_COMMAND(
        OUTPUT "${mo_file}"
        DEPENDS "${po_uptodate_file}"
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} 
        ARGS --output-file=${mo_file} --check-format "${po_build_file}"
        )
      IF(create_mo_locale_targets)
        ADD_CUSTOM_TARGET(${target_basename}_mo_${locale} DEPENDS ${mo_file})
      ENDIF(create_mo_locale_targets)
      
      IF(NOT "${mo_install_dir}" STREQUAL "")
        INSTALL_FILES(
          "${mo_install_dir}/${locale}/LC_MESSAGES" FILES ${mo_file})
      ENDIF(NOT "${mo_install_dir}" STREQUAL "")
    ENDFOREACH(locale ${locale_list})

    IF(create_mo_target OR add_mo_target_to_all)
      SET(target_name "${target_basename}_mo")
      IF(add_mo_target_to_all)
        ADD_CUSTOM_TARGET(${target_name} ALL DEPENDS ${mo_files})
      ELSE(add_mo_target_to_all)
        ADD_CUSTOM_TARGET(${target_name} DEPENDS ${mo_files})
      ENDIF(add_mo_target_to_all)
    ENDIF(create_mo_target OR add_mo_target_to_all)
    
  ENDIF(NOT "${GETTEXT_MSGFMT_EXECUTABLE}" STREQUAL "")

ENDMACRO(KWWidgets_CREATE_MO_TARGETS)
