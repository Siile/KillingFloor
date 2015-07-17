@echo off

%comspec% /k ""C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"" x86

for /f "usebackq delims=" %%x in (`WMIC CPU Get NumberOfCores`) do c:\bam\bam -j %%x server_release

set /p choice="Press enter to exit"
