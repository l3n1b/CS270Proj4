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

    subVars(); //replace $ variables with their values
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

    int rc = chdir(dirName.c_str());
    
    if (rc < 0) { //not valid directory 
        cout << "Error: not a valid directory path." << endl;
    }
    else{
      cout << "Directory changed successfully." << endl;  
      char tmp[256];
      getcwd(tmp, 256);
      dictionary[1].value = tmp; //updates cwd
    }
}

void listVariables(){ // lv // Should do what printDict does now
    printf("list variables\n"); //Debug
    for(size_t i = 0; i < tokens.size(); i++){
        if(tokens[i].compare("outto:") == 0){
            error("Cannot write to a file with lv");
            return;
        }
    }

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

    //set up variables and pointers for redirecting i/o streams in functions
    int readIn = -1;
    int writeOut = -1;

    //check for infrom: and outto: tokens
    for(size_t i = 0; i < tokens.size(); i++){


        if(tokens[i].compare("infrom:") == 0){
            readIn = i; // sets readIn to index of infrom: token
        }
        if(tokens[i].compare("outto:") == 0){
            writeOut = i; // sets writeOut to index of outto: token
        }
    } //need to remove tokens touched so they aren't passed to execute

    //printf("%d, %d\n", readIn, writeOut); //debug
    int saveIn;
    int saveOut;
    int fdIn;
    int fdOut;

    if(readIn >= 0){
        saveIn = dup(STDIN_FILENO);
        fdIn = open(tokens[readIn+1].c_str(), O_RDONLY|O_CREAT, 0777); //O_RDONLY isn't an error even though it has the red squigly
        //printf("file in fd: %d\n", fdIn); //debug
        dup2(fdIn, STDIN_FILENO);
        tokens.erase(tokens.begin() + readIn); //remove infrom: token and filename token
        tokens.erase(tokens.begin() + readIn);
    }

    if(writeOut >= 0){
        int offset = 0;
        if(readIn < writeOut && readIn != -1){ //if readIn already removed tokens
             offset = 2;
        }

        saveOut = dup(STDOUT_FILENO);
        fdOut = open(tokens[writeOut+1 - offset].c_str(), O_RDWR|O_CREAT, 0777); //O_RDONLY isn't an error even though it has the red squigly
        dup2(fdOut, STDOUT_FILENO);
        
        tokens.erase(tokens.begin() + writeOut - offset); //remove outto and following token
        tokens.erase(tokens.begin() + writeOut - offset); //-2 because infrom: and file tokens already removed
    }

    pid_t pid = fork();
    if(pid == 0){ //forks and only replaces child process

        //printf("wot? %s\n", tokens[1].c_str()); //DEBUG
        /* test code modified from execve man page example */
        //char chrAddress[] = "/home/jrli238/Documents/proj4/myecho"; //address of myecho test file
        char chrAddress[tokens[1].length() + 1];
        strcpy(chrAddress, tokens[1].c_str());

        char *newargv[256];
        char *newenviron[] = { NULL };

        newargv[0] = chrAddress;
        
        size_t i = 2;
        for(i = 2; i < tokens.size(); i++){
            newargv[i-1] = &tokens[i][0];
        }

        newargv[i-1] = NULL; //last argumnet must be NULL
        //printf("argv: %d %s\n", i, newargv[i-1]); //debug

        if((chrAddress[0] == '.' && chrAddress[1] == '/') || chrAddress[0] == '/'){
            execve(chrAddress, newargv, newenviron);
            perror("execve");   /* execve() returns only on error */
            exit(EXIT_FAILURE);
        } else {
            printf("no\n");
            exit(0);
        }
    } else {
        wait(NULL);
    }

    /* redirect input/output back to simple shell */
    if(readIn >= 0){
        //fixes files
        close(fdIn);
        dup2(saveIn, STDIN_FILENO);
        close(saveIn);
    }    

    if(writeOut >= 0){
        //fix files
        //printf("into file\n"); //debug
        close(fdOut);
        dup2(saveOut, STDOUT_FILENO);
        close(saveOut);
        //printf("out to stdout\n"); //debug
    }    
}

void assign(){ // =
    printf("assign variable\n");
    
    for(size_t i = 0; i<dictionary.size(); i++)
    {
        //cout << dictionary[i].name << " and " << tokens[0] << endl;
        if(dictionary[i].name.compare(tokens[0]) == 0)
        {
            if(dictionary[1].name.compare(tokens[0]) == 0) //catches CWD overwrite
            {
                cout << "You cannot overwrite CWD." << endl;
                return;
            }
            cout << "Overwriting variable: " << dictionary[i].name << endl;
            dictionary[i].value = tokens[2];
            return;
        }
    }
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

        //redirInOut(); //calls interpret command after managing input and output streams
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
//empty input file puts EOF on stdin
//Shouldn't be able to change CWD

/* LIST OF THINGS TO FIX AT END */
//remove bitches
//remove print tokens
//make tar ball
//make README file
//try to break ! execve