#include "confidenceOF.h"


// Global Parameters Definition.
float g_pyrScale   = 0.8f;
int   g_pyramidNum = 4;

int main(int argc, char **argv)
{
  // gtFlowImgGen(argv[4],argv[5]);
#if 0
  printf("CMD Line Includes: exeFile[0], preFrame[1], curFrame[2], mvFile[3], GTNocMV[4], GTOccMV[5]\n");
#endif

#if 0
  // Integrate All Feature Vectors.
  integAllFVs();
  return 0;
#endif

  Mat preFrame, curFrame, grad, gradRes;
  Mat predFrame;
  Mat colorFrame;

  // FileName parser
  string inDir   = argv[1];
  string outDir  = argv[2];
  string curName = argv[3];
  string preName = curName;
  preName.replace(preName.length()-1, preName.length(), "1");

  string preFileName = inDir+"\\image\\"+preName+".png";
  string curFileName = inDir+"\\image\\"+curName+".png";

  preFrame = imread(preFileName, CV_LOAD_IMAGE_GRAYSCALE);
  curFrame = imread(curFileName, CV_LOAD_IMAGE_GRAYSCALE);

  if( !curFrame.data || !curFrame.data )
  { 
    return -1; 
  }

  // Original Picture Size.
  int inWidthOrg  = preFrame.cols;
  int inHeightOrg = preFrame.rows;

  // For Video Coding Picture Size. This is the MB=16x16 Level.
  int inWidth  = (inWidthOrg  + 15) >> 4;
  int inHeight = (inHeightOrg + 15) >> 4;

  string partName = curName;
  partName.erase(partName.length()-2,2);
  char widthS[20], heightS[20];
  itoa(inWidth*16,  widthS,  10);
  itoa(inHeight*16, heightS, 10);
  string sparseMVFileName = inDir+"\\NVENCOutData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv1.bin";
  
  string nocGTFlowName = inDir+"\\flow_noc\\"+curName+".png";

  // Double Check Input PNG and YUV FileName!
#if 0
  string kittiYuvFileName = "C:\\sequence\\000070_1248x384.yuv"; //Get The KittiFileName. Should Be Change!
  const char*  yuvFileName = kittiYuvFileName.data();
  // memcpy(yuvFileName, kittiYuvFileName, kittiYuvFileName.length());
  FILE* fpYUV = fopen(yuvFileName, "rb");
  // 10 Frame.
  Mat curFrame1(inHeight*16, inWidth*16, CV_LOAD_IMAGE_GRAYSCALE);
  fseek   (fpYUV, inHeight*16*inWidth*16*1.5*10L, SEEK_SET);
  uchar sampleVal;
  for ( int i=0; i<inHeight*16; i++)
  {
	  for ( int j=0; j<inWidth*16; j++)
	  {
		  fread (&sampleVal, 1, 1, fpYUV);
		  curFrame1.at<uchar>(i,j) = sampleVal;
	  }
  }

  
  for (int i=0; i<inHeightOrg; i++)
  {
    for (int j=0; j<inWidthOrg; j++)
    {
      if (curFrame1.at<uchar>(i,j) != curFrame.at<uchar>(i,j))
      {
        fprintf(stdout, "Input CUR PNG is not same as the YUV File!\n");
        return 0;
      }
    }
  }

  // 11 Frame.
  Mat preFrame1(inHeight*16, inWidth*16, CV_LOAD_IMAGE_GRAYSCALE);
  fseek   (fpYUV, inHeight*16*inWidth*16*1.5*11L, SEEK_SET);
  for ( int i=0; i<inHeight*16; i++)
  {
	  for ( int j=0; j<inWidth*16; j++)
	  {
		  fread (&sampleVal, 1, 1, fpYUV);
		  preFrame1.at<uchar>(i,j) = sampleVal;
	  }
  }

  for (int i=0; i<inHeightOrg; i++)
  {
    for (int j=0; j<inWidthOrg; j++)
    {
      if ( preFrame1.at<uchar>(i,j) != preFrame.at<uchar>(i,j) )
      {
        fprintf(stdout, "Input REF(Previous) PNG is not same as the YUV File!\n");
        return 0;
      }
    }
  }

  fclose (fpYUV);
#endif

  // float th = atof(argv[4]);
  // fbtcCombineConf(curName, inDir, outDir, preFrame, curFrame, th);
  // return 0;

  //float th = atof(argv[4]);
  //tcBasedConfMeasure(curName, inDir, outDir, curFrame, preFrame, th);
  //return 0;

#if 0
  // grad Cal.
  grad = getSobelEdge(curFrame);
  
  // get Prediction Frame.
  predFrame = getPredFrame(sparseMVFileName.data(),preFrame);

  // Output Grad And Prediction Img.
  string  outGradImg = outDir+"\\GradImg\\"+curName+".png";
  string  outPredImg = outDir+"\\predImg\\"+curName+".png";
  imwrite(outGradImg, grad);
  imwrite(outPredImg, predFrame);
#endif
  
  // get Motion Vector Field. Add Median Filter First?
  float *mvx=NULL, *mvy=NULL;
  mvx = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvy = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  getMVField(sparseMVFileName.data(), preFrame, mvx, mvy); 

  Mat quadMVFile1(inHeight*4,  inWidth*4,  CV_32FC3);
  Mat fullMVFile1(inHeight*16, inWidth*16, CV_32FC3);

  FlowImage mvSmall(inWidth*4, inHeight*4);
  // mvSmall.setFlowU()
  for ( int32_t i=0; i<mvSmall.height(); i++ )
  {
    for ( int32_t j=0; j<mvSmall.width(); j++ )
    {
      quadMVFile1.at<Vec3f>(i,j)[0] = mvx[j+i*inWidth*4];
      quadMVFile1.at<Vec3f>(i,j)[1] = mvy[j+i*inWidth*4];
      quadMVFile1.at<Vec3f>(i,j)[2] = 1;

      mvSmall.setFlowU(j, i, mvx[j+i*inWidth*4]);
      mvSmall.setFlowV(j, i, mvy[j+i*inWidth*4]);
      mvSmall.setValid(j, i, 1);

      float data0 = quadMVFile1.at<Vec3f>(i,j)[0];
      float data1 = quadMVFile1.at<Vec3f>(i,j)[1];
      float data2 = quadMVFile1.at<Vec3f>(i,j)[2];
    }
  }
  
  Mat quadMVFile1Med;
  medianBlur(quadMVFile1, quadMVFile1Med, 5);
  resize(quadMVFile1Med, fullMVFile1, Size(quadMVFile1.cols*4, quadMVFile1.rows*4));

  // Get the Ground truth based ROC.
#if 0
  float ratio = atof(argv[4]);
  calOptConfMap(fullMVFile1, nocGTFlowName, outDir, curName, ratio);
  return 0;
#endif

#if 0 // Small MV Flow Generation.
  float  max_flow = mvSmall.maxFlow();
  string outSmallMVOF = outDir+"\\SmallMVOF\\"+curName+".png";
  mvSmall.writeColor(outSmallMVOF, max_flow);

  FlowImage mvFull(fullMVFile1.cols, fullMVFile1.rows);
  // mvSmall.setFlowU()
  for ( int32_t i=0; i<mvFull.height(); i++ )
  {
    for ( int32_t j=0; j<mvFull.width(); j++ )
    {
      float mvx = fullMVFile1.at<Vec3f>(i,j)[0];
      float mvy = fullMVFile1.at<Vec3f>(i,j)[1];

      mvFull.setFlowU(j, i, fullMVFile1.at<Vec3f>(i,j)[0]);
      mvFull.setFlowV(j, i, fullMVFile1.at<Vec3f>(i,j)[1]);
      mvFull.setValid(j, i, 1);
    }
  }
  // float  max_flowFull = mvFull.maxFlow();
  // mvFull.writeColor("out.png", max_flowFull);
#endif

  // Small MV Confidence Measure.
  // Current Using Random Forest Method To train the Model.

  // Generate Gradient Magnitude Feature.
  float EPETH = atof(argv[4]);

  string featureName    = "FV_"+curName+".data";
  const char*    ftName = featureName.data();

  genGMFeature      (curFrame);
  genEdgeDisFeature (curFrame);
  genTempGradFeature(quadMVFile1, curFrame);
  genPhotoConstancyFeature(predFrame, curFrame);
  genTCFeature(curName, inDir, curFrame);
  genClassifyLabel(fullMVFile1, nocGTFlowName, EPETH);

  bool isTrainingModel = false;
  genAllFeatureVector(curFrame.cols, curFrame.rows, curName, isTrainingModel);

  return 0;

  predictConfidence(ftName, outDir, curName);

  string fileConfOut = outDir+"\\confData\\"+curName+".data";
  const  char* confOutName = fileConfOut.data();
  FILE*  fpConfidence=fopen(confOutName,"r");
  float* smallFrameConf=NULL;
  smallFrameConf = (float*)malloc(curFrame.rows*curFrame.cols*sizeof(float));
  for (int i=0; i<curFrame.rows; i++)
  {
    for (int j=0; j<curFrame.cols; j++)
    {
      fscanf(fpConfidence, "%f\n", &smallFrameConf[j+i*curFrame.cols]);
    }
  }
  fclose(fpConfidence);

  remove(ftName);

  return 0;

  // Scalar Median Filter / (Proposing Other Filters) To remove the Outliers
  // Can Refer wiki: https://en.wikipedia.org/wiki/Median_filter
  scalarMedianFilterMV(mvx, mvy, inWidth*4, inHeight*4, true, 5);

  // MV UpSampling Using Original Image Guided Interpolation.
  float *mvxFullPic, *mvyFullPic;
  mvxFullPic = (float *)malloc(inWidth*16*inHeight*16*sizeof(float));
  mvyFullPic = (float *)malloc(inWidth*16*inHeight*16*sizeof(float));
  
  // Gradient Guided MV interpolation.
  gradBasedMVScaling(mvx, mvy, inWidth*4, inHeight*4, curFrame, mvxFullPic, mvyFullPic);
 
  // MV Post-Processing. Median Filter.
  scalarMedianFilterMV(mvxFullPic, mvyFullPic, inWidth*16, inHeight*16);

#if 1
  // FullPic MV Flow Generation.
  FlowImage mvFullPic(inWidth*16, inHeight*16);
  // mvSmall.setFlowU()
  for ( int32_t i=0; i<mvFullPic.height(); i++ )
  {
    for ( int32_t j=0; j<mvFullPic.width(); j++ )
    {
      mvFullPic.setFlowU(j, i, mvxFullPic[j+i*inWidth*16]);
      mvFullPic.setFlowV(j, i, mvyFullPic[j+i*inWidth*16]);
      mvFullPic.setValid(j, i, 1);
    }
  }
  float max_flowFullPic = mvFullPic.maxFlow();
  string fullPicMVFileName = outDir+"\\fullPicMVOF\\"+curName+".png";
  mvFullPic.writeColor(fullPicMVFileName, max_flowFullPic);
#endif


  // MV Confidence Measure.


  // Output The Final MV.
  png::image< png::rgb_pixel_16 > mvFinalImg(inWidthOrg, inHeightOrg);
  png::rgb_pixel_16 val;
  for ( int32_t i=0; i<inHeightOrg; i++ )
  {
    for ( int32_t j=0; j<inWidthOrg; j++ )
    {
      float mvx = mvxFullPic[j+i*inWidth*16];
      float mvy = mvyFullPic[j+i*inWidth*16];
      val.red   = int(mvx*64)+32768;
      val.green = int(mvy*64)+32768;
      val.blue  = 1;

      // Confidence Smaller Than a Threshold. Then set Valid To false.
      if (smallFrameConf[j+i*inWidthOrg]<=0.05) 
      {
        val.red   = 0;
        val.green = 0;
        val.blue  = 0;
      }
      mvFinalImg.set_pixel(j, i, val);
    }
  }
  string mvFinalImgName = outDir+"\\data\\"+curName+".png";
  mvFinalImg.write(mvFinalImgName);

  // Evaluate the Data.
  eval (curName, outDir, inDir+"\\flow_noc", inDir+"\\flow_occ");

  // Free
  free(mvx);
  free(mvy);
  free(mvxFullPic);
  free(mvyFullPic);
  free(smallFrameConf);

  return 0;
}

