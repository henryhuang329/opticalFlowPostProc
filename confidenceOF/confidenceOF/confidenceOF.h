#ifndef _CON_OF_H_
#define _CON_OF_H_

#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "opencv2\\imgproc\\imgproc.hpp"
#include "opencv2\\highgui\\highgui.hpp"
#include "opencv2\\opencv.hpp"
#include "png.h"
#include "pngconf.h"
#include "pngdebug.h"
#include "pngstruct.h"
#include "pnginfo.h"
#include "pnglibconf.h"
#include "pngpriv.h"
#include "pngstruct.h"
#include "png.hpp"
#include "io_flow.h"

using std::string;
using namespace cv;
using namespace png;
using namespace std;

Mat  getSobelEdge(Mat src);
Mat  getPredFrame(const char* mvFileName, Mat preFrame);
void motionToColor(Mat flow, Mat &color);
void makecolorwheel(vector<Scalar> &colorwheel);
void gtFlowImgGen(char* noc, char* occ);
void getMVField(const char* mvFileName, Mat preFrame, float* mvx, float* mvy);
void gradBasedMVScaling(float* mvx, float* mvy, int inMVWidth, int inMVHeight, Mat curFrame, float* mvxFullPic, float* mvyFullPic);
void interpMV(Mat orgFrame, float* mvx, float* mvy);
void scalarMedianFilterMV(float* mvx, float* mvy, int width, int height, bool winSizeConf=false, int winsize=3);
template <typename T>
void insertSort(T* x, int length);

void genGMFeature(Mat frame);
void genEdgeDisFeature(Mat curFrame);

bool eval (string prefix, string result_dir, string gt_noc_dir, string gt_occ_dir);

void genTempGradFeature(Mat mvFrame, Mat curFrame);
void genPhotoConstancyFeature(Mat predFrame, Mat curFrame);
void genClassifyLabel(Mat mvFrame, string nocGTFlowName, float EPETH);
void genAllFeatureVector(int width, int height,string curName, bool isTraingModel);
void genTCFeature(string curName, string inDir, Mat curFrame);
void integAllFVs();

int  predictConfidence(const char* testData, string outDir, string curName);
void getRatioC2DivC1(string fileNameC1, string fileNameC2, float* ratioC2DivC1);

void  getMVNewVersion(string fileName, Mat frame, float* mvx, float* mvy);
float get4x4BlkMean(Mat curFrame, int locX, int locY);

int  getDirDistance(Mat frame, const int* step, int pelX, int pelY);
void discardSmallVldRegion(Mat frame, Mat outFrame);
void discardSRbyMBRatio(Mat frame, Mat outFrame);

void bwLabelFunc(Mat bwFrame);
void bwLabelContour(const Mat& imgBw, Mat & imgLabeled);
void fbtcCombineConf(string curName, string inDir, string outDir, Mat preFrame, Mat curFrame, float th);
void calOptConfMap(Mat fullMVMat, string nocGTFlowName, string outDir, string curName, float ratio);
void tcBasedConfMeasure(string curName, string inDir, string outDir, Mat curFrame, Mat preFrame, float th);

#define UNKNOWN_FLOW_THRESH 1e9

// Global Parameters.
extern float g_pyrScale;
extern int   g_pyramidNum;

typedef struct _a{
  float mvx;
  float mvy;
  unsigned c1;
  unsigned c2;
  unsigned cost1;
  unsigned cost2;
} MVS;
#endif
