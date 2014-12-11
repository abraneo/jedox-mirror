@rem Removes the Jedox Suite Tomcat Service (for Palo ETL)
@echo off
cd %~dp0
call service remove JedoxSuiteTomcatService
pause
