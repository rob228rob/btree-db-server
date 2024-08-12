Course project on computer operating systems and
architecture
"Development of algorithms for data storage and management
based on dynamic data structures"

To run this code, use:

```
mkdir build
cmake ..
./cw_os
```

Results:
  
  During the course of the course project on developing an application in the C++ programming language for managing data collections, the main focus was on strict compliance with the standards of the C++17 language and building a flexible and modular program architecture. 

This aspect has proven to be critically important for ensuring the stability, reliability and portability of code on various platforms.
A key element of the work was the creation of an abstract storage_interface interface that uses associative B-tree containers to organize data. 

This approach was chosen based on the analysis of modern data processing and management methods presented in the C++ documentation. As part of this interface, the storage_strategy class was implemented, designed to work with data in RAM and on the file system, respectively. 

This approach provided a clear separation of the logic of working with different types of storage, which significantly increased the modularity and reusability of the code, as well as simplified its testing and debugging.

The user_data class has become one of the central components of the system, responsible for storing user information. 
In this implementation, special attention was paid to proper memory management and efficient handling of string data through the use of a string pool. 

The class provides a complete set of methods for manipulating data, including constructors, assignment operators, and specialized methods for creating objects from string data. 

This functionality allowed us to ensure high performance and minimize the likelihood of memory leaks.
An essential part of the work was to ensure effective user interaction with the application. 

To do this, a user-friendly console interface has been implemented, which allows you to enter commands interactively, as well as execute predefined scripts by reading commands from files. 
This approach has significantly improved the usability of the program and simplified the process of its configuration and operation.
