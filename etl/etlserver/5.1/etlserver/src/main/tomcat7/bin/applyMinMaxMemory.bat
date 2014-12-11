cd %~dp0
call setenv.bat
tomcat7.exe //US//JedoxSuiteTomcatService --JvmMs %Min_Memory% --JvmMx %Max_Memory%
NET STOP JedoxSuiteTomcatService
NET START JedoxSuiteTomcatService