// Simple Insert Sort Fun.
template <typename T>
void insertSort( T* x, int length )
{
  for (int i = 1; i < length; i++)
  {
    if (x[i-1] > x[i])
    {
      T   temp = x[i];
      int j = i;
      while (j > 0 && x[j-1] > temp)
      {
        x[j] = x[j-1];
        j--;
      }
      x[j] = temp;
    }
  }
}

// boundary is not filtered
void scalarMedianFilterMV(float* mvx, float* mvy, int width, int height, bool winSizeConf, int winsize)
{
  int WINSIZE = 3;
  float *winMVx=NULL;
  float *winMVy=NULL;
  float  tmpx[9], tmpy[9];
  
  winMVx = (float *)malloc(WINSIZE*WINSIZE*sizeof(float));
  winMVy = (float *)malloc(WINSIZE*WINSIZE*sizeof(float));

  for (int i=0+(WINSIZE/2); i<height-(WINSIZE/2); i++)
  {
    for (int j=0+(WINSIZE/2); j<width-(WINSIZE/2); j++)
    {
      // Get The Pixel Window.
      int winCnt = 0;
      for (int m=-(WINSIZE/2); m<=(WINSIZE/2); m++)
      {
        for (int n=-(WINSIZE/2); n<=(WINSIZE/2); n++)
        {
          winMVx[winCnt] = mvx[j+n+(i+m)*width];
          winMVy[winCnt] = mvy[j+n+(i+m)*width];
          winCnt++;
        }
      }
      
      // Sort MVx and MVy.
      insertSort(winMVx,winCnt);
      insertSort(winMVy,winCnt);
      
      mvx[j+i*width] = winMVx[(winCnt)/2];
      mvy[j+i*width] = winMVy[(winCnt)/2];
    }
  }
  free(winMVx);
  free(winMVy);
}

void gradBasedMVScaling(float* mvx, float* mvy, int inMVWidth, int inMVHeight, Mat curFrame, float* mvxFullPic, float* mvyFullPic)
{
  int width  = 4*inMVWidth;
  int height = 4*inMVHeight;
  Mat paddingFrame(height, width, CV_8UC1);
  Mat halfFrame(height/2, width/2, CV_8UC1);

  float* mvxHalf, *mvyHalf;
  mvxHalf = (float *)malloc(inMVWidth*2*inMVHeight*2*sizeof(float));
  mvyHalf = (float *)malloc(inMVWidth*2*inMVHeight*2*sizeof(float));

  for (int i=0; i<inMVHeight*2; i+=2)
  {
    for (int j=0; j<inMVWidth*2; j+=2 )
    {
      mvxHalf[j+i*inMVWidth*2] = mvx[(j/2)+(i/2)*inMVWidth];
      mvyHalf[j+i*inMVWidth*2] = mvy[(j/2)+(i/2)*inMVWidth];
    }
  }

  // Get Padding Frame.
  for (int i=0; i<height; i++)
  {
    for (int j=0; j<width; j++)
    {
      bool inCurFrame = i<curFrame.rows && j<curFrame.cols;
      if (inCurFrame)                                paddingFrame.at<uchar>(i,j) = curFrame.at<uchar>(i, j);
      else if (i>=curFrame.rows && j>=curFrame.cols) paddingFrame.at<uchar>(i,j) = curFrame.at<uchar>(curFrame.rows-1, curFrame.cols-1);
      else if (i>=curFrame.rows)                     paddingFrame.at<uchar>(i,j) = curFrame.at<uchar>(curFrame.rows-1, j);
      else if (j>=curFrame.cols)                     paddingFrame.at<uchar>(i,j) = curFrame.at<uchar>(i,  curFrame.cols-1);
    }
  }
  
  // Scaling Padding Picture.
  resize(paddingFrame, halfFrame, Size(), 0.5, 0.5, INTER_CUBIC);

#if 0
  // Output Image.
  imwrite("./out/paddingFrame.png",paddingFrame);
  imwrite("./out/halfFrame.png",   halfFrame);
#endif

  // Interpolate Half MV Picture.
  interpMV(halfFrame, mvxHalf, mvyHalf);

  for (int i=0; i<height; i+=2)
  {
    for (int j=0; j<width; j+=2 )
    {
      mvxFullPic[j+i*width] = mvxHalf[(j/2)+(i/2)*inMVWidth*2];
      mvyFullPic[j+i*width] = mvyHalf[(j/2)+(i/2)*inMVWidth*2];
    }
  }

  // Interpolate Full MV Picture.
  interpMV(paddingFrame, mvxFullPic, mvyFullPic);

  free(mvxHalf);
  free(mvyHalf);
}

void interpMV(Mat orgFrame, float* mvx, float* mvy)
{
  int width = orgFrame.cols;
  int height= orgFrame.rows;
  uchar* graySamples=NULL;
  int SIGMA = 25;
  graySamples = (uchar *)malloc(width*height*sizeof(uchar));
  for (int i=0; i<orgFrame.rows; i++)
  {
    for (int j=0; j<orgFrame.cols; j++)
    {
      graySamples[j+i*width] = orgFrame.at<uchar>(i,j);
    }
  }

  // Image Interpolation illustration.
  // MVA     MV1(I)   MVB    
  // MV2(I)  MV3(I)   MV4(I)
  // MVC     MV1'(I)  MVD
  // End illustration.

  // Interpolate MV3 Firstly.
  float neiAx, neiAy;
  float neiBx, neiBy;
  float neiCx, neiCy;
  float neiDx, neiDy;
  bool  inPicA, inPicB, inPicC, inPicD;
  int   diffA=0,    diffB=0,    diffC=0,    diffD=0;
  float ratioA=0.0,  ratioB=0.0,  ratioC=0.0,  ratioD=0.0;
  float weightA=0.0, weightB=0.0, weightC=0.0, weightD=0.0;
  float sumWeight=0.0;
  float mvxA=0.0, mvxB=0.0, mvxC=0.0, mvxD=0.0;
  float mvyA=0.0, mvyB=0.0, mvyC=0.0, mvyD=0.0;
  for ( int i=1; i<height; i+=2 )
  {
    for ( int j=1; j<width; j+=2 )
    {
      inPicA = (i-1)>=0 && (i-1)<height && (j-1)>=0 && (j-1)<width;
      inPicB = (i-1)>=0 && (i-1)<height && (j+1)>=0 && (j+1)<width;
      inPicC = (i+1)>=0 && (i+1)<height && (j-1)>=0 && (j-1)<width;
      inPicD = (i+1)>=0 && (i+1)<height && (j+1)>=0 && (j+1)<width;

      if (inPicA) diffA = abs(graySamples[j+i*width]-graySamples[(j-1)+(i-1)*width]);
      if (inPicB) diffB = abs(graySamples[j+i*width]-graySamples[(j+1)+(i-1)*width]);
      if (inPicC) diffC = abs(graySamples[j+i*width]-graySamples[(j-1)+(i+1)*width]);
      if (inPicD) diffD = abs(graySamples[j+i*width]-graySamples[(j+1)+(i+1)*width]);
      
      ratioA = std::exp(-((float)diffA*diffA/(2*(SIGMA*SIGMA))));
      ratioB = std::exp(-((float)diffB*diffB/(2*(SIGMA*SIGMA))));
      ratioC = std::exp(-((float)diffC*diffC/(2*(SIGMA*SIGMA))));
      ratioD = std::exp(-((float)diffD*diffD/(2*(SIGMA*SIGMA))));

      sumWeight = inPicA*ratioA+inPicB*ratioB+inPicC*ratioC+inPicD*ratioD;
      assert(sumWeight!=0.0f);

      weightA = ratioA*inPicA/sumWeight;
      weightB = ratioB*inPicB/sumWeight;
      weightC = ratioC*inPicC/sumWeight;
      weightD = ratioD*inPicD/sumWeight;

      if (inPicA) mvxA = mvx[(j-1)+(i-1)*width];
      if (inPicB) mvxB = mvx[(j+1)+(i-1)*width];
      if (inPicC) mvxC = mvx[(j-1)+(i+1)*width];
      if (inPicD) mvxD = mvx[(j+1)+(i+1)*width];

      if (inPicA) mvyA = mvy[(j-1)+(i-1)*width];
      if (inPicB) mvyB = mvy[(j+1)+(i-1)*width];
      if (inPicC) mvyC = mvy[(j-1)+(i+1)*width];
      if (inPicD) mvyD = mvy[(j+1)+(i+1)*width];

      mvx[j+i*width] = mvxA*weightA+mvxB*weightB+mvxC*weightC+mvxD*weightD;
      mvy[j+i*width] = mvyA*weightA+mvyB*weightB+mvyC*weightC+mvyD*weightD;
    }
  }
  
  // Interpolate The MV1 and MV2 Secondly.
  //         MVA    
  // MVB     MV2/1(I)  MVC
  //         MVD
  for (int i=0; i<height; i++)
  {
    for (int j=((i%2)?0:1); j<width; j+=2)
    {
      inPicA = (i-1)>=0 && (i-1)<height && (j-0)>=0 && (j-0)<width;
      inPicB = (i-0)>=0 && (i-0)<height && (j-1)>=0 && (j-1)<width;
      inPicC = (i+0)>=0 && (i+0)<height && (j+1)>=0 && (j+1)<width;
      inPicD = (i+1)>=0 && (i+1)<height && (j+0)>=0 && (j+0)<width;

      if (inPicA) diffA = abs(graySamples[j+i*width]-graySamples[(j-0)+(i-1)*width]);
      if (inPicB) diffB = abs(graySamples[j+i*width]-graySamples[(j-1)+(i-0)*width]);
      if (inPicC) diffC = abs(graySamples[j+i*width]-graySamples[(j+1)+(i+0)*width]);
      if (inPicD) diffD = abs(graySamples[j+i*width]-graySamples[(j+0)+(i+1)*width]);

      ratioA = std::exp(-((float)diffA*diffA/(2*(SIGMA*SIGMA))));
      ratioB = std::exp(-((float)diffB*diffB/(2*(SIGMA*SIGMA))));
      ratioC = std::exp(-((float)diffC*diffC/(2*(SIGMA*SIGMA))));
      ratioD = std::exp(-((float)diffD*diffD/(2*(SIGMA*SIGMA))));

      sumWeight = inPicA*ratioA+inPicB*ratioB+inPicC*ratioC+inPicD*ratioD;
      assert(sumWeight!=0.0f);

      weightA = ratioA*inPicA/sumWeight;
      weightB = ratioB*inPicB/sumWeight;
      weightC = ratioC*inPicC/sumWeight;
      weightD = ratioD*inPicD/sumWeight;

      if (inPicA) mvxA = mvx[(j-0)+(i-1)*width];
      if (inPicB) mvxB = mvx[(j-1)+(i-0)*width];
      if (inPicC) mvxC = mvx[(j+1)+(i+0)*width];
      if (inPicD) mvxD = mvx[(j+0)+(i+1)*width];

      if (inPicA) mvyA = mvy[(j-0)+(i-1)*width];
      if (inPicB) mvyB = mvy[(j-1)+(i-0)*width];
      if (inPicC) mvyC = mvy[(j+1)+(i+0)*width];
      if (inPicD) mvyD = mvy[(j+0)+(i+1)*width];

      mvx[j+i*width] = mvxA*weightA+mvxB*weightB+mvxC*weightC+mvxD*weightD;
      mvy[j+i*width] = mvyA*weightA+mvyB*weightB+mvyC*weightC+mvyD*weightD;
    }
  }

  free(graySamples);
}

void gtFlowImgGen(char* noc, char* occ)
{
  // output noc/occ Flows.
  FlowImage F_gt_noc(noc);
  FlowImage F_gt_occ(occ);

  float max_flow = F_gt_occ.maxFlow();
  int   cntValidNoc=0, cntValidOcc=0;

  F_gt_noc.writeColor("./out/gtFlowNoc.png",max_flow);
  F_gt_occ.writeColor("./out/gtFlowOcc.png",max_flow);

  assert (F_gt_noc.height()==F_gt_occ.height());
  assert (F_gt_noc.width() ==F_gt_occ.width() );

  for ( int32_t i=0; i<F_gt_noc.height(); i++ )
  {
    for ( int32_t j=0; j<F_gt_noc.width(); j++ )
    {
      if (F_gt_noc.isValid(j,i)) cntValidNoc++;
      if (F_gt_occ.isValid(j,i)) cntValidOcc++;
    }
  }

  printf("Valid NOC Samples Percentage Is:%f\n", ((float)cntValidNoc)/(F_gt_noc.width()*F_gt_noc.height()));
  printf("Valid OCC Samples Percentage Is:%f\n", ((float)cntValidOcc)/(F_gt_noc.width()*F_gt_noc.height()));
}

