# Microsoft Developer Studio Project File - Name="pcgpack_dbl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pcgpack_dbl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pcgpack_dbl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pcgpack_dbl.mak" CFG="pcgpack_dbl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pcgpack_dbl - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pcgpack_dbl - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
F90=df.exe
RSC=rc.exe

!IF  "$(CFG)" == "pcgpack_dbl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE F90 /compile_only /nologo /warn:nofileopt
# ADD F90 /compile_only /nologo /warn:nofileopt
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pcgpack_dbl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE F90 /check:bounds /compile_only /debug:full /nologo /traceback /warn:argument_checking /warn:nofileopt
# ADD F90 /check:bounds /compile_only /debug:full /nologo /traceback /warn:argument_checking /warn:nofileopt
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ   /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "pcgpack_dbl - Win32 Release"
# Name "pcgpack_dbl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;f90;for;f;fpp"
# Begin Source File

SOURCE=.\pc0v.f
# End Source File
# Begin Source File

SOURCE=.\pc0vi.f
# End Source File
# Begin Source File

SOURCE=.\pcaxpy.f
# End Source File
# Begin Source File

SOURCE=.\pcbtrc.f
# End Source File
# Begin Source File

SOURCE=.\pccg.f
# End Source File
# Begin Source File

SOURCE=.\pccgx.f
# End Source File
# Begin Source File

SOURCE=.\pccred.f
# End Source File
# Begin Source File

SOURCE=.\pcd2i.f
# End Source File
# Begin Source File

SOURCE=.\pcd2ii.f
# End Source File
# Begin Source File

SOURCE=.\pcd2it.f
# End Source File
# Begin Source File

SOURCE=.\pcdmv.f
# End Source File
# Begin Source File

SOURCE=.\pcdmva.f
# End Source File
# Begin Source File

SOURCE=.\pcdrv.f
# End Source File
# Begin Source File

SOURCE=.\pcdrvx.f
# End Source File
# Begin Source File

SOURCE=.\pcdslv.f
# End Source File
# Begin Source File

SOURCE=.\pcend.f
# End Source File
# Begin Source File

SOURCE=.\pcgcr.f
# End Source File
# Begin Source File

SOURCE=.\pcgmr.f
# End Source File
# Begin Source File

SOURCE=.\pcgmrx.f
# End Source File
# Begin Source File

SOURCE=.\pciabs.f
# End Source File
# Begin Source File

SOURCE=.\pciblk.f
# End Source File
# Begin Source File

SOURCE=.\pcic.f
# End Source File
# Begin Source File

SOURCE=.\pcic1.f
# End Source File
# Begin Source File

SOURCE=.\pcicb.f
# End Source File
# Begin Source File

SOURCE=.\pcicmc.f
# End Source File
# Begin Source File

SOURCE=.\pcicmr.f
# End Source File
# Begin Source File

SOURCE=.\pcicms.f
# End Source File
# Begin Source File

SOURCE=.\pcicpr.f
# End Source File
# Begin Source File

SOURCE=.\pcicpy.f
# End Source File
# Begin Source File

SOURCE=.\pcictr.f
# End Source File
# Begin Source File

SOURCE=.\pcictx.f
# End Source File
# Begin Source File

SOURCE=.\pcicx.f
# End Source File
# Begin Source File

SOURCE=.\pciipr.f
# End Source File
# Begin Source File

SOURCE=.\pcijab.f
# End Source File
# Begin Source File

SOURCE=.\pcijup.f
# End Source File
# Begin Source File

SOURCE=.\pcilu.f
# End Source File
# Begin Source File

SOURCE=.\pcilu1.f
# End Source File
# Begin Source File

SOURCE=.\pciluf.f
# End Source File
# Begin Source File

SOURCE=.\pcilus.f
# End Source File
# Begin Source File

SOURCE=.\pciluz.f
# End Source File
# Begin Source File

SOURCE=.\pcily.f
# End Source File
# Begin Source File

SOURCE=.\pciprm.f
# End Source File
# Begin Source File

