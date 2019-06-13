@echo off
call make.bat clean
call make.bat release
call make.bat clean
call make.bat debug
call make.bat clean