Mat getSobelEdge(Mat src)
{
  Mat grad;

  int scale = 1;
  int delta = 0;
  int ddepth = CV_16S;
  
  Mat grad_x, grad_y;
  Mat abs_grad_x, abs_grad_y;

  // x Direction.
  Sobel( src, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
  convertScaleAbs( grad_x, abs_grad_x );

  // y Direction.
  Sobel( src, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
  convertScaleAbs( grad_y, abs_grad_y );

  // Combine Direction.
  addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );

  return grad;
}

void getMVField(const char* mvFileName, Mat preFrame, float* mvx, float* mvy)
{
  FILE* fp = fopen(mvFileName, "rb");

  int inWidthOrg  = preFrame.cols;
  int inHeightOrg = preFrame.rows;

  int inWidth  = (inWidthOrg  + 15) >> 4;
  int inHeight = (inHeightOrg + 15) >> 4;

  for (int mby = 0; mby < inHeight; mby++) 
  {
    for (int mbx=0; mbx < inWidth; mbx++) 
    {
      for(int i8 = 0; i8 < 4; i8++) 
      {
        for(int i4 = 0; i4 < 4; i4 ++) 
        {
          int tx = 4*mbx + (i8%2)*2 + (i4%2);
          int ty = 4*mby + (i8/2)*2 + (i4/2);

          fread(&mvx[tx+ty*inWidth*4], sizeof(float), 1, fp);
          fread(&mvy[tx+ty*inWidth*4], 1, sizeof(float), fp);
        }
      }
    }
  }
}

Mat getPredFrame(const char* mvFileName, Mat preFrame)
{
  FILE* fp = fopen(mvFileName, "rb");
  
  int inWidthOrg  = preFrame.cols;
  int inHeightOrg = preFrame.rows;

  Mat pred(inHeightOrg, inWidthOrg, CV_8UC1);

  float *mvx=NULL, *mvy=NULL;
   
  int inWidth  = (inWidthOrg  + 15) >> 4;
  int inHeight = (inHeightOrg + 15) >> 4;

  int cha = preFrame.channels();
  int type = preFrame.type();
  int type2 = pred.type();
  int depth = preFrame.depth();
  int depth1=pred.depth();

  mvx = (float *)malloc(inWidth*inHeight*16*sizeof(float));
  mvy = (float *)malloc(inWidth*inHeight*16*sizeof(float));

  for (int mby = 0; mby < inHeight; mby++) 
  {
    for (int mbx=0; mbx < inWidth; mbx++) 
    {
      for(int i8 = 0; i8 < 4; i8++) 
      {
        for(int i4 = 0; i4 < 4; i4 ++) 
        {
          int tx = 4*mbx + (i8%2)*2 + (i4%2);
          int ty = 4*mby + (i8/2)*2 + (i4/2);

          fread(&mvx[tx+ty*inWidth*4], sizeof(float), 1, fp);
          fread(&mvy[tx+ty*inWidth*4], 1, sizeof(float), fp);
        }
      }
    }
  }

  for (int mby=0; mby<4*inHeight; mby++)
  {
    for (int mbx=0; mbx<4*inWidth; mbx++)
    {
      int xTmp = floor(mvx[mbx+mby*4*inWidth]);
      int yTmp = floor(mvy[mbx+mby*4*inWidth]);

      int xPos = mbx*4;
      int yPos = mby*4;

      for (int i=0; i<4; i++)
      {
        for (int j=0; j<4; j++)
        {
          int startX = xPos+xTmp+j;
          int startY = yPos+yTmp+i;

          if (startX<0)               startX = 0;
          else if(startX>=inWidthOrg)  startX=inWidthOrg-1;

          if ( startY<0)              startY = 0;
          else if (startY>=inHeightOrg)startY = inHeightOrg-1;
          
          bool insidePic = (xPos+j)<inWidthOrg && (yPos+i)<inHeightOrg;

          if ( insidePic )
          {
            for(int n=0;n<preFrame.channels();n++)
            {
              pred.at<uchar>(yPos+i, (xPos+j)*preFrame.channels()+n) = preFrame.at<uchar>(startY, startX*preFrame.channels()+n);
            }
          }
        }
      }
    }
  }

  free(mvx);
  free(mvy);
  fclose(fp);
  return pred;
}

void motionToColor(Mat flow, Mat &color)  
{  
  if (color.empty())  
    color.create(flow.rows, flow.cols, CV_8UC3);  

  static vector<Scalar> colorwheel; //Scalar r,g,b  
  if (colorwheel.empty())  
    makecolorwheel(colorwheel);  

  // determine motion range:  
  float maxrad = -1;  

  // Find max flow to normalize fx and fy  
  for (int i= 0; i < flow.rows; ++i)   
  {  
    for (int j = 0; j < flow.cols; ++j)   
    {  
      Vec3b flow_at_point = flow.at<Vec3b>(i, j);  
      float fx = (float)flow_at_point[2];  
      float fy = (float)flow_at_point[1];  
      if ((fabs(fx) >  UNKNOWN_FLOW_THRESH) || (fabs(fy) >  UNKNOWN_FLOW_THRESH))  
        continue;  
      float rad = sqrt(fx * fx + fy * fy);  
      maxrad = maxrad > rad ? maxrad : rad;  
    }  
  }  

  for (int i= 0; i < flow.rows; ++i)   
  {  
    for (int j = 0; j < flow.cols; ++j)   
    {  
      uchar *data = color.data + color.step[0] * i + color.step[1] * j;  
      Vec3b flow_at_point = flow.at<Vec3b>(i, j);  

      float fx = flow_at_point[2] / maxrad;  
      float fy = flow_at_point[1] / maxrad;  
      if ((fabs(fx) >  UNKNOWN_FLOW_THRESH) || (fabs(fy) >  UNKNOWN_FLOW_THRESH))  
      {  
        data[0] = data[1] = data[2] = 0;  
        continue;  
      }  
      float rad = sqrt(fx * fx + fy * fy);  

      float angle = atan2(-fy, -fx) / CV_PI;  
      float fk = (angle + 1.0) / 2.0 * (colorwheel.size()-1);  
      int k0 = (int)fk;  
      int k1 = (k0 + 1) % colorwheel.size();  
      float f = fk - k0;  
      //f = 0; // uncomment to see original color wheel  

      for (int b = 0; b < 3; b++)   
      {  
        float col0 = colorwheel[k0][b] / 255.0;  
        float col1 = colorwheel[k1][b] / 255.0;  
        float col = (1 - f) * col0 + f * col1;  
        if (rad <= 1)  
          col = 1 - rad * (1 - col); // increase saturation with radius  
        else  
          col *= .75; // out of range  
        data[2 - b] = (int)(255.0 * col);  
      }  
    }  
  }  
}

void makecolorwheel(vector<Scalar> &colorwheel)  
{  
  int RY = 15;  
  int YG = 6;  
  int GC = 4;  
  int CB = 11;  
  int BM = 13;  
  int MR = 6;  

  int i;  

  for (i = 0; i < RY; i++) colorwheel.push_back(Scalar(255,       255*i/RY,     0));  
  for (i = 0; i < YG; i++) colorwheel.push_back(Scalar(255-255*i/YG, 255,       0));  
  for (i = 0; i < GC; i++) colorwheel.push_back(Scalar(0,         255,      255*i/GC));  
  for (i = 0; i < CB; i++) colorwheel.push_back(Scalar(0,         255-255*i/CB, 255));  
  for (i = 0; i < BM; i++) colorwheel.push_back(Scalar(255*i/BM,      0,        255));  
  for (i = 0; i < MR; i++) colorwheel.push_back(Scalar(255,       0,        255-255*i/MR));  
}  

void genGMFeature(Mat frame)
{
  int   width = frame.cols;
  int   height= frame.rows;
  float* gradMGData = NULL;

  extern int   g_pyramidNum;
  extern float g_pyrScale;

  gradMGData = (float*)malloc(width*height*g_pyramidNum*sizeof(float));

  Mat   grad;
  Mat   gradFullImg(height, width, CV_8UC1);

  for ( int i=1; i<=g_pyramidNum; i++ )
  {
    Mat scaledFrame(height, width, CV_8UC1);
    resize(frame, scaledFrame, Size(width, height)); //Size(frame.cols*4, frame.rows*4));

    grad = getSobelEdge(scaledFrame);
    // imwrite("1.png", scaledFrame);

    int cha = grad.channels();
    int type2 = grad.type();
    int depth = grad.depth();

    resize(grad, gradFullImg, Size(frame.cols, frame.rows));

    // imwrite("1.png", gradFullImg);

    for (int m=0; m<frame.rows; m++)
    {
      for (int n=0; n<frame.cols; n++)
      {
        uchar pel = gradFullImg.at<uchar>(m,n);
        gradMGData[n*g_pyramidNum+m*frame.cols*g_pyramidNum+i-1] = (float)gradFullImg.at<uchar>(m,n);
        
        // printf( "%d\n", n*g_pyramidNum+m*frame.cols*g_pyramidNum+i-1);
      }
    }
    // imwrite("2.png", gradFullImg);
    height = height*g_pyrScale;
    width  = width*g_pyrScale;
  }

  // Write Features Into File.
  FILE *fp;
  fp=fopen("FV_GradMagnitude.dat", "wb");
  fwrite(gradMGData, sizeof(float), frame.cols*frame.rows*g_pyramidNum, fp);
  fclose(fp);

  //FILE *fpT1;
  //fpT1=fopen("FV_GradMagnitude.data", "w");
  //for (int i=0; i<frame.rows*frame.cols; i++)
  //{
  //  fprintf(fpT1, "%f,%f,%f,%f\n", gradMGData[4*i], gradMGData[4*i+1], gradMGData[4*i+2], gradMGData[4*i+3]);
  //}
  //fclose(fpT1);

  free(gradMGData);
}

// Get Histogram.
BOOL GetHistogram(unsigned char *pImageData, int nWidth, int nHeight, int nWidthStep,   
                  int *pHistogram)   
{   
  int            i     = 0;   
  int            j     = 0;   
  unsigned char *pLine = NULL;   
  // Reset Histgram.   
  memset(pHistogram, 0, sizeof(int) * 256);   
  for (pLine = pImageData, j = 0; j < nHeight; j++, pLine += nWidthStep)   
  {   
    for (i = 0; i < nWidth; i++)   
    {   
      pHistogram[pLine[i]]++;   
    }   
  }   
  return TRUE;   
}   

// Using OSTU Method to get Threshold.
// However it proves not Efficient.
int getEdgeThreshold( Mat edgeDistMat)   
{   
  int    nWidth     = edgeDistMat.cols;
  int    nHeight    = edgeDistMat.rows;
  int    nWidthStep = nWidth;
  int    i          = 0;   
  int    j          = 0;   
  int    nTotal     = 0;   
  int    nSum       = 0;   
  int    A          = 0;   
  int    B          = 0;   
  double u          = 0;   
  double v          = 0;   
  double dVariance  = 0;   
  double dMaximum   = 0;   
  int    nThreshold = 0;   
  int    nHistogram[256];
  unsigned char* pImageData = NULL;
  pImageData = (unsigned char*)malloc(nWidth*nHeight*sizeof(unsigned char));

  for (int m=0; m<nHeight; m++)
  {
    for (int n=0; n<nWidth; n++)
    {
      pImageData[nWidth*m+n] = edgeDistMat.at<uchar>(m,n);
    }
  }

  // Get Histogram.   
  GetHistogram(pImageData, nWidth, nHeight, nWidthStep, nHistogram);   
  for (i = 0; i < 256; i++)   
  {   
    nTotal += nHistogram[i];   
    nSum   += (nHistogram[i] * i);   
  }   
  for (j = 0; j < 256; j++)   
  {   
    A = 0;   
    B = 0;   
    for (i = 0; i < j; i++)   
    {   
      A += nHistogram[i];   
      B += (nHistogram[i] * i);   
    }   
    if (A > 0)   
    {   
      u = B / A;   
    }   
    else  
    {   
      u = 0;   
    }   
    if (nTotal - A > 0)    
    {   
      v = (nSum - B) / (nTotal - A);   
    }   
    else  
    {   
      v = 0;   
    }   
    dVariance = A * (nTotal - A) * (u - v) * (u - v);   
    if (dVariance > dMaximum)   
    {   
      dMaximum = dVariance;   
      nThreshold = j;   
    }   
  }   
  free(pImageData);
  return nThreshold;   
}   

// float imgTemp[1920][1920];
void genEdgeDisFeature(Mat frame)
{
  int    width = frame.cols;
  int    height= frame.rows;
  float* edgeDistData = NULL;

  edgeDistData = (float*)malloc(width*height*g_pyramidNum*sizeof(float));

  Mat   edgeDist;
  Mat   edgeDistFull(height, width, CV_32FC1);

  for ( int i=1; i<=g_pyramidNum; i++ )
  {
    Mat bEdgeFrame    (height, width, CV_8UC1);
    Mat scaledFrame   (height, width, CV_8UC1);
    Mat distTransFrame(height, width, CV_32FC1);
    resize(frame, scaledFrame, Size(width, height)); //Size(frame.cols*4, frame.rows*4));

    edgeDist = getSobelEdge(scaledFrame);
    // imwrite("1_edge.png", edgeDist);
    
    // using OSTU Method to get Edge.
    // uchar edgeTH = getEdgeThreshold(edgeDist);
    uchar edgeTH = 128;

    threshold(edgeDist, bEdgeFrame, edgeTH, 255, THRESH_BINARY);
    // Reverse Image.
    for (int m=0; m<bEdgeFrame.rows; m++)
    {
      for (int n=0; n<bEdgeFrame.cols; n++)
      {
        bEdgeFrame.at<uchar>(m,n) = bEdgeFrame.at<uchar>(m,n)==255 ? 0:255;
      }
    }
    // imwrite("1.png", bEdgeFrame);

    distanceTransform(bEdgeFrame, distTransFrame, CV_DIST_L2, 3);

    //for (int m=0; m<distTransFrame.rows; m++)
    //{
    //  for (int n=0; n<distTransFrame.cols; n++)
    //  {
    //    imgTemp[m][n] = distTransFrame.at<float>(m,n);
    //  }
    //}
    
    // imwrite("dist.png", distTransFrame);

    resize(distTransFrame, edgeDistFull, Size(frame.cols, frame.rows));

    for (int m=0; m<edgeDistFull.rows; m++)
    {
      for (int n=0; n<edgeDistFull.cols; n++)
      {
        edgeDistData[n*g_pyramidNum+m*edgeDistFull.cols*g_pyramidNum+i-1] = (float)edgeDistFull.at<float>(m,n);
      }
    }
    
    height = height*g_pyrScale;
    width  = width*g_pyrScale;
  }

  // Write Features Into File.
  FILE *fp;
  fp=fopen("FV_EdgeDistance.dat", "wb");
  fwrite(edgeDistData, sizeof(float), frame.cols*frame.rows*g_pyramidNum, fp);
  fclose(fp);

  //FILE *fpT1;
  //fpT1=fopen("FV_EdgeDistance.data", "w");
  //for (int i=0; i<frame.rows*frame.cols; i++)
  //{
  //  fprintf(fpT1, "%f,%f,%f,%f\n", edgeDistData[4*i], edgeDistData[4*i+1], edgeDistData[4*i+2], edgeDistData[4*i+3]);
  //}
  //fclose(fpT1);

  free(edgeDistData);
}

void genTempGradFeature(Mat mvFrame, Mat curFrame)
{
  int    width   = mvFrame.cols*4;
  int    height  = mvFrame.rows*4;
  int    widthMV = mvFrame.cols;
  int    heightMV= mvFrame.rows;
  float* tmpGradFeature = NULL;

  tmpGradFeature = (float*)malloc(curFrame.cols*curFrame.rows*g_pyramidNum*2*sizeof(float));

  // Mat   edgeDist;
  Mat tempGrad(width, height, CV_32FC3);
  for (int i=1; i<=g_pyramidNum; i++)
  {
    Mat scaledMV(heightMV, widthMV, CV_32FC3);
    Mat gradMVx(heightMV, widthMV, CV_32FC3),    gradMVy(heightMV, widthMV, CV_32FC3);
    Mat gradScaleMV(heightMV, widthMV, CV_32FC3);
    Mat gradSmallMV(mvFrame.rows, mvFrame.cols, CV_32FC3);
    Mat gradFullEncMV(mvFrame.rows*4, mvFrame.cols*4, CV_32FC3);

    resize(mvFrame, scaledMV, Size(widthMV, heightMV));
   
    // Sobel Grad.
    Sobel(scaledMV, gradMVx, CV_32F, 1, 0);
    Sobel(scaledMV, gradMVy, CV_32F, 0, 1);

    float pelA, pelB;
    for (int m=0; m<gradScaleMV.cols; m++)
    {
      for (int n=0; n<gradScaleMV.rows; n++)
      {
        for (int k=0; k<2; k++)
        {
          pelA = gradMVx.at<Vec3f>(n,m)[k];
          pelB = gradMVy.at<Vec3f>(n,m)[k];
          gradScaleMV.at<Vec3f>(n,m)[k] = (abs(pelA)+abs(pelB))/2.0;
        }
        gradScaleMV.at<Vec3f>(n,m)[2] = 0.0;
      }
    }

    resize(gradScaleMV, gradSmallMV,  Size(mvFrame.cols, mvFrame.rows));
    resize(gradSmallMV, gradFullEncMV,Size(mvFrame.cols*4, mvFrame.rows*4));
    // imwrite("1.png", gradFullEncMV);

    for (int m=0; m<curFrame.rows; m++)
    {
      for (int n=0; n<curFrame.cols; n++)
      {
        tmpGradFeature[n*g_pyramidNum*2+m*curFrame.cols*g_pyramidNum*2+i*2-2] = gradFullEncMV.at<Vec3f>(m,n)[0];
        tmpGradFeature[n*g_pyramidNum*2+m*curFrame.cols*g_pyramidNum*2+i*2-1] = gradFullEncMV.at<Vec3f>(m,n)[1];
      }
    }

    widthMV  = widthMV*g_pyrScale;
    heightMV = heightMV*g_pyrScale;
  }

  // Write Features Into File.
  FILE *fp;
  fp=fopen("FV_tempGrad.dat", "wb");
  fwrite(tmpGradFeature, sizeof(float), curFrame.cols*curFrame.rows*g_pyramidNum*2, fp);
  fclose(fp);

  //FILE *fpT1;
  //fpT1=fopen("FV_tempGrad.data", "w");
  //for (int i=0; i<curFrame.rows*curFrame.cols; i++)
  //{
  //  fprintf(fpT1, "%f,%f,%f,%f,",  tmpGradFeature[8*i],   tmpGradFeature[8*i+1], tmpGradFeature[8*i+2], tmpGradFeature[8*i+3]);
  //  fprintf(fpT1, "%f,%f,%f,%f\n", tmpGradFeature[8*i+4], tmpGradFeature[8*i+5], tmpGradFeature[8*i+6], tmpGradFeature[8*i+7]);
  //}
  //fclose(fpT1);

  free(tmpGradFeature);
}

void genPhotoConstancyFeature(Mat predFrame, Mat curFrame)
{
  int width = curFrame.cols;
  int height= curFrame.rows;
  ushort* resData = NULL;
  resData = (ushort*)malloc(curFrame.cols*curFrame.rows*sizeof(ushort));

  for (int i=0; i<predFrame.rows; i++)
  {
    for (int j=0; j<predFrame.cols; j++)
    {
      resData[j+i*predFrame.cols] = (ushort)abs(predFrame.at<uchar>(i,j)-curFrame.at<uchar>(i,j));
    }
  }

  // Write Features Into File.
  FILE *fp;
  fp=fopen("FV_photoConstancy.dat", "wb");
  fwrite(resData, sizeof(ushort), curFrame.cols*curFrame.rows, fp);
  fclose(fp);

  free(resData);
}

void genClassifyLabel(Mat mvFrame, string nocGTFlowName, float EPETH)
{
  Mat nocGTFlow = imread(nocGTFlowName, -1);
  
  int cha   = nocGTFlow.channels();
  int type  = nocGTFlow.type();
  int depth = nocGTFlow.depth();

  Mat gtFlowFloat(nocGTFlow.rows, nocGTFlow.cols, CV_32FC3);

  // FlowImage nocGTFlow(nocGTFlowName);
  uchar* classifyLabel = NULL;
  classifyLabel = (uchar*)malloc(nocGTFlow.rows*nocGTFlow.cols*sizeof(uchar));

  float  ERRORTH = EPETH;
  ushort channelB, channelG, channelR;

  // Code of Parsing the Ground Truth Optical Flow.
  for (int i=0; i<nocGTFlow.rows; i++)
  {
    for (int j=0; j<nocGTFlow.cols; j++)
    {
      channelB = nocGTFlow.at<Vec3s>(i,j)[0];
      channelG = nocGTFlow.at<Vec3s>(i,j)[1];
      channelR = nocGTFlow.at<Vec3s>(i,j)[2];

      if (channelB)
      {
        gtFlowFloat.at<Vec3f>(i,j)[0] = (channelR-32768.0f)/64.0f;
        gtFlowFloat.at<Vec3f>(i,j)[1] = (channelG-32768.0f)/64.0f;
        gtFlowFloat.at<Vec3f>(i,j)[2] = 1.0;
      }
      else
      {
        gtFlowFloat.at<Vec3f>(i,j)[0] = 0.0f;
        gtFlowFloat.at<Vec3f>(i,j)[1] = 0.0f;
        gtFlowFloat.at<Vec3f>(i,j)[2] = 0.0f;
      }
    }
  }

  for (int i=0; i<gtFlowFloat.rows; i++)
  {
    for (int j=0; j<gtFlowFloat.cols; j++)
    {
      float fu = gtFlowFloat.at<Vec3f>(i,j)[0]-mvFrame.at<Vec3f>(i,j)[0];
      float fv = gtFlowFloat.at<Vec3f>(i,j)[1]-mvFrame.at<Vec3f>(i,j)[1];
      float f_err = sqrt(fu*fu+fv*fv);
      if (gtFlowFloat.at<Vec3f>(i,j)[2]>0.5)
      {
        if (f_err>ERRORTH) 
          classifyLabel[j+i*nocGTFlow.cols] = 0;
        else               
          classifyLabel[j+i*nocGTFlow.cols] = 1;
      }
      else
        classifyLabel[j+i*nocGTFlow.cols] = 2;
    }
  }

  // Write Features Into File.
  FILE *fp;
  fp=fopen("FV_ClassifyLabel.dat", "wb");
  fwrite(classifyLabel, sizeof(uchar), gtFlowFloat.cols*gtFlowFloat.rows, fp);
  fclose(fp);

  free(classifyLabel);
}

void genAllFeatureVector(int width, int height, string curName, bool isTraingModel)
{
  uchar  classifyLable;
  float  gradMagFeat[4];
  float  edgeDistance[4];
  float  tempGrad[8];
  ushort photoConstant; 

  FILE* fpClassifyLabel = fopen("FV_ClassifyLabel.dat", "rb");
  FILE* fpGradMagnitude = fopen("FV_GradMagnitude.dat", "rb");
  FILE* fpEdgeDistance  = fopen("FV_EdgeDistance.dat",  "rb");
  FILE* fptempGrad      = fopen("FV_tempGrad.dat",      "rb");
  FILE* fpphotoConstancy= fopen("FV_photoConstancy.dat","rb");
 
  string featureName    = "FV_"+curName+".data";
  const char*    ftName = featureName.data();
  FILE*  fpAllFeature   = fopen(ftName,   "w");

  for ( int i=0; i<width*height; i++ )
  {
    fread(&classifyLable, sizeof(uchar), 1, fpClassifyLabel );
    fread(gradMagFeat,    sizeof(float), 4, fpGradMagnitude );
    fread(edgeDistance,   sizeof(float), 4, fpEdgeDistance  );
    fread(tempGrad,       sizeof(float), 8, fptempGrad      );
    fread(&photoConstant, sizeof(ushort),1, fpphotoConstancy);

    if (classifyLable<2 && isTraingModel)
    {
      fprintf(fpAllFeature, "%u,",          classifyLable);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", gradMagFeat[0],  gradMagFeat[1],  gradMagFeat[2],  gradMagFeat[3]);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", edgeDistance[0], edgeDistance[1], edgeDistance[2], edgeDistance[3]);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", tempGrad[0],     tempGrad[1],     tempGrad[2],     tempGrad[3]);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", tempGrad[4],     tempGrad[5],     tempGrad[6],     tempGrad[7]);
      fprintf(fpAllFeature, "%d\n",         photoConstant);
    }
    else if (isTraingModel==false)
    {
      fprintf(fpAllFeature, "%u,",          classifyLable);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", gradMagFeat[0],  gradMagFeat[1],  gradMagFeat[2],  gradMagFeat[3]);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", edgeDistance[0], edgeDistance[1], edgeDistance[2], edgeDistance[3]);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", tempGrad[0],     tempGrad[1],     tempGrad[2],     tempGrad[3]);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", tempGrad[4],     tempGrad[5],     tempGrad[6],     tempGrad[7]);
      fprintf(fpAllFeature, "%d\n",         photoConstant);
    }
  }

  fclose(fpAllFeature);
  fclose(fpClassifyLabel);
  fclose(fpGradMagnitude);
  fclose(fpEdgeDistance);
  fclose(fptempGrad);
  fclose(fpphotoConstancy);

  // Delete FileName
  remove("FV_ClassifyLabel.dat" );
  remove("FV_GradMagnitude.dat" );
  remove("FV_EdgeDistance.dat"  );
  remove("FV_tempGrad.dat"      );
  remove("FV_photoConstancy.dat");
}

