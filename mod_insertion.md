# Add support for sockets service for vizzy
Read .cache/guide/SocketsMod.xml as reference for compilation

## Features
 - add a setting on configuration to enable sockets service mod 
 - add the new blocks and put them in the docs and in stl (as mod support only)
 - for the event blocks, add `on socketReceive(port, data...)` event as well as `on socketError(data)`