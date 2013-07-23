#!/usr/sbin/dtrace -s

/*
 * Analyze queries to boostcache server
 *
 * USAGE: ./analyze.d PID
 *
 * TODO: add more events/probes/informatation/stats/aggregation
 */

#pragma D option quiet

dtrace:::BEGIN
{
    printf("boostcached (%d) is now under dtrace\n", $1);
}

dtrace:::END
{
    printf("dtrace is detached from boostcached (%d)\n", $1);
}

pid$1::*executeCommand*:entry
{
    self->m_start = timestamp;
    self->m_end = 0;
}

pid$1::*executeCommand*:return
/self->m_start/
{
    this->m_end = timestamp - self->m_start;
    printf("query %d ms\n", (self->m_start - self->m_end) / 1000000)
}

