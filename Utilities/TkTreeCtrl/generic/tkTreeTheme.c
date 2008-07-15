/* 
 * tkTreeTheme.c --
 *
 *  This module implements platform-specific visual themes.
 *
 * Copyright (c) 2006 Tim Baker
 *
 * RCS: @(#) $Id: tkTreeTheme.c,v 1.5 2008-07-15 22:21:04 barre Exp $
 */

#if defined(WIN32) && !defined(WINVER)
#define WINVER 0x0501 /* Cygwin */
#endif

#include "tkTreeCtrl.h"

/* These must agree with tkTreeColumn.c */
#define COLUMN_STATE_NORMAL 0
#define COLUMN_STATE_ACTIVE 1
#define COLUMN_STATE_PRESSED 2

#ifdef WIN32
#include "vtkWin32Header.h" // vtkSetWindowLong
#include "tkWinInt.h"

#include <uxtheme.h>
#include <tmschema.h>

#ifndef DTBG_OMITBORDER
#  ifndef DTBG_CLIPRECT
#   define DTBG_CLIPRECT        0x00000001
#  endif
#  ifndef DTBG_DRAWSOLID
#   define DTBG_DRAWSOLID       0x00000002
#  endif
#  ifndef DTBG_OMITBORDER
#   define DTBG_OMITBORDER      0x00000004
#  endif
#  ifndef DTBG_OMITCONTENT
#   define DTBG_OMITCONTENT     0x00000008
#  endif
#  ifndef DTBG_COMPUTINGREGION
#   define DTBG_COMPUTINGREGION 0x00000010
#  endif
#  ifndef DTBG_MIRRORDC
#   define DTBG_MIRRORDC        0x00000020
#  endif
    typedef struct _DTBGOPTS
    {
  DWORD dwSize;
  DWORD dwFlags;
  RECT rcClip;
    } DTBGOPTS, *PDTBGOPTS;
#endif // _DTBGOPTS

#include <basetyps.h> /* Cygwin */
#ifndef TMT_CONTENTMARGINS
#define TMT_CONTENTMARGINS 3602
#endif

typedef HTHEME (STDAPICALLTYPE OpenThemeDataProc)(HWND hwnd,
    LPCWSTR pszClassList);
typedef HRESULT (STDAPICALLTYPE CloseThemeDataProc)(HTHEME hTheme);
typedef HRESULT (STDAPICALLTYPE DrawThemeBackgroundProc)(HTHEME hTheme,
    HDC hdc, int iPartId, int iStateId, const RECT *pRect,
    OPTIONAL const RECT *pClipRect);
typedef HRESULT (STDAPICALLTYPE DrawThemeBackgroundExProc)(HTHEME hTheme,
    HDC hdc, int iPartId, int iStateId, const RECT *pRect,
    DTBGOPTS *PDTBGOPTS);
typedef HRESULT (STDAPICALLTYPE DrawThemeParentBackgroundProc)(HWND hwnd,
    HDC hdc, OPTIONAL const RECT *prc);
typedef HRESULT (STDAPICALLTYPE DrawThemeEdgeProc)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT *pDestRect,
    UINT uEdge, UINT uFlags, RECT *pContentRect);
typedef HRESULT (STDAPICALLTYPE DrawThemeTextProc)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount,
    DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect);
typedef HRESULT (STDAPICALLTYPE GetThemeBackgroundContentRectProc)(
    HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
    const RECT *pBoundingRect, RECT *pContentRect);
typedef HRESULT (STDAPICALLTYPE GetThemeBackgroundExtentProc)(HTHEME hTheme,
    HDC hdc, int iPartId, int iStateId, const RECT *pContentRect,
    RECT *pExtentRect);
typedef HRESULT (STDAPICALLTYPE GetThemeMarginsProc)(HTHEME, HDC,
    int iPartId, int iStateId, int iPropId, OPTIONAL RECT *prc,
    MARGINS *pMargins);
typedef HRESULT (STDAPICALLTYPE GetThemePartSizeProc)(HTHEME, HDC, int iPartId,
    int iStateId, RECT *prc, enum THEMESIZE eSize, SIZE *psz);
