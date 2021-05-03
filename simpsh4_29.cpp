//Started from Github commit "what??"

#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;

struct variable {
    string name;
    string value;
};

vector<variable> dictionary; //initialize variable list globally
//PATH = dictionary[0].value
vector<string> tokens; //initialize token list globally

void error(string errMsg){ //error handling function
    printf("%s\n", errMsg.c_str());
}

//function for substituting variables in when $ is found
void subVars(){ 
    for(size_t i = 0; i < tokens.size(); i++){ //loop through tokens, type size_t instead of int so signeeness is the same in comparison
        string token = tokens.at(i);   
 
        if(token[0] == '$'){ //check for $ variable indicator
            token = token.substr(1, token.length()); //remove $ from token
            printf("token %s starts with $\n", tokens.at(i).c_str());

            for(size_t j = 0; j < dictionary.size(); j++){ //loop through variables
                if(token.compare(dictionary.at(j).name) == 0){
                    tokens.at(i) = dictionary.at(j).value;
                }
            }
            
        }
    }

    return;
}

void quit(){ // quit
    printf("quit shell\n"); //Debug
    exit(0); //exit with status 0
    
}

//Scanner function to read a line and generate a list of tokens
void scanner(){ 

    printf("%s ", dictionary[2].value.c_str()); //prints prompt string before input
    char lineIn[256]; 
    fgets(lineIn, 256, stdin); //reads user input

    if(feof(stdin)) // End-of-file EOF in standard input meaning CTRL+D was pressed
        quit();

    tokens.clear(); //empties tokens vector

    // for(int i = 0; i < 256; i++) debug
    //     printf("%x ", lineIn[i]);
    
    // if(lineIn[0] == 0){ //exits if CTRL+D pressed on empty line except it doesn't work
    //     exit(0);
    // }  


    string newToken = "";

    for(size_t i = 0; i < strlen(lineIn); i++){
        char next = lineIn[i];
        if (next == '#' && i > 0){
            error("Encountered invalid symbol # in input");
            tokens.clear();
            return;
        }

        if (next == '\"') { //character is " so read everything between quotes as a single token
            do { //reads up until next "
                i++;
                next = lineIn[i];
                newToken += next;
            } while(next != '\"');

            newToken.pop_back(); //remove final " from end string

        } else if(next != 0x20 && next != 0x09 && next != 0x0a && next != 0x0d){ //whitespace is 0x20, 0x09, 0x0a (newline), and 0x0d
            newToken += next; //build token

        } else { //character is whitespace
            if(newToken.length() > 0) //make sure token isn't empty (would occur if user typed multiple spaces)
                tokens.push_back(newToken); //add new token to array

            newToken = ""; //reset token
        }
    }

    subVars();
}

void printTokens(){ //Debug function to list everything in tokens list
    for(size_t i = 0; i < tokens.size(); i++){ //print out all of the tokens as read by scanner function
        printf("%s\n", tokens.at(i).c_str()); //.c_str() allows printf to print c++ strings
    }
}

void comment(){ // # //handle comment lines // I don't think we actually need to do anything
    printf("comment\n"); //Debug
}

void changeDirectory(string dirName){ // cd COME BACK LATER

//if there is not a /home/mbr335, you should be able to assume? 

    /* Code Joseph experimented with */
    DIR *directory;
    struct dirent *de;


    //leni test
    int rc = chdir(dirName.c_str());

    cout << "I'm about to go into error?" << endl; //debug baybeeeee
    if (rc < 0) {
        cout << "Error: not a valid directory path." << endl;
    }
    else{
      cout << "Directory changed successfully." << endl; //debug  
      dictionary[1].value = dirName; //updates cwd
    }
    

    //end leni test


    if(!(directory = opendir(dictionary[1].value.c_str()))){
        error("Failed to open directory");
    } else { //directory opened
        //prints every file in directory indicated by CWD
        while (0 != (de = readdir(directory))){
            printf("Found file: %s\n", de->d_name);
        }
    }

    closedir(directory);
    // end of Joseph code

//WHAT I'M TRYING TO DO IS MERGE OVERLAPPING STRINGS
//check if this is legit https://docs.w3cub.com/cpp/filesystem/exists
//hello world
    string base;
    //cout << dictionary[1].value << endl;
    //cout << dictionary[1].value.find(dirName) << endl;
    
    /* Possibly 'twas kinda dumb
    if(dictionary[1].value.find(dirName) != string::npos){ //if the inputted directory name is part of the directory
    //you are currently in 
    printf("you are in overwriting function\n"); //DEBUG
    
        int val = dictionary[1].value.find(dirName); //this is the value at which the dirName begins to overlap in CWD
        base = dictionary[1].value.substr(0, val); //this is where they differ
        dictionary[1].value = base + dirName; //new directory name yeet
    }
    else{
    printf("you are NOT in overwriting function\n"); //DEBUG
    //changes value of CWD
    base = dictionary[1].value;
    dirName = base + dirName;
    dictionary[1].value = dirName;
    }
   */
    printf("change directory\n"); //Debug
}

