/**
 * File:   myhjson.h
 * Author: Yongkang Li
 *
 * Created on July 05, 2023, 15:03 PM
 */

#ifndef MY_HJSON_H
#define MY_HJSON_H

#include <hjson/hjson.h>

namespace aalta
{
    Hjson::Value *make_hjson(Transition *t);
    void print_hjson(Hjson::Value *hjson_ptr);
    void print_hjson(Hjson::Value hjson_);
}

#endif