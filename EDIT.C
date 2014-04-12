#define     INCL_GPIPRIMITIVES
#define     INCL_WIN
#include    <os2.h>

#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>

#include    "edit.h"

/*----------------------------------------------------------------------*/
/*  Prototype for edit class only.                                      */
/*----------------------------------------------------------------------*/
USHORT   EntryFldCharProc ( HWND    hwnd, SUBCLASSCTRLDATA  *pSubCls,
                               USHORT  key );

void     EntryFldChar     ( HWND, USHORT, MPARAM, MPARAM );

extern   PFNWP       pfnEntryWndProc;

MRESULT EXPENTRY EditWndProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
    switch ( msg )  {

    case WM_CREATE :
        EntryFldCreate ( hwnd, ( CREATESTRUCT * ) mp2 );
        pfnEntryWndProc  ( hwnd, msg, mp1, mp2 );
        if ( WinQueryWindowTextLength ( hwnd ) )
            WinSetWindowText ( hwnd, "" );
        return ( MRESULT ) FALSE;

    case WM_CHAR                  :
        EntryFldChar ( hwnd, msg, mp1, mp2 );
        return ( MRESULT ) TRUE;

    /*------------------------------------------------------------------*/
    /*  Release all the memory used to process this window.             */
    /*------------------------------------------------------------------*/
    case WM_DESTROY         :
        EntryFldDestroy ( hwnd );
        return ( MRESULT ) FALSE;

    case  WM_PAINT          :
        pfnEntryWndProc ( hwnd, msg, mp1, mp2 );
        EntryFldPaint ( hwnd );
        break;
    }                                   /* Switch : msg                 */
    return   pfnEntryWndProc ( hwnd, msg, mp1, mp2 );
}

/*----------------------------------------------------------------------*/
/*  EntryFldChar                                                     */
/*                                                                      */
/*      The big kahuna. This is what a entry field is all about. Make   */
/*      sure only worthy characters are allowed to be entered and put   */
/*      in the separating characters appropriatly.                      */
/*----------------------------------------------------------------------*/
void  EntryFldChar ( HWND    hwnd, USHORT  msg, MPARAM  mp1, MPARAM  mp2 ){

    SUBCLASSCTRLDATA  *pSubCls;
    USHORT                usFlags, vkey, key;
    UCHAR                 scancode;

    pSubCls  = WinQueryWindowPtr ( hwnd, QWL_USER );

    usFlags  = SHORT1FROMMP ( mp1 );
    vkey     = SHORT2FROMMP ( mp2 );
    scancode = (UCHAR)   CHAR4FROMMP ( mp1 );

    if ( usFlags & KC_KEYUP || ! ( usFlags & KC_CHAR  ) )  {
        pfnEntryWndProc ( hwnd, msg, mp1, mp2 );
        return;
    }

    switch ( vkey )  {
    case    VK_TAB          :
    case    VK_NEWLINE      :
    case    VK_ENTER        :
    case    VK_BACKSPACE    :
    case    VK_DELETE       :
    case    VK_BACKTAB      :
    case    VK_ESC          :
        pfnEntryWndProc ( hwnd, msg, mp1, mp2 );
        return;
    }

    if ( usFlags & KC_SHIFT )  {
        switch ( scancode )  {
        case  0x47   :                  /*  Home    */
        case  0x4B   :                  /*  Left    */
        case  0x4D   :                  /*  Right   */
        case  0x4F   :                  /*  End     */
            pfnEntryWndProc ( hwnd, msg, mp1, mp2 );
            return;
        }
    }

    if ( key = EntryFldCharProc ( hwnd, pSubCls,
                                    (USHORT) CHAR1FROMMP((ULONG) mp2) ))  {

        ((UCHAR *) &mp2)[0] = ( UCHAR ) key;
        pfnEntryWndProc ( hwnd, msg, mp1, mp2 );

    }
}

/*----------------------------------------------------------------------*/
/*    EntryFldCharProc                                               */
/*                                                                      */
/*      This function handles the code specific to real characters      */
/*      All virtual and editing characters have already been thrown     */
/*      away. Just validate each character and change the case if needed*/
/*----------------------------------------------------------------------*/
USHORT  EntryFldCharProc ( HWND    hwnd, SUBCLASSCTRLDATA  *pSubCls,
                              USHORT  key )  {

    BOOL          fValid = FALSE;

    /*------------------------------------------------------------------*/
    /*  Check the char entered against the base class for the entry     */
    /*  field.                                                          */
    /*------------------------------------------------------------------*/
    switch ( pSubCls->ucFldType )  {
    case   '9'   :
        fValid = isdigit ( key );
        break;

    case   'A'   :
        key    = toupper ( key );
    case   'a'   :
        fValid = isalpha ( key );
        break;

    case   'N'   :
        key = toupper ( key );
    case   'n'   :
        fValid = isalnum ( key );
        break;

    case   'X'   :
        fValid = ( isalnum ( key ) || key == ' ');
        break;
    }                           /* Switch : Field Type          */
    if ( ! fValid )  {
        WinAlarm ( HWND_DESKTOP, WA_NOTE );
        return  0;
    }
    else  {
        return  key;
    }
}

