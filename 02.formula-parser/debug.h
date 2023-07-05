/**
 * File:   aaltasolver.h
 * Author: Yongkang Li
 *
 * Created on July 06, 2023, 00:23 AM
 */
 
#ifndef DEBUG_H
#define DEBUG_H
 

#ifdef DEBUG
#define dout std::cout
#else
#define dout 0 && std::cout
#endif


#endif