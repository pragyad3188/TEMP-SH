# TEMP-SH
A simple basic shell in linux kernel made in C language.
This project has a list of built-in commands, as well as the others which bash features.

## How To Run

Make sure you have GCC compiler installed. If you don't refer here.

Step 1. Download the source code to system in zip format or clone repo.
```
git clone https://github.com/pragyad3188/TEMP-SH.git
``` 
Step 2. On linux shell navigate to the folder 
```
cd TEMP-SH
```
Step 3. compile the shell using command 
```
gcc -o runshell  temp-sh.c utilities.c utilities.h linkedlist.c linkedlist.h
```
Step 4. run using command 
```
./runshell
```

## Tech Stack Used

- C language
- Use of pipe syscall present in `<unistd.h>`.
- Libraries such as `<stdlib.h>`for adding color.
- Some C libraries such as `<sys/wait.h>`,`<sys/types.h>` and `<signal.h>`.

## Important Logic Used
Some basic utilities as thought in the project course have been kept intact like
* Support for background jobs
* Builtin necessary command for "cd" as it cannot be executed in a child
* Use of fork() and execv() to process bash commands 
* Exiting the shell.
* Support for Signal Handling
* Killing of Processes

Some logic used for the Pipeline
* For a single stage pipeline, fork() two child processes from the parent process.They both share a pipe in common.
* For the first command make the stdout as one end of the pipe
* For the second command make the stdin as the other end of the previous pipe
* Wait for both the process to execute. 


## New Features

1. The support for a sinlge pipeline is successfully added using the pipe call privided by linux. Please Note that there must a space before and after "|" while giving the piped instruction. 
**Usage**
Run the shell and type in following command
```
 cat abc.txt
 ```
We can see a number of texts displayed and some repeated.
We can sort them and get the uniq ones only using pipeline by the following commmand.

```
    sort abc.txt | uniq
``` 
The output consists of sorted unique values.

2. Additional suppport for colours has been added to the shell.As of now red,blue,yellow,green,white  are supported but many more can be added. 
3. `sh_history()` method : Similar to the bash shell an history function is added which keeps track of the most recent 100 commands. They can be displayed in the following manner:- 
   **Usage** : 
   Once the shell is running type:- 
   ```
   history
   ``` 
4. `sh_help()` method : A quick guide for anyone stuck with the shell.It shows a list of features available 
   **Usage** : 
  Once the shell is running type:- 
   ```
   help
   ```

## Working Demo
- The working of the pipelines along with the colour codes and the history and help command has been demonstarted by the pictures below.
  
![Image 1](/image1.png)

![Image 1](/image2.png)

## Future Goals

- Addition of bash loop functionalities like for and while
- Addition and implementation of multiple pipelines
