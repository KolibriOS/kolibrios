c-- window.c
c-- collections.c
c-- menu.c

@echo off
@del _window
@del _collections
@del _menu

@rename window.com _window
@rename collections.com _collections
@rename menu.com _menu

@del warning.txt
@echo on

@pause