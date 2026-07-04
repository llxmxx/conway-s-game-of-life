# conway's game of life
implementation of john conway's game of life in c++ using raylib.
<img width="600" height="417" alt="image" src="https://github.com/user-attachments/assets/50930943-dc51-4d2a-aa3f-88b8256fa596" />
<img width="602" height="419" alt="image" src="https://github.com/user-attachments/assets/b6a564e7-873a-4b5d-a7bd-44d73eaaebb5" />


### what is it?
conway's game of life is a zero-player cellular automaton devised by mathematician john horton conway. each cell on a grid is dead or alive, and its state evolves over generations based solely on the number of living neighbours.

### features
#### simulation
pannable & zoomable grid, adjustable simulation speed, generation and population counters, pause, play and step controls
#### editing tools
brush, eraser, line, rectangle, selection
#### pattern library
glider, lightweight spaceship, middleweight spaceship, heavyweight spaceship, gosper glider gun, pulsar, pentadecathlon, acorn, r-pentomino
#### save/load
save multiple(upto 10) worlds, load previously saved worlds
#### ui
dark/light theme, toast notifications

### controls
#### mouse
##### left click - place/erase cells or move selection
##### right click - erase/place cells
##### middle click - pan
##### mouse wheel - zoom
#### keyboard
|key | action|
|----|-------|
|space | play/pause|
|n | step one generation|
|ctrl + z | undo|
|ctrl + y | redo|
|ctrl + c | copy selection|
|ctrl + v | paste|
|ctrl + x | cut selection|
|x | mirror selection/pattern|
|y | rotate selection/pattern|
|del | delete selection|
|c | clear and reset grid|
|r | randomize grid|
|- | decrease speed|
|+ | increase speed|

### installation
1. download the latest release
2. extract the zip file
3. run the .exe
