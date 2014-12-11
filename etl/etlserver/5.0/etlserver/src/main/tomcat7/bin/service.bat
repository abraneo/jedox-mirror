@echo off
rem Licensed to the Apache Software Foundation (ASF) under one or more
rem contributor license agreements.  See the NOTICE file distributed with
rem this work for additional information regarding copyright ownership.
rem The ASF licenses this file to You under the Apache License, Version 2.0
rem (the "License"); you may not use this file except in compliance with
rem the License.  You may obtain a copy of the License at
rem
rem     http://www.apache.org/licenses/LICENSE-2.0
rem
rem Unless required by applicable law or agreed to in writing, software
rem distributed under the License is distributed on an "AS IS" BASIS,
rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
rem See the License for the specific language governing permissions and
rem limitations under the License.

if "%OS%" == "Windows_NT" setlocal
rem ---------------------------------------------------------------------------
rem NT Service Install/Uninstall script
rem
rem Options
rem install                Install the service using Tomcat7 as service name.
rem                        Service is installed using default settings.
rem remove                 Remove the service from the System.
rem
rem name        (optional) If the second argument is present it is considered
rem                        to be new service name
rem
rem $Id: service.bat 1000718 2010-09-24 06:00:00Z mturk $
rem ---------------------------------------------------------------------------

set "SELF=%~dp0%service.bat"
rem Guess CATALINA_HOME if not defined
set "CURRENT_DIR=%cd%"
if not "%CATALINA_HOME%" == "" goto gotHome
set "CATALINA_HOME=%cd%"
rem start JedoxSuite_CUSTOM_CONFIG
if exist "%CATALINA_HOME%\bin\paloTomcat7.exe" goto okHome
rem end JedoxSuite_CUSTOM_CONFIG
rem CD to the upper dir
cd ..
set "CATALINA_HOME=%cd%"
:gotHome
rem start JedoxSuite_CUSTOM_CONFIG
if exist "%CATALINA_HOME%\bin\paloTomcat7.exe" goto okHome
echo The paloTomcat7.exe was not found...
rem end JedoxSuite_CUSTOM_CONFIG
echo The CATALINA_HOME environment variable is not defined correctly.
echo This environment variable is needed to run this program
goto end
:okHome

rem start JedoxSuite_CUSTOM_CONFIG
rem echo Setting the Java Home variable
call bin\setenv.bat
set JAVA_HOME=%Java_Directory%
rem echo Java is installed: %JAVA_HOME%

rem echo "ok Home"
rem Make sure prerequisite environment variables are set
if "%JAVA_HOME%" == "" goto noJavaHome
goto okJavaHome
:noJavaHome
echo Java is not found
goto end
:okJavaHome
if not "%CATALINA_BASE%" == "" goto gotBase
set "CATALINA_BASE=%CATALINA_HOME%"
:gotBase

set "EXECUTABLE=%CATALINA_HOME%\bin\paloTomcat7.exe"

rem Settings for Palo Tomcat Service
call "%CATALINA_HOME%\bin\setenv.bat"
rem Set default Service name
set SERVICE_NAME=JedoxSuiteTomcatService
set PR_DISPLAYNAME=Jedox Suite Tomcat Service
rem end JedoxSuite_CUSTOM_CONFIG

if "x%1x" == "xx" goto displayUsage
set SERVICE_CMD=%1
shift
if "x%1x" == "xx" goto checkServiceCmd
:checkUser
if "x%1x" == "x/userx" goto runAsUser
if "x%1x" == "x--userx" goto runAsUser
set SERVICE_NAME=%1
rem start JedoxSuite_CUSTOM_CONFIG
set PR_DISPLAYNAME=%1
rem end JedoxSuite_CUSTOM_CONFIG
shift
if "x%1x" == "xx" goto checkServiceCmd
goto checkUser
:runAsUser
shift
if "x%1x" == "xx" goto displayUsage
set SERVICE_USER=%1
shift
runas /env /savecred /user:%SERVICE_USER% "%COMSPEC% /K \"%SELF%\" %SERVICE_CMD% %SERVICE_NAME%"
goto end
:checkServiceCmd
if /i %SERVICE_CMD% == install goto doInstall
if /i %SERVICE_CMD% == remove goto doRemove
if /i %SERVICE_CMD% == uninstall goto doRemove
echo Unknown parameter "%1"
:displayUsage
echo.
echo Usage: service.bat install/remove [service_name] [/user username]
goto end

:doRemove
rem Remove the service
"%EXECUTABLE%" //DS//%SERVICE_NAME%
if not errorlevel 1 goto removed
echo Failed removing '%SERVICE_NAME%' service
goto end
:removed
echo The service '%SERVICE_NAME%' has been removed
goto end