SOURCE=.\pciprp.f
# End Source File
# Begin Source File

SOURCE=.\pcirsw.f
# End Source File
# Begin Source File

SOURCE=.\pcitx.f
# End Source File
# Begin Source File

SOURCE=.\pcity.f
# End Source File
# Begin Source File

SOURCE=.\pclu1c.f
# End Source File
# Begin Source File

SOURCE=.\pclu1r.f
# End Source File
# Begin Source File

SOURCE=.\pclub.f
# End Source File
# Begin Source File

SOURCE=.\pclub1.f
# End Source File
# Begin Source File

SOURCE=.\pclub2.f
# End Source File
# Begin Source File

SOURCE=.\pclub3.f
# End Source File
# Begin Source File

SOURCE=.\pclub4.f
# End Source File
# Begin Source File

SOURCE=.\pclub5.f
# End Source File
# Begin Source File

SOURCE=.\pclubf.f
# End Source File
# Begin Source File

SOURCE=.\pclup.f
# End Source File
# Begin Source File

SOURCE=.\pclup1.f
# End Source File
# Begin Source File

SOURCE=.\pclup2.f
# End Source File
# Begin Source File

SOURCE=.\pclup3.f
# End Source File
# Begin Source File

SOURCE=.\pclup4.f
# End Source File
# Begin Source File

SOURCE=.\pclup5.f
# End Source File
# Begin Source File

SOURCE=.\pclupf.f
# End Source File
# Begin Source File

SOURCE=.\pcmlc.f
# End Source File
# Begin Source File

SOURCE=.\pcmlic.f
# End Source File
# Begin Source File

SOURCE=.\pcmlir.f
# End Source File
# Begin Source File

SOURCE=.\pcmlr.f
# End Source File
# Begin Source File

SOURCE=.\pcmuc.f
# End Source File
# Begin Source File

SOURCE=.\pcmuic.f
# End Source File
# Begin Source File

SOURCE=.\pcmuir.f
# End Source File
# Begin Source File

SOURCE=.\pcmur.f
# End Source File
# Begin Source File

SOURCE=.\pcmv.f
# End Source File
# Begin Source File

SOURCE=.\pcmvc.f
# End Source File
# Begin Source File

SOURCE=.\pcmvr.f
# End Source File
# Begin Source File

SOURCE=.\pcmvs.f
# End Source File
# Begin Source File

SOURCE=.\pcnegv.f
# End Source File
# Begin Source File

SOURCE=.\pcnopr.f
# End Source File
# Begin Source File

SOURCE=.\pcomn.f
# End Source File
# Begin Source File

SOURCE=.\pcop.f
# End Source File
# Begin Source File

SOURCE=.\pcpack.f
# End Source File
# Begin Source File

SOURCE=.\pcpaxp.f
# End Source File
# Begin Source File

SOURCE=.\pcpcnf.f
# End Source File
# Begin Source File

SOURCE=.\pcpcsf.f
# End Source File
# Begin Source File

SOURCE=.\pcprmb.f
# End Source File
# Begin Source File

SOURCE=.\pcprmp.f
# End Source File
# Begin Source File

SOURCE=.\pcpunp.f
# End Source File
# Begin Source File

SOURCE=.\pcrc1b.f
# End Source File
# Begin Source File

SOURCE=.\pcrc1p.f
# End Source File
# Begin Source File

SOURCE=.\pcrc2b.f
# End Source File
# Begin Source File

SOURCE=.\pcrcpr.f
# End Source File
# Begin Source File

SOURCE=.\pcrcpy.f
# End Source File
# Begin Source File

SOURCE=.\pcrdpp.f
# End Source File
# Begin Source File

SOURCE=.\pcrdpt.f
# End Source File
# Begin Source File

SOURCE=.\pcredb.f
# End Source File
# Begin Source File

SOURCE=.\pcredp.f
# End Source File
# Begin Source File

SOURCE=.\pcredu.f
# End Source File
# Begin Source File

