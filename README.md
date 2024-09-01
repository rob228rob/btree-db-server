# <div style="color: red; font-weight: bold;"> **Warning:** This project is not intended for blatant copying or theft. Please respect the intellectual property and effort invested in this work. </div>

# Course Project: Data Storage and Management Algorithms

## Overview

This project focuses on developing algorithms for data storage and management using dynamic data structures in C++. The main objectives were strict adherence to C++17 standards and the creation of a flexible, modular program architecture. These priorities were critical in ensuring the code's stability, reliability, and portability across different platforms.

## Features

- **Modular Architecture**: The project is built around a clear separation of concerns, enhancing code reusability and ease of testing.
- **Abstract Interface**: The `storage_interface` was designed as an abstract interface using associative B-tree containers to manage data efficiently.
- **Storage Strategies**: Implemented `storage_strategy` class to handle data in both RAM and file system, ensuring flexibility and modularity in data management.
- **Efficient Memory Management**: The `user_data` class handles user information with a focus on efficient memory usage, leveraging a string pool to manage string data and prevent memory leaks.
- **Interactive Console Interface**: A user-friendly console interface supports both interactive command entry and script execution from files, improving usability and simplifying operation.

## Installation and Usage

To build and run the project:

```sh
mkdir build
cd build
cmake ..
./cw_os
```

## Results

The project successfully demonstrated the importance of adhering to C++17 standards and modular design in developing robust and portable applications. The implementation of an abstract interface and clear separation of storage strategies significantly enhanced the system's modularity and reusability. The focus on efficient memory management and a user-friendly interface contributed to the overall performance and usability of the application.
