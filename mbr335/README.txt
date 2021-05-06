GROUP MEMBERS:
Leni Broady - worked with CD and integrating into entire program. Made sure variables
              could be changed except CWD in assign function. Also researched, added to main, 
              and went back into old projects to find resources to add to this one (i.e. signal handlers, etc.),
              helped with debugging.
Joseph Lindemuth - Wrote scanner function, and functions for !, unset, lv, and part of =. Wrote most helper functions, 
                   and started main. Laid out skeleton code in the beginning to start the project. 


FILES:
(1) Makefile
(2) simpsh.cpp
(3) README


ALGORITHMS:
    subVars(): loops through tokens to find $. When $ is found, it indicates a substitution of variables. 
    quit(): quits shell with exit(0)
    scanner(): reads user input and loops through characters in input to find special cases 
    printTokens(): loops through tokens to print tokens as read by scanner function. Used for debugging.
    changeDirectory(): uses getcwd to change directory and updates CWD variable
    listVariables(): loops through tokens and prints variables as read by scanner function. Checks for outto and throws error if it is present. 
    unset(): loops through each dictionary value to check if it matches- if it does, it is unset. Does not unset default variables. 
    execute(): sets up multiple cases (infrom/outto, etc.) and forks to execute selected file. Then, it waits for you to get done. 
    assign(): assigns value to variable in dictionary vector by looping through and comparing variable to existing variables. 
              If it does not exist, it adds a new variable. Otherwise, it overwrites. 
    interpretCommand(): calls appropriate function from input token using if/else statements. 


SPECIAL FEATURES/KNOWN LIMITATIONS:
(1) If you type something into the shell, you will have to type CTRL+D twice to exit the shell. 