void integAllFVs()
{
  string dir = ".\\FV_0000";
  FILE*  fp;

  uchar  classifyLable;
  float  gradMagFeat[4];
  float  edgeDistance[4];
  float  tempGrad[8];
  ushort photoConstant; 

  FILE*  fpAllFeature   = fopen("allFeatureTraining.data",   "w");

  string fileName;
  char   tmp[10];
  FILE*  fpFeature; //   = fopen(ftName,   "r");
  
  for (int i=0; i<=20; i++)
  {
    itoa(i, tmp, 10);
    if (i<10) fileName = dir+"0"+tmp+"_10.data";
    else      fileName = dir+tmp+"_10.data";

    const char*    ftName = fileName.data();

    if ((fpFeature=fopen(ftName, "r"))==NULL) {
      perror("fopen");
      exit(1);
    }

    while (!feof(fpFeature))
    {
      fscanf(fpFeature, "%u,",          &classifyLable);
      fscanf(fpFeature, "%f,%f,%f,%f,", &gradMagFeat[0],  &gradMagFeat[1],  &gradMagFeat[2],  &gradMagFeat[3]);
      fscanf(fpFeature, "%f,%f,%f,%f,", &edgeDistance[0], &edgeDistance[1], &edgeDistance[2], &edgeDistance[3]);
      fscanf(fpFeature, "%f,%f,%f,%f,", &tempGrad[0],     &tempGrad[1],     &tempGrad[2],     &tempGrad[3]);
      fscanf(fpFeature, "%f,%f,%f,%f,", &tempGrad[4],     &tempGrad[5],     &tempGrad[6],     &tempGrad[7]);
      fscanf(fpFeature, "%d\n",         &photoConstant);

      fprintf(fpAllFeature, "%u,",          classifyLable);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", gradMagFeat[0],  gradMagFeat[1],  gradMagFeat[2],  gradMagFeat[3]);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", edgeDistance[0], edgeDistance[1], edgeDistance[2], edgeDistance[3]);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", tempGrad[0],     tempGrad[1],     tempGrad[2],     tempGrad[3]);
      fprintf(fpAllFeature, "%f,%f,%f,%f,", tempGrad[4],     tempGrad[5],     tempGrad[6],     tempGrad[7]);
      fprintf(fpAllFeature, "%d\n",         photoConstant);
    }
    fclose(fpFeature);
  }
  fclose(fpAllFeature);
}

