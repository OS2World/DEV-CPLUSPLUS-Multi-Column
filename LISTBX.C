/*----------------------------------------------------------------------*/
/*  Defines and Include Files                                           */
/*----------------------------------------------------------------------*/
#define INCL_PM
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>


#define COLUMN_SEPARATOR        20

#define HEADING_MAXCOLS      0
#define HEADING_COLWIDTH     2
#define HEADING_TOP          6
#define HEADING_BOTTOM		10
#define HEADING_CURCOL		14


void  DrawLine ( HPS     hps,        HWND   hwnd,       RECTL  *prcl,
                 USHORT  fsCmd,      LONG   lForeColor, LONG   lBackColor,
                 char   *pszHeading, BOOL   fGrid );

void  MeasureLine ( unsigned char *pszColumn, LONG  alColWidth[],
                           USHORT  usMaxCols, LONG  alWidthTable[] );

/*======================================================================*/
/* Measure the text size for a owner drawn list box.			*/
/*======================================================================*/
LONG EXPENTRY LBMeasure(HWND hwnd, USHORT idListBox, USHORT idItem,
                LONG alColWidth[], USHORT usMaxCols, USHORT idHdg,
                PSZ pszHdgTop, PSZ pszHdgBottom) {

    USHORT          usTextLen;
    HPS             hps;
    FONTMETRICS     fm;
    LONG            alWidthTable[256];
    HWND            hwndHdg;
    BYTE           *pszBuf;
    USHORT          ilColumn;
    LONG            lWidth;

    /*------------------------------------------------------------------*/
    /* Save info in the heading window's reserved memory.               */
    /*------------------------------------------------------------------*/
    if ( idHdg )  {
        if ( ( hwndHdg = WinWindowFromID ( hwnd, idHdg ) ) != NULL )  {
            WinSetWindowUShort ( hwndHdg, HEADING_MAXCOLS,  usMaxCols    );
            WinSetWindowPtr    ( hwndHdg, HEADING_COLWIDTH, alColWidth   );
            WinSetWindowPtr    ( hwndHdg, HEADING_TOP,      pszHdgTop    );
            WinSetWindowPtr    ( hwndHdg, HEADING_BOTTOM,   pszHdgBottom );
            WinSetWindowULong  ( hwndHdg, HEADING_CURCOL,   0 );
        }
    }

    /*------------------------------------------------------------------*/
    /* Get the font metrics and char width table for the current font   */
    /*------------------------------------------------------------------*/
    hps = WinGetPS ( hwnd );
    GpiQueryFontMetrics ( hps, (LONG) sizeof(FONTMETRICS), &fm );
    GpiQueryWidthTable  ( hps, 1L, 255L, alWidthTable );
    WinReleasePS        ( hps );

    /*------------------------------------------------------------------*/
    /* Measure the top heading line.                                    */
    /*------------------------------------------------------------------*/
    if ( pszHdgTop )  {
        MeasureLine ( pszHdgTop, alColWidth, usMaxCols, alWidthTable );
    }

    /*------------------------------------------------------------------*/
    /* Measure the bottom heading line.                                 */
    /*------------------------------------------------------------------*/
    if ( pszHdgBottom )  {
        MeasureLine ( pszHdgBottom, alColWidth, usMaxCols, alWidthTable );
    }

    /*------------------------------------------------------------------*/
    /* Measure the text from the list box.                              */
    /*------------------------------------------------------------------*/
    usTextLen =  SHORT1FROMMR ( WinSendDlgItemMsg ( hwnd, idListBox, LM_QUERYITEMTEXTLENGTH,
        MPFROMSHORT ( idItem ), 0 ) );
    pszBuf = malloc ( usTextLen + 1 );

    WinSendDlgItemMsg ( hwnd, idListBox, LM_QUERYITEMTEXT,
        MPFROM2SHORT ( idItem, usTextLen + 1 ), MPFROMP(pszBuf) );

    MeasureLine ( pszBuf, alColWidth, usMaxCols, alWidthTable );

    /*------------------------------------------------------------------*/
    /* Calculate the width of the line.                                 */
    /*------------------------------------------------------------------*/
    for (ilColumn = 0, lWidth = 0; ilColumn < usMaxCols; ilColumn++)
            lWidth += alColWidth[ilColumn];

    lWidth -= COLUMN_SEPARATOR;

    free ( pszBuf );
    /*------------------------------------------------------------------*/
    /* Return the height and width.                                     */
    /*------------------------------------------------------------------*/
    return ( MAKELONG(fm.lMaxBaselineExt, lWidth) );
}

/*======================================================================*/
/* Draw the text for a owner drawn list box.				*/
/*======================================================================*/
VOID EXPENTRY LBDraw(HWND hwnd, USHORT idListBox, POWNERITEM poi,
                LONG alColWidth[], USHORT usMaxCols, USHORT idHdg,
		BOOL fGrid) {

    USHORT          usTextLen;
    LONG            lForeColor, lBackColor, lHdgColumn;
    HWND            hwndHdg;
    CHAR           *pszBuf;
    LONG            alWidthTable[256];
    FONTMETRICS     fm;
    HPS             hps;

    /*------------------------------------------------------------------*/
    /* Get info from the heading window's reserved memory.              */
    /*------------------------------------------------------------------*/
    if ( idHdg )  {
        hwndHdg    = WinWindowFromID     ( hwnd, idHdg );
        lHdgColumn = WinQueryWindowULong ( hwndHdg, HEADING_CURCOL );

        /*--------------------------------------------------------------*/
        /* If we've scrolled horizontally, re-paint the heading.        */
        /*--------------------------------------------------------------*/
        if ( lHdgColumn != poi->rclItem.xLeft )  {
            WinSetWindowULong ( hwndHdg, HEADING_CURCOL, poi->rclItem.xLeft );

            WinInvalidateRect ( hwndHdg, NULL, FALSE );
            WinUpdateWindow   ( hwndHdg );
        }
    }

    /*------------------------------------------------------------------*/
    /* Get the text to be drawn.                                        */
    /*------------------------------------------------------------------*/
    usTextLen =  SHORT1FROMMR ( WinSendDlgItemMsg ( hwnd, idListBox, LM_QUERYITEMTEXTLENGTH,
        MPFROMSHORT ( poi->idItem ), 0 ) );
    pszBuf = malloc ( usTextLen + 1 );

    WinSendDlgItemMsg ( hwnd, idListBox, LM_QUERYITEMTEXT,
        MPFROM2SHORT(poi->idItem, usTextLen + 1), MPFROMP(pszBuf) );

    /*------------------------------------------------------------------*/
    /* If fsState is TRUE, text is to be hilited.  Otherwise, use       */
    /* default colors.                                                  */
    /*------------------------------------------------------------------*/
    if (poi->fsState) {
            lForeColor = SYSCLR_HILITEFOREGROUND;
            lBackColor = SYSCLR_HILITEBACKGROUND;
    }
    else {
            lForeColor = SYSCLR_WINDOWTEXT;
            lBackColor = SYSCLR_WINDOW;
    }

    /*------------------------------------------------------------------*/
    /* Clear the rectangle.                                             */
    /*------------------------------------------------------------------*/
    WinFillRect ( poi->hps, &poi->rclItem, lBackColor);

    /*------------------------------------------------------------------*/
    /* Get the font metrics and character width table for the           */
    /* current font.                                                    */
    /*------------------------------------------------------------------*/
    hps = WinGetPS ( hwnd );
    GpiQueryFontMetrics ( hps, (LONG) sizeof(FONTMETRICS), &fm );
    GpiQueryWidthTable  ( hps, 1L, 255L, alWidthTable );
    WinReleasePS ( hps );

    MeasureLine ( pszBuf, alColWidth, usMaxCols, alWidthTable );
    /*------------------------------------------------------------------*/
    /* Get the first column.                                            */
    /*------------------------------------------------------------------*/

    DrawLine ( poi->hps, hwndHdg, &poi->rclItem, DT_TOP, lForeColor,
               lBackColor, pszBuf, fGrid );

    free ( pszBuf );

    /*------------------------------------------------------------------*/
    /* Tell PM what we've done.                                         */
    /*------------------------------------------------------------------*/
    poi->fsState = poi->fsStateOld = FALSE;
    return;
}

/*======================================================================*/
/* Window procedure for ADSLBHeading class.				*/
/*======================================================================*/
MRESULT EXPENTRY HeadingWndProc(HWND hwnd, USHORT msg, MPARAM mp1,
		MPARAM mp2) {

    HPS             hps;
    RECTL           rclWindow;

    switch ( msg )  {
    case WM_PAINT:
        /*--------------------------------------------------------------*/
        /* Get the size of the rectangle.                               */
        /*--------------------------------------------------------------*/
        hps = WinBeginPaint ( hwnd, NULL, NULL );
        WinQueryWindowRect  ( hwnd, &rclWindow );

        /*--------------------------------------------------------------*/
        /* Clear the rectangle.                                         */
        /*--------------------------------------------------------------*/
        WinFillRect ( hps, &rclWindow, SYSCLR_DIALOGBACKGROUND );

        /*--------------------------------------------------------------*/
        /* Draw the top heading line.                                   */
        /*--------------------------------------------------------------*/
        DrawLine ( hps, hwnd, &rclWindow, DT_TOP, SYSCLR_WINDOWSTATICTEXT, SYSCLR_DIALOGBACKGROUND,
                   WinQueryWindowPtr ( hwnd, HEADING_TOP ), FALSE );

        /*--------------------------------------------------------------*/
        /* Draw the bottom heading line.                                */
        /*--------------------------------------------------------------*/
        DrawLine ( hps, hwnd, &rclWindow, DT_BOTTOM, SYSCLR_WINDOWSTATICTEXT, SYSCLR_DIALOGBACKGROUND,
                   WinQueryWindowPtr ( hwnd, HEADING_BOTTOM ), FALSE );

        WinEndPaint ( hps );
        break;
    }

    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

void  MeasureLine ( unsigned char *pszCol, LONG alColWidth[], USHORT usMaxCols, LONG  alWidthTable[] )
{
    PSZ             pszNextCol;
    USHORT          usCol;
    LONG            lWidth;

    /*------------------------------------------------------------------*/
    /* Loop processing each column.                                     */
    /*------------------------------------------------------------------*/
    for ( usCol = 0; pszCol  &&  usCol < usMaxCols; usCol++ )  {
        /*--------------------------------------------------------------*/
        /* NULL terminate the column.                                   */
        /*--------------------------------------------------------------*/
        if ( ( pszNextCol = strchr ( pszCol, '\t' ) ) != NULL )
            *pszNextCol++ = 0;

        /*--------------------------------------------------------------*/
        /* Calculate the width of the column.                           */
        /*--------------------------------------------------------------*/
        for ( lWidth = 0; *pszCol; pszCol++ )  {
            if ( *pszCol == '\r' ||  *pszCol == '\n' )
                continue;

            lWidth += alWidthTable[*pszCol - 1];
        }

        lWidth += COLUMN_SEPARATOR;

        /*--------------------------------------------------------------*/
        /* Save width of data in column table.                          */
        /*--------------------------------------------------------------*/
        if ( lWidth > alColWidth[usCol] )
            alColWidth[usCol] = lWidth;

        /*--------------------------------------------------------------*/
        /* Get the next column.                                         */
        /*--------------------------------------------------------------*/
        pszCol = pszNextCol;
        if ( pszNextCol != NULL )
            pszNextCol[-1] = '\t';
    }
}

void  DrawLine ( HPS     hps,   HWND   hwnd,       RECTL  *prcl,
                 USHORT  fsCmd, LONG   lForeColor, LONG   lBackColor,
                 char  *pszHdg, BOOL fGrid )  {

    CHAR           *pszCol, *pszNextCol;
    USHORT          usCol, usMaxCols;
    LONG           *plColWidth, lHeadingColumn;

    /*------------------------------------------------------------------*/
    /* Get info from the heading window's reserved memory.              */
    /*------------------------------------------------------------------*/
    usMaxCols        = WinQueryWindowUShort ( hwnd, HEADING_MAXCOLS  );
    plColWidth       = WinQueryWindowPtr    ( hwnd, HEADING_COLWIDTH );
    lHeadingColumn   = WinQueryWindowULong  ( hwnd, HEADING_CURCOL   );

    /*------------------------------------------------------------------*/
    /* Draw heading line.                                               */
    /*------------------------------------------------------------------*/
    if ( pszHdg )
        pszCol = pszHdg;
    else
        pszCol = NULL;

    /*------------------------------------------------------------------*/
    /* Use xLeft from list box for heading.                             */
    /*------------------------------------------------------------------*/
    prcl->xLeft = lHeadingColumn;

    /*------------------------------------------------------------------*/
    /* Loop processing each column.                                     */
    /*------------------------------------------------------------------*/
    for ( usCol = 0; pszCol && usCol < usMaxCols; usCol++ )  {
        /*--------------------------------------------------------------*/
        /* NULL terminate the column.                                   */
        /*--------------------------------------------------------------*/
        if ( ( pszNextCol = strchr ( pszCol, '\t' ) ) != NULL )
            *pszNextCol++ = 0;

        /*--------------------------------------------------------------*/
        /* Calculate the right side of the  rectangle for this column.  */
        /*--------------------------------------------------------------*/
        prcl->xRight = prcl->xLeft + plColWidth[usCol] - COLUMN_SEPARATOR;

        /*--------------------------------------------------------------*/
        /* Draw the text either center, left or right justified.        */
        /*--------------------------------------------------------------*/
        if ( *pszCol == '\n' )
            WinDrawText ( hps, strlen ( pszCol + 1 ), pszCol + 1, prcl,
                lForeColor, lBackColor, fsCmd | DT_CENTER );
        else if ( *pszCol == '\r' )
            WinDrawText ( hps, strlen ( pszCol + 1 ), pszCol + 1, prcl,
                lForeColor, lBackColor, fsCmd | DT_RIGHT );
        else
            WinDrawText ( hps, strlen ( pszCol ), pszCol, prcl,
                lForeColor, lBackColor, fsCmd | DT_LEFT );

        /*--------------------------------------------------------------*/
        /* If specified, draw a grid.                                   */
        /*--------------------------------------------------------------*/
        if ( fGrid )  {
            prcl->xLeft  -= COLUMN_SEPARATOR / 2;
            prcl->xRight += COLUMN_SEPARATOR / 2;

            WinDrawBorder ( hps, prcl, 1, 1, lForeColor, lBackColor,
                            DB_STANDARD );

            prcl->xLeft += COLUMN_SEPARATOR / 2;
        }

        /*--------------------------------------------------------------*/
        /* Update the left side of the rectangle for the next column.   */
        /*--------------------------------------------------------------*/
        prcl->xLeft += plColWidth[usCol];

        /*--------------------------------------------------------------*/
        /* Get the next column.                                         */
        /*--------------------------------------------------------------*/
        pszCol = pszNextCol;
        if ( pszNextCol != NULL )
            pszNextCol[-1] = '\t';
    }
}



void ExtractCols ( char *pszText, ... )  {

    int      i = 0, j;
    char    *pCol;
    va_list  marker;

    va_start( marker, pszText );        /* Initialize variable args     */

    while ( TRUE )  {

        if ( pszText[i] == '\r'|| pszText[i] == '\b' || pszText[i] == '\n' )
            i++;

        pCol = va_arg( marker, char * );

        if ( pCol == NULL )  {
            for ( ; pszText[i] != '\t' && pszText[i]; i++ );
        }
        else  {
            for ( j = 0; pszText[i] != '\t' && pszText[i]; j++, i++ )  {
                pCol[ j ] = pszText[ i ];
            }
            pCol[j] = 0;
        }

        if ( ! pszText[i] )
            break;

        if ( pszText[i] == '\t' )
            i++;
    }
    va_end( marker );                   /* Reset variable arguments     */
}
