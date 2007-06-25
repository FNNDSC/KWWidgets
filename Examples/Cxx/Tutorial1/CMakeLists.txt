PROJECT(Tutorial1)

# Find KWWidgets

FIND_PACKAGE(KWWidgets REQUIRED)
INCLUDE(${KWWidgets_USE_FILE})

# The name of our executable and the corresponding source files.

SET(STEP "1" CACHE STRING "Select which step of the tutorial to build")

SET(EXE_NAME "KWTutorial1")
SET(EXE_SRCS 
  "${EXE_NAME}.cxx"
  ${CMAKE_CURRENT_SOURCE_DIR}/Step${STEP}/vtkKWMyWidget.cxx
  )

# Create the executable, and link it against the KWWidgets libraries

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/Step${STEP}
)
ADD_EXECUTABLE(${EXE_NAME} WIN32 ${EXE_SRCS})
TARGET_LINK_LIBRARIES(${EXE_NAME} ${KWWidgets_LIBRARIES})

# Since this tutorial is a standalone external project:
# - Generate a few small scripts (.bat, .sh, .csh) that can be sourced to set
# the various environments variables (PATH, TCLLIBPATH, LD_LIBRARY_PATH, etc.) 
# required by this executable and its known third-party dependencies (VTK, ITK,
# SOV, KWWidgets, etc.).
# - Generate a lightweight C launcher for this *specific* executable: It sets
# the above environment variables before launching the executable itself.

INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
KWWidgets_GENERATE_SETUP_PATHS_SCRIPTS("${CMAKE_CURRENT_BINARY_DIR}")
SET(LAUNCHER_EXE_NAME "${EXE_NAME}Launcher")
KWWidgets_GENERATE_SETUP_PATHS_LAUNCHER(
  "${CMAKE_CURRENT_BINARY_DIR}" "${LAUNCHER_EXE_NAME}" "" "${EXE_NAME}")

# If needed, copy the Tcl/Tk support files required at run-time 
# to initialize Tcl/Tk. This is only triggered if VTK was built
# against a Tcl/Tk static library.

INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
IF(NOT KWWidgets_SOURCE_DIR)
  KWWidgets_COPY_TCL_TK_SUPPORT_FILES("${PROJECT_BINARY_DIR}/lib")
ENDIF(NOT KWWidgets_SOURCE_DIR)
