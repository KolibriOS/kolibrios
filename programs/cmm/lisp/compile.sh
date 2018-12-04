#!/bin/bash
for file in `find ./ -type f -name "*.c"`
do
   cmm $file;
done