/* 
 * File:   main.h
 * Author: Matthieu Keller <keller.mdpa@gmail.com>
 *
 *  copyright : Creative Commons :  CC BY-NC-SA 3.0
 *                  Attribution-NonCommercial-ShareAlike 3.0 Unported
 * http://creativecommons.org/licenses/by-nc-sa/3.0/
 *
 * Created on April 3, 2011, 6:29 PM
 */

#ifndef MAIN_HPP
#define	MAIN_HPP

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <math.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#define PI 3.14159265

using namespace std;

/**
 * convert hsv2rgb
 * @param hue the inpout value
 * @return the output value
 */
CvScalar hsv2rgb(float hue);

/**
 *
 * @param name the input picture (it has to be in the same 
 *                                  folder than the main)
 * @return the histogramme of the input picture
 */
CvHistogram* calcHist(char* name);

/**
 * Track an object
 * @param argv the picture to follow
 */
void tracking(char** argv);

#endif	/* MAIN_HPP */

