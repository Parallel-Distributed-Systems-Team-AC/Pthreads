# Pthreads
This repository holds the files for the parallel implementation of the Coloring Algorithm using Pthreads


## Compiling
Run this command for compiling and creating the executable:

    gcc our_parallel_functions.c  main_pthreadsV1.8.c mmio.c  -o "executable_name" -O3 -lpthread  

## File Loading 

When running the program you will notice that a list of all graph .mtx files appers asking the user to give a number in order to open a specific file. This was implemented, taking into concideration the different ways files needed to be loaded, in order to automate in a way the loading procedure. 

In order for this to be functional you need to add all the .mtx files you want to test in the `files` folder. The first .mtx file is already added to the folder. 
