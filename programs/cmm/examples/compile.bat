c-- window.c
c-- collections.c
c-- menu.c
c-- mixcolors.c

@echo off
@del _window
@del _collections
@del _menu
@del _mixcolors

@rename window.com _window
@rename collections.com _collections
@rename menu.com _menu
@rename mixcolors.com _mixcolors

@del warning.txt
@echo on

@pause