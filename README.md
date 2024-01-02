**IMPORTANT!**

## MAIN DBA INTERFACE PROGRAM

1. Read the design documents.
2. First of all, take a look at `queryExamples.txt`
3. Do `make` (the Makefile of the system is in the package). Then
4. Do `mkdir ./config` : The program requires that there be a config directory in the base directory. 
       The metadata file is to be stored there;
5. Do `touch ./dataspace.rdb` : There must be `dataspace.rdb` file in the base directory.  All
   the data will be found in this file, but the program will fail if this file does not exist. The
   first time the program is run, this file should be empty;
6. Do "chmod 755 (or 700 or any code that gives RWE permissions to the owner of the "dbms".

==> Now you can run the main program `dbms`.




## TOP-LEVEL DBMS LIBRARY INTERFACE FUNCTIONS for developing an application program

We define an interface function so that a third party application can make use of our database system.

**Prototype**:

```c++
extern char* result;

int sql(char* query);

    char* query: any SQL query or a DBA command
```



**Return values**:

+ Returns 0, if successful.
+ Otherwise, returns negative integers depending on the error-causing functions.



**Implementation**:

We define a global variable (a character array) in the library that holds the results of the query. 

The application can access the global variable via "extern char* result". 

For the table-manipulation functions such as SELECT or INSERT, the result will be stored in the global variable. For other house-keeping functions such as OPEN DB, the result will be returned through the return value of the sql function.

In order to store the results from the table operations, display_table() function should be modified in such a way that all the "cout" or "printf" calls are followed by memcpy or sprintf function calls so that the results should also get copied into the global variable.



## Sample sql statements and DBA commands supported by the DBMS

It can be found in [queryExamples.txt](./queryExamples.txt).