/**
 * File:   myhjson.cpp
 * Author: Yongkang Li
 *
 * Created on July 05, 2023, 15:03 PM
 */

#include "transition.h"
#include "myhjson.h"
#include "debug.h"
#include <iostream>

namespace aalta {
    Hjson::Value *make_hjson(Transition *t)
    {
        Hjson::Value *tMap = new Hjson::Value();
        (*tMap)["label"] = t->label()->to_set_string();
        (*tMap)["next"] = t->next()->to_set_string();
        return tMap;
    }
    void print_hjson(Hjson::Value *hjson_ptr)
    {
        // print_hjson(*hjson_ptr);
    }
    void print_hjson(Hjson::Value hjson_)
    {
        // dout << Hjson::Marshal(hjson_, {quoteAlways: true, quoteKeys: true, separator: true}) << std::endl;
    }
};