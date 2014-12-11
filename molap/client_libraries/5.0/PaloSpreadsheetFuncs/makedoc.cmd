doxygen >makedoc.log 2>&1
doxygen Doxyfile.PHPPalo >>makedoc.log 2>&1
doxygen Doxyfile.PaloXLL >>makedoc.log 2>&1

"%ProgramFiles%\HTML Help Workshop\hhc.exe" doc\html\index.hhp  >>makedoc.log 2>&1
"%ProgramFiles%\HTML Help Workshop\hhc.exe" doc\html.PHPPalo\index.hhp  >>makedoc.log 2>&1
"%ProgramFiles%\HTML Help Workshop\hhc.exe" doc\html.PaloXLL\index.hhp  >>makedoc.log 2>&1

pause