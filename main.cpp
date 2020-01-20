#include<iostream>
#include <conio.h>
#include "ERI.h"
#include "v3.h"
#include "image.h"
#include "ppc.h"
#include "config.h"

#include <chrono> 
#include <cstdlib>
#include "path.h"

using namespace std;
using namespace std::chrono;

#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
using namespace cv;


int main(int argc, char* argv[])
{
	//playstillmanually();
	//testforwardbackward();
	//testPlayBackManualPathStillImage();
	//testPlayBackHMDPathStillImage(); //playone still image 
	//testPlayBackHMDPathVideoPixelInterval();   //play only one frame of the video
	//testTiling();
	//testViewDirectionAvgRotation();
	//Writeh264VideoUptoNframes();
	//
	//STOP;
	
	//testBoundingBox();

	//getcheckerboard();

	//getssim();
	//testRotationxy();

	//testbilinear();
	//testEncodingDecoding();
	//STOP;
	//testrotationxyframe();
	//out_video_file();
	//testvideoendecodenew();
	//testvideoEncodeNew4s(4);
	//testPlayBackHMDPathVideo();
	//temp_calc();
	//testWriteh264tiles();
	//STOP;
	//makeVideo4thSecVar(10, 0); //reqtime=chunkN, avgSize=initial skip
	/*
	for (int i = 1; i < 5; i++)
	{
		makeVideoRegularSpecificPanTiltChunkN("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/rhino.webm", 10, 0, i, 100); //reqtime=chunkN, avgSize=initial skip //0 for 1s, 10 for 4s
	}
	
	STOP;
		*/																															   //makeVideoRegularSpecificPanTiltChunkN("./Video/source/diving.avi", 10, 0, 1, 0); //reqtime=chunkN, avgSize=initial skip
	//GenerateEncoding4AllDirection();
	//testvideodecodeNqual();
	//testDownloadVideoHttp(1,1);  //same conditions for the parameters as with the 4th sec var parameters

	//createSmallTilesFromLongTiles("./Video/source/tileS/diving.avi_2_8.avi");

	//testSeperateCoREinto4tiles();
	//testCoREtileDecodeTimeSeperateVsOnAsingleFrame();

	//testWriteh264tiles();
	
	/*

	makeVideoRegularSpecificPanTiltChunkN("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/diving.avi", 10, 0, 4, 400); //reqtime=chunkN, avgSize=initial skip
	makeVideoRegularSpecificPanTiltChunkN("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/diving.avi", 10, 0, 5, 400); //reqtime=chunkN, avgSize=initial skip
	makeVideoRegularSpecificPanTiltChunkN("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/diving.avi", 10, 0, 6, 400); //reqtime=chunkN, avgSize=initial skip
	makeVideoRegularSpecificPanTiltChunkN("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/diving.avi", 10, 0, 7, 400); //reqtime=chunkN, avgSize=initial skip
	makeVideoRegularSpecificPanTiltChunkN("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/diving.avi", 10, 0, 8, 400); //reqtime=chunkN, avgSize=initial skip
	makeVideoRegularSpecificPanTiltChunkN("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/diving.avi", 10, 0, 9, 400); //reqtime=chunkN, avgSize=initial skip
	
	*/


	//testCombineMultipleTileIntoSingleFrame();
	//testNtileDecodeTimeSeperateVsOnAsingleFrame();

	//testBugInSR(1, 1, "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving", "./bwLogs/bw1962kB.txt", "./Video/source/diving.txt", 2, 3);

	
	//GenerateEncodingRegularSpecificPanTiltChunk();
	//GenerateEncoding4sVarSpecificPanTiltChunk(10,10,1);																		//dl//extraSec
	
	//testSrAccrossSpecificFrame("http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving", "./bwLogs/bw1962kB.txt", "C:/inetpub/wwwroot/3vid2crf3trace/hmdTraceFinal/stableNdynamicTraces/divingS.txt", 2, 10, 1500);
	//testDownloadVideoHttpTileQualChngLineMeasurementFoVPlus("http://127.0.0.5:80/3vid2crf3trace/Tiles/crf30/1s/roller.AVI", "C:/inetpub/wwwroot/3vid2crf3trace/bw1962kB.txt", "C:/inetpub/wwwroot/3vid2crf3trace/hmdTraceFinal/stableNdynamicTraces/rollerD.txt", 1, 4, 6);

	//testDownloadVideoHttp4thSecVarAndroid("http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving", "C:/inetpub/wwwroot/3vid2crf3trace/bw1962kB.txt", "C:/inetpub/wwwroot/3vid2crf3trace/android.txt", 2, 6,1500);

	//testDownloadVideoHttp4thSecVar("http://127.0.0.5:80/3vid2crf3trace/4s6s/mobisys/crf30/diving/30_diving.AVI6", "C:/inetpub/wwwroot/3vid2crf3trace/bw646kB.txt", "C:/inetpub/wwwroot/3vid2crf3trace/hmdTraceFinal/stableNdynamicTraces/divingD.txt", 0, 6,1500);  //nextDlChunkSec 0 means variable, else value corresponds request time.

	//testDownloadVideoHttpTileDelayMeasurementFoVPlus("http://127.0.0.5:80/3vid2crf3trace/Tiles/crf30/1s/roller.AVI", "./bwLogs/bw646kB.txt", "C:/inetpub/wwwroot/3vid2crf3trace/hmdTraceFinal/stableNdynamicTraces/rollerD.txt", 1, 4, 6);
	//STOP;
	//Tiles:FoV
	//testCPUtimeTiles("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/diving.avi");
	//testDownloadVideoHttpTileDelayMeasurementFoVMobisysBoth("http://127.0.0.5:80/3vid2crf3trace/Tiles/crf30/1s/roller.AVI", "C:/inetpub/wwwroot/3vid2crf3trace/bw1962kB.txt", "C:/inetpub/wwwroot/3vid2crf3trace/hmdTraceFinal/stableNdynamicTraces/divingS.txt", 1, 4, 6,0,100);

	
	GenerateEncoding4AllDirection();
	
	STOP;


	
	string videoFile(argv[1]);
	const char* videoF = videoFile.c_str();
	string bwFile(argv[2]);
	const char* bwF = bwFile.c_str();
	string dvFile(argv[3]);
	const char* dvF = dvFile.c_str();
	string argv4(argv[4]);
	int functionSelect = stoi(argv4);
	string argv5(argv[5]);
	int reqTime = stoi(argv5);

	string argv6(argv[6]);
	int avgSize = stoi(argv6);

	

	if (functionSelect==1)
	{
		testDownloadVideoHttpTileBlankPixelMeasurementFoV((char*)videoF, (char*)bwF, (char*)dvF, 1, 4, 6);
	}
		
	else if (functionSelect == 2)
	{
		testDownloadVideoHttpTileBlankPixelMeasurementFoVPlus((char*)videoF, (char*)bwF, (char*)dvF, 1, 4, 6);
	}
	else if (functionSelect == 3)
	{
		testDownloadVideoHttpTileQualChngLineMeasurementFoVPlus((char*)videoF, (char*)bwF, (char*)dvF, 1, 4, 6);
	}

	else if (functionSelect == 4)
	{
		testDownloadVideoHttpTileQualChngLineMeasurementFoVPlus((char*)videoF, (char*)bwF, (char*)dvF, 1, 4, 6);
	}

	else if (functionSelect == 5)
	{
		testDownloadVideoHttpTileDelayMeasurementFoV((char*)videoF, (char*)bwF, (char*)dvF, 1, 4, 6);
	}

	else if (functionSelect == 6)  //final
	{
		testDownloadVideoHttpTileDelayMeasurementFoVPlus((char*)videoF, (char*)bwF, (char*)dvF, 1, 4, 6);
	}
	else if (functionSelect == 7)  //final
	{
		//testDownloadVideoHttpTileDelayMeasurementFoVOnly((char*)videoF, (char*)bwF, (char*)dvF, 1, 4, 6);
	}	
	else if (functionSelect == 11) 
	{		
		//testDownloadVideoHttp4thSecVarDelayFRmeasurement((char*)videoF, (char*)bwF, (char*)dvF, reqTime, 3,100);
	}
	else if (functionSelect == 12) //final
	{
		testDownloadVideoHttp4thSecVar((char*)videoF, (char*)bwF, (char*)dvF, reqTime, 6,avgSize);
	}
	else if (functionSelect == 13) {	
		cout << "this one selected" << endl;
		
		makeVideoRegularSpecificPanTiltChunkN((char*)videoF, -180,10, 1, avgSize); //reqtime=chunkN, avgSize=initial skip
	}


	else { cout << "No option selected...." << endl; }
	
	
		//testDownloadVideoHttpTileMeasurement((char*)videoF, (char*)bwF, (char*)dvF, 1, 4, 6);
										
	//testDownloadVideoHttpTile("http://127.0.0.5:80/3vid2crf3trace/tiles/diving.avi", "C:/inetpub/wwwroot/3vid2crf3trace/bwLogs/bw1962kB.txt", "C:/inetpub/wwwroot/3vid2crf3trace/diving.txt",4,4, 6);
	
	//testCPUtime("C:/inetpub/wwwroot/3vid2crf3trace/4s3s/crf18/diving_1_10_-180.avi");
	//testCPUtimeTiles("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/diving.avi");
	
	
	//run10simulations();   
		
	return 0;
}


