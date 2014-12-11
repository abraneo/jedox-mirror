@echo off

SETLOCAL ENABLEDELAYEDEXPANSION

IF NOT EXIST "%JAVA_HOME%\bin\java.exe" goto nojava
SET JAVACMD="%JAVA_HOME%\bin\java.exe"

IF EXIST "lib" SET LIB="lib"
IF EXIST "..\lib" SET LIB="..\lib"

for %%i in (%0) do cd /d %%~dpi

set JCP=

for %%i in (%LIB%\*.jar) do set JCP=!JCP!;%%i
for %%i in (config\lib\*.jar) do set JCP=!JCP!;%%i

%JAVACMD% -Xmx1024m -cp %JCP% com.jedox.etl.core.run.StandaloneClient %*
goto end
 
:nojavahome
echo Warning: JAVA_HOME environment variable is not set.
goto end
 
:nojava
echo Error: JAVA_HOME is not defined correctly.
echo We cannot execute %JAVACMD%
goto end
 
:end