typedef HRESULT (STDAPICALLTYPE GetThemeTextExtentProc)(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, LPCWSTR pszText, int iCharCount,
    DWORD dwTextFlags, const RECT *pBoundingRect, RECT *pExtentRect);
typedef BOOL (STDAPICALLTYPE IsThemeActiveProc)(VOID);
typedef BOOL (STDAPICALLTYPE IsThemePartDefinedProc)(HTHEME, int, int);
typedef HRESULT (STDAPICALLTYPE IsThemeBackgroundPartiallyTransparentProc)(
    HTHEME, int, int);

typedef struct
{
    OpenThemeDataProc        *OpenThemeData;
    CloseThemeDataProc        *CloseThemeData;
    DrawThemeBackgroundProc      *DrawThemeBackground;
    DrawThemeBackgroundExProc      *DrawThemeBackgroundEx;
    DrawThemeParentBackgroundProc    *DrawThemeParentBackground;
    DrawThemeEdgeProc        *DrawThemeEdge;
    DrawThemeTextProc        *DrawThemeText;
    GetThemeBackgroundContentRectProc    *GetThemeBackgroundContentRect;
    GetThemeBackgroundExtentProc    *GetThemeBackgroundExtent;
    GetThemeMarginsProc        *GetThemeMargins;
    GetThemePartSizeProc      *GetThemePartSize;
    GetThemeTextExtentProc      *GetThemeTextExtent;
    IsThemeActiveProc        *IsThemeActive;
    IsThemePartDefinedProc      *IsThemePartDefined;
    IsThemeBackgroundPartiallyTransparentProc   *IsThemeBackgroundPartiallyTransparent;
} XPThemeProcs;

typedef struct
{
    HINSTANCE hlibrary;
    XPThemeProcs *procs;
    int registered;
    int themeEnabled;
    SIZE buttonOpen;
    SIZE buttonClosed;
} XPThemeData;

typedef struct TreeThemeData_
{
    HTHEME hThemeHEADER;
    HTHEME hThemeTREEVIEW;
} TreeThemeData_;

static XPThemeProcs *procs = NULL;
static XPThemeData *appThemeData = NULL; 
TCL_DECLARE_MUTEX(themeMutex)

/*
 *----------------------------------------------------------------------
 *
 * LoadXPThemeProcs --
 *  Initialize XP theming support.
 *
 *  XP theme support is included in UXTHEME.DLL
 *  We dynamically load this DLL at runtime instead of linking
 *  to it at build-time.
 *
 * Returns:
 *  A pointer to an XPThemeProcs table if successful, NULL otherwise.
 */