void getRatioC2DivC1(string fileNameC1, string fileNameC2, float* ratioC2DivC1)
{
  // float thr = 0;
  // png::image< png::rgb_pixel_16 > image(argv[1]);
  png::image< png::gray_pixel_16 > image_c1(fileNameC1);
  png::image< png::gray_pixel_16 > image_c2(fileNameC2);
  // sscanf(argv[4], "%f", &thr);
  // printf("THR %f\n", thr);

  int width  = image_c1.get_width();
  int height = image_c1.get_height();
  // FILE* fp_stat = fopen("cost_stats.txt", "w");
  for (int y = 0; y < height; y++)
  {
    for (int x=0; x < width; x++)
    {
      int   c1 = image_c1.get_pixel(x, y);
      int   c2 = image_c2.get_pixel(x, y);
      float fc = 1.0*c2/(c1+0.0001);

      ratioC2DivC1[x+y*width] = fc;

      //fprintf(fp_stat, "%d\t%d\t\n", c1, c2);
      //printf("%5d %5d, mvx %f mvy %f\n", mbx, mby, mvx, mvy);
      //png::rgb_pixel_16 val;
      //val = image.get_pixel(x, y); 
      //if(fc < thr) {
      //  val.blue = 0;
      //  image.set_pixel(x, y, val);
      //}
    }
  }
  // fclose(fp_stat);
  // image.write(argv[1]);//output filename
}

void getMVNewVersion(string fileName, Mat frame, float* mvx, float* mvy)
{
  const char* fileNameChar = fileName.data();
  
  int inWidthOrg  = frame.cols;
  int inHeightOrg = frame.rows;

  int inWidth  = (inWidthOrg  + 15) >> 4;
  int inHeight = (inHeightOrg + 15) >> 4;

  FILE* fp = fopen(fileNameChar, "rb");//bin file
  if(fp== NULL)
  {
    printf("cannot open file\n");
    exit(1);
  }

  for (int mby = 0; mby < inHeight; mby++)
  {
    for (int mbx=0; mbx < inWidth; mbx++)
    {
      for(int i8 = 0; i8 < 4; i8++) 
      {
        for(int i4 = 0; i4 < 4; i4 ++) 
        {
          int tx = 4*mbx + (i8%2)*2 + (i4%2);
          int ty = 4*mby + (i8/2)*2 + (i4/2);

          MVS t;
          fread(&t, sizeof(MVS), 1, fp);

          mvx[tx+ty*inWidth*4] = t.mvx;
          mvy[tx+ty*inWidth*4] = t.mvy;
        }
      }
    }
  }
}

float get4x4BlkMean(Mat curFrame, int locX, int locY)
{
  float mean;
  int   cnt, picWidth, picHeight;
  
  cnt = 0;
  picWidth  = curFrame.cols;
  picHeight = curFrame.rows;
  uInt sum = 0;

  for (int i=locY; i<locY+4; i++)
  {
    for (int j=locX; j<locX+4; j++)
    {
      bool insidePic = i>=0&&i<picHeight&&j>=0&&j<picWidth;

      if (insidePic)
      {
        sum+=curFrame.at<uchar>(i,j);
        cnt++;
      }
    }
  }
  mean = (float)sum/cnt;

  return mean;
}

int getDirDistance(Mat frame, const int* step, int pelX, int pelY)
{
  int xstep = step[0];
  int ystep = step[1];
  int stepCnt = 1;

  while (1)
  {
    int x = pelX+xstep*stepCnt;
    int y = pelY+ystep*stepCnt;

    if (x<0||x>=frame.cols||y<0||y>=frame.rows)
    {
      break;
    }
    if (frame.at<uchar>(y,x)==255)
    {
      break;
    }
    stepCnt++;
  }

  return stepCnt;
}

void  discardSmallVldRegion(Mat frame, Mat outFrame)
{
  const int step[8][2] = {{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1}}; //(widthDir, heightDir)
  int step8Dir[8];
  int width = frame.cols;
  int height= frame.rows;

  for (int i=0; i<height; i++)
  {
    for (int j=0; j<width; j++)
    {
      if (frame.at<uchar>(i,j)==255)
      {
        // outFrame.at<uchar>(i,j)=255;
        continue;
      }
      // Get the distance to the inValid Region in 8 Directions.
      for (int m=0; m<8; m++)
      {
        step8Dir[m] = getDirDistance(frame, step[m], j, i);
      }
      insertSort<int>(step8Dir, 8);
      int mean = (step8Dir[0]+step8Dir[7]+1)/2;
      if (mean<8) // Threshold. Personal Set
      {
        outFrame.at<uchar>(i,j) = 255;
      }
    }
  }
}

void discardSRbyMBRatio(Mat frame, Mat outFrame)
{
  Mat confMatTC = frame;
  int totalVld = 0;
  int realVld = 0;
  
  // Original Picture Size.
  int inWidthOrg  = frame.cols;
  int inHeightOrg = frame.rows;

  // For Video Coding Picture Size. This is the MB=16x16 Level.
  int inWidth  = (inWidthOrg  + 15) >> 4;
  int inHeight = (inHeightOrg + 15) >> 4;

  for (int i=0; i<inHeight; i++)
  {
    for (int j=0; j<inWidth; j++)
    {
      totalVld = 0;
      realVld  = 0;
      for (int m=0; m<16; m++)
      {
        for (int n=0; n<16; n++)
        {
          int pelX = j*16+n;
          int pelY = i*16+m;

          if (pelX>=0 && pelX<outFrame.cols && pelY>=0 && pelY<outFrame.rows)
          {
            totalVld++;
            if (confMatTC.at<uchar>(pelY,pelX)==0)
            {
              realVld++;
            }
          }
        }
      }
      if (realVld>=0.95*totalVld)
      {
        for (int m=0; m<16; m++)
        {
          for (int n=0; n<16; n++)
          {
            int pelX = j*16+n;
            int pelY = i*16+m;

            if (pelX>=0 && pelX<outFrame.cols && pelY>=0 && pelY<outFrame.rows)
            {
              // outFrame.at<uchar>(pelY,pelX) = 0;
            }
          }
        }
      }
      else
      {
        for (int m=0; m<16; m++)
        {
          for (int n=0; n<16; n++)
          {
            int pelX = j*16+n;
            int pelY = i*16+m;

            if (pelX>=0 && pelX<outFrame.cols && pelY>=0 && pelY<outFrame.rows)
            {
              outFrame.at<uchar>(pelY,pelX) = 255;
            }
          }
        }
      }
    }
  }
}