/*----------------------------------------------------------------------*/
/*  EntryFldCreate                                                   */
/*                                                                      */
/*      Initialize the Entry field control structure with the standard  */
/*      values. Required field, solid line, no character list, Black    */
/*----------------------------------------------------------------------*/
SUBCLASSCTRLDATA  *EntryFldCreate ( HWND  hwnd, CREATESTRUCT *pCr )  {

    SUBCLASSCTRLDATA  *pSubCls;
    ULONG                 ulStyle;

    /*--------------------------------------------------------------*/
    /*  Default Conditions                                          */
    /*      Line Type       :   Solid   No Error                    */
    /*      Char List       :   None                                */
    /*      Line Style      :   2       Required                    */
    /*      Field Type      :   Defined in .DLG                     */
    /*      Line Color      :   Black                               */
    /*--------------------------------------------------------------*/

    pSubCls  = malloc  ( sizeof ( SUBCLASSCTRLDATA ) );
    if ( pSubCls == NULL )
        return  NULL;

    memset ( pSubCls, 0, sizeof ( SUBCLASSCTRLDATA ) );

    WinSetWindowPtr ( hwnd, QWL_USER, pSubCls );

    pSubCls->fValidState  = TRUE;
    pSubCls->ulLineType   = LINETYPE_SOLID;
    pSubCls->ulLineColor  = SYSCLR_WINDOWFRAME;
    pSubCls->usLineStyle  = ES_REQUIRED;
    pSubCls->ulFlags      = 0;

    if ( pCr != NULL )  {
        pSubCls->ucFldType = pCr->pszClass[0];
    }
    /*------------------------------------------------------------------*/
    /*  If there is not a margin create one and while we are here turn  */
    /*  on auto scrolling                                               */
    /*------------------------------------------------------------------*/
    ulStyle = WinQueryWindowULong ( hwnd, QWL_STYLE );
    ulStyle |= ES_MARGIN | ES_AUTOSCROLL;

    /*------------------------------------------------------------------*/
    /*  Right Justify the entry field if it is a numeric field          */
    /*------------------------------------------------------------------*/
    if ( pSubCls->ucFldType == '9' )  {
        ulStyle |= ES_RIGHT;
    }
    WinSetWindowULong  ( hwnd, QWL_STYLE, ulStyle );

    return  pSubCls;
}

/*----------------------------------------------------------------------*/
/*  EntryFldDestroy                                                  */
/*                                                                      */
/*      Release all memory used by this window.                         */
/*----------------------------------------------------------------------*/
void    EntryFldDestroy ( HWND   hwnd )  {

    SUBCLASSCTRLDATA  *pSubCls;

    pSubCls  = WinQueryWindowPtr ( hwnd, QWL_USER );

    free ( pSubCls );
    WinSetWindowPtr ( hwnd, QWL_USER, NULL );
}

/*----------------------------------------------------------------------*/
/*  Get out your brushes and paint the window.                          */
/*  Put a border around the window as required either error border,     */
/*  required border or default border.                                  */
/*----------------------------------------------------------------------*/
void   EntryFldPaint ( HWND hwnd )  {

    SUBCLASSCTRLDATA  *pSubCls;
    HPS                   hps;
    SWP                   swp;
    RECTL                 rcl;
    POINTL                aptl[4];

    pSubCls = WinQueryWindowPtr ( hwnd, QWL_USER );

    hps = WinGetPS ( WinQueryWindow ( hwnd, QW_PARENT, 0 ) );

    WinQueryWindowPos ( hwnd, &swp );
                                                        /*--------------*/
    aptl[2].x = aptl[3].x = swp.x;                      /* Left         */
    aptl[0].x = aptl[1].x = swp.cx +  swp.x - 1;        /* Right        */
                                                        /*--------------*/
    aptl[0].y = aptl[3].y = swp.y;                      /* Bottom       */
    aptl[1].y = aptl[2].y = swp.cy + swp.y - 1;         /* Top          */
                                                        /*--------------*/
    GpiSetLineType ( hps, LINETYPE_SOLID );
    GpiMove        ( hps, &aptl[3] );
    GpiSetColor    ( hps, SYSCLR_DIALOGBACKGROUND );
    GpiPolyLine    ( hps, 4L, aptl );

    GpiSetLineType ( hps, pSubCls->ulLineType );
    GpiMove ( hps, &aptl[3] );

    switch   ( pSubCls->usLineStyle )   {
    case        ES_OPTIONAL          :  /*  Optional Border         */
    case        ES_REQUIRED          :  /*  Required Border         */
        GpiSetColor    ( hps, pSubCls->ulLineColor );
        break;
    case        ES_OPTIONAL_RDONLY   :  /*  Invisible Optional Border  */
    case        ES_REQUIRED_RDONLY   :  /*  Invisible Required Border  */
        GpiSetColor    ( hps, SYSCLR_DIALOGBACKGROUND );
        break;
    }
    GpiPolyLine  ( hps, 4L, aptl );
    WinReleasePS ( hps );

    WinQueryWindowRect ( hwnd, &rcl );
    WinValidateRect    ( hwnd, &rcl, FALSE );
    rcl.xLeft     += 2;
    rcl.xRight    -= 2;
    rcl.yTop      -= 2;
    rcl.yBottom   += 2;
    WinInvalidateRect  ( hwnd, &rcl, FALSE );
    WinShowWindow      ( hwnd, TRUE );

}