static XPThemeProcs *
LoadXPThemeProcs(HINSTANCE *phlib)
{
    OSVERSIONINFO os;

    /*
     * We have to check whether we are running at least on Windows XP.
     * In order to determine this we call GetVersionEx directly, although
     * it would be a good idea to wrap it inside a function similar to
     * TkWinGetPlatformId...
     */
    ZeroMemory(&os, sizeof(os));
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&os);
    if (os.dwMajorVersion >= 5 && os.dwMinorVersion >= 1) {
  /*
   * We are running under Windows XP or a newer version.
   * Load the library "uxtheme.dll", where the native widget
   * drawing routines are implemented.
   */
  HINSTANCE handle;
  *phlib = handle = LoadLibrary("uxtheme.dll");
  if (handle != 0)
  {
      /*
       * We have successfully loaded the library. Proceed in storing the
       * addresses of the functions we want to use.
       */
      XPThemeProcs *procs = (XPThemeProcs*)ckalloc(sizeof(XPThemeProcs));
#define LOADPROC(name) \
  (0 != (procs->name = (name ## Proc *)GetProcAddress(handle, #name) ))

      if (   LOADPROC(OpenThemeData)
    && LOADPROC(CloseThemeData)
    && LOADPROC(DrawThemeBackground)
    && LOADPROC(DrawThemeBackgroundEx)
    && LOADPROC(DrawThemeParentBackground)
    && LOADPROC(DrawThemeEdge)
    && LOADPROC(DrawThemeText)
    && LOADPROC(GetThemeBackgroundContentRect)
    && LOADPROC(GetThemeBackgroundExtent)
    && LOADPROC(GetThemeMargins)
    && LOADPROC(GetThemePartSize)
    && LOADPROC(GetThemeTextExtent)
    && LOADPROC(IsThemeActive)
    && LOADPROC(IsThemePartDefined)
    && LOADPROC(IsThemeBackgroundPartiallyTransparent)
      )
      {
    return procs;
      }
#undef LOADPROC
      ckfree((char*)procs);
  }
    }
    return 0;
}

int TreeTheme_DrawHeaderItem(TreeCtrl *tree, Drawable drawable, int state,
    int arrow, int x, int y, int width, int height)
{
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    RECT rc;
    HRESULT hr;

    int iPartId = HP_HEADERITEM;
    int iStateId = HIS_NORMAL;

    switch (state)
    {
  case COLUMN_STATE_ACTIVE:  iStateId = HIS_HOT; break;
  case COLUMN_STATE_PRESSED: iStateId = HIS_PRESSED; break;
    }

    if (!appThemeData->themeEnabled || !procs)
  return TCL_ERROR;

    hTheme = tree->themeData->hThemeHEADER;
    if (!hTheme)
  return TCL_ERROR;

#if 0 /* Always returns FALSE */
    if (!procs->IsThemePartDefined(
  hTheme,
  iPartId,
  iStateId))
    {
  return TCL_ERROR;
    }
#endif

    rc.left = x;
    rc.top = y;
    rc.right = x + width;
    rc.bottom = y + height;

    /* Is transparent for the default XP style. */
    if (procs->IsThemeBackgroundPartiallyTransparent(
  hTheme,
  iPartId,
  iStateId))
    {
#if 1
  /* What color should I use? */
  Tk_Fill3DRectangle(tree->tkwin, drawable, tree->border, x, y, width, height, 0, TK_RELIEF_FLAT);
#else
  /* This draws nothing, maybe because the parent window is not
   * themed */
  procs->DrawThemeParentBackground(
      hwnd,
      hDC,
      &rc);
#endif
    }

    hDC = TkWinGetDrawableDC(tree->display, drawable, &dcState);

#if 0
    {
  /* Default XP theme gives rect 3 pixels narrower than rc */
  RECT contentRect, extentRect;
  hr = procs->GetThemeBackgroundContentRect(
      hTheme,
      hDC,
      iPartId,
      iStateId,
      &rc,
      &contentRect
  );
  dbwin("GetThemeBackgroundContentRect width=%d height=%d\n",
      contentRect.right - contentRect.left,
      contentRect.bottom - contentRect.top);

  /* Gives rc */
  hr = procs->GetThemeBackgroundExtent(
      hTheme,
      hDC,
      iPartId,
      iStateId,
      &contentRect,
      &extentRect
  );
  dbwin("GetThemeBackgroundExtent width=%d height=%d\n",
      extentRect.right - extentRect.left,
      extentRect.bottom - extentRect.top);
    }
#endif

    hr = procs->DrawThemeBackground(
  hTheme,
  hDC,
  iPartId,
  iStateId,
  &rc,
  NULL);

    TkWinReleaseDrawableDC(drawable, hDC, &dcState);

    if (hr != S_OK)
  return TCL_ERROR;

    return TCL_OK;
}

int TreeTheme_GetHeaderContentMargins(TreeCtrl *tree, int state, int arrow, int bounds[4])
{
    Window win = Tk_WindowId(tree->tkwin);
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    HRESULT hr;
    MARGINS margins;

    int iPartId = HP_HEADERITEM;
    int iStateId = HIS_NORMAL;

    switch (state)
    {
  case COLUMN_STATE_ACTIVE:  iStateId = HIS_HOT; break;
  case COLUMN_STATE_PRESSED: iStateId = HIS_PRESSED; break;
    }

    if (!appThemeData->themeEnabled || !procs)
  return TCL_ERROR;

    hTheme = tree->themeData->hThemeHEADER;
    if (!hTheme)
  return TCL_ERROR;

    hDC = TkWinGetDrawableDC(tree->display, win, &dcState);

    /* The default XP themes give 3,0,0,0 which makes little sense since
     * it is the *right* side that should not be drawn over by text; the
     * 2-pixel wide header divider is on the right */
    hr = procs->GetThemeMargins(
  hTheme,
  hDC,
  iPartId,
  iStateId,
  TMT_CONTENTMARGINS,
  NULL,
  &margins);

    TkWinReleaseDrawableDC(win, hDC, &dcState);

    if (hr != S_OK)
  return TCL_ERROR;

    bounds[0] = margins.cxLeftWidth;
    bounds[1] = margins.cyTopHeight;
    bounds[2] = margins.cxRightWidth;
    bounds[3] = margins.cyBottomHeight;
/*
dbwin("margins %d %d %d %d\n", bounds[0], bounds[1], bounds[2], bounds[3]);
*/
    return TCL_OK;
}

int TreeTheme_DrawHeaderArrow(TreeCtrl *tree, Drawable drawable, int up,
    int x, int y, int width, int height)
{
#if 1
    XColor *color;
    GC gc;
    int i;

    if (!appThemeData->themeEnabled || !procs)
  return TCL_ERROR;

    color = Tk_GetColor(tree->interp, tree->tkwin, "#ACA899");
    gc = Tk_GCForColor(color, drawable);

    if (up) {
  for (i = 0; i < height; i++) {
      XDrawLine(tree->display, drawable, gc,
    x + width / 2 - i, y + i,
    x + width / 2 + i + 1, y + i);
  }
    } else {
  for (i = 0; i < height; i++) {
      XDrawLine(tree->display, drawable, gc,
    x + width / 2 - i, y + (height - 1) - i,
    x + width / 2 + i + 1, y + (height - 1) - i);
  }
    }

    Tk_FreeColor(color);
    return TCL_OK;
#else
    /* Doesn't seem that Microsoft actually implemented this */
    Window win = Tk_WindowId(tree->tkwin);
    HWND hwnd = Tk_GetHWND(win);
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    RECT rc;
    HRESULT hr;

    int iPartId = HP_HEADERSORTARROW;
    int iStateId = up ? HSAS_SORTEDUP : HSAS_SORTEDDOWN;

    if (!appThemeData->themeEnabled || !procs)
  return TCL_ERROR;

    hTheme = tree->themeData->hThemeHEADER;
    if (!hTheme)
  return TCL_ERROR;

    if (!procs->IsThemePartDefined(
  hTheme,
  iPartId,
  iStateId))
    {
  return TCL_ERROR;
    }

    hDC = TkWinGetDrawableDC(tree->display, drawable, &dcState);

    rc.left = x;
    rc.top = y;
    rc.right = x + width;
    rc.bottom = y + height;

    hr = procs->DrawThemeBackground(
  hTheme,
  hDC,
  iPartId,
  iStateId,
  &rc,
  NULL);

    TkWinReleaseDrawableDC(drawable, hDC, &dcState);
    return TCL_OK;
#endif /* 0 */
}

int TreeTheme_DrawButton(TreeCtrl *tree, Drawable drawable, int open,
    int x, int y, int width, int height)
{
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    RECT rc;
    HRESULT hr;
    int iPartId, iStateId;

    if (!appThemeData->themeEnabled || !procs)
  return TCL_ERROR;

    iPartId  = TVP_GLYPH;
    iStateId = open ? GLPS_OPENED : GLPS_CLOSED;

    hTheme = tree->themeData->hThemeTREEVIEW;
    if (!hTheme)
  return TCL_ERROR;

#if 0 /* Always returns FALSE */
    if (!procs->IsThemePartDefined(
  hTheme,
  iPartId,
  iStateId))
    {
  return TCL_ERROR;
    }
#endif

    hDC = TkWinGetDrawableDC(tree->display, drawable, &dcState);

    rc.left = x;
    rc.top = y;
    rc.right = x + width;
    rc.bottom = y + height;
    hr = procs->DrawThemeBackground(
  hTheme,
  hDC,
  iPartId,
  iStateId,
  &rc,
  NULL);

    TkWinReleaseDrawableDC(drawable, hDC, &dcState);

    if (hr != S_OK)
  return TCL_ERROR;

    return TCL_OK;
}

int TreeTheme_GetButtonSize(TreeCtrl *tree, Drawable drawable, int open,
    int *widthPtr, int *heightPtr)
{
    HTHEME hTheme;
    HDC hDC;
    TkWinDCState dcState;
    HRESULT hr;
    SIZE size;
    int iPartId, iStateId;

    if (!appThemeData->themeEnabled || !procs)
  return TCL_ERROR;

    /* Use cached values */
    size = open ? appThemeData->buttonOpen : appThemeData->buttonClosed;
    if (size.cx > 1) {
  *widthPtr = size.cx;
  *heightPtr = size.cy;
  return TCL_OK;
    }

    iPartId  = TVP_GLYPH;
    iStateId = open ? GLPS_OPENED : GLPS_CLOSED;

    hTheme = tree->themeData->hThemeTREEVIEW;
    if (!hTheme)
  return TCL_ERROR;

#if 0 /* Always returns FALSE */
    if (!procs->IsThemePartDefined(
  hTheme,
  iPartId,
  iStateId))
    {
  return TCL_ERROR;
    }
#endif

    hDC = TkWinGetDrawableDC(tree->display, drawable, &dcState);

    /* Returns 9x9 for default XP style */
    hr = procs->GetThemePartSize(
  hTheme,
  hDC,
  iPartId,
  iStateId,
  NULL,
  TS_DRAW,
  &size
    );

    TkWinReleaseDrawableDC(drawable, hDC, &dcState);

    /* With RandomN of 10000, I eventually get hr=E_HANDLE, invalid handle */
    /* Not any longer since I don't call OpenThemeData/CloseThemeData for
     * every call. */
    if (hr != S_OK)
  return TCL_ERROR;

    /* Gave me 0,0 for a non-default theme, even though glyph existed */
    if ((size.cx <= 1) && (size.cy <= 1))
  return TCL_ERROR;

    /* Cache the values */
    if (open)
  appThemeData->buttonOpen = size;
    else
  appThemeData->buttonClosed = size;

    *widthPtr = size.cx;
    *heightPtr = size.cy;
    return TCL_OK;
}

int TreeTheme_GetArrowSize(TreeCtrl *tree, Drawable drawable, int up, int *widthPtr, int *heightPtr)
{
    if (!appThemeData->themeEnabled || !procs)
  return TCL_ERROR;

    *widthPtr = 9;
    *heightPtr = 5;
    return TCL_OK;
}

#if !defined(WM_THEMECHANGED)
#define WM_THEMECHANGED 0x031A
#endif

static LRESULT WINAPI
WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  /*Tcl_Interp *interp = (Tcl_Interp *)GetWindowLong(hwnd, GWL_USERDATA); */
    Tcl_Interp *interp = (Tcl_Interp *)vtkGetWindowLong(hwnd,vtkGWL_USERDATA);

    switch (msg) {
  case WM_THEMECHANGED:
      Tcl_MutexLock(&themeMutex);
      appThemeData->themeEnabled = procs->IsThemeActive();
      appThemeData->buttonClosed.cx = appThemeData->buttonOpen.cx = -1;
      Tcl_MutexUnlock(&themeMutex);
      Tree_TheWorldHasChanged(interp);
      break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

static CHAR windowClassName[32] = "TreeCtrlMonitorClass";

static BOOL
RegisterThemeMonitorWindowClass(HINSTANCE hinst)
{
    WNDCLASSEX wc;
    
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC)WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hinst;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName  = windowClassName;
    wc.lpszClassName = windowClassName;
    
    return RegisterClassEx(&wc);
}

static HWND
CreateThemeMonitorWindow(HINSTANCE hinst, Tcl_Interp *interp)
{
    CHAR title[32] = "TreeCtrlMonitorWindow";
    HWND hwnd;

    hwnd = CreateWindow(windowClassName, title, WS_OVERLAPPEDWINDOW,
  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
  NULL, NULL, hinst, NULL);
    if (!hwnd)
  return NULL;

    /* SetWindowLong(hwnd, GWL_USERDATA, (LONG)interp); */
    vtkSetWindowLong(hwnd, vtkGWL_USERDATA, (vtkLONG)interp);

    ShowWindow(hwnd, SW_HIDE);
    UpdateWindow(hwnd);

    return hwnd;
}

typedef struct PerInterpData PerInterpData;
struct PerInterpData
{
    HWND hwnd;
};

static void FreeAssocData(ClientData clientData, Tcl_Interp *interp)
{
    PerInterpData *data = (PerInterpData *) clientData;

    DestroyWindow(data->hwnd);
    ckfree((char *) data);
}

void TreeTheme_ThemeChanged(TreeCtrl *tree)
{
    Window win = Tk_WindowId(tree->tkwin);
    HWND hwnd = Tk_GetHWND(win);

    if (tree->themeData->hThemeHEADER != NULL) {
  procs->CloseThemeData(tree->themeData->hThemeHEADER);
  tree->themeData->hThemeHEADER = NULL;
    }
    if (tree->themeData->hThemeTREEVIEW != NULL) {
  procs->CloseThemeData(tree->themeData->hThemeTREEVIEW);
  tree->themeData->hThemeTREEVIEW = NULL;
    }

    if (!appThemeData->themeEnabled || !procs)
  return;

    tree->themeData->hThemeHEADER = procs->OpenThemeData(hwnd, L"HEADER");
    tree->themeData->hThemeTREEVIEW = procs->OpenThemeData(hwnd, L"TREEVIEW");
}

int TreeTheme_Init(TreeCtrl *tree)
{
    Window win = Tk_WindowId(tree->tkwin);
    HWND hwnd = Tk_GetHWND(win);

    if (!appThemeData->themeEnabled || !procs)
  return TCL_ERROR;

    tree->themeData = (TreeThemeData) ckalloc(sizeof(TreeThemeData_));

    /* http://www.codeproject.com/cs/miscctrl/themedtabpage.asp?msg=1445385#xx1445385xx */
    /* http://msdn2.microsoft.com/en-us/library/ms649781.aspx */

    tree->themeData->hThemeHEADER = procs->OpenThemeData(hwnd, L"HEADER");
    tree->themeData->hThemeTREEVIEW = procs->OpenThemeData(hwnd, L"TREEVIEW");
    return TCL_OK;
}

int TreeTheme_Free(TreeCtrl *tree)
{
    if (tree->themeData != NULL) {
  if (tree->themeData->hThemeHEADER != NULL)
      procs->CloseThemeData(tree->themeData->hThemeHEADER);
  if (tree->themeData->hThemeTREEVIEW != NULL)
      procs->CloseThemeData(tree->themeData->hThemeTREEVIEW);
  ckfree((char *) tree->themeData);
    }
    return TCL_OK;
}

int TreeTheme_InitInterp(Tcl_Interp *interp)
{
    HWND hwnd;
    PerInterpData *data;

    Tcl_MutexLock(&themeMutex);

    /* This is done once per-application */
    if (appThemeData == NULL)
    {
  appThemeData = (XPThemeData *) ckalloc(sizeof(XPThemeData));
  appThemeData->procs = LoadXPThemeProcs(&appThemeData->hlibrary);
  appThemeData->registered = FALSE;
  appThemeData->themeEnabled = FALSE;
  appThemeData->buttonClosed.cx = appThemeData->buttonOpen.cx = -1;

  procs = appThemeData->procs;

  if (appThemeData->procs) {
      /* Check this again if WM_THEMECHANGED arrives */
      appThemeData->themeEnabled = procs->IsThemeActive();

      appThemeData->registered =
    RegisterThemeMonitorWindowClass(Tk_GetHINSTANCE());
  }
    }

    Tcl_MutexUnlock(&themeMutex);

    if (!procs || !appThemeData->registered)
  return TCL_ERROR;

    /* Per-interp */
    hwnd = CreateThemeMonitorWindow(Tk_GetHINSTANCE(), interp);
    if (!hwnd)
  return TCL_ERROR;

    data = (PerInterpData *) ckalloc(sizeof(PerInterpData));
    data->hwnd = hwnd;
    Tcl_SetAssocData(interp, "TreeCtrlTheme", FreeAssocData, (ClientData) data);

    return TCL_OK;
}

#elif defined(MAC_OSX_TK)

#include <Carbon/Carbon.h>
#include "tkMacOSXInt.h"

static RgnHandle oldClip = NULL, boundsRgn = NULL;

int TreeTheme_DrawHeaderItem(TreeCtrl *tree, Drawable drawable, int state,
    int arrow, int x, int y, int width, int height)
{
    MacDrawable *macWin = (MacDrawable *) drawable;
    Rect bounds;
    ThemeButtonDrawInfo info;
    CGrafPtr saveWorld;
    GDHandle saveDevice;
    GWorldPtr destPort;

    bounds.left = macWin->xOff + x;
    bounds.top = macWin->yOff + y;
    bounds.right = bounds.left + width;
    bounds.bottom = bounds.top + height;

    switch (state) {
  case COLUMN_STATE_ACTIVE:  info.state = kThemeStateActive /* kThemeStateRollover */; break;
  case COLUMN_STATE_PRESSED: info.state = kThemeStatePressed; break;
  default:       info.state = kThemeStateActive; break;
    }
    /* Background window */
    if (!tree->isActive)
  info.state = kThemeStateInactive;
    info.value = (arrow != 0) ? kThemeButtonOn : kThemeButtonOff;
    info.adornment = (arrow == 1) ? kThemeAdornmentHeaderButtonSortUp : kThemeAdornmentNone;

    destPort = TkMacOSXGetDrawablePort(drawable);
    GetGWorld(&saveWorld, &saveDevice);
    SetGWorld(destPort, 0);
    TkMacOSXSetUpClippingRgn(drawable);

    /* Save the old clipping region because we are going to modify it. */
    if (oldClip == NULL)
  oldClip = NewRgn();
    GetClip(oldClip);

    /* Create a clipping region as big as the header. */
    if (boundsRgn == NULL)
  boundsRgn = NewRgn();
    RectRgn(boundsRgn, &bounds);

    /* Set the clipping region to the intersection of the two regions. */
    SectRgn(oldClip, boundsRgn, boundsRgn);
    SetClip(boundsRgn);

    /* Draw the left edge outside of the clipping region. */
    bounds.left -= 1;

    (void) DrawThemeButton(&bounds, kThemeListHeaderButton, &info,
  NULL,  /*prevInfo*/
  NULL,  /*eraseProc*/
  NULL,  /*labelProc*/
  (UInt32)NULL);  /*userData*/

    SetClip(oldClip);
    SetGWorld(saveWorld,saveDevice);

    return TCL_OK;
}

/* List headers are a fixed height on Aqua */
int TreeTheme_GetHeaderFixedHeight(TreeCtrl *tree, int *heightPtr)
{
    SInt32 metric;

    GetThemeMetric(kThemeMetricListHeaderHeight, &metric);
    *heightPtr = metric;
    return TCL_OK;
}

int TreeTheme_GetHeaderContentMargins(TreeCtrl *tree, int state, int arrow, int bounds[4])
{
    Rect inBounds, outBounds;
    ThemeButtonDrawInfo info;
    SInt32 metric;

    inBounds.left = 0;
    inBounds.top = 0;
    inBounds.right = 100;
    GetThemeMetric(kThemeMetricListHeaderHeight, &metric);
    inBounds.bottom = metric;

    switch (state) {
  case COLUMN_STATE_ACTIVE:  info.state = kThemeStateActive /* kThemeStateRollover */; break;
  case COLUMN_STATE_PRESSED: info.state = kThemeStatePressed; break;
  default:       info.state = kThemeStateActive; break;
    }
    /* Background window */
    if (!tree->isActive)
  info.state = kThemeStateInactive;
    info.value = (arrow != 0) ? kThemeButtonOn : kThemeButtonOff;
    info.adornment = (arrow == 1) ? kThemeAdornmentHeaderButtonSortUp : kThemeAdornmentNone;

    (void) GetThemeButtonContentBounds(
  &inBounds,
  kThemeListHeaderButton,
  &info,
  &outBounds);

    bounds[0] = outBounds.left - inBounds.left;
    bounds[1] = outBounds.top - inBounds.top;
    bounds[2] = inBounds.right - outBounds.right;
    bounds[3] = inBounds.bottom - outBounds.bottom;

    return TCL_OK;
}

int TreeTheme_DrawHeaderArrow(TreeCtrl *tree, Drawable drawable, int up, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_DrawButton(TreeCtrl *tree, Drawable drawable, int open, int x, int y, int width, int height)
{
    MacDrawable *macWin = (MacDrawable *) drawable;
    Rect bounds;
    ThemeButtonDrawInfo info;
    CGrafPtr saveWorld;
    GDHandle saveDevice;
    GWorldPtr destPort;

    bounds.left = macWin->xOff + x;
    bounds.top = macWin->yOff + y;
    bounds.right = bounds.left + width;
    bounds.bottom = bounds.top + height;

    info.state = kThemeStateActive;
    info.value = open ? kThemeDisclosureDown : kThemeDisclosureRight;
    info.adornment = kThemeAdornmentNone;

    destPort = TkMacOSXGetDrawablePort(drawable);
    GetGWorld(&saveWorld, &saveDevice);
    SetGWorld(destPort, 0);
    TkMacOSXSetUpClippingRgn(drawable);

    /* Drawing the disclosure triangles produces a white background.
     * To avoid this, set the clipping region to the exact area where
     * pixels are drawn. */

    /* Save the old clipping region because we are going to modify it. */
    if (oldClip == NULL)
  oldClip = NewRgn();
    GetClip(oldClip);

    /* Create a clipping region containing the pixels of the button. */
    if (boundsRgn == NULL)
  boundsRgn = NewRgn();
    (void) GetThemeButtonRegion(&bounds, kThemeDisclosureButton, &info,
  boundsRgn);

    /* Set the clipping region to the intersection of the two regions. */
    SectRgn(oldClip, boundsRgn, boundsRgn);
    SetClip(boundsRgn);

    (void) DrawThemeButton(&bounds, kThemeDisclosureButton, &info,
  NULL,  /*prevInfo*/
  NULL,  /*eraseProc*/
  NULL,  /*labelProc*/
  (UInt32)NULL);  /*userData*/

    /* Restore the original clipping region. */
    SetClip(oldClip);

    SetGWorld(saveWorld,saveDevice);

    return TCL_OK;
}

int TreeTheme_GetButtonSize(TreeCtrl *tree, Drawable drawable, int open, int *widthPtr, int *heightPtr)
{
    SInt32 metric;

    (void) GetThemeMetric(
  kThemeMetricDisclosureTriangleWidth,
  &metric);
    *widthPtr = metric;

    (void) GetThemeMetric(
  kThemeMetricDisclosureTriangleHeight,
  &metric);
    *heightPtr = metric;

    return TCL_OK;
}

int TreeTheme_GetArrowSize(TreeCtrl *tree, Drawable drawable, int up, int *widthPtr, int *heightPtr)
{
    return TCL_ERROR;
}

void TreeTheme_ThemeChanged(TreeCtrl *tree)
{
}

int TreeTheme_Init(TreeCtrl *tree)
{
    return TCL_OK;
}

int TreeTheme_Free(TreeCtrl *tree)
{
    return TCL_OK;
}

int TreeTheme_InitInterp(Tcl_Interp *interp)
{
    return TCL_OK;
}

#else /* MAC_OSX_TK */

int TreeTheme_DrawHeaderItem(TreeCtrl *tree, Drawable drawable, int state, int arrow, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_GetHeaderContentMargins(TreeCtrl *tree, int state, int arrow, int bounds[4])
{
    return TCL_ERROR;
}

int TreeTheme_DrawHeaderArrow(TreeCtrl *tree, Drawable drawable, int up, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_DrawButton(TreeCtrl *tree, Drawable drawable, int open, int x, int y, int width, int height)
{
    return TCL_ERROR;
}

int TreeTheme_GetButtonSize(TreeCtrl *tree, Drawable drawable, int open, int *widthPtr, int *heightPtr)
{
    return TCL_ERROR;
}

int TreeTheme_GetArrowSize(TreeCtrl *tree, Drawable drawable, int up, int *widthPtr, int *heightPtr)
{
    return TCL_ERROR;
}

void TreeTheme_ThemeChanged(TreeCtrl *tree)
{
}

int TreeTheme_Init(TreeCtrl *tree)
{
    return TCL_OK;
}

int TreeTheme_Free(TreeCtrl *tree)
{
    return TCL_OK;
}

int TreeTheme_InitInterp(Tcl_Interp *interp)
{
    return TCL_OK;
}

#endif /* !WIN32 && !MAC_OSX_TK */

