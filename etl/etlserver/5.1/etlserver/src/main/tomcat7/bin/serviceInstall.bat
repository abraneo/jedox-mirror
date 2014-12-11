@rem Installs the  Jedox Suite Tomcat Service (for Palo ETL)
@echo off
cd /d %~dp0
call service install JedoxSuiteTomcatService
NET START JedoxSuiteTomcatService
pause