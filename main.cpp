/* 
 * File:   main.cpp
 * Author: Matthieu Keller <keller.mdpa@gmail.com>
 *
 *  copyright : Creative Commons :  CC BY-NC-SA 3.0 
 *          Attribution-NonCommercial-ShareAlike 3.0 Unported 
 * http://creativecommons.org/licenses/by-nc-sa/3.0/
 *
 * Created on March 31, 2011, 4:14 PM
 *
 * This programme track a colored object by passing his picture in argument
 */

// to build an opencv file use (compiler options) :
// -lcv -lhighgui

#include "main.hpp"

// Webcam input
IplImage* pictIn;

IplImage *image = 0, *hsv = 0, *hue = 0, *mask = 0, 
        *backproject = 0, *histimg = 0;
CvHistogram *hist = 0;

int backproject_mode = 0;
int track_object = -1;
int show_hist = 1;
CvRect selection;
CvRect track_window;
CvBox2D track_box;
CvConnectedComp track_comp;
int hdims = 16;
float hranges_arr[] = {0, 180};
float* hranges = hranges_arr;
int vmin = 10, vmax = 256, smin = 30;
// in order to record
CvVideoWriter* videoWriter; 

/**
 * convert hsv2rgb
 * @param hue the inpout value
 * @return the output value
 */
