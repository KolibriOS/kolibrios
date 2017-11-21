The tool pe2kos is written by Rabid Rabbit and slightly rectified by me,
diamond. It is used in projects xonix and fara (the author is Rabid Rabbit),
written in Visual C++, at last step after a compilation, when it is needed
to get from a program in the Windows-exe format a true Kolibri-program.
The tool only converts the format of executable, so to get working program
one must satisfy to certain conditions. Of course, a program must
communicate with the outer world by Kolibri facilities (i.e. int 0x40)
and must not use any Windows-libraries. In addition program is required
to be placed on zero address (linker option "/base:0"). How to write
such programs - look to already mentioned projects xonix and fara.
There is two versions of the tool: for programs which use path to executable
file (last dword in MENUET01-header), and others.
Select wanted version.
Usage: (in command line) "pe2kos <source file> <destination file>".
For example, "pe2kos xonix.exe xonix".