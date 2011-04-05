/* 
 * File:   main.h
 * Author: Matthieu Keller <keller.mdpa@gmail.com>
 *
 *  copyright : Creative Commons : Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 * http://creativecommons.org/licenses/by-nc-sa/3.0/
 *
 * Created on April 3, 2011, 6:29 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#include <cstdlib>
#include <iostream>
#include <stdio.h>

#ifdef __unix__
#include <opencv/cv.h>
#include <opencv/highgui.h>
#else
#include <cv.h>
#include <highgui.h>
#endif

using namespace std;

/**
 * convert hsv2rgb
 * @param hue the inpout value
 * @return the output value
 */
CvScalar hsv2rgb(float hue);

/**
 *
 * @param name the input picture (it has to be in the same folder than the main)
 * @return the histogramme of the input picture
 */
CvHistogram* calcHist(char* name);

/**
 * Track an object
 * @param argv the picture to follow
 */
void tracking(char** argv);

#endif	/* MAIN_H */

