# Delibird 2020
### Pandemiga Team
UTN's Operating Systems Project

[Link to the Objectives can be found here.](https://docs.google.com/document/d/1be91Gn93O2Vp8frZoV1i5CmtOG0scE1PS8dMHsCP314/edit)

TLDR: Project in C that implements Sockets, Multithreading, Server-client arquitectures, Unit testing, and 4 processes working congruently to catch and manage pokemons in memory.

# Instructions

## VSCODE:

-Clone the repository

-Open VSCode

-File->Open Workspace

-Select "vscode-workspace"

-To compile and execute, Select Tasks -> Execute Task -> (the process you want, with or without Valgrind)
If you'd rather use keybinds you can do so as explained [here](https://code.visualstudio.com/docs/editor/tasks#_binding-keyboard-shortcuts-to-tasks) 

-To Debug, go to the Debug section to the left and select the program to execute. Then, press F5 or the green button.
Debugging DOES NOT compile first and DOES NOT use Valgrind.

## CONSOLE:

-Go to the project's file (broker, for instance)

-Write 'make'

-Write './(projectsname)'

-If you want, write './exec' or './exec_valgrind' to do both steps at the same time.
