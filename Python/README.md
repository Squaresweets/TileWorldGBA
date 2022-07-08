So you want to do stuff without the pico W, sure!

Just change the driver for the pico to then set up your python IDE of choice (I use [pycharm](https://www.jetbrains.com/pycharm/)). Install the libraries needed using pip and make sure that TileWorldGBA_mb.gba is in the same directory as the python file. 

1. Change the driver for the pico to libusbK (v3.1.0.0) using [Zadig](https://zadig.akeo.ie/) 
2. Put the gbusb.utf firmware on the pico
2. Set up your python IDE (I use [pycharm](https://www.jetbrains.com/pycharm/))
3. Install the libraries needed using pip
4. Make sure TileWorldGBA_mb.gba is in the same directory as the python file
5. Run TileWorldClient.py