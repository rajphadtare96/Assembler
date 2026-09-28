Assignment: Scan C Files and Extract Program Descriptions

Name: Raj Phadtare
Roll No.: 25111046




This program recursively scans a given directory and its subdirectories
to find all C source (.c) files.

For every C file, it reads the beginning of the file and extracts the
program name and description from the comment header.

The extracted information is displayed on the terminal and also saved
to output.txt.


Compile using GCC:

gcc program.c -o program


Run the executable:

./program

Enter the directory path when prompted.

Example:

Enter directory path: /home/raj/nasm


The program prints the program names and descriptions on the screen.

The same output is also saved in output.txt.


stdio.h
stdlib.h
string.h
dirent.h
sys/stat.h


1. Understanding recursive directory traversal.
2. Reading only the header comments from each C file.
3. Extracting the filename and description using sscanf().
4. Writing the extracted data to both the terminal and output.txt.
5. Handling directories and files correctly using dirent.h.



Used recursion to scan all subdirectories.
Used fgets() to read file contents line by line.
Used sscanf() to extract filename and description.
Used fprintf() to store the output in output.txt.


The program successfully scans all C source files, extracts their
program descriptions, displays them on the terminal, and saves the
results in output.txt.
