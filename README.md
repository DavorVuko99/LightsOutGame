# Project: Lights Out

For this project, you will create a C++ graphics program that allows the user to play the game Lights Out.


## Requirements
Generate a fully functioning game, that can be beaten by a user. 
* Have a start screen
* Generate a random sequnece of squares that are turned on and off
* Ensure the sequance is beatable
* With every click of the mouse on square the square should turn on or off and so should all the adjacent ones
* Have a click counter
* when all the lights are turned off the user should be taken a win screen
* The win screen will display a win message, the number of clicks and the amount of time it took to win.

## Lights Out
Lights Out was a handheld game. You can play it online here: [https://www.logicgamesonline.com/lightsout/](https://www.logicgamesonline.com/lightsout/).

### Base Game
In its simplest form, the 5x5 grid of lights begins with all of the lights lit. When you click on one of the lights, it toggles itself and the (up to) four lights it borders.

Here's a gif of the beginning of the game, where the lights have a red outline hover effect: 

![Lights-Out-Game-Start.gif](Lights-Out-Game-Start.gif)

When you make all the lights go off, you win the game and can no longer click on the lights:

![Lights-Out-Game-End.gif](Lights-Out-Game-End.gif)

For testing purposes, here are the lights you need to click to get from a fully lit start to a fully unlit end:

| | | |*|*|
|-|-|-|-|-|
|*|*| |*|*|
|*|*|*| | |
| |*|*|*| |
|*| |*|*| |

## Future Alterations:
I actually have some plans to expand on this game in a few ways:
* On the start screen I wan to add a level selector, to allow the user to choose a 5x5, 6x6 or 7x7 array of lights
* I want to add a button to select diffent themes of the buttom
* I want to a audio to the game, music, game sounds or other clicks.

