#include <iostream>
#include <stdio.h>
#include <math.h>

// #include "mail.h"
#include "io_flow.h"
#include "utils.h"
#include "confidenceOF.h"

using namespace std;

vector<float> flowErrorsOutlier (FlowImage &F_gt,FlowImage &F_orig,FlowImage &F_ipol) {

  // check file size
  if (F_gt.width()!=F_orig.width() || F_gt.height()!=F_orig.height()) {
    cout << "ERROR: Wrong file size!" << endl;
    throw 1;
  }

  // extract width and height
  int32_t width  = F_gt.width();
  int32_t height = F_gt.height();

  // init errors
  vector<float> errors;
  for (int32_t i=0; i<2*5; i++)
    errors.push_back(0);
  int32_t num_pixels = 0;
  int32_t num_pixels_result = 0;

  // for all pixels do
  for (int32_t u=0; u<width; u++) {
    for (int32_t v=0; v<height; v++) {
      float fu = F_gt.getFlowU(u,v)-F_ipol.getFlowU(u,v);
      float fv = F_gt.getFlowV(u,v)-F_ipol.getFlowV(u,v);
      float f_err = sqrt(fu*fu+fv*fv);
      if (F_gt.isValid(u,v)) {
        for (int32_t i=0; i<5; i++)
          if (f_err>(float)(i+1))
            errors[i*2+0]++;
        num_pixels++;
        if (F_orig.isValid(u,v)) {
          for (int32_t i=0; i<5; i++)
            if (f_err>(float)(i+1))
              errors[i*2+1]++;
          num_pixels_result++;
        }
      }
    }
  }

  // check number of pixels
  if (num_pixels==0) {
    cout << "ERROR: Ground truth defect => Please write me an email!" << endl;
    throw 1;
  }

  // normalize errors
  for (int32_t i=0; i<errors.size(); i+=2)
    errors[i] /= max((float)num_pixels,1.0f);
  if (num_pixels_result>0)
    for (int32_t i=1; i<errors.size(); i+=2)
      errors[i] /= max((float)num_pixels_result,1.0f);

  // push back density
  errors.push_back((float)num_pixels_result/max((float)num_pixels,1.0f));

  // return errors
  return errors;
}

vector<float> flowErrorsAverage (FlowImage &F_gt,FlowImage &F_orig,FlowImage &F_ipol) {

  // check file size
  if (F_gt.width()!=F_orig.width() || F_gt.height()!=F_orig.height()) {
    cout << "ERROR: Wrong file size!" << endl;
    throw 1;
  }

  // extract width and height
  int32_t width  = F_gt.width();
  int32_t height = F_gt.height();

  // init errors
  vector<float> errors;
  for (int32_t i=0; i<2; i++)
    errors.push_back(0);
  int32_t num_pixels = 0;
  int32_t num_pixels_result = 0;

  // for all pixels do
  for (int32_t u=0; u<width; u++) {
    for (int32_t v=0; v<height; v++) {
      float fu = F_gt.getFlowU(u,v)-F_ipol.getFlowU(u,v);
      float fv = F_gt.getFlowV(u,v)-F_ipol.getFlowV(u,v);
      float f_err = sqrt(fu*fu+fv*fv);
      if (F_gt.isValid(u,v)) {
        errors[0] += f_err;
        num_pixels++;
        if (F_orig.isValid(u,v)) {
          errors[1] += f_err;
          num_pixels_result++;
        }
      }
    }
  }

  // normalize errors
  errors[0] /= max((float)num_pixels,1.0f);
  errors[1] /= max((float)num_pixels_result,1.0f);

  // return errors
  return errors;
}

