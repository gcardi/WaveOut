# WaveOut
## Very simple audio output class for bcc32c/bcc64 compilers

### Description

It is a very small header-only library for sending audio with Windows. It consists of only one header file, although an "empty" .cpp file is paired with the .h file to form a so-called Unit.

Internally, it uses a callback procedure from Multimedia API (mmeapi.h header) to produce another callback in the context of the running class instance (object), so be careful because the code of the callback runs in the context of a separated multimedia thread. 

### Usage
