#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include "mbed.h"


template <typename F>
void call(F f);

template <typename F>
void call_every(events::EventQueue::duration ms, F f);

void dispatch_start();

void dispatch_stop();

#endif