# tte

tte (tiny text editor) is a terminal based text editor written in C from scratch, trying to be very minimalistic and dependency independent (it's not even using **curses**). 

This project was mainly created for educational purposes, so is very commented!

Thanks to [antirez](http://antirez.com) for inspiring me with his project `kilo` and [Jeremy Ruten](https://twitter.com/yjerem) for his tutorials.

## Installation

### Compiling
```
git clone https://github.com/GrenderG/tte.git
cd tte/
make install
```
### Downloading executable
Download it from [here](https://github.com/GrenderG/tte/releases/latest), then
```
sudo mv tte /usr/local/bin/
sudo chmod +x /usr/local/bin/tte
``` 

## Usage
```
tte [file_name]
tte -h | --help
tte -v | --version
```
If you are planning to use special characters like (á, é, í, ó, ú, ¡, ¿, ...) you must use `ISO 8859-1` encoding in your terminal. See [this issue](https://github.com/GrenderG/tte/issues/2) for more info.

## Keybindings
The key combinations chosen here are the ones that fit the best for me.
```
Ctrl-Q : Exit
Ctrl-F : Search text (ESC, arrows and enter to interact once searching)
Ctrl-S : Save
Ctrl-E : Flip line upwards
Ctrl-D : Flip line downwards
Ctrl-C : Copy line
Ctrl-X : Cut line
Ctrl-V : Paste line
Ctrl-P : Pause tte (type "fg" to resume)
```

## Current supported languages
* C (`*.c`, `*.h`)
* C++ (`*.cpp`, `*.hpp`, `*.cc`)
* Java (`*.java`)
* Bash (`*.sh`)
* Python (`*.py`)
* PHP (`*.php`)
* JavaScript (`*.js`, `*.jsx`)
* JSON (`*.json`, `*.jsonp`)
* XML (partially) (`*.xml`)
* SQL (`*.sql`)
* Ruby (`*.rb`)

## Images
![First screenshot](https://raw.githubusercontent.com/GrenderG/tte/master/images/scr_1.png)
