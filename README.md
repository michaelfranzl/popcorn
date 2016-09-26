# Popcorn

Popcorn is a multi-platform (Windows, Linux, Mac OSX), server-less, instant messaging desktop application that allows small to medium-sized groups on the same LAN subnet to send messages ("pop-ups", hence the name "popcorn") as well as files of arbitrary size.

Communication in the LAN is server-less: Participant discovery is done via UDP broadcast, after which TCP connections are created directly between participants which serve as permanent communication channels.

Popcorn is implemented as a thin and generic Qt C++ API which is exposed to the Javascript space via a QtWebkit/QtWebpage widget. The entire 'business logic' can be implemented in Javascript, whereas the Qt C++ code has been kept to a minimum.

JavaScript can control the C++ backend to implement a simple communication protocol. The user interface can be implemented in HTML and JavaScript.

Unlike most instant messaging applications, incoming messages can raise the window from the minified state (if configured to do so). Popcorn is therefore suitable to communicate messages that require immediate attention, as is the case in closely networked work groups.

This C++ backend also includes an interface to a SQlite database.



## Motivation

I started this project as a better replacement for a Windows-only LAN messaging client that has been around for years.


## Project status

The code is fairly stable on all three platforms (Windows, Mac OSX and Linux). Code documentation is lacking, however also code is documentation.

I do not provide precompiled packages. Therefore, at this point, this project serves only for educational or experimental purposes.


## WAN extension

It is possible to transparently expand the communication to users outside of the LAN subnet since the TCP sockets don't care if they are connected to the LAN or the WAN. For this purpose, a simple and central instant message relaying server can be implemented.


## Compilation

The compilation of the program requires some knowledge of build systems of the 3 major operating systems.


### Debian/Ubuntu

First install dependencies:

    apt-get install qt5-default libqt5webkit5-dev libqt5network5 libqt5sql5-sqlite libqt5widgets5 libqt5gui5 libqt5multimedia5 libqt5multimedia5-plugins qtmultimedia5-dev libqt5x11extras5-dev libxss-dev build-essential

Go into the source directory and run `qmake`:
    
    qtchooser -run-tool=qmake -qt=5
    
To compile:

    make
    
Then run it:

    ./poporn
    
Get help with:

    ./popcorn -h
    

### Windows and Mac OSX

Please refer to the Qt documentation.