:doInstall
rem Install the service
echo Installing the service   '%SERVICE_NAME%' ...
echo Using CATALINA_HOME:     "%CATALINA_HOME%"
echo Using CATALINA_BASE:     "%CATALINA_BASE%"
echo Using JAVA_HOME:         "%JAVA_HOME%"
echo Using Log directory:     "%Log_Directory%"
echo Using optional settings: "%JVM_Optional_Parameters%"
echo Using MinMemory:         %Min_Memory%MB
echo Using MaxMemory:         %Max_Memory%MB
echo Using MaxPermMemory:     %PermGen_Max_Memory%MB 

rem Use the environment variables as an example
rem Each command line option is prefixed with PR_

rem start JedoxSuite_CUSTOM_CONFIG
set PR_DESCRIPTION=Jedox Suite Tomcat Service
rem end JedoxSuite_CUSTOM_CONFIG
set "PR_INSTALL=%EXECUTABLE%"
rem start JedoxSuite_CUSTOM_CONFIG
set "PR_LOGPATH=%Log_Directory%"
rem end JedoxSuite_CUSTOM_CONFIG
set "PR_CLASSPATH=%CATALINA_HOME%\bin\bootstrap.jar;%CATALINA_BASE%\bin\tomcat-juli.jar;%CATALINA_HOME%\bin\tomcat-juli.jar"

rem Set the server jvm from JAVA_HOME
rem start JedoxSuite_CUSTOM_CONFIG
rem use JAVA_HOME
set "PR_JVM=%JAVA_HOME%\bin\server\jvm.dll"
rem end JedoxSuite_CUSTOM_CONFIG
if exist "%PR_JVM%" goto foundJvm
set "PR_JVM=%JAVA_HOME%\jre\bin\server\jvm.dll"
rem end JedoxSuite_CUSTOM_CONFIG
if exist "%PR_JVM%" goto foundJvm
rem Set the client jvm from JAVA_HOME
rem start JedoxSuite_CUSTOM_CONFIG
set "PR_JVM=%JAVA_HOME%\bin\client\jvm.dll"
rem end JedoxSuite_CUSTOM_CONFIG
if exist "%PR_JVM%" goto foundJvm
set PR_JVM=auto
:foundJvm
echo Using JVM:              "%PR_JVM%"
set STARTUP_MODE=auto
"%EXECUTABLE%" //IS//%SERVICE_NAME% --StartClass org.apache.catalina.startup.Bootstrap --StopClass org.apache.catalina.startup.Bootstrap --StartParams start --StopParams stop --Startup %STARTUP_MODE%  --DependsOn JedoxSuiteMolapService
if not errorlevel 1 goto installed
echo Failed installing '%SERVICE_NAME%' service
goto end
:installed
rem Clear the environment variables. They are not needed any more.
set PR_DISPLAYNAME=
set PR_DESCRIPTION=
set PR_INSTALL=
rem set PR_LOGPATH=
set PR_CLASSPATH=
set PR_JVM=

rem start JedoxSuite_CUSTOM_CONFIG
rem CUSTOM_CONFIG additional security options to allow Palo ETL to access files from the Network
set SECURITY_POLICY_FILE=%CATALINA_BASE%\conf\catalina.policy
set SECURITY_POLICY_PARAM=-Djava.security.manager;-Djava.security.policy=%SECURITY_POLICY_FILE%;
echo Using security policy %SECURITY_POLICY_PARAM%
rem end JedoxSuite_CUSTOM_CONFIG

rem Set extra parameters
rem start JedoxSuite_CUSTOM_CONFIG
"%EXECUTABLE%" //US//%SERVICE_NAME% --JvmOptions "-Dcatalina.base=%CATALINA_BASE%;%SECURITY_POLICY_PARAM%-Dcatalina.home=%CATALINA_HOME%;-Djava.endorsed.dirs=%CATALINA_HOME%\endorsed;%JVM_Optional_Parameters%-XX:MaxPermSize=%PermGen_Max_Memory%m;-Djava.library.path=%Library_Path%;-Dcatalina.logs=%PR_LOGPATH%" --StartMode jvm --StopMode jvm
rem end JedoxSuite_CUSTOM_CONFIG
rem More extra parameters
rem set "PR_LOGPATH=%CATALINA_BASE%\logs"
set PR_STDOUTPUT=auto
set PR_STDERROR=auto
rem start JedoxSuite_CUSTOM_CONFIG
"%EXECUTABLE%" //US//%SERVICE_NAME% ++JvmOptions "-Djava.io.tmpdir=%CATALINA_BASE%\temp;-Djava.util.logging.manager=org.apache.juli.ClassLoaderLogManager" --JvmMs %Min_Memory% --JvmMx %Max_Memory%
rem http://drumcoder.co.uk/blog/2011/jan/12/maxpermsize-tomcat-windows-service/
rem "%EXECUTABLE%" //US//%SERVICE_NAME% ++JvmOptions="-XX:MaxPermSize=256m"
rem end JedoxSuite_CUSTOM_CONFIG
echo The service '%SERVICE_NAME%' has been installed.

:end
cd "%CURRENT_DIR%"
