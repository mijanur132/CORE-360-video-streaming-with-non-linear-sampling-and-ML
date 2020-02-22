#include <jni.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <ostream>
#include <string_view>
#include <locale>
#include <android/log.h>
#include"ERI.h"
#include "image.h"
#include "path.h"
#include "../../../../../../../../opencv/build/include/opencv2/videoio.hpp"
#include <vector>

using namespace std;
using namespace cv;

typedef unsigned char byte;
vector<vector <Mat>> loadedFrameVec;
int framloaded;



extern "C"
{
JNIEXPORT void JNICALL Java_com_example_core_MainActivity_image(JNIEnv *env, jobject instance, jlong addr) {

    Mat* pMatGr=(Mat*)addr;
    Mat convImage= imread("http://10.0.2.2:80/1.jpg");
    int a=convImage.empty();
    __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "Image loaded??....................................%d>> ", a);
    if (convImage.empty()) {
        *pMatGr = Mat::zeros(200, 400, CV_8UC3);
    }
    else{
        *pMatGr=convImage;
    }
}
}

extern "C"
{
JNIEXPORT jint JNICALL Java_com_example_core_MainActivity_loadVideoFromDevice(JNIEnv *env,
                                                                             jobject instance,
                                                                             jlong addr,
                                                                             jstring videoPath,
                                                                             jint chunkN) {
    __android_log_print(ANDROID_LOG_VERBOSE,"MyApp", "came here()");

    Mat *pMatGr = (Mat *) addr;
    jboolean iscopy;
    const char *vpath = (env)->GetStringUTFChars(videoPath, &iscopy);
    VideoCapture cap1(vpath);
    if (!cap1.isOpened()) {
        __android_log_print(ANDROID_LOG_VERBOSE,"MyApp", "emptyframe.................");
        return 0;
    }
    for (int fi = 0; fi < 120; fi++)
    {
        Mat frame;
        cap1 >> frame;
        __android_log_print(ANDROID_LOG_VERBOSE,"MyApp","%d, =%d", fi,chunkN);
        if (frame.empty()) {
            __android_log_print(ANDROID_LOG_VERBOSE,"MyApp", "emptyframe.................");
            frame = Mat::zeros(100 + fi, 400 + fi, CV_8UC3);//dummy frame
        } else {
        }
        int chunkno = (int) chunkN;
        //__android_log_print(ANDROID_LOG_VERBOSE,"MyApp", "fi=%d, CN=%d>> ", fi,chunkno);
        if (fi==0)
        {loadedFrameVec[chunkno][0]=frame;}
        else{loadedFrameVec[chunkno].push_back(frame);}

        frame.release();
    }
    __android_log_print(ANDROID_LOG_VERBOSE,"MyApp"," loaded..........................=%d,=%d", chunkN, loadedFrameVec[chunkN].size());
    return 1;
}
}

extern "C"
{
JNIEXPORT void JNICALL Java_com_example_core_MainActivity_CoREoperationPerFrame(JNIEnv *env, jobject instance, jlong addr, jint fi, jint chunkN, jint cameraPan, jint baseAngle) {

    Mat* pMatGr=(Mat*)addr;

    while(fi>=loadedFrameVec[chunkN].size() && loadedFrameVec[chunkN].size()<80)
   {
     //  __android_log_print(ANDROID_LOG_VERBOSE,"MyApp", "looping fi=%d, size=%d, chunkN=%d>> ", fi,loadedFrameVec[chunkN].size(), chunkN);
   }
    __android_log_print(ANDROID_LOG_VERBOSE,"MyApp", "..................................................................................looping fi=%d, size=%d, chunkN=%d, pan=%d>> ", fi,loadedFrameVec[chunkN].size(), chunkN, cameraPan);
    *pMatGr=loadedFrameVec[chunkN][fi].clone();
    CoRE_operation_per_frame(*pMatGr, cameraPan, baseAngle); //xxxOpt: pass fi as an input parameter instead of image vec[fi], use pMatGr as output parameter
    Mat m;
    loadedFrameVec[chunkN][fi]=m;
    return;
}
}

extern "C"
{
JNIEXPORT void JNICALL Java_com_example_core_MainActivity_initCoREparameters(JNIEnv *env, jobject instance) {
    Mat mat=Mat::zeros(900, 800, CV_8UC3);//dummy frame;
    for (int i = 0; i <30 ; ++i) {
        vector<Mat> temp;
        temp.push_back(mat);
        loadedFrameVec.push_back(temp);
    }

    ERI eri(3840,2048);
    Path path1;
    path1.updateReriCs(0);
    eri.atanvalue();
    eri.xz2LonMap();
    eri.xz2LatMap();
    path1.nonUniformListInit();
    path1.mapx();
    return;
}
}









