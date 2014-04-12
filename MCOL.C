/*----------------------------------------------------------------------*/
/*  Defines and Include Files                                           */
/*----------------------------------------------------------------------*/
#define INCL_PM
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include  "mcol.h"

/*----------------------------------------------------------------------*/
/*  Holds the widths of each individual column                          */
/*----------------------------------------------------------------------*/
LONG        alWidth[3];

/*----------------------------------------------------------------------*/
/*  Prototypes                                                          */
/*----------------------------------------------------------------------*/

void    main        ( int argc, char *argv[] );
MRESULT EXPENTRY MColDlgProc  ( HWND, USHORT, MPARAM, MPARAM );

LONG EXPENTRY LBMeasure(HWND hwnd, USHORT idListBox, USHORT idItem,
                LONG alColWidth[], USHORT usMaxCols, USHORT idHdg,
                PSZ pszHdgTop, PSZ pszHdgBottom);

VOID EXPENTRY LBDraw(HWND hwnd, USHORT idListBox, POWNERITEM poi,
                LONG alColWidth[], USHORT usMaxCols, USHORT idHdg,
                BOOL fGrid);

void ExtractCols ( char *pszText, ... );
MRESULT EXPENTRY EditWndProc    ( HWND, USHORT, MPARAM, MPARAM );
MRESULT EXPENTRY HeadingWndProc ( HWND, USHORT, MPARAM, MPARAM );

PFNWP       pfnEntryWndProc;

void main ( int argc, char *argv[] )
{

    HAB          hab;
    HMQ          hmq;
    CLASSINFO    clsInfo;

    hab = WinInitialize ( 0 );
    hmq = WinCreateMsgQueue ( hab, 0 );

    WinQueryClassInfo ( hab, WC_ENTRYFIELD, &clsInfo );

    WinRegisterClass ( hab, "Heading",  HeadingWndProc,  0, 18 );
    WinRegisterClass ( hab, "9", EditWndProc, CS_SIZEREDRAW, clsInfo.cbWindowData );
    WinRegisterClass ( hab, "a", EditWndProc, CS_SIZEREDRAW, clsInfo.cbWindowData );
    WinRegisterClass ( hab, "N", EditWndProc, CS_SIZEREDRAW, clsInfo.cbWindowData );

    pfnEntryWndProc = clsInfo.pfnWindowProc;

    WinDlgBox ( HWND_DESKTOP, HWND_DESKTOP, MColDlgProc, 0, IDD_MCOL, 0 );

    WinDestroyMsgQueue ( hmq );
    WinTerminate       ( hab );
}


MRESULT EXPENTRY MColDlgProc  ( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2 ) {

    char    szLine[100], szField1[33], szField2[33], szField3[33];
    USHORT  usId;

    switch ( msg )  {

    case WM_CONTROL      :
        switch ( SHORT1FROMMP ( mp1 ) )  {
        case DID_LISTBOX :
            usId = SHORT1FROMMR ( WinSendDlgItemMsg ( hwnd, DID_LISTBOX,
                    LM_QUERYSELECTION, MPFROMSHORT ( LIT_FIRST ), 0 ) );

            if ( usId == LIT_NONE )  {
                break;
            }
            WinSendDlgItemMsg ( hwnd, DID_LISTBOX, LM_QUERYITEMTEXT,
                MPFROM2SHORT ( usId, sizeof ( szLine ) ), szLine );

            ExtractCols ( szLine, szField1, szField2, szField3 );
            WinSetDlgItemText ( hwnd, DID_FIELD1, szField1 );
            WinSetDlgItemText ( hwnd, DID_FIELD2, szField2 );
            WinSetDlgItemText ( hwnd, DID_FIELD3, szField3 );
        }
        break;

    case WM_COMMAND      :
        switch ( SHORT1FROMMP ( mp1 ) )  {
        case DID_CANCEL               :
            WinDismissDlg ( hwnd, 0 );
            break;

        case DID_ADD                  :
            WinQueryDlgItemText ( hwnd, DID_FIELD1, 33, szField1 );
            WinQueryDlgItemText ( hwnd, DID_FIELD2, 33, szField2 );
            WinQueryDlgItemText ( hwnd, DID_FIELD3, 33, szField3 );

            sprintf ( szLine, "%s\t%s\t%s", szField1, szField2, szField3 );

            WinEnableWindowUpdate ( WinWindowFromID ( hwnd, DID_LISTBOX ), FALSE );
            WinSendDlgItemMsg ( hwnd, DID_LISTBOX, LM_INSERTITEM, (MPARAM) LIT_END,
                    (MPARAM) szLine );
            WinShowWindow ( WinWindowFromID ( hwnd, DID_LISTBOX ), TRUE );

            break;
        }
        break;

    case WM_MEASUREITEM          :
        switch ( SHORT1FROMMP ( mp1 ) )  {
        case   DID_LISTBOX       :
            return (MRESULT) ( LBMeasure ( hwnd, SHORT1FROMMP ( mp1 ),
                    SHORT1FROMMP ( mp2 ), alWidth, 3, DID_HEADING,
                    "column\tcolumn\tcolumn", "1\t2\t3" ) );
        }
        break;

    case WM_DRAWITEM             :
        switch ( SHORT1FROMMP ( mp1 ) )  {
        case   DID_LISTBOX       :
            LBDraw ( hwnd, SHORT1FROMMP ( mp1 ), (POWNERITEM) mp2,
                     alWidth, 3, DID_HEADING, FALSE );
            return (MRESULT) TRUE;

        }
        break;

    default                     :
        return WinDefDlgProc ( hwnd, msg, mp1, mp2 );
    }
    return  0;
}
