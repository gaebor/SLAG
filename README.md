# SLAG
Standalone Lightweight Abstract Graph

## Aim
The SLAG aims to be a modular process graph, in which you can load, connect and run modules, interacting with each other.
It wants to be OS independent, as much as it is possible.
## Usage
The interface of the SLAG is rather simple, you have to implement a given set of C functions, and pack it in a shared library (SO) or a dynamic library (DLL).
From now on I call such a thing a *Library*.

If you implemented the required functions, your Library will be noticed by SLAG and SLAG will be able to run your code.
## The requirements
These should be implemented in your Library
* **Instantiate**: Your Library have to set up a *Module* in order to perform calculations
* **Initialize**: your instantiated Module have the one-time opportunity to get informations from the user.
* **Compute**: This function will be called from time-to-time and receive *Messages* and send out *Messages*
* **DestroyModule**: you have the opportunity to de-allocate or destroy your module
* **DestroyMessage**: every Message you produced will be Destroyed by this very function.

### Definitions
* **Module**: everything which can be called by name and can perform a computation.
* **Message**: everything what your Module sends out in its Compute function.

# Platforms

In theory, SLAG can be implemented for any platform which can load and run a C Library.
Of course, your Library have to compile for that particular platform (that's YOUR job).
Although, it is very understandable to fulfil one of the following Platform Requirements:

Input capability | Platform service | Output capability
---------------- | ---------------- | -----------------
you can locate and read (text) files | File Access | you can create and write files with given path
you can receive network packages, listen to ports | Network Access | you can send packages to network ports or provide a web service
you can capture graphic events, mouse, keyboard shortcuts, windowed applications, menus | Graphic Capability | you can put images and control objects on a window/gui

