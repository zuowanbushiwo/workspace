@ECHO OFF
rem =====================================================================
REM File:		svn_vers.bat
REM Purpose:	Checks svn revision number and replaces placeholder
REM				in header file version.h. Run this file from CCS4 in a 
rem				pre-build step:
rem				cmd /c "${PROJECT_ROOT}/svn_vers.bat"
REM	Input:		None
REM Author:		Markus Lindroth
REM Date:		2013-May-27
REM Copyright:	Limes Audio AB
REM E-mail:		markus.borgh@limesaudio.com
rem =====================================================================

rem Version numbering
set major=0
set minor=1

rem Batch file directory
set batch_dir=%~dp0

REM Directory to check svn revision for (one step up from TrueVoice, should be trunk or some tagged version)
SET x=%batch_dir%..\
REM Name of source file
SET source=%batch_dir%temp.h
REM Name of destination file
SET dest=%batch_dir%..\include\version.h


REM Prompt info
ECHO.
ECHO --- %0: Checking SVN version and creating header file

rem Create source file
(
echo /*****************************************************************************
echo  * THIS FILE IS AUTOMATICALLY GENERATED. ANY CHANGES TO THIS FILE
echo  * WILL BE LOST IN NEXT BUILD. CHANGES SHOULD BE MADE IN FILE
echo  * SVN_VERS.BAT
echo  *****************************************************************************/
echo /*****************************************************************************
echo  *	\file		version.h
echo  *
echo  *	\date		2013-May-27
echo  *
echo  *	\brief		Version info for Limes Audio True Voice module
echo  *
echo  *	Copyright	Limes Audio AB
echo  *
echo  *	This code should not be modified, copied, duplicated, reproduced, licensed
echo  *	or sublicensed without the prior written consent of Limes Audio AB.
echo  *	This code, or any right in the code, should not be transferred or conveyed
echo  *	without the prior written consent of Limes Audio AB.
echo  *
echo  *****************************************************************************/
echo #ifndef TV_VERSION_H
echo #define TV_VERSION_H
echo.
echo // Major version number
echo #define TV_VERSION_MAJOR	%major%
echo // Minor version number
echo #define TV_VERSION_MINOR	%minor%
echo // This is the SVN version number for latest commit for all files in this
echo // folder. The placeholder is updated in a pre-build step by the batch
echo // file "svn_vers.bat".
echo #define TV_VERSION_BUILD	$WCREV$
echo.
echo #endif // TV_VERSION_H
) > %source%


REM Get SVN revision report
subwcrev %x% %source% %dest%

REM Remove temp file
del %source%

@ECHO ON