void bwLabelFunc(Mat bwFrame)
{
  int width = bwFrame.cols;
  int height= bwFrame.rows;
  vector<int> stRun, endRun, rowRun, runLabelInit;
  int totalRuns = 0;
  int lastLineStIdx = 0, lastLineEndIdx = 0;
  vector<pair<int,int>> equivalences;

  // initial. The first Line.
  //for (int j=0; j<width; j++)
  //{
  //  uchar pelVal = bwFrame.at<uchar>(0,j);
  //  uchar pelValPre, pelValNext; // Get the Pre and Next Pixel Value.
  //  if (j==0) pelValPre = 255;
  //  else      pelValPre = bwFrame.at<uchar>(0,j-1);

  //  if (j<width-1) pelValNext = bwFrame.at<uchar>(0,j+1);
  //  else           pelValNext = 255;

  //  if (pelValPre==255 && pelVal==0) // start Valid.
  //  {
  //    stRun.push_back(j);
  //    rowRun.push_back(0);
  //    runLabelInit.push_back(totalRuns);
  //    totalRuns++;
  //  }

  //  if (pelVal==0 && pelValNext==255) // End Valid.
  //  {
  //    endRun.push_back(j);
  //  }
  //}
  lastLineStIdx  = 0;
  lastLineEndIdx = 0;

  for (int i=0; i<height; i++)
  {
    for (int j=0; j<width; j++)
    {
      uchar pelVal = bwFrame.at<uchar>(i,j);
      uchar pelValPre, pelValNext; // Get the Pre and Next Pixel Value.
      if (j==0) pelValPre = 255;
      else      pelValPre = bwFrame.at<uchar>(i,j-1);

      if (j<width-1) pelValNext = bwFrame.at<uchar>(i,j+1);
      else           pelValNext = 255;

      if (pelValPre==255 && pelVal==0) // start Valid.
      {
        stRun.push_back(j);
        rowRun.push_back(i);
      }

      if (pelVal==0 && pelValNext==255) // End Valid.
      {
        endRun.push_back(j);

        // Get Label.
        int curLabel = -1;
        for (int m=lastLineStIdx; m<lastLineEndIdx; m++) // Last Line Valid. 
        {
          int startRunLastLine = stRun[m];
          int endRunLastLine   = endRun[m];

          int startRunCur  = stRun[stRun.size()-1];
          int endRunCur    = endRun[stRun.size()-1];
          
          if (startRunLastLine<=endRunCur+1 && endRunLastLine+1>=startRunCur) // Connection Label.
          {
            if (curLabel==-1)
            {
              curLabel = runLabelInit[m];
            }
            else // Set Connection Equal Label.
            {
              equivalences.push_back(make_pair(curLabel, runLabelInit[m]));
            }
          }
        }
        if (curLabel==-1)
        {
          curLabel = totalRuns;
          totalRuns++;
        }

        runLabelInit.push_back(curLabel);
      }
    }
    lastLineStIdx  = lastLineEndIdx; // Update Last Line's Start/End Run.
    lastLineEndIdx = endRun.size();
  }

  // DAG; Find the Same Connection Label.
  int maxLabel = *max_element(runLabelInit.begin(), runLabelInit.end());
  vector<vector<bool>> eqTab(totalRuns, vector<bool>(totalRuns, false)); // graph Init.
  // Construct Graph.
  vector<pair<int, int>>::iterator vecPairIt = equivalences.begin();
  while (vecPairIt != equivalences.end())
  {
    eqTab[vecPairIt->first][vecPairIt->second] = true;
    eqTab[vecPairIt->second][vecPairIt->first] = true;
    vecPairIt++;
  }

  vector<int> labelFlag(totalRuns, 0);
  vector<vector<int>> equaList;
  vector<int> tempList;
  // cout << maxLabel << endl;
  for (int i = 0; i <= maxLabel; i++)
  {
    if (labelFlag[i])
    {
      continue;
    }
    labelFlag[i] = equaList.size() + 1;
    tempList.push_back(i);

    // BFS Search Algorithm.
    for (vector<int>::size_type j = 0; j < tempList.size(); j++)
    {
      for (vector<bool>::size_type k = 0; k != eqTab[tempList[j]].size(); k++)
      {
        if (eqTab[tempList[j]][k] && !labelFlag[k])
        {
          tempList.push_back(k);
          labelFlag[k] = equaList.size() + 1;
        }
      }
    }
    equaList.push_back(tempList);
    tempList.clear();
  }
  // cout << equaList.size() << endl;
  for (vector<int>::size_type i = 0; i != runLabelInit.size(); i++)
  {
    runLabelInit[i] = labelFlag[runLabelInit[i]];
  }
}

void bwLabelContour(const Mat& imgBw, Mat & imgLabeled)
{
  // Padding a Pixel around the pixel. 
  Mat imgClone = Mat(imgBw.rows + 1, imgBw.cols + 1, imgBw.type(), Scalar(0));
  imgBw.copyTo(imgClone(Rect(1, 1, imgBw.cols, imgBw.rows)));

  imgLabeled.create(imgClone.size(), imgClone.type());
  imgLabeled.setTo(Scalar::all(0));

  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(imgClone, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

  vector<int> contoursLabel(contours.size(), 0);
  int numlab = 1;
  // Outer Nested Contours
  for (vector<vector<Point>>::size_type i = 0; i < contours.size(); i++)
  {
    if (hierarchy[i][3] >= 0) // Nested Contours.
    {
      continue;
    }
    for (vector<Point>::size_type k = 0; k != contours[i].size(); k++)
    {
      imgLabeled.at<uchar>(contours[i][k].y, contours[i][k].x) = numlab;
    }
    contoursLabel[i] = numlab++;
  }
  // Inner Nested Contours
  for (vector<vector<Point>>::size_type i = 0; i < contours.size(); i++)
  {
    if (hierarchy[i][3] < 0) // Not Nested Contours.
    {
      continue;
    }
    for (vector<Point>::size_type k = 0; k != contours[i].size(); k++)
    {
      imgLabeled.at<uchar>(contours[i][k].y, contours[i][k].x) = contoursLabel[hierarchy[i][3]];
    }
  }
  // Assign to the remaining non-contour Samples.
  for (int i = 0; i < imgLabeled.rows; i++)
  {
    for (int j = 0; j < imgLabeled.cols; j++)
    {
      if (imgClone.at<uchar>(i, j) != 0 && imgLabeled.at<uchar>(i, j) == 0)
      {
        imgLabeled.at<uchar>(i, j) = imgLabeled.at<uchar>(i, j - 1);
      }
    }
  }
  // Discard the Neighboring 1 Samples.
  imgLabeled = imgLabeled(Rect(1, 1, imgBw.cols, imgBw.rows)).clone(); 
}

void fbtcCombineConf(string curName, string inDir, string outDir, Mat preFrame, Mat curFrame, float th)
{
  // Original Picture Size.
  int inWidthOrg  = preFrame.cols;
  int inHeightOrg = preFrame.rows;

  // For Video Coding Picture Size. This is the MB=16x16 Level.
  int inWidth  = (inWidthOrg  + 15) >> 4;
  int inHeight = (inHeightOrg + 15) >> 4;

  string partName = curName;
  partName.erase(partName.length()-2,2);
  char widthS[20], heightS[20];
  itoa(inWidth*16,  widthS,  10);
  itoa(inHeight*16, heightS, 10);
  string sparseMVFileName = inDir+"\\NVENCOutData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv1.bin";

  string c1FileName = inDir+"\\tcInData\\data\\"+partName+widthS+"x"+heightS+"_out\\c1.png";
  string c2FileName = inDir+"\\tcInData\\data\\"+partName+widthS+"x"+heightS+"_out\\c2.png";

  string mvTCForwardFileName  = inDir+"\\tcInData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv_F.bin";
  string mvTCForwardFileName1 = inDir+"\\tcInData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv_F_1.bin";
  string mvFBForwardFileName  = inDir+"\\fbInData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv_F.bin";
  string mvFBBackwardFileName = inDir+"\\fbInData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv_B.bin";

  // Get C2/C1 ratio.
  float* ratioC2DivC1 = NULL;
  ratioC2DivC1 = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  getRatioC2DivC1(c1FileName, c2FileName, ratioC2DivC1);

  float *mvTCForwardx=NULL,  *mvTCForwardy=NULL;
  float *mvTCForward_1x=NULL,*mvTCForward_1y=NULL;
  float *mvFBForwardx=NULL,  *mvFBForwardy=NULL;
  float *mvFBBackwardx=NULL, *mvFBBackwardy=NULL;
  mvTCForwardx   = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvTCForwardy   = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvTCForward_1x = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvTCForward_1y = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvFBForwardx   = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvFBForwardy   = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvFBBackwardx  = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvFBBackwardy  = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));

  getMVNewVersion(mvTCForwardFileName.data(),  preFrame, mvTCForwardx,   mvTCForwardy  );
  getMVNewVersion(mvTCForwardFileName1.data(), preFrame, mvTCForward_1x, mvTCForward_1y);
  getMVNewVersion(mvFBForwardFileName.data(),  preFrame, mvFBForwardx,   mvFBForwardy  );
  getMVNewVersion(mvFBBackwardFileName.data(), preFrame, mvFBBackwardx,  mvFBBackwardy );

  // DownScaling the TC MVs.
  // Attention Please!
  for (int i=0; i<inHeight*4; i++)
  {
    for (int j=0; j<inWidth*4; j++)
    {
      mvTCForwardx[i*inWidth*4+j]   /= 4.0;
      mvTCForward_1x[i*inWidth*4+j] /= 4.0;
      mvTCForwardy[i*inWidth*4+j]   /= 4.0;
      mvTCForward_1y[i*inWidth*4+j] /= 4.0;
    }
  }

  Mat fTCForward  (inHeight*4,  inWidth*4,  CV_32FC3);
  Mat fTCForward_1(inHeight*4,  inWidth*4,  CV_32FC3);
  Mat fFBForward  (inHeight*4,  inWidth*4,  CV_32FC3);
  Mat fFBBackward (inHeight*4,  inWidth*4,  CV_32FC3);

  for ( int i=0; i<inHeight*4; i++ )
  {
    for ( int j=0; j<inWidth*4; j++ )
    {
      fTCForward.at<Vec3f>(i,j)[0]   = mvTCForwardx[j+i*inWidth*4];
      fTCForward.at<Vec3f>(i,j)[1]   = mvTCForwardy[j+i*inWidth*4];
      fTCForward.at<Vec3f>(i,j)[2]   = 1;

      fTCForward_1.at<Vec3f>(i,j)[0] = mvTCForward_1x[j+i*inWidth*4];
      fTCForward_1.at<Vec3f>(i,j)[1] = mvTCForward_1y[j+i*inWidth*4];
      fTCForward_1.at<Vec3f>(i,j)[2] = 1;

      fFBForward.at<Vec3f>(i,j)[0]   = mvFBForwardx[j+i*inWidth*4];
      fFBForward.at<Vec3f>(i,j)[1]   = mvFBForwardy[j+i*inWidth*4];
      fFBForward.at<Vec3f>(i,j)[2]   = 1;

      fFBBackward.at<Vec3f>(i,j)[0]   = mvFBBackwardx[j+i*inWidth*4];
      fFBBackward.at<Vec3f>(i,j)[1]   = mvFBBackwardy[j+i*inWidth*4];
      fFBBackward.at<Vec3f>(i,j)[2]   = 1;
    }
  }

  Mat fTCForwardMed;
  Mat fTCForward_1Med;
  Mat fFBForwardMed;
  Mat fFBBackwardMed;

  Mat fTCForwardFull;
  Mat fTCForward_1Full;
  Mat fFBForwardFull;
  Mat fFBBackwardFull;

  Mat fTCForwardMedFull;
  Mat fTCForward_1MedFull;

  // Median Filter Very Efficient?
  medianBlur(fTCForward,   fTCForwardMed,   5);
  medianBlur(fTCForward_1, fTCForward_1Med, 5);
  medianBlur(fFBForward,   fFBForwardMed,   5);
  medianBlur(fFBBackward,  fFBBackwardMed,  5);

  resize(fTCForwardMed  , fTCForwardFull  , Size(fTCForward.cols*4  , fTCForward.rows*4  ));
  resize(fTCForward_1Med, fTCForward_1Full, Size(fTCForward_1.cols*4, fTCForward_1.rows*4));
  resize(fFBForwardMed  , fFBForwardFull  , Size(fFBForward.cols*4  , fFBForward.rows*4  ));
  resize(fFBBackwardMed , fFBBackwardFull , Size(fFBBackward.cols*4 , fFBBackward.rows*4 ));

  Mat confMatFB  (curFrame.rows, curFrame.cols, CV_8UC1);
  Mat confMatTC  (curFrame.rows, curFrame.cols, CV_8UC1);
  Mat confMatPKNR(curFrame.rows, curFrame.cols, CV_8UC1);
  Mat confCombine(curFrame.rows, curFrame.cols, CV_8UC1);

  float TCThreshold = th;
  float FBThreshold = th;

  bool  tcCheckVld;
  bool  fbCheckVld;

  string fileCombineOut = outDir+"\\confData\\"+curName+".data";
  const  char* confOutFileName = fileCombineOut.data();
  FILE*  fpConf=fopen(confOutFileName,"w");

  for (int i=0; i<curFrame.rows; i++)
  {
    for (int j=0; j<curFrame.cols; j++)
    {
      // PKNR Check.
      int xblk4 = j/4;
      int yblk4 = i/4;
      if (ratioC2DivC1[xblk4+yblk4*inWidth*4] < 1.1f) confMatPKNR.at<uchar>(i,j) = 0;
      else                                            confMatPKNR.at<uchar>(i,j) = 255;

      // TC Check.
      float TCforwardMVx = fTCForwardFull.at<Vec3f>(i,j)[0];
      float TCforwardMVy = fTCForwardFull.at<Vec3f>(i,j)[1];

      float TCforward_1MVx = fTCForward_1Full.at<Vec3f>(i,j)[0];
      float TCforward_1MVy = fTCForward_1Full.at<Vec3f>(i,j)[1];

      float absMVx = abs(TCforwardMVx-TCforward_1MVx);
      float absMVy = abs(TCforwardMVy-TCforward_1MVy);

      float tcMVMag= absMVx+absMVy;

      if (tcMVMag<=TCThreshold) confMatTC.at<uchar>(i,j) = 0;
      else                      confMatTC.at<uchar>(i,j) = 255;

      if (tcMVMag<=TCThreshold) tcCheckVld = true;
      else                      tcCheckVld = false;

      // FB Check.
      float forwardMVx = fFBForwardFull.at<Vec3f>(i,j)[0];
      float forwardMVy = fFBForwardFull.at<Vec3f>(i,j)[1];

      float backwardMVx, backwardMVy;
      float fbMVMag=100000.0f;

      // Attention this is the Core Idea Of the FB Check.
      // The Corresponding Pixel in the backward Frame. 
      int   backLocX   = j+(int)forwardMVx;
      int   backLocY   = i+(int)forwardMVy;

      if (backLocX>=0 && backLocX<fFBBackwardFull.cols && backLocY>=0 && backLocY<fFBBackwardFull.rows)
      {
        backwardMVx = fFBBackwardFull.at<Vec3f>(backLocY,backLocX)[0];
        backwardMVy = fFBBackwardFull.at<Vec3f>(backLocY,backLocX)[1];

        fbMVMag = abs(forwardMVx+backwardMVx)+abs(forwardMVy+backwardMVy);

        if (fbMVMag<=FBThreshold) confMatFB.at<uchar>(i,j) = 0;
        else                      confMatFB.at<uchar>(i,j) = 255;

        if (fbMVMag<=FBThreshold) fbCheckVld = true;
        else                      fbCheckVld = false;
      }
      else
      {
        confMatFB.at<uchar>(i,j) = 255;
        fbCheckVld = false;
      }

      // FB/TC/BlkSimilar Confidence Measure.
      //if (fbCheckVld)
      //{
      //  confCombine.at<uchar>(i,j) = 0;
      //}
      //else if (fbCheckVld==false && tcCheckVld)
      //{
      //  // Get The Prediction Location.
      //  int curTC_1MVx = j+(int)(TCforward_1MVx*4);
      //  int curTC_1MVy = i+(int)(TCforward_1MVy*4);

      //  if (curTC_1MVx>=0 && curTC_1MVx<curFrame.cols && curTC_1MVy>=0 && curTC_1MVy<curFrame.rows)
      //  {
      //    float meanCur = get4x4BlkMean(curFrame, j,          i);
      //    float meanRef = get4x4BlkMean(curFrame, curTC_1MVx, curTC_1MVy);

      //    if (abs(meanCur-meanRef) <= 3.0f)
      //    {
      //      confCombine.at<uchar>(i,j) = 0;
      //    }
      //    else
      //    {
      //      confCombine.at<uchar>(i,j) = 255;
      //    }
      //  }
      //  else
      //  {
      //    confCombine.at<uchar>(i,j) = 255;
      //  }
      //}
      //else
      //{
      //  confCombine.at<uchar>(i,j) = 255;
      //}

      if (fbCheckVld&&tcCheckVld)
      {
        confCombine.at<uchar>(i,j) = 0;
      }
      else
      {
        confCombine.at<uchar>(i,j) = 255;
      }

      // fprintf(fpConf, "%d\n", confMatFB.at<uchar>(i,j)/250 );
    }
  }
  // fclose(fpConf);
  // imwrite("1.png", confMatFB);
  // imwrite("2.png", confMatTC);
  // imwrite("3.png", confCombine);
  // imwrite("3.png", confMatPKNR);

  // bwLabelFunc(confCombine);

  //Mat out;
  //for (int i=0; i<confCombine.rows; i++)
  //{
  //  for (int j=0; j<confCombine.cols; j++)
  //  {
  //    confCombine.at<uchar>(i,j) = confCombine.at<uchar>(i,j)==255?0:255;
  //  }
  //}
  // bwLabelContour(confCombine, out);
  // imwrite("out.png",out);

  Mat outCombine = confCombine;

  discardSRbyMBRatio(confCombine, outCombine);

  // discardSmallVldRegion(confCombine, outCombine);
  // imwrite("4.png", outCombine);

  for (int i=0; i<outCombine.rows; i++)
  {
    for (int j=0; j<outCombine.cols; j++)
    {
      fprintf(fpConf, "%d\n", outCombine.at<uchar>(i,j)/250 );
    }
  }
  fclose(fpConf);

  free(ratioC2DivC1);

  free(mvTCForwardx   );
  free(mvTCForwardy   );
  free(mvTCForward_1x );
  free(mvTCForward_1y );
  free(mvFBForwardx   );
  free(mvFBForwardy   );
  free(mvFBBackwardx  );
  free(mvFBBackwardy  );
}

