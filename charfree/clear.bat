
@rem clear.bat to clear unuseful files
@rem 

@echo off
echo del unuseful files

pause

echo call my func
call:myDosFunc bin\output
call:myDosFunc bin_debug\output
call:myDosFunc CodeGenerater\x64
call:myDosFunc ipch

call:del_file bin\*.pdb
call:del_file bin\*.ilk

call:del_file bin_debug\*.pdb
call:del_file bin_debug\*.ilk
call:del_file *.sdf

echo.&goto:eof
@rem end of bat 

@rem functions

:: my func
::                 -- %~1: argument description here
:myDosFunc
echo here the myDosFunc
echo it could do a lot of things
echo. it could do %~1 
SETLOCAL
REM.--function body here
set LocalVar1=%~1
(ENDLOCAL & REM -- RETURN VALUES
    IF "%~1" NEQ "" SET %LocalVar1% = %~1
	echo  local var is %LocalVar1% 
	IF EXIST %LocalVar1% rd %LocalVar1% /S /Q
)

goto:eof

:: my func
::                 -- %~1: argument description here
:del_file
echo. it could do %~1 
SETLOCAL
REM.--function body here
set LocalVar2=%~1
(ENDLOCAL & REM -- RETURN VALUES
    IF "%~1" NEQ "" SET %LocalVar2% = %~1
	echo  local var is %LocalVar2% 
	IF EXIST "%LocalVar2%" DEL /Q %LocalVar2% >NUL
)

goto:eof