void redirInOut(){
    int readIn = -1;
    int writeOut = -1;
    for(size_t i = 0; i < tokens.size(); i++){
        if(tokens[i].compare("infrom:") == 0)
            readIn = i; // sets readIn to index of infrom: token
        if(tokens[i].compare("outto:") == 0)
            writeOut = i; // sets writeOut to index of outto: token
    }

    if(readIn >= 0){
        int fdIn = open(tokens[readIn+1].c_str(), O_RDONLY|O_CREAT, 0777); //O_RDONLY isn't an error even though it has the red squigly
        //printf("file in fd: %d\n", fdIn); //debug
        dup2(fdIn, 0);
    }

    //could fork and wait to not have to worry about file opening
    //pid_t pid = fork();
   //if(pid == 0){} //child
    if(writeOut >= 0){
        int saved = dup(STDOUT_FILENO);
        int fdOut = open(tokens[writeOut+1].c_str(), O_RDWR|O_CREAT, 0777); //O_RDONLY isn't an error even though it has the red squigly
        dup2(fdOut, STDOUT_FILENO);

        //fix files - needs to happen after action
        printf("into file\n");
        close(fdOut);
        dup2(saved, STDOUT_FILENO);
        close(saved);
        printf("out to stdout\n");
    }
    
}

void listVariables(){ // lv // Should do what printDict does now
    printf("list variables\n"); //Debug
    redirInOut(); //checks for infrom: and outto: tokens and redirects input and output appropriately


    for(size_t i = 0; i < dictionary.size(); i++){ //print out all of the tokens as read by scanner function
        printf("%s = %s\n", dictionary.at(i).name.c_str(), dictionary.at(i).value.c_str()); //.c_str() allows printf to print c++ strings
    }

}

void unset(string varName){ // unset
    printf("unset variable\n"); //Debug
    
    bool skip = (varName.compare(dictionary[0].name) == 0 || varName.compare(dictionary[1].name) == 0 || varName.compare(dictionary[2].name) == 0);
    bool success = false;

    if(!skip){
        for(size_t i = 0; i < dictionary.size(); i++){
            if(dictionary[i].name.compare(varName) == 0){
                    dictionary.erase(dictionary.begin() + i);
                    success = true;
            }
        }
    } else {
        error("Cannot unset default variables");
        return;
    }

    if(success){
        printf("Variable %s successfully removed\n", varName.c_str());
    } else {
        printf("Variable %s could not be found\n", varName.c_str());
    }
}

void execute(){ // !
    printf("execute program\n"); //Debug

    pid_t pid = fork();
    if(pid == 0){ //forks and only replaces child process

        //printf("wot? %s\n", tokens[1].c_str()); //DEBUG
        /* test code modified from execve man page example */
        //char chrAddress[] = "/home/jrli238/Documents/proj4/myecho"; //address of myecho test file
        char chrAddress[tokens[1].length() + 1];
        strcpy(chrAddress, tokens[1].c_str());

        // char hello[] = "hello";
        // char world[] = "world";

        char *newargv[256];
        char *newenviron[] = { NULL };

        newargv[0] = chrAddress;
        
        // char arg1[tokens[1].length() + 1];
        // strcpy(arg1, tokens[2].c_str());
        // newargv[1] = arg1;
        // newargv[2] = NULL;

        
        size_t i;
        for(i = 2; i < tokens.size(); i++){
            char tempStr[tokens[i].length() + 1];
            strcpy(tempStr, tokens[i].c_str());
            newargv[i-1] = tempStr; //newargv[1] is a pointer to tempStr, but tempStr is destroyed after this loop
            // printf("tempStr: %s\n", tempStr);
            // printf("argv %d: %s\n", i, newargv[i-1]);
        }
        // printf("argv: %d %s\n", i-1, newargv[i-2]);
        newargv[i-1] = NULL; //last argumnet must be NULL
        // printf("argv: %d %s\n", i, newargv[i-1]);


        // for(int i = 0; i < 2; i++){
        //     printf("%s\n", newargv[i]);
        // }
        // printf("argv0: %s\n", newargv[0]);
        // printf("argv1: %s\n", newargv[1]);

        execve(chrAddress, newargv, newenviron);
        perror("execve");   /* execve() returns only on error */
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }
}

