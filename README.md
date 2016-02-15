# OpenGL Cureves - Project 1b
__Author: R. Alex Clark__

Simple project working with OpenGl curves. All files included are based off of the project codebase provided by J. Peters and the ![OpenGL Tutorials](http://www.opengl-tutorial.org/download/). I claim none of the prebuilt codebase as my own.

## Setup

* Download the OpenGl Tutorials provided with the link.
* Setup the files in the misc05 picking project included. 
* Remove references to the Bullet Physics and other builds from the Makefiles (you will not need them and they will prevent compilation of the project if not removed now).
* Compile project with cmake (GUI or command line tool both work fine) and run project build

## Implementation of Assigned Tasks

### Task 1 - Subdivision
Pressing and releasing key '1' causes subdivision of the verticies to occur. This can be layered up to 5 times. After level 5, subdivision will reset to standard.

### Task 2 - Bezier Curves
Pressing and releasing key '2' displays standard bezier curves calculated using following formulas:
* Control Point c0: Avg. of (((2P[i-1] + P[i]) / 3) + 2P[i] + P[i+1])) / 2
* Control Point c1: 2P[i] + P[i+1] / 3
* Control Point c2: P[i] + 2P[i+1] / 3
* Control Point c3: (((P[i] + 2P[i+1] / 3) + (2P[i+1] + P[i+2]))) / 2

### Task 3 - Catmull Rom Curves w/ Decastlejau Algorithm
Pressing and releasing key '3' displays Catmull-Rom splines (red) and curve calculated using the Decastlejau algorithm (green curve).

## Notes
R. Alex Clark - 2016
I do not claim any of this material's codebase as my own. All of the work included is either from the codebase or of my own design and I have received no unacknowledged aid on this project.

Project does not work on OSX, as the Intel GPUs are incompatible with OpenGL 3.3+ using the current project setup given by the OpenGL tutorial build.
