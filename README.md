# picseye
A project solution towards resolving small dataset image recognition

## clj-server
- client JNI implementations
- clojure java interop
- RESTful APIs

## cpp-client
- connect to server to request image matching
- via zeromq and event polling

## cpp-common
common protocol and data structures

## cpp-server
- single thread looper 
- block handling
- query search-tree

## ios-client
- cpp-client wrapped
- Objective-c++ OpenCV 2.4
- directly access clj-server

## tools
### descriptors-import
extract images descriptors and save them to database

### local-pics-tester
test matching results

### pics-fetch
fetch images from remote servers