CvScalar hsv2rgb(float hue) {

    int rgb[3], p, sector;
    static const int sector_data[][3] = {
        {0, 2, 1},
        {1, 2, 0},
        {1, 0, 2},
        {2, 0, 1},
        {2, 1, 0},
        {0, 1, 2}
    };

    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255 * (hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return cvScalar(rgb[2], rgb[1], rgb[0], 0);
}



void drawArrow(IplImage *image, CvPoint p, CvPoint q, 
               CvScalar color, int arrowMagnitude = 9,
               int thickness=1, int line_type=8, int shift=0) {
    //draw the principle line 
    cvLine(image, p, q, color, thickness, line_type, shift);
    // calcul the alpha angle
    double angle = atan2((double)p.y-q.y, (double)p.x-q.x);
    p.x = (int) ( q.x +  arrowMagnitude * cos(angle + PI/4));
    p.y = (int) ( q.y +  arrowMagnitude * sin(angle + PI/4));
    cvLine(image, p, q, color, thickness, line_type, shift);
    p.x = (int) ( q.x +  arrowMagnitude * cos(angle - PI/4));
    p.y = (int) ( q.y +  arrowMagnitude * sin(angle - PI/4));
    cvLine(image, p, q, color, thickness, line_type, shift);
}

/**
 *
 * @param name the input picture (it has to be in the same 
 *                                      folder than the main)
 * @return the histogramme of the input picture
 */
CvHistogram* calcHist(char* name) {
    IplImage *src = 0;
    IplImage *image = 0, *hsv = 0, *hue = 0, *mask = 0; //, *histimg = 0;
    int hdims = 16;
    float hranges_arr[] = {0, 180};
    float* hranges = hranges_arr;
    int vmin = 10, vmax = 256, smin = 30;
    CvHistogram* hist=0;

    if ((src = cvLoadImage(name, 1)) == 0){
		fprintf(stderr, "error could not load the image...\n");
		return hist;
	}

    int i, bin_w;

    hist = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
    cvZero(histimg);

    image = cvCreateImage(cvGetSize(src), 8, 3);
    image->origin = src->origin;
    hsv = cvCreateImage(cvGetSize(src), 8, 3);
    hue = cvCreateImage(cvGetSize(src), 8, 1);
    mask = cvCreateImage(cvGetSize(src), 8, 1);
    cvCopy(src, image, 0);
    cvCvtColor(image, hsv, CV_BGR2HSV);

    cvInRangeS(hsv, cvScalar(0, smin, MIN(vmin, vmax), 0),
            cvScalar(180, 256, MAX(vmin, vmax), 0), mask);
    cvSplit(hsv, hue, 0, 0, 0);


    cvCalcHist(&hue, hist, 0, mask);

    // we create histimg to show the histogramme
    bin_w = histimg->width / hdims;
    for (i = 0; i < hdims; i++) {
        int val = cvRound(cvGetReal1D(hist->bins, i) 
                          * histimg->height / 255);
        CvScalar color = hsv2rgb(i * 180.f / hdims);
        cvRectangle(histimg, cvPoint(i*bin_w, histimg->height),
                cvPoint((i + 1) * bin_w, histimg->height - val),
                color, -1, 8, 0);
    }
    return hist;
}

/**
 * Track an object
 * @param argv the picture to follow
 */
void tracking(char** argv) {
    string order;

    if (!image) {
        /* allocate all the buffers */
        image = cvCreateImage(cvGetSize(pictIn), 8, 3);
        image->origin = pictIn->origin;
        hsv = cvCreateImage(cvGetSize(pictIn), 8, 3);
        hue = cvCreateImage(cvGetSize(pictIn), 8, 1);
        mask = cvCreateImage(cvGetSize(pictIn), 8, 1);
        backproject = cvCreateImage(cvGetSize(pictIn), 8, 1);
        hist = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
        histimg = cvCreateImage(cvSize(320, 200), 8, 3);
        cvZero(histimg);
        hist = calcHist(argv[1]);
    }

    cvCopy(pictIn, image, 0);
    cvCvtColor(image, hsv, CV_BGR2HSV);

    if (track_object) {
        cvInRangeS(hsv, cvScalar(0, smin, MIN(vmin, vmax), 0),
                cvScalar(180, 256, MAX(vmin, vmax), 0), mask);
        cvSplit(hsv, hue, 0, 0, 0);

        if (track_object < 0) {
            float max_val = 0.f;

            // selection of the ROI,
            // at the begining it s all the picture
            selection.x = 0;
            selection.y = 0;
            selection.width = pictIn->width;
            selection.height = pictIn->height;

            // application of the RIO
            cvSetImageROI(hue, selection);
            cvSetImageROI(mask, selection);

            cvGetMinMaxHistValue(hist, 0, &max_val, 0, 0);
            cvConvertScale(hist->bins, hist->bins, max_val
                           ? 255. / max_val : 0., 0);
            cvResetImageROI(hue);
            cvResetImageROI(mask);
            track_window = selection;
            track_object = 1;
        }

        // calculates the back projection of a histogram
        cvCalcBackProject(&hue, backproject, hist);
        cvAnd(backproject, mask, backproject, 0);

        cvCamShift(backproject, track_window,
                cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
                &track_comp, &track_box);
        track_window = track_comp.rect;

        // we track the center of the object
        // and order what to do to put it in the center of the camera
        // TODO replace that by real commande
        if (track_box.center.x > pictIn->width / 2) {
            order += " Va a droite";
        } else if (track_box.center.x < pictIn->width / 2) {
            order += " Va a gauche";
        }

        if (track_box.center.y > pictIn->height / 2) {
            order += " Descend";
        } else if (track_box.center.y < pictIn->height / 2) {
            order += " Monte";
        }

        if (track_box.size.height < 5 && track_box.size.width < 5) {
            track_object = -1;
        }

        cout << order << endl;
        order = "";

        if (backproject_mode)
            cvCvtColor(backproject, image, CV_GRAY2BGR);
        if (!image->origin)
            track_box.angle = -track_box.angle;

        if (track_box.size.width > 0 && track_box.size.height > 0){
        cvEllipseBox(image, track_box, CV_RGB(255, 0, 0));
        drawArrow(image, cvPoint(image->width/2, image->height),
                  cvPoint(int(track_box.center.x), int(track_box.center.y)),
                  CV_RGB(100, 0, 255));
        }
    }
}

/*
 *
 */
int main(int argc, char** argv) {

    if (argc != 2) {
		fprintf(stderr, "error plese type an argv...\n");
        return EXIT_FAILURE;
    }

    // real tracking
    // webcam index
//    int index = -1;
    // we read the webcam
//    CvCapture* capture = cvCreateCameraCapture(index);


    // video tracking for test
    CvCapture* capture = cvCreateFileCapture("SAUCE_Test_Vid.wmv");

    // we check if it s reading
    if (!capture) {
        fprintf(stderr, "Could not initialize capturing...\n");
        return -1;
    }

    printf("Hot keys: \n"
            "\tESC - quit the program\n"
            "\tc - stop the tracking\n"
            "\tb - switch to/from backprojection view\n");

    // creation of the windows
    cvNamedWindow("Histogram", 1);
    cvNamedWindow("input", 1);
    cvNamedWindow("backproject", 1);


    // we cath a frame
    if (!cvGrabFrame(capture)) {
        fprintf(stderr, "Could not capturing the frame ...\n");
        return EXIT_FAILURE;
    }

    // in order to record
    pictIn = cvRetrieveFrame(capture);
//    videoWriter = cvCreateVideoWriter("plop.avi", 
    //      CV_FOURCC('M','J','P','G'), 25, 
    //      cvSize(pictIn->width, pictIn->height), 1);

    // variable for key
    int c;
    // do loop
    while (1) {

        // we retrive theframe
        pictIn = cvRetrieveFrame(capture);

        //tracking
        tracking(argv);

        // we show pictIn
        
        cvShowImage("input", image);
        cvShowImage("Histogram", histimg);
        cvShowImage("backproject", backproject);
        
        // in order to record 
        cvWriteFrame(videoWriter, image);

        c = cvWaitKey(10);
        int d;
        // in order to make a pause
        if (c == 32){
            d = 0;
            while (d != 32){
                d = cvWaitKey(10);
                if (d == 27){
                    c = 27;
                    break;
                }
            }
        } 
        if (c == 27)
            break;

        switch (c) {
            case 'b':
                backproject_mode ^= 1;
                break;
            case 'c':
                track_object = 0;
                cvZero(histimg);
                break;
            default:
                break;
        }


        // we catch the next frame
        if (!cvGrabFrame(capture))
            break;
    }

    // we destroy the windows
    cvDestroyAllWindows();

    // the pictures
    pictIn = NULL;
	image = NULL;

    // we release
    cvReleaseCapture(&capture);

    return EXIT_SUCCESS;
}
