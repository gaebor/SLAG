# SLAG
Standalone Lightweight Abstract Graph

## Aim
The SLAG aims to be a modular process graph, in which you can load, connect and run modules, interacting with each other.
It wants to be OS independent, as much as it is possible.

## Dependencies
In order to compile SLAG all you need is
* compiler which supports C++11, or VisualC++
* [AsyncQueue](https://github.com/gaebor/human_readable/AsyncQueue)
* Optionally [OpenCV](https://opencv.org/)

## Usage
The interface of the SLAG is rather simple, you have to implement a given set of C functions, and pack it in a shared library (SO) or a dynamic library (DLL).
From now on I call such a thing a *Library*.

If you implemented the required functions, your Library will be noticed by the SLAG executable and SLAG will be able to run your code.
## The requirements
These should be implemented in your Library
* **Instantiate**: Your Library have to set up a *Module* in order to perform calculations
* **Initialize**: your instantiated Module have the one-time opportunity to get informations from the user.
* **Compute**: This function will be called from time-to-time and receive *Messages* and send out *Messages*
* **DestroyModule**: you have the opportunity to de-allocate or destroy your module
* **DestroyMessage**: every Message you produced will be Destroyed by this very function.

### Definitions
* **Module**: everything which can be called by name and can perform a computation.
  It can be a pointer to a C function (with a given signature) or a pointer to a C++ instance, inherited from an abstract class (with a given set of members)
* **Message**: everything what your Module sends out in its Compute function.
  It can be a pointer to a variable (what your Module allocated, you know its type and your Module will free it).

## Platforms
In theory, SLAG can be implemented for any platform which can load and run a C Library.
Of course, your Library have to compile for that particular platform (that's YOUR job).
Although, it is very understandable to fulfil __one of the following Platform Requirements__:

Input capability | Platform service | Output capability
---------------- | ---------------- | -----------------
you can locate and read (text) files | File Access | you can create and write files with given path
you can receive network packages, listen to ports | Network Access | you can send packages to network ports or provide a web service
you can capture graphic events, mouse, keyboard shortcuts, menus | Graphic Capability | you can put images and control objects on the screen/gui

If one of the following capabilities are available on your platform (including both input and output side), then SLAG be tuned and initialized
according to user needs and it can perform your calculations and it can show the results to you as well.

If none of these requirements are available then we are talking about a washing machine
and, as sorry as I am, SLAG cannot perform a decent run on your platform.

Just for fun, let's imagine the following scenarios:
* SLAG runs on your Smart Phone, it can access neither files nor the network, but you can tap and see the screen.
  That is perfectly enough for SLAG, as log as you can load and run the Modules from Library.
  It is totally fine for a facial recognition application.
* SLAG runs on your RaspberryPi, with a perfectly fine console and file access, but without network or GUI.
  That's suitable for any traditional console application, so is for SLAG.
* You don't have a console, neither network, just a window, that's Microsoft Windows.
  SLAG can render your output text and images into a window. You can interact with your Modules via mouse.
* Last, but not least, imagine that your Platform have nothing but a network access, like a distant server.
  Even is you cannot SSH to the server, you can upload your Modules, run them on the server and see the output in a server-hosted webpage.

These are far from implementation, but the internal operating logic of the SLAG makes all these possible.
