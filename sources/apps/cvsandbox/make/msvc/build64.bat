@echo off
call make64.bat clean
call make64.bat release
call make64.bat clean
call make64.bat debug
call make64.bat clean
