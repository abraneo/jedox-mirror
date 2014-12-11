@rem Removes the Jedox Suite Tomcat Service (for Palo ETL)
@echo off
cd /d %~dp0
call service remove JedoxSuiteTomcatService
pause