SOURCE=.\pcric.f
# End Source File
# Begin Source File

SOURCE=.\pcrilu.f
# End Source File
# Begin Source File

SOURCE=.\pcrsad.f
# End Source File
# Begin Source File

SOURCE=.\pcrsbs.f
# End Source File
# Begin Source File

SOURCE=.\pcrsfb.f
# End Source File
# Begin Source File

SOURCE=.\pcrsfm.f
# End Source File
# Begin Source File

SOURCE=.\pcrsfp.f
# End Source File
# Begin Source File

SOURCE=.\pcrslu.f
# End Source File
# Begin Source File

SOURCE=.\pcrspf.f
# End Source File
# Begin Source File

SOURCE=.\pcrsps.f
# End Source File
# Begin Source File

SOURCE=.\pcrsrb.f
# End Source File
# Begin Source File

SOURCE=.\pcrsrp.f
# End Source File
# Begin Source File

SOURCE=.\pcsbge.f
# End Source File
# Begin Source File

SOURCE=.\pcsbgp.f
# End Source File
# Begin Source File

SOURCE=.\pcsc1b.f
# End Source File
# Begin Source File

SOURCE=.\pcsc1p.f
# End Source File
# Begin Source File

SOURCE=.\pcsc2b.f
# End Source File
# Begin Source File

SOURCE=.\pcscal.f
# End Source File
# Begin Source File

SOURCE=.\pcsd2f.f
# End Source File
# Begin Source File

SOURCE=.\pcsd2s.f
# End Source File
# Begin Source File

SOURCE=.\pcsijb.f
# End Source File
# Begin Source File

SOURCE=.\pcsp.f
# End Source File
# Begin Source File

SOURCE=.\pcspfa.f
# End Source File
# Begin Source File

SOURCE=.\pcspr.f
# End Source File
# Begin Source File

SOURCE=.\pcspsl.f
# End Source File
# Begin Source File

SOURCE=.\pcsrb.f
# End Source File
# Begin Source File

SOURCE=.\pcsrfb.f
# End Source File
# Begin Source File

SOURCE=.\pcsrfp.f
# End Source File
# Begin Source File

SOURCE=.\pcsrij.f
# End Source File
# Begin Source File

SOURCE=.\pcsrix.f
# End Source File
# Begin Source File

SOURCE=.\pcsriz.f
# End Source File
# Begin Source File

SOURCE=.\pcsro2.f
# End Source File
# Begin Source File

SOURCE=.\pcsrob.f
# End Source File
# Begin Source File

SOURCE=.\pcsrop.f
# End Source File
# Begin Source File

SOURCE=.\pcsrp.f
# End Source File
# Begin Source File

SOURCE=.\pcsrrb.f
# End Source File
# Begin Source File

SOURCE=.\pcsrrp.f
# End Source File
# Begin Source File

SOURCE=.\pcsrsb.f
# End Source File
# Begin Source File

SOURCE=.\pcsrsp.f
# End Source File
# Begin Source File

SOURCE=.\pcsrti.f
# End Source File
# Begin Source File

SOURCE=.\pcsrzp.f
# End Source File
# Begin Source File

SOURCE=.\pcsscl.f
# End Source File
# Begin Source File

SOURCE=.\pctran.f
# End Source File
# Begin Source File

SOURCE=.\pcunpk.f
# End Source File
# Begin Source File

SOURCE=.\pcvadd.f
# End Source File
# Begin Source File

SOURCE=.\pcvdiv.f
# End Source File
# Begin Source File

SOURCE=.\pcverf.f
# End Source File
# Begin Source File

SOURCE=.\pcvers.f
# End Source File
# Begin Source File

SOURCE=.\pcvmul.f
# End Source File
# Begin Source File

SOURCE=.\pcvsma.f
# End Source File
# Begin Source File

SOURCE=.\pcvsub.f
# End Source File
# Begin Source File

SOURCE=.\pcvtm.f
# End Source File
# Begin Source File

SOURCE=.\pcvunp.f
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# End Group
# End Target
# End Project