void assign(){ // =
    printf("assign variable\n");
    dictionary.push_back({tokens[0], tokens[2]});
}

void interpretCommand(){ //master function to call appropriate function based on first two tokens

    if(tokens.size() == 0) //avoid out of range error
        return;

    string firstToken = tokens.at(0);
    string secondToken = "";

    if(tokens.size() >= 2) //avoid out of range error
        secondToken = tokens.at(1);
    
    if(firstToken.compare("#") == 0){ // if first token is # indicating comment
        comment();

    } else if (firstToken.compare("cd") == 0){
        changeDirectory(secondToken);

    } else if (firstToken.compare("lv") == 0){
        listVariables();

    } else if (firstToken.compare("unset") == 0){
        unset(secondToken);

    } else if (firstToken.compare("!") == 0){
        execute();

    }  else if (firstToken.compare("quit") == 0){
        quit();

    } else if (secondToken.compare("=") == 0){
        assign();

    } else { //First token is just text
        error("Invalid command");
        return;
    }

}

void sigint_handler(int sig){ //SIGINT handler

    sigset_t mask, prev_mask;
    sigfillset(&mask);                         //place all signals in mask
    sigprocmask(SIG_BLOCK, &mask, &prev_mask); //block all signals

    printf(" Caught sigint BITCHES!\n"); //debug, remove BITCHES before submission

    // Restore previous blocked set 
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
}

typedef void handler_t(int);
handler_t *Signal(int signum, handler_t handler){
  struct sigaction action, old_action;

  action.sa_handler = handler;
  sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
  action.sa_flags = 0; /* Restart syscalls if possible */

  if (sigaction(signum, &action, &old_action) < 0)
    fprintf(stderr, "%s: %s\n", "Signal error", strerror(errno));
  return (old_action.sa_handler);
}

int main(int argc, char **argv){    

    dictionary.push_back({"PATH", "/bin:/usr/bin:/home/mbr335/bin"}); //add default variables to dictionary of variables
    char tmp[256];
    getcwd(tmp, 256);
    dictionary.push_back({"CWD", tmp});
    dictionary.push_back({"PS", "cs270>"});

    Signal(SIGINT, sigint_handler);  //install SIGINT handler

    /* loop forever, read line of input, break input into tokens,  
    check for valid syntax, carry out operation or print error message*/
    for(;;){ //main infinite loop

        scanner(); //run scanner to read user input into tokens list
        
        printTokens(); //Debug

        interpretCommand(); //call command function based on user input
    }
}

/* NOTES */
//when calling execve, we need to set up the third argument which is the environment variables pointer
//close any open files before fork L34, M18
//Would probably be smarter to implement interpretCommand with switch/case statement instead of if-elses
//Need outto and infrom capabilities
//look for infrom: and outto: for redirecting input/output
//Could be insanely optimized by making pointers to input using strsep instead of copying everything to vectors
//Stuff about directories: Lecture 34 at minute 33
//CWD cannot be changed directly, but must be kept up to date with cd command
// remove any //debug lines

/* KNOWN PROBLEMS */
//Scanner doesn't handle "" correctly
//Handling CTRL+D is bad
//execve doesn't take arguments
//make variable assignments with same name replace eachother
//empty input file puts EOF on stdin

/* WDLD? (what did Leni do ) */
//fixed CWD so it automatically updates
