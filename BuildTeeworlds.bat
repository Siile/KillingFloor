@echo off

%comspec% /k ""C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"" x86

c:\bam\bam release

set /p choice="Press enter to exit"