void plotVectorField (FlowImage &F_gt_noc, FlowImage &F_gt_occ, FlowImage &F,string dir,const char* prefix) {

  // command for execution
  char command[1024];
  
  // write flow field to file
  FILE *fp = fopen((dir + prefix + ".txt").c_str(),"w");
  for (int32_t u=5; u<F.width()-5; u+=15) {
    for (int32_t v=5; v<F.height()-5; v+=15) {
        if (F_gt_noc.isValid(u,v)) {
            fprintf(fp,"%d %d %f %f 0xFF0000\n",u,v,F_gt_noc.getFlowU(u,v),F_gt_noc.getFlowV(u,v));
        }
        if (!F_gt_occ.isValid(u,v)){//MVs of top half picture
            fprintf(fp,"%d %d %f %f 0x00FF00\n",u,v,F.getFlowU(u,v),F.getFlowV(u,v));
        } else if (F_gt_noc.isValid(u,v)){//mask out occluded area
            fprintf(fp,"%d %d %f %f 0x00FF00\n",u,v,F.getFlowU(u,v),F.getFlowV(u,v));
        }else{
            //printf("MASK: %d, %d \n",u,v);
        }
    }
  }
  fclose(fp);
  
  // create png + eps
  for (int32_t j=0; j<1; j++) {

    // open file  
    FILE *fp = fopen((dir + prefix + ".gp").c_str(),"w");

    // save gnuplot instructions
    if (j==0) {
      fprintf(fp,"set term png size %d,%d truecolor font \"Helvetica\" 11\n",F.width(),F.height());
      fprintf(fp,"set output \"%s.png\"\n",prefix);
    } else {
      fprintf(fp,"set term postscript eps enhanced color\n");
      fprintf(fp,"set output \"%s.eps\"\n",prefix);
      fprintf(fp,"set size ratio -1\n");
    }

    // plot options (no borders)    
    fprintf(fp,"set lmargin 0\n");
    fprintf(fp,"set bmargin 0\n");
    fprintf(fp,"set rmargin 0\n");
    fprintf(fp,"set tmargin 0\n");
    fprintf(fp,"set noxtic\n");
    fprintf(fp,"set noytic\n");
    fprintf(fp,"set nokey\n");
    fprintf(fp,"set clip one\n");
    fprintf(fp,"set multiplot \n");
    
    // plot original picture
    fprintf(fp,"plot \"../image/%s.png\" binary filetype=png w rgbimage\n",prefix);

    fprintf(fp,"set yrange [%d:0] reverse\n",F.height()-1);
    fprintf(fp,"set xrange [0:%d]\n",F.width()-1);
    fprintf(fp,"unset autoscale\n");

    // plot error curve
    fprintf(fp,"plot \"%s.txt\" using 1:2:3:4:5 w vector lc rgbcolor variable\n",prefix);
    
    fprintf(fp,"unset multiplot");
    // close file
    fclose(fp);
    
    // run gnuplot => create png + eps
    sprintf(command,"cd %s;  /home/scratch.gpu_ip/video/sequence/kitti/tools/gnuplot %s",dir.c_str(),((string)prefix + ".gp").c_str());
    system(command);
    
    // resize png to original size, using aliasing
    //if (j==0) {
    //  sprintf(command,"mogrify -resize %dx%d %s",F.width(),F.height(),(dir + prefix + ".png").c_str());
    //  system(command);
    //}
  }
  
  // create pdf and crop
  //sprintf(command,"cd %s; ps2pdf %s.eps %s_large.pdf",dir.c_str(),prefix,prefix);
  //system(command);
  //sprintf(command,"cd %s; pdfcrop %s_large.pdf %s.pdf",dir.c_str(),prefix,prefix);
  //system(command);
  //sprintf(command,"cd %s; rm %s_large.pdf",dir.c_str(),prefix);
  //system(command);
}

