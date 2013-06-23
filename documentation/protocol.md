Intro
====

- Will be based on http://redis.io/topics/protocol
- memcached protocol is too specific for machine parsing I think

Examples
========

- for bash & nc:

```shell
echo $'*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n' | nc -q10 localhost 9876
```
