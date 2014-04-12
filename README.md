What is this
------------

Memory object caching system

Why boostcache?
---------------

For now it is boost because used boost tree, but maybe in future it will be boost because it is fast.

Build status and benchmarks
---------------------------

[![Build Status and Benchmarks](https://travis-ci.org/azat/boostcache.png?branch-coverity)](https://travis-ci.org/azat/boostcache)

[![Coverity scan](https://scan.coverity.com/projects/1583/badge.svg)](https://scan.coverity.com/projects/1583)

![boostcache benchmarks](http://tiny.cc/boostcache-benchmarks "benchmarks")

Used libraries
--------------

- [boost program options](http://www.boost.org/libs/program_options)
- [boost intrusive containers (rbtree|avltree)](http://www.boost.org/libs/intrusive)
- OR [stxxl library](http://stxxl.sourceforge.net/)
- [boost asio](http://www.boost.org/libs/asio)
- [cmake for builds](http://cmake.org/)
- [boost log](http://www.boost.org/libs/log)

Building
--------

boostcache is using cmake for it

You can also view into [.travis.yml](.travis.yml),
but in short you can run next commands on debian like distribution:

```shell
sudo apt-get install libboost1.54-all-dev

mkdir .cmake
cd .cmake
cmake ..
make

# Add autocompletion for bash
. ../utils/bash_completion
# Run server
./boostcached -f
# Run CLI client
../utils/boostcache
Go ahead!
```

