# Read/Write file example
# Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv3

import file

fw=file.open('my.txt','w') # Open file for writing
fw.write("I love KolibriOS") # Write symbols to my.txt file
fw.close() # Close file

fr=file.open('my.txt', 'r') # Open file for reading
str=fr.read() # Read symbols from file
print(str)    # Print to console
fr.close()    # Close file
