# The plan:
Test if this works: https://github.com/peterharperuk/pico-examples/blob/524a0ca9f926e661da06ac7bc4bc0f6f9632c393/pico_w/tls_client/picow_tls_client.c
First I am probably going to have to clone mbedtls with this: https://github.com/peterharperuk/pico-sdk/commit/72b8313c84266d9dbe05be67111c034bc0503cce
If that works we have a base to put other stuff on which supports tls, that is positive

Next up, port https://github.com/samjkent/picow-websocket to work with the previous system.
Hopefully connect to the Tileworld server and send the connect packet to make sure all is well.

Next up, port multiboot into c, shouldn't be too hard, but idk how to actually put files like the .gba file onto the pico.
Then get the sio system working with the gba

Finally, make a system which allows you to input text into the gba to allow you to choose a wifi SSID and password. (maybe)