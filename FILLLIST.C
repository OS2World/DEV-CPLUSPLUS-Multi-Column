static char szRevision[] = "$Revision:   4.0  $     filllist.c";

/*----------------------------------------------------------------------*/
/*                                                                      */
/*      A set up routines to fill list boxes. The only one of true      */
/*      value is ADSFillAnyListBox I would like to see all the others   */
/*      deleted.                                                        */
/*      (May 4 1992 - Old methods deleted )                             */
/*----------------------------------------------------------------------*/
#define     INCL_WIN
#define     INCL_DOSMODULEMGR
#include   <os2.h>

#define     DBMSOS2
#include   <sybfront.h>
#include   <sybdb.h>

#include   <string.h>
#include   <ctype.h>
#include   <stdlib.h>

#include   "adslen.h"
#include   "ads.h"
#include   "adsstr.h"
#include   "errors.h"

/*----------------------------------------------------------------------*/
/*   ADSFillAnyListBox                                                  */
/*      Any list box can be filled with this function.                  */
/*                                                                      */
/*          dbproc  -   Pointer to our DBPROCESS structure              */
/*                                                                      */
/*            hwnd  -   Window handle to the list box                   */
/*                                                                      */
/*       asBindCol  -   An array containing the order in which the      */
/*                      returned columns are to be displayed            */
/*                                                                      */
/*   pszStoredProc  -   The SQL command to use to get the rows we want  */
/*                                                                      */
/*----------------------------------------------------------------------*/
USHORT  ADSFillAnyListBox ( DBPROCESS *dbproc,   HWND hwnd,
                         SHORT     asBindCol[],  CHAR *pszStoredProc )  {

    char         *apCol[ 60 + 1 ], *pszLine, szMsg[60], szMoney[30], *pszName;
    int           TotLen, ColLen, ColType[60], i;
    USHORT        cnt = 0;
    BOOL          fAlloc = FALSE;
    HMODULE       hmodADS;
    USERATTR     *pShare;

    pShare = ADSGetShrSeg ( );
    /*------------------------------------------------------------------*/
    /*  Display painting of the List Box and empty it out.              */
    /*------------------------------------------------------------------*/
    WinEnableWindowUpdate ( hwnd, FALSE );
    ADSSetWait ( );
    /*------------------------------------------------------------------*/
    /*  Execute the SQL code                                            */
    /*------------------------------------------------------------------*/
    dbcmd     ( dbproc, pszStoredProc );
    dbsqlexec ( dbproc );

    while ( dbresults ( dbproc ) == SUCCEED )  {

        fAlloc = TRUE;
        /*--------------------------------------------------------------*/
        /*  For each column to be returned allocate a buffer and bind   */
        /*  this buffer to the appropriate column.                      */
        /*--------------------------------------------------------------*/
        for ( TotLen = i = 0; asBindCol[i]; i++ )  {

            ColType[i]  = dbcoltype ( dbproc, abs ( asBindCol[i] ) );
            switch ( ColType[i] )  {
            case   SQLDATETIMN  :
            case   SQLDATETIME  :   ColLen   = L_DATE_TIME; break;

            case   SQLINTN      :
            case   SQLINT1      :
            case   SQLBIT       :
            case   SQLINT2      :
            case   SQLINT4      :   ColLen   = 12;          break;

            case   SQLMONEY     :
            case   SQLMONEYN    :   ColLen   = L_AMOUNT;    break;

            case   SQLFLT8      :
            case   SQLFLTN      :   ColLen   = 12;          break;

            default             :
            case   SQLCHAR      :
                ColLen   = (int) dbcollen  ( dbproc, abs ( asBindCol[i] ) ) + 1;
                pszName  = dbcolname ( dbproc, abs ( asBindCol[i] ) );
                /*------------------------------------------------------*/
                /*  If date                                             */
                /*------------------------------------------------------*/
                if ( pszName[0] == 0 && ColLen == 8+1 )
                    ColType[i] = SQLDATETIME;
                break;
            }
            if ( ( apCol[i] = malloc ( ColLen ) ) == NULL )  {
                dbcanquery ( dbproc );
                break;
            }

            TotLen  += ColLen + 2;
            dbbind ( dbproc, abs ( asBindCol[i] ), NTBSTRINGBIND, ColLen, apCol[i] );
        }
        /*--------------------------------------------------------------*/
        /*  Allocate a buffer big enough to hold the combinded columns  */
        /*--------------------------------------------------------------*/
        if ( ( pszLine = malloc ( TotLen ) ) == NULL )  {
            dbcanquery ( dbproc );
            break;
         }

        while ( dbnextrow ( dbproc ) != NO_MORE_ROWS )  {
            pszLine[0] = 0;
            /*----------------------------------------------------------*/
            /*  If the asBindCol number is less than 0 this indicates   */
            /*  that this column is to be right justified. So           */
            /*  concatenate a \r on to the string                       */
            /*----------------------------------------------------------*/
            if ( asBindCol[0] < 0 )
                strcat ( pszLine, "\r" );
            strcat ( pszLine, apCol[0] );
            for ( i = 1; asBindCol[i]; i++ )  {

                strcat ( pszLine, "\t" );
                if ( asBindCol[i] < 0 )
                    strcat ( pszLine, "\r" );
                if ( ColType[i] == SQLDATETIME || ColType[i] == SQLDATETIMN )  {
                    char    szDate[L_DATE];
					if ( apCol[i][2] == '/' )  {
                        ADSConvertDateFromMDY ( pShare->szDateFmtCode, szDate, apCol[i] );
                        strcpy ( apCol[i], szDate );
                    }
                }
                if ( ColType[i] == SQLMONEY || ColType[i] == SQLMONEYN )  {
        /*          DBMONEY   mMoney;                                   */
        /*          ADSFormatMoney ( NULL, pShare->szDefaultCurrCode,   */
        /*          DBMONEY   *pmMoney, apCol[i] );                     */
        /*                                                              */
        /*          strcat ( pszLine, "$ " );                           */
                    strcat ( pszLine, apCol[i] );
                }
                else  {
                    strcat ( pszLine, apCol[i] );
                }
            }
            if ( SHORT1FROMMR ( WinSendMsg ( hwnd, LM_SEARCHSTRING,
                            MPFROM2SHORT ( LSS_CASESENSITIVE, LIT_FIRST ),
                            MPFROMP      ( pszLine ) ) )
                                      == LIT_NONE )  {
                cnt++;
                if ( WinSendMsg ( hwnd, LM_INSERTITEM,
                        MPFROMSHORT ( LIT_END ), MPFROMP( pszLine ) )
                                         == ( MRESULT ) LIT_MEMERROR )  {
                    dbcanquery (dbproc);
                    DosLoadModule ( szMsg, sizeof ( szMsg ), "ADS", &hmodADS );

                    WinLoadString ( WinQueryAnchorBlock ( hwnd ), hmodADS,
                                    IDS_LIST_BOX_FULL, sizeof( szMsg ), szMsg );
                    DosFreeModule ( hmodADS );
                    ADSMsgBox ( dbproc, TOO_MANY_ROWS, szMsg );
                    /*--------------------------------------------------*/
                    /*  cancel extra rows if the List Box exploded      */
                    /*--------------------------------------------------*/
                    goto EMERGENCY;
                }                           /* Insert Failed check      */
            }                               /* Duplicate search check   */
        }                                   /* while next row loop      */
    }                                       /* while results  loop      */
EMERGENCY:
    ADSSetArrow ( );
    WinShowWindow ( hwnd, TRUE );

    /*------------------------------------------------------------------*/
    /*  Free all buffers                                                */
    /*------------------------------------------------------------------*/
    if ( fAlloc )
    {
        for ( i = 0; asBindCol[i]; i++ )
            free ( apCol[i] );

        free ( pszLine );
    }
    return  cnt;
}

void ADSSetMCHeading ( HWND hwndLB, LONG *alColWidth, USHORT  usMaxCols )
{

    char    Buf[80];

    memset ( alColWidth, 0, usMaxCols * sizeof (LONG) );

    /*------------------------------------------------------------*/
    /* Disable the list box, delete everything from the list box  */
    /* and then either insert a dummy row and delete the dummy	  */
    /* row.  Then enable the list box. The heading won't show up  */
    /* on an empty multi column list box			  */
    /*------------------------------------------------------------*/
    WinEnableWindowUpdate ( hwndLB, FALSE );

    /*----------------------------------------------------------*/
    /* Need to input a line or the column lengths are incorrect */
    /*----------------------------------------------------------*/
    Buf[0] = ' ';
    Buf[1] =  0 ;
    for ( usMaxCols--; usMaxCols; usMaxCols-- )
        strcat ( Buf, "\t " );

    WinSendMsg ( hwndLB, LM_INSERTITEM, ( MPARAM ) 0, ( MPARAM ) Buf );
    WinSendMsg ( hwndLB, LM_DELETEITEM, 0, 0 );
    WinShowWindow ( hwndLB, TRUE );
}