// Simple Insert Sort Fun.
void insertSortEPE( vector<float>& pelEPE, vector<pair<int,int>>& pelLoc)
{
  int length = pelLoc.size();
  for (int i = 1; i < length; i++)
  {
    if (pelEPE[i-1]<pelEPE[i])
    {
      float temp = pelEPE[i];
      pair<int,int> loctmp = pelLoc[i];

      int j = i;

      // find the insert Loc.
      while (j > 0 && pelEPE[j-1] < temp)
      {
        pelEPE[j] = pelEPE[j-1];
        pelLoc[j] = pelLoc[j-1];
        j--;
      }
      pelEPE[j] = temp;
      pelLoc[j] = loctmp;
    }

    /*if (x[i-1] > x[i])
    {
      T   temp = x[i];
      int j = i;
      while (j > 0 && x[j-1] > temp)
      {
        x[j] = x[j-1];
        j--;
      }
      x[j] = temp;
    }*/
  }
}

// Based on the Optical Flow and Ground Truth Optical Flow.
// Discard the number of Pixels with the largest EPE 
void calOptConfMap(Mat fullMVMat, string nocGTFlowName, string outDir, string curName, float ratio)
{
  Mat nocGTFlow = imread(nocGTFlowName, -1);
  Mat gtFlowFloat(nocGTFlow.rows, nocGTFlow.cols, CV_32FC3);

  Mat confMap(nocGTFlow.rows, nocGTFlow.cols, CV_8UC1);

  ushort channelB, channelG, channelR;

  vector<pair<int,int>> pelLoc;
  vector<float> pelEPE;

  float sumEPE = 0;

  // Code of Parsing the Ground Truth Optical Flow.
  for (int i=0; i<nocGTFlow.rows; i++)
  {
    for (int j=0; j<nocGTFlow.cols; j++)
    {
      channelB = nocGTFlow.at<Vec3s>(i,j)[0];
      channelG = nocGTFlow.at<Vec3s>(i,j)[1];
      channelR = nocGTFlow.at<Vec3s>(i,j)[2];

      if (channelB)
      {
        confMap.at<uchar>(i,j) = 0;

        gtFlowFloat.at<Vec3f>(i,j)[0] = (channelR-32768.0f)/64.0f;
        gtFlowFloat.at<Vec3f>(i,j)[1] = (channelG-32768.0f)/64.0f;
        gtFlowFloat.at<Vec3f>(i,j)[2] = 1.0;

        pelLoc.push_back(make_pair(i,j));
        float mvXError = abs(gtFlowFloat.at<Vec3f>(i,j)[0]-fullMVMat.at<Vec3f>(i,j)[0]);
        float mvYError = abs(gtFlowFloat.at<Vec3f>(i,j)[1]-fullMVMat.at<Vec3f>(i,j)[1]);
        
        float epe = sqrt(mvXError*mvXError+mvYError*mvYError);

        sumEPE += epe;
        pelEPE.push_back(epe);
      }
      else
      {
        confMap.at<uchar>(i,j) = 255;

        gtFlowFloat.at<Vec3f>(i,j)[0] = 0.0f;
        gtFlowFloat.at<Vec3f>(i,j)[1] = 0.0f;
        gtFlowFloat.at<Vec3f>(i,j)[2] = 0.0f;
      }
    }
  }

  insertSortEPE(pelEPE, pelLoc);

  int omitPelNum = (1.0-ratio)*pelLoc.size();

  for (int i=0; i<omitPelNum; i++)
  {
    pair<int,int> loc = pelLoc[i];

    confMap.at<uchar>(loc.first,loc.second) = 255;
  }

  string fileCombineOut = outDir+"\\confData\\"+curName+".data";
  const  char* confOutFileName = fileCombineOut.data();
  FILE*  fpConf=fopen(confOutFileName,"w");
  for (int i=0; i<confMap.rows; i++)
  {
    for (int j=0; j<confMap.cols; j++)
    {
      fprintf(fpConf, "%d\n", confMap.at<uchar>(i,j)/250 );
    }
  }
  fclose(fpConf);
}

double getDirRegion(float mvx, float mvy)
{
  double PI = 3.1415926;
  double angel = atan2(mvy,mvx)*180/PI;
  int dirRegion=-1;
  if (angel>=-185 && angel<-135) // 185: some margins.
    dirRegion = 4;
  else if (angel>=-135 && angel<-90)
    dirRegion = 5;
  else if (angel>=-90 && angel<-45)
    dirRegion = 6;
  else if (angel>=-45 && angel<0)
    dirRegion = 7;
  else if (angel>=0 && angel<45)
    dirRegion = 0;
  else if (angel>=45 && angel<90)
    dirRegion = 1;
  else if (angel>=90 && angel<135)
    dirRegion = 2;
  else if (angel>=135 && angel<=185)
    dirRegion = 3;
  else
  {
    printf("Error DirRegion!\n");
    exit(0);
  }

  return angel;
}

void dumpFlowImg(Mat frame, string outName)
{
  FlowImage F_orig(frame.cols, frame.rows);

  for (int i=0; i<frame.rows; i++)
  {
    for (int j=0; j<frame.cols; j++)
    {
      float mvx = frame.at<Vec3f>(i,j)[0];
      float mvy = frame.at<Vec3f>(i,j)[1];
      float vld = frame.at<Vec3f>(i,j)[2];

      F_orig.setFlowU(j, i, mvx);
      F_orig.setFlowV(j, i, mvy);
      F_orig.setValid(j, i, vld);
    }
  }

  // find maximum ground truth flow
  float max_flow = F_orig.maxFlow();

  // save original flow image
  F_orig.writeColor(outName,max_flow);
}

Mat getGroundTruthMat(string inDir, string curName)
{
  string nocGTFlowName = inDir+"\\flow_noc\\"+curName+".png";

  Mat nocGTFlow = imread(nocGTFlowName, -1);

  Mat gtFlowFloat(nocGTFlow.rows, nocGTFlow.cols, CV_32FC3);

  ushort channelB, channelG, channelR;

  // Code of Parsing the Ground Truth Optical Flow.
  for (int i=0; i<nocGTFlow.rows; i++)
  {
    for (int j=0; j<nocGTFlow.cols; j++)
    {
      channelB = nocGTFlow.at<Vec3s>(i,j)[0];
      channelG = nocGTFlow.at<Vec3s>(i,j)[1];
      channelR = nocGTFlow.at<Vec3s>(i,j)[2];

      if (channelB)
      {
        gtFlowFloat.at<Vec3f>(i,j)[0] = (channelR-32768.0f)/64.0f;
        gtFlowFloat.at<Vec3f>(i,j)[1] = (channelG-32768.0f)/64.0f;
        gtFlowFloat.at<Vec3f>(i,j)[2] = 1.0;
      }
      else
      {
        gtFlowFloat.at<Vec3f>(i,j)[0] = 0.0f;
        gtFlowFloat.at<Vec3f>(i,j)[1] = 0.0f;
        gtFlowFloat.at<Vec3f>(i,j)[2] = 0.0f;
      }
    }
  }

  return gtFlowFloat;
}

bool checkInsertVector(int pelY, int pelX, float mvy, float mvx, Vector<pair<int,int>>& pelLoc, vector<Vector<pair<float,float>>>& frameProj)
{
  for (int i=0; i<pelLoc.size(); i++)
  {
    int locY = pelLoc[i].first;
    int locX = pelLoc[i].second;

    if (pelY==locY && pelX==locX)
    {
      Vector<pair<float,float>>& tmp = frameProj[i];
      tmp.push_back(make_pair(mvy, mvx));

      return true;
    }
  }
  return false;
}

