#define ES_OPTIONAL          1
#define ES_REQUIRED          2

#define ES_OPTIONAL_RDONLY   ES_OPTIONAL | ES_READONLY
#define ES_REQUIRED_RDONLY   ES_REQUIRED | ES_READONLY

/*----------------------------------------------------------------------*/
/*  FORMAT structure                                                    */
/*----------------------------------------------------------------------*/
typedef  struct  {
    USHORT        usLimit;              /* Maximum count input limit    */
    ULONG         fFormat;
    USHORT      (*pfnValidChar)( int ); /* func to validate each char   */
    CHAR         *pszText;              /* Text + Mask                  */
    CHAR         *pszTextOnly;          /* Text only no mask            */
    CHAR         *pszBlank;             /* Blank field image            */
}   FORMAT;

/*----------------------------------------------------------------------*/
/*  SUBCLASSCTRLDATA                                                 */
/*                                                                      */
/*      A pointer to this structure is saved in the QWL_USER word of    */
/*      each user defined entry field. If it is a 'Formatted' field     */
/*      only the pFmt pointer is used. For 'A', '9', 'N', 'X' ... fields*/
/*      the pFldCtrl pointer is used.                                   */
/*----------------------------------------------------------------------*/
typedef  struct  {
    UCHAR         ucFldType;        /* $, F (Fmt'd), 9#APNXM*DdYy!|L    */
    USHORT        usLineStyle;      /* Required, Optional or Read Only  */
    ULONG         ulFlags;          /* control flags see below          */
    ULONG         ulLineType;       /* Type of line to display solid .. */
    ULONG         ulLineColor;      /* current color of the line        */
    CHAR         *pCharList;        /* restricted list of valid chars   */

    FORMAT        sFmt;             /* ptr to 'Formatted' fields        */
    BOOL          fValidState;      /*  Previous entry field text       */
}   SUBCLASSCTRLDATA;

/*----------------------------------------------------------------------*/
/*  Structure used to list the valid Formatting characters and the      */
/*  function to validate that set of characters.                        */
/*----------------------------------------------------------------------*/
typedef  struct  {
    USHORT  usPic;                  /* Mask Character               */
    int   (*pfnPicFunc) ( int c );  /* Validation func for Mask Chr */
}   PICINFO;

/*----------------------------------------------------------------------*/
/*  Prototypes used by the edit class entry fields                      */
/*----------------------------------------------------------------------*/
SUBCLASSCTRLDATA  *
         EntryFldCreate   ( HWND  hwnd,  CREATESTRUCT *pCr );
void     EntryFldDestroy  ( HWND  hwnd );
void     EntryFldPaint    ( HWND  hwnd );
