Usage instructions -

1) By default log file is created in /usbhd0/1. If the folder is uavailable,
the program will throw an error. Configure the folder from ftpc.ini

2) Browse the local and remote folders using UP/DOWN arrow keys and press ENTER
to download/upload the file. Scrolling might not work due to lack of support
from the boxlib library

3) It might be difficult to read log file contents using certain text editors.
gedit works fine


Known issues -

1) Uploading large files may not work. I do not know whether this is an FTPC
issue or a network-stack realted issue

2) FTPC may freeze on rare occasions. Simply close and restart it

3) Download may fail abruptly if disk becomes full. Unfortunately, as of now,
there is no support for checking available disk space beforehand from kernel
itself

4) Text in console and log file is not properly formatted


Future improvements -

1) Display more informative error messages (especially in GUI)

2) Allow resizing of GUI window and align GUI elements automatically