void tcBasedConfMeasure(string curName, string inDir, string outDir, Mat curFrame, Mat preFrame, float th)
{
  // TC Projection Based MV Confidence.

  // Original Picture Size.
  int inWidthOrg  = curFrame.cols;
  int inHeightOrg = curFrame.rows;

  // For Video Coding Picture Size. This is the MB=16x16 Level.
  int inWidth  = (inWidthOrg  + 15) >> 4;
  int inHeight = (inHeightOrg + 15) >> 4;

  string partName = curName;
  partName.erase(partName.length()-2,2);
  char widthS[20], heightS[20];
  itoa(inWidth*16,  widthS,  10);
  itoa(inHeight*16, heightS, 10);
  
  string mvTCForwardFileName  = inDir+"\\tcInData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv_F.bin";
  string mvTCForwardFileName1 = inDir+"\\tcInData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv_F_1.bin";

  float *mvTCForwardx=NULL,  *mvTCForwardy=NULL;
  float *mvTCForward_1x=NULL,*mvTCForward_1y=NULL;

  mvTCForwardx   = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvTCForwardy   = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvTCForward_1x = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvTCForward_1y = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));

  getMVNewVersion(mvTCForwardFileName.data(),  curFrame, mvTCForwardx,   mvTCForwardy  );
  getMVNewVersion(mvTCForwardFileName1.data(), curFrame, mvTCForward_1x, mvTCForward_1y);

  Mat fTCForward  (inHeight*4,  inWidth*4,  CV_32FC3);
  Mat fTCForward_1(inHeight*4,  inWidth*4,  CV_32FC3);

  for ( int i=0; i<inHeight*4; i++ )
  {
    for ( int j=0; j<inWidth*4; j++ )
    {
      fTCForward.at<Vec3f>(i,j)[0]   = mvTCForwardx[j+i*inWidth*4];
      fTCForward.at<Vec3f>(i,j)[1]   = mvTCForwardy[j+i*inWidth*4];
      fTCForward.at<Vec3f>(i,j)[2]   = 1;

      fTCForward_1.at<Vec3f>(i,j)[0] = mvTCForward_1x[j+i*inWidth*4];
      fTCForward_1.at<Vec3f>(i,j)[1] = mvTCForward_1y[j+i*inWidth*4];
      fTCForward_1.at<Vec3f>(i,j)[2] = 1;
    }
  }

  Mat fTCForwardMed;
  Mat fTCForward_1Med;
  Mat fTCForwardFull;
  Mat fTCForward_1Full;

  // Median Filter Very Efficient?
  medianBlur(fTCForward,   fTCForwardMed,   5);
  medianBlur(fTCForward_1, fTCForward_1Med, 5);

  resize(fTCForwardMed  , fTCForwardFull  , Size(fTCForward.cols*4  , fTCForward.rows*4  ));
  resize(fTCForward_1Med, fTCForward_1Full, Size(fTCForward_1.cols*4, fTCForward_1.rows*4));

  // dumpFlowImg(fTCForwardFull, "forward.png");
  // dumpFlowImg(fTCForward_1Full, "forward_1.png");

  Mat confMat(curFrame.rows, curFrame.cols, CV_8UC1);

  // Mat gtFlow = getGroundTruthMat(inDir, curName);
  // vector<pair<float, float>> tcProjGtPair;

  // TC Back Projection Method.
#if 0
  for (int i=0; i<curFrame.rows; i++)
  {
    for (int j=0; j<curFrame.cols; j++)
    {
      float mvx = fTCForwardFull.at<Vec3f>(i,j)[0];
      float mvy = fTCForwardFull.at<Vec3f>(i,j)[1];
      int  pelX = j-(int)mvx; // Back projection to get the object.
      int  pelY = i-(int)mvy; // Back projection to get the object.

      if (pelX>=0&&pelX<curFrame.cols&&pelY>=0&&pelY<curFrame.rows)
      {
        float mvx_1 = fTCForward_1Full.at<Vec3f>(pelY,pelX)[0];
        float mvy_1 = fTCForward_1Full.at<Vec3f>(pelY,pelX)[1];

        int regionTC_1 = getDirRegion(mvx_1, mvy_1);

        float tcProjEPE = abs(mvx-mvx_1)+abs(mvy-mvy_1);
        float gtmvx = gtFlow.at<Vec3f>(i,j)[0];
        float gtmvy = gtFlow.at<Vec3f>(i,j)[1];
        float gtEPE = abs(mvx-gtmvx)+abs(mvy-gtmvy);

        if (gtFlow.at<Vec3f>(i,j)[2])
        {
          tcProjGtPair.push_back(make_pair(tcProjEPE, gtEPE));
        }

        if (tcProjEPE <=th*4)
        {
          confMat.at<uchar>(i,j) = 0; // valid.
        }
        else
        {
          confMat.at<uchar>(i,j) = 255; // invalid
        }
      }
      else
      {
        confMat.at<uchar>(i,j) = 255; // invalid.
      }
    }
  }
#endif

  Vector<Vector<pair<float,float>>> frameProj(curFrame.rows*curFrame.cols, Vector<pair<float,float>>(0));

  for (int i=0; i<curFrame.rows; i++)
  {
    for (int j=0; j<curFrame.cols; j++)
    {
      float mvx_1 = fTCForward_1Full.at<Vec3f>(i,j)[0];
      float mvy_1 = fTCForward_1Full.at<Vec3f>(i,j)[1];

      int pelX = j + int(mvx_1+0.5); // round operation.
      int pelY = i + int(mvy_1+0.5); // round operation.

      if (pelX>=0&&pelX<curFrame.cols&&pelY>=0&&pelY<curFrame.rows)
      {
        frameProj[pelX+pelY*curFrame.cols].push_back(make_pair(mvy_1,mvx_1));
      }
    }
  }

  for (int i=0; i<curFrame.rows; i++)
  {
    for (int j=0; j<curFrame.cols; j++)
    {
      float mvx = fTCForwardFull.at<Vec3f>(i,j)[0];
      float mvy = fTCForwardFull.at<Vec3f>(i,j)[1];

      uchar curVal = curFrame.at<uchar>(i,j);

      if (frameProj[j+i*curFrame.cols].size() != 0)
      {
        int idx;
        int minError = 1000000;
        
        for (int m=0; m<frameProj[j+i*curFrame.cols].size(); m++)
        {
          float mvxProj = frameProj[j+i*curFrame.cols][m].second;
          float mvyProj = frameProj[j+i*curFrame.cols][m].first;

          int pelXProj = j - (int)(mvxProj+0.5);
          int pelYProj = i - (int)(mvyProj+0.5);

          uchar preVal = preFrame.at<uchar>(pelYProj, pelXProj);

          if (abs(preVal-curVal)<minError)
          {
            idx = m;
            minError = abs(preVal-curVal);
          }
        }

        float mvxProj = frameProj[j+i*curFrame.cols][idx].second;
        float mvyProj = frameProj[j+i*curFrame.cols][idx].first;
        
        double projAng = atan2(mvyProj, mvxProj);
        double curAng  = atan2(mvy, mvx);

        int error = (int)(abs(projAng-curAng)*180/3.1415926);
        error = error>180 ? 360-error : error;

        float EPE = abs(mvx-mvxProj)+abs(mvy-mvyProj);

        if (EPE <= th)
        {
          confMat.at<uchar>(i,j) = 0;
        }
        else
        {
          confMat.at<uchar>(i,j) = 255;
        }
      }
      else
      {
        confMat.at<uchar>(i,j) = 0;
      }
    }
  }

#if 0
  //ofstream file("out.txt");
  //for (int i=0; i<tcProjGtPair.size(); i++)
  //{
  //  file<<tcProjGtPair[i].first<<"\t"<<tcProjGtPair[i].second<<"\n";
  //}
  //
  //file.close();
#endif

  imwrite("1.png", confMat);
  
  // Output to File.
  string fileCombineOut = outDir+"\\confData\\"+curName+".data";
  const  char* confOutFileName = fileCombineOut.data();
  FILE*  fpConf=fopen(confOutFileName,"w");
  for (int i=0; i<confMat.rows; i++)
  {
    for (int j=0; j<confMat.cols; j++)
    {
      fprintf(fpConf, "%d\n", confMat.at<uchar>(i,j)/250 );
    }
  }
  fclose(fpConf);

  free(mvTCForwardx);
  free(mvTCForwardy);
  free(mvTCForward_1x);
  free(mvTCForward_1y);
}

void genTCFeature(string curName, string inDir, Mat curFrame)
{
  // TC Projection Based MV Confidence.

  // Original Picture Size.
  int inWidthOrg  = curFrame.cols;
  int inHeightOrg = curFrame.rows;

  // For Video Coding Picture Size. This is the MB=16x16 Level.
  int inWidth  = (inWidthOrg  + 15) >> 4;
  int inHeight = (inHeightOrg + 15) >> 4;

  string partName = curName;
  partName.erase(partName.length()-2,2);
  char widthS[20], heightS[20];
  itoa(inWidth*16,  widthS,  10);
  itoa(inHeight*16, heightS, 10);

  string mvTCForwardFileName  = inDir+"\\tcInData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv_F.bin";
  string mvTCForwardFileName1 = inDir+"\\tcInData\\data\\"+partName+widthS+"x"+heightS+"_out\\mv_F_1.bin";

  float *mvTCForwardx=NULL,  *mvTCForwardy=NULL;
  float *mvTCForward_1x=NULL,*mvTCForward_1y=NULL;

  mvTCForwardx   = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvTCForwardy   = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvTCForward_1x = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));
  mvTCForward_1y = (float *)malloc(inWidth*4*inHeight*4*sizeof(float));

  getMVNewVersion(mvTCForwardFileName.data(),  curFrame, mvTCForwardx,   mvTCForwardy  );
  getMVNewVersion(mvTCForwardFileName1.data(), curFrame, mvTCForward_1x, mvTCForward_1y);

  Mat fTCForward  (inHeight*4,  inWidth*4,  CV_32FC3);
  Mat fTCForward_1(inHeight*4,  inWidth*4,  CV_32FC3);

  for ( int i=0; i<inHeight*4; i++ )
  {
    for ( int j=0; j<inWidth*4; j++ )
    {
      fTCForward.at<Vec3f>(i,j)[0]   = mvTCForwardx[j+i*inWidth*4];
      fTCForward.at<Vec3f>(i,j)[1]   = mvTCForwardy[j+i*inWidth*4];
      fTCForward.at<Vec3f>(i,j)[2]   = 1;

      fTCForward_1.at<Vec3f>(i,j)[0] = mvTCForward_1x[j+i*inWidth*4];
      fTCForward_1.at<Vec3f>(i,j)[1] = mvTCForward_1y[j+i*inWidth*4];
      fTCForward_1.at<Vec3f>(i,j)[2] = 1;
    }
  }

  Mat fTCForwardMed;
  Mat fTCForward_1Med;
  Mat fTCForwardFull;
  Mat fTCForward_1Full;

  // Median Filter Very Efficient?
  medianBlur(fTCForward,   fTCForwardMed,   5);
  medianBlur(fTCForward_1, fTCForward_1Med, 5);

  resize(fTCForwardMed  , fTCForwardFull  , Size(fTCForward.cols*4  , fTCForward.rows*4  ));
  resize(fTCForward_1Med, fTCForward_1Full, Size(fTCForward_1.cols*4, fTCForward_1.rows*4));

  float* mvMag = NULL;
  mvMag = (float*)malloc(curFrame.cols*curFrame.rows*sizeof(float));

  for (int i=0; i<curFrame.rows; i++)
  {
    for (int j=0; j<curFrame.cols; j++)
    {
      // TC Check.
      float TCforwardMVx = fTCForwardFull.at<Vec3f>(i,j)[0];
      float TCforwardMVy = fTCForwardFull.at<Vec3f>(i,j)[1];

      float TCforward_1MVx = fTCForward_1Full.at<Vec3f>(i,j)[0];
      float TCforward_1MVy = fTCForward_1Full.at<Vec3f>(i,j)[1];

      float absMVx = abs(TCforwardMVx-TCforward_1MVx);
      float absMVy = abs(TCforwardMVy-TCforward_1MVy);

      mvMag[j+i*curFrame.cols]= absMVx+absMVy;
    }
  }

  // Write Features Into File.
  FILE *fp;
  fp=fopen("FV_mvMag.dat", "wb");
  fwrite(mvMag, sizeof(float), curFrame.cols*curFrame.rows, fp);
  fclose(fp);

  free(mvMag);
  free(mvTCForwardx);
  free(mvTCForwardy);
  free(mvTCForward_1x);
  free(mvTCForward_1y);
}