bool eval (string prefix, string result_dir, string gt_noc_dir, string gt_occ_dir) {

  // vector for storing the errors
  vector< vector<float> > errors_noc_out;
  vector< vector<float> > errors_occ_out;
  vector< vector<float> > errors_noc_avg;
  vector< vector<float> > errors_occ_avg;

    
  // output
  // mail->msg("Processing: %s.png",prefix.c_str());

  // load ground truth flow maps
  FlowImage F_gt_noc(gt_noc_dir + "/" + prefix + ".png");
  FlowImage F_gt_occ(gt_occ_dir + "/" + prefix + ".png");

  // check submitted result
  string image_file = result_dir + "/data/" + prefix + ".png";
  if (!imageFormat(image_file,png::color_type_rgb,16,F_gt_noc.width(),F_gt_noc.height())) {
          // mail->msg("ERROR: Input must be png, 3 channels, 16 bit, %d x %d px",
          //                F_gt_noc.width(),F_gt_noc.height());
          return false;        
  }

  // load submitted result
  FlowImage F_orig(image_file);

  // interpolate missing values
  FlowImage F_ipol(F_orig); 
  //F_ipol.interpolateBackground();     

  // add flow errors
  vector<float> errors_noc_out_curr = flowErrorsOutlier(F_gt_noc,F_orig,F_ipol);
  vector<float> errors_occ_out_curr = flowErrorsOutlier(F_gt_occ,F_orig,F_ipol);
  vector<float> errors_noc_avg_curr = flowErrorsAverage(F_gt_noc,F_orig,F_ipol);
  vector<float> errors_occ_avg_curr = flowErrorsAverage(F_gt_occ,F_orig,F_ipol);
  errors_noc_out.push_back(errors_noc_out_curr);
  errors_occ_out.push_back(errors_occ_out_curr);
  errors_noc_avg.push_back(errors_noc_avg_curr);
  errors_occ_avg.push_back(errors_occ_avg_curr);


  // save errors of error images to text file
  FILE *errors_noc_out_file = fopen((result_dir + "/errors_noc_out/" + prefix + ".txt").c_str(),"w");
  FILE *errors_occ_out_file = fopen((result_dir + "/errors_occ_out/" + prefix + ".txt").c_str(),"w");
  FILE *errors_noc_avg_file = fopen((result_dir + "/errors_noc_avg/" + prefix + ".txt").c_str(),"w");
  FILE *errors_occ_avg_file = fopen((result_dir + "/errors_occ_avg/" + prefix + ".txt").c_str(),"w");
  if (errors_noc_out_file==NULL || errors_occ_out_file==NULL ||
                  errors_noc_avg_file==NULL || errors_occ_avg_file==NULL) {
          // mail->msg("ERROR: Couldn't generate/store output statistics!");
          return false;
  }
  for (int32_t j=0; j<errors_noc_out_curr.size(); j++) {
          fprintf(errors_noc_out_file,"%f ",errors_noc_out_curr[j]);
          fprintf(errors_occ_out_file,"%f ",errors_occ_out_curr[j]);
  }
  for (int32_t j=0; j<errors_noc_avg_curr.size(); j++) {
          fprintf(errors_noc_avg_file,"%f ",errors_noc_avg_curr[j]);
          fprintf(errors_occ_avg_file,"%f ",errors_occ_avg_curr[j]);
  }
  fprintf(errors_noc_out_file,"\n");
  fprintf(errors_occ_out_file,"\n");
  fprintf(errors_noc_avg_file,"\n");
  fprintf(errors_occ_avg_file,"\n");
  fclose(errors_noc_out_file);
  fclose(errors_occ_out_file);
  fclose(errors_noc_avg_file);
  fclose(errors_occ_avg_file);

  // save error images
  png::image<png::rgb_pixel> F_err = F_ipol.errorImage(F_gt_noc,F_gt_occ);
  F_err.write(result_dir + "/errors_img/" + prefix + ".png");

  // find maximum ground truth flow
  float max_flow = F_gt_occ.maxFlow();

  // save original flow image
  F_orig.writeColor(result_dir + "/flow_orig/" + prefix + ".png",max_flow);

  // save original flow image
  F_gt_noc.writeColor(result_dir + "/flow_gt_noc/gt_noc_" + prefix + ".png",max_flow);
  F_gt_occ.writeColor(result_dir + "/flow_gt_occ/gt_occ_" + prefix + ".png",max_flow);
  
  // save interpolated flow image
  F_ipol.writeColor(result_dir + "/flow_ipol/" + prefix + ".png",max_flow);

  // save interpolated flow vector field
  // plotVectorField(F_gt_noc, F_gt_occ, F_ipol,result_dir + "/flow_field/",prefix.c_str());

}

//int32_t main (int32_t argc,char *argv[]) {
//
//  // we need 6 arguments
//  if (argc!=5) {
//    cout << "Usage: ./eval_flow prefix result_dir gt_noc_dir gt_occ_dir" << endl;
//    return 1;
//  }
//
//  // read arguments
//  string prefix = string(argv[1])+"_10";
//  string  result_dir = argv[2];
//  string  gt_noc_dir = argv[3];
//  string  gt_occ_dir = argv[4];
//  
//  // init notification mail
//  Mail *mail;
//  if (argc==4) mail = new Mail(argv[3]);
//  else         mail = new Mail();
//  mail->msg("Thank you for participating in our evaluation!");
//
//  // run evaluation
//  eval(mail,prefix, result_dir, gt_noc_dir, gt_occ_dir);
//
//  // send mail and exit
//  delete mail;
//  return 0;
//}

