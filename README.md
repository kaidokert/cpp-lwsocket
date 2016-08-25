=====
stripped down BSD tcp socket wrapper, lwip-compatible
=====

API similar to Poco sockets, but without exceptions. Instead of exceptions, TSocket provide good()/bad() methods which need to be checked after connect(),recv() etc.

### TODO
* Add cmake build files
* travis
* Add [Catch](https://github.com/philsquared/Catch) / [Lest](https://github.com/martinmoene/lest) tests
* add cpplint
