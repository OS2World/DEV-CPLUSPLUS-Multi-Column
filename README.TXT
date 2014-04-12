The files you have here contain all the code that is necessary to build
dialog boxes with multi-column list boxes as well as the window proc for
simple entry fields that restrict the user to certain types of input (
alpha only, numeric only, force to upper ... ).

Restrictive Entry Fields
    After looking at many of the libraries and DLLs that were available
    3 or 4 years ago that 'claimed' to have reliable restricted data
    entry fields we wrote our own. All they we ever saw acted too
    different from normal entry fields to be added in to our
    application. The code is very simple. If fact it probably could be
    reduced in size a little more but since I am superstitious about
    changing working code many of the swithches and ifs used in the
    WM_CHAR code remain. This is the base code for all of our other
    special entry fields ( International money and true formatted fields
    such as Social Security #, Phone #s ... ). The module edit.c can
    easily be extended to define any type of data to be entered.

Multi-Column List Boxes
    This is a nice example of implementing a multi-column list box
    without resorting to setting the list box font to a non-porportional
    (ugly) font. Each column of the text string to be added to the list
    box is measured to find the maximum width entry for that column a
    buffer of LONGs saves the max width. The WinEnableWindowUpdate
    before the insert and the WinShowWindow after allows the list box to
    be repainted for each insert. Without this code the columns will be
    painted in a jagged manner until each column is repainted
    individually. The window proc to the list box also accesses a user
    defined window that contains the text of the column headings.


Files List
    Readme      -   This file
    MCol.c      -   main and Dlg Box proc code
    MCol.Dlg    -   Dlg definition
    MCol.h      -   Dlg defines
    MCol.rc     -   RC file
    MCol.mak    -   MSC 6.0 make file
    Edit.c      -   Edit field window proc code
    Edit.h      -   Edit field defines and structure definitions
    ListBx.c    -   Multi-Column window proc code


For more information please contact:

    Bp. Gary T. James
    American Airlines
    4200 American Blvd.
    Fort Worth, TX. 76155
    Tel: (817) 963-3821 or (817) 963-3431
        OR
    CIS: 75010,3221
