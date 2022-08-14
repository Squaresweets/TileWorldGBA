# TileWorldGBA
[![Discord](https://img.shields.io/discord/672474661388288021.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/6hswr9j)
[<img src="https://img.shields.io/twitter/follow/SqSweetsGames?style=social" /></a>](https://twitter.com/SqSweetsGames)
[<img src="https://forthebadge.com/images/badges/you-didnt-ask-for-this.svg" height=20/></a>](https://forthebadge.com)

<p align="center">
  <img src="https://github.com/Squaresweets/TileWorldGBA/blob/main/Art/Box.gif" alt="gamebox"/>
</p>

# What and why
Basically, I wanted to port something to the Gameboy Advance, and I thought it would be kinda funny to make a cross platform multiplayer game since it hadn't been done before. The perfect candidete for this was [TileWorld](https://tileworld.org) since I had worked on the mobile port and it is a pretty simple game. This whole thing has taken a long time, partially due to exams and partially due to lots of problem solving.

For playing this you have two options, either using a normal pico connected to your computer using the stuff found in the Python folder, or using the custom firmware to do it all just on a Pico W.

Latest video about the project can be found [here](https://www.youtube.com/watch?v=uGY5kjLEVD8)

Using this project requires [this adapter](https://stacksmashing.gumroad.com/l/gb-link)!

# Development checklist
- Physics ✔️
- Server -> Python communicaiton ✔️
- Python -> GBA communication ✔️
- Multiboot (Hard) ✔️
- Spawn loading ✔️
- Location information sending ✔️
- Loading screen ✔️
- Receiving tile places ✔️
- New chunks loading ✔️
- Changing tiles ✔️
- Other players ✔️
- Map system ✔️
- Wireless
  - TLS ✔️
  - Custom WS client ✔️
  - Downloading ROM from github ✔️
  - Sending data via PIO
  - Multiboot
- 3D printed game holder, and potentially a custom PCB

# Notes
The collision system used is a port of [this](https://github.com/dfranx/Colly) system by the creator of TileWorld.

Also, I just wanted to put it out there that this is my first C project, and my first GBA game, so none of this is particularly optimised or neat. This is a passion project and was done completly for fun and to learn C, so feedback is always appreciated!
