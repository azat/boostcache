What is this
===========

Memory object caching system

[![Build Status](https://travis-ci.org/azat/boostcache.png?branch=master)](https://travis-ci.org/azat/boostcache)

Why boostcache?
===============

For now it is boost because used boost tree, but maybe in future it will be boost because it is fast.

Used libraries
==============

- [boost program options](http://www.boost.org/libs/program_options)
- [boost algorithms (rbtree|avltree)](http://www.boost.org/libs/intrusive)
- OR [stxxl library](http://stxxl.sourceforge.net/)
- [boost asio](http://www.boost.org/libs/asio)
- [cmake for builds](http://cmake.org/)
- [boost log (proposal)](http://boost-log.sourceforge.net/)

Building
========

boostcache is using cmake for it

You can also view into [.travis.yml](.travis.yml),
but in short you can run next commands on debian like distribution:

```shell
sudo add-apt-repository -y ppa:azat/boost-unofficial
sudo apt-get update
sudo apt-get install libboost1.53-all-dev

mkdir .cmake
cd .cmake
cmake ..
make
```
