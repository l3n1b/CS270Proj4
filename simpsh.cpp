#include <string>
#include <cstring>
#include <iostream>
#include <vector>

//test
//more test

using namespace std;

struct variable {
    string name;
    string value;
};

vector<variable> dictionary; //initialize variable list globally
vector<string> tokens; //initialize token list globally

//Scanner function to read a line and generate a list of tokens
void scanner(){ 

    char lineIn[256]; 
    fgets(lineIn, 256, stdin);

    string newToken = "";

    for(int i = 0; i < strlen(lineIn); i++){
        char next = lineIn[i];

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
}

void printTokens(){ //Debug function to list everything in tokens list
    for(int i = 0; i < tokens.size(); i++){ //print out all of the tokens as read by scanner function
        printf("%s\n", tokens.at(i).c_str()); //.c_str() allows printf to print c++ strings
    }
}

void error(string errMsg){ //error handling function
    printf("%s\n", errMsg.c_str());
}

void comment(){ // # //handle comment lines // I don't think we actually need to do anything
    printf("comment\n"); //Debug
}

void changeDirectory(){ // cd 
    printf("change directory\n"); //Debug
}

void listVariables(){ // lv // Should do what printDict does now
    printf("list variables\n"); //Debug

    for(int i = 0; i < dictionary.size(); i++){ //print out all of the tokens as read by scanner function
        printf("%s = %s\n", dictionary.at(i).name.c_str(), dictionary.at(i).value.c_str()); //.c_str() allows printf to print c++ strings
    }

}

void unset(){ // unset
    printf("unset variable\n"); //Debug

}

void execute(){ // !
    printf("execute program\n"); //Debug

}

void quit(){ // quit
    printf("quit shell\n"); //Debug
    
}

void assign(){ // =
    printf("assign variable\n");
    
}

void interpretCommand(){ //master function to call appropriate function based on first two tokens
    string firstToken = tokens.at(0);
    string secondToken = "";

    if(tokens.size() >= 2) //avoid out of range error
        secondToken = tokens.at(1);
    
    if(firstToken.compare("#") == 0){ // if first token is # indicating comment
        comment();

    } else if (firstToken.compare("cd") == 0){
        changeDirectory();

    } else if (firstToken.compare("lv") == 0){
        listVariables();

    } else if (firstToken.compare("unset") == 0){
        unset();

    } else if (firstToken.compare("!") == 0){
        execute();

    }  else if (firstToken.compare("quit") == 0){
        quit();

    } else if (secondToken.compare("=") == 0){
        assign();

    } else { //First token is just text
        error("Invalid command");
    }
}

int main(int argc, char **argv){    

    dictionary.push_back({"PATH", "/bin:/usr/bin:/home/jrli238/bin"}); //add default variables to dictionary of variables
    dictionary.push_back({"CWD", "/home/jrli238"});
    dictionary.push_back({"PS", "cs270>"});
    
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
//close any open files before fork
//Catch sigint so child process can be terminated but shell is not
//any # occurring other than the first token is a syntax error.
//Would probably be smarter to implement interpretCommand with switch/case statement instead of if-elses
//Need outto and infrom capabilities
//look for infrom: and outto: for redirecting input/output
//variable names ($ substitutes value) or general string (filename, argument, value)
//Could be insanely optimized by making pointers to input using strsep instead of copying everything to vectors
//Stuff about directories: Lecture 34 at minute 33

/* KNOWN PROBLEMS */
//Scanner doesn't handle "" correctly



