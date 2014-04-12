PROJ = MCOL
PROJFILE = MCOL.MAK
DEBUG = 1

PWBRMAKE  = pwbrmake
NMAKEBSC1  = set
CC  = cl
CFLAGS_G  = /AL /W3 /G2 /Zp /BATCH
CFLAGS_D  = /Gs /Gi$(PROJ).mdt /Zi /Od
CFLAGS_R  = /Ot /Oi /Ol /Oe /Og /Gs
MAPFILE_D  = NUL
MAPFILE_R  = NUL
LFLAGS_G  = /STACK:8192  /BATCH
LFLAGS_D  = /CO /FAR /PACKC /PACKD /PMTYPE:PM
LFLAGS_R  = /EXE /FAR /PACKC /PACKD /PMTYPE:PM
LINKER  = link
ILINK  = ilink
LRF  = echo > NUL
RC  = rc

OBJS  = MCOL.obj edit.obj LISTBX.obj
RESS  = MCOL.res
SBRS  = MCOL.sbr edit.sbr LISTBX.sbr

all: $(PROJ).exe

.SUFFIXES:
.SUFFIXES: .sbr .obj .res .c .rc

MCOL.obj : MCOL.C

MCOL.sbr : MCOL.C

MCOL.res : MCOL.RC

edit.obj : edit.c

edit.sbr : edit.c

LISTBX.obj : LISTBX.C

LISTBX.sbr : LISTBX.C


$(PROJ).bsc : $(SBRS)
        $(PWBRMAKE) @<<
$(BRFLAGS) $(SBRS)
<<

$(PROJ).exe : $(OBJS) $(RESS)
!IF $(DEBUG)
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_D)
$(LLIBS_G: = +^
) +
$(LLIBS_D: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_D);
<<
!ELSE
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_R)
$(LLIBS_G: = +^
) +
$(LLIBS_R: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_R);
<<
!ENDIF
        $(LINKER) @$(PROJ).lrf
        $(RC) $(RESS) $@


.c.sbr :
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FR$@ $<
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FR$@ $<
!ENDIF

.c.obj :
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /Fo$@ $<
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /Fo$@ $<
!ENDIF

.rc.res :
        $(RC) /r $< $@


run: $(PROJ).exe
        $(PROJ).exe $(RUNFLAGS)

debug: $(PROJ).exe
        CVP $(CVFLAGS) $(PROJ).exe $(RUNFLAGS)
