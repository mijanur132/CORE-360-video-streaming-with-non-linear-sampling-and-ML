//#include"curl/curl.h"
#include"image.h"
#include<fstream>
#include<conio.h>
#include<string.h>
#include "config.h"
#include "ssim.h"
#include <math.h>
#include <future>
#include <mutex>
#include <thread>
#include <queue>
#include<iostream>
#include <filesystem>

#include <C:\opencv\build\include\opencv2\opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>

using namespace cv;
using namespace std;
namespace fs = std::experimental::filesystem;


std::string filename;
int Is_MInv_calculated;
M33 M_Inv;
vector<vector <Mat>> frameQvec;
vector<vector<vector <Mat>>>frameQvecTiles;
int ChngReriCS = 0;

//float samplingRateFrame[2000][2000];

int NextChunkDownloaded=0;
int decodeCount=0;
void makeVideo4thSecVarSpecificPanTiltChunkN(char* fileName, float pan, float tilt, int ChnkN, int extraFrame, int extraSec);
void makeVideo4thSecVarSpecificPanTiltChunkNvariableLastSection(char* fileName, int extraSec, float pan, float tilt, int chunkN, int extraFrame, int howManySecondsfor4thSec);
void makeVideoRegularSpecificPanTiltChunkN(string fName, float pan, float tilt, int chunkN,int beginFrame);
void videowriterhelperxWithFileNameWithExtraSec(char* filename, int extraSec, int chunkN, float pan, float tilt, int fps, int cols, int rows, int starting_frame, int ending_frame, vector<Mat> file2wr);
void makeVideoRegularSpecificPanTiltChunkNAllSameFrame(string fileName, float pan, float tilt, int chunkN, int beginFrame);
auto startDisplaying = std::chrono::high_resolution_clock::now();

void displayImage(Mat ret1)
{
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - startDisplaying;
	cout << "Time=" << elapsed.count()*1000 << endl;
	startDisplaying = finish;
	namedWindow("PALASH CoRE Player", WINDOW_NORMAL);
	resizeWindow("PALASH CoRE Player", 800, 600);
	imshow("PALASH CoRE Player", ret1);
	waitKey(1);
	

}

void displayImage33ms()
{
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - startDisplaying;
	while (elapsed.count()*1000 < 33)
	{
		finish = std::chrono::high_resolution_clock::now();
		elapsed = finish - startDisplaying; 
		//cout << "While we are wasting time...........................wait--->" << endl;
	}
	//cout << "imTime=" << elapsed.count()*1000 << endl;
	startDisplaying = std::chrono::high_resolution_clock::now();
}

//................................... http.................................//
string datax;
/*
size_t download(char* buf, size_t size, size_t nmemb, void* userP)
{	//
	//variable data to be saved
	//size*nmemb is the size of the buf(fer)
	for (int c = 0; c < size*nmemb; c++)
	{
		datax.push_back(buf[c]);
	}
	return size * nmemb; //return the number of bytes we handled
}
string fetchTextOverHttp(char* addr)
{	//		//variable data to be saved
	curl_global_init(CURL_GLOBAL_ALL); //pretty simple
	CURL* conHandle = curl_easy_init(); //make an "easy" handle
		curl_easy_setopt(conHandle, CURLOPT_URL, addr);
	curl_easy_setopt(conHandle, CURLOPT_VERBOSE, 1L); //outputs status
	curl_easy_setopt(conHandle, CURLOPT_WRITEFUNCTION, &download); //set our callback to handle data
	curl_easy_perform(conHandle); //get the file
	return datax;
	//cout << datax << endl; //should output an html file (if the set url is valid)
}
//................................... http End.................................//
*/
class queueclass {
	std::queue<Mat> Fqueue;
public:
	void pushframe(Mat newframe)
	{
		std::lock_guard<std::mutex> lock(mutex);
		Fqueue.push(newframe);

	}
	Mat  getframe()
	{
		std::lock_guard<std::mutex> lock(mutex);
		Mat outFrame = Fqueue.front();
		Fqueue.pop();
		return outFrame;
	}
	bool empty()
	{
		if (Fqueue.empty())
			return 1;
		else return 0;
	}
private:
	std::mutex mutex;

}frameQ;


void DownLoadChunk4thSecVar(string filename, int chunkD, int fps, int extraFrame, int timeNeededms, int chunkN)
{	
	
	auto start = std::chrono::high_resolution_clock::now();
	Mat frame;
	
 	VideoCapture cap1(filename);

	if (!cap1.isOpened())
	{
		cout << "**********************************" << endl;
		cout << "Cannot open the video file: " << filename << endl;
		waitKey(100);
		STOP;
	}
		
	for (int j = 0; j < fps*chunkD+extraFrame; j++)
	{
		//cout << j <<" "<<chunkN<< endl;
		cap1 >> frame;
		
		if (!frame.empty())
		{
			//displayImage(frame);
			frameQvec[chunkN][j]=frame.clone();
		}
		else
		{
			cout << "Cannot open, the video file has no frame: " <<j<< endl;
			break;
		}
	}
	//cap1.release();
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedms = (finish - start)*1000;
	while (elapsedms.count() < timeNeededms)
	{
		finish = std::chrono::high_resolution_clock::now();
		elapsedms = (finish - start)*1000;
		//cout << "Time inside.......................=" << elapsedms.count() << endl;
	}
	NextChunkDownloaded = NextChunkDownloaded+ 1;
	decodeCount = 0;
	cout <<" dl= "<< chunkN <<" "<< frameQvec[chunkN].size()<< endl;
	cout << " Downloaded=" << filename << " NextChunkDownloaded= "<< NextChunkDownloaded<<endl;
	cout << "Time forcasted and needed to load= " << timeNeededms << " " << elapsedms.count() << endl;
	ChngReriCS = 1;
	return;

}

void DownLoadTilesChunk(string filename, int chunkN, int chunkD, int tileN, int fps, int timeNeededms)
{
	auto start = std::chrono::high_resolution_clock::now();
	Mat frame;

	VideoCapture cap1(filename);

	if (!cap1.isOpened())
	{
		cout << "**********************************" << endl;
		cout << "Cannot open the video file: " << filename << endl;
		waitKey(100);
		STOP;
	}

	for (int j = 0; j < fps*chunkD; j++)
	{

		cap1 >> frame;

		if (!frame.empty())
		{
			frameQvecTiles[tileN][chunkN][j] = frame.clone();
		}
		else
		{
			cout << "Cannot open, the video file has no frame: " << j << endl;
			break;
		}
	}
	//cap1.release();
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedms = (finish - start) * 1000;
	while (elapsedms.count() < timeNeededms)
	{
		finish = std::chrono::high_resolution_clock::now();
		elapsedms = (finish - start) * 1000;
		//cout << "Time inside.......................=" << elapsedms.count() << endl;
	}
	NextChunkDownloaded = NextChunkDownloaded + 1;
	decodeCount = 0;
	//cout << " dl= " << chunkN << " " << frameQvec[tileN][chunkN].size() << endl;
	cout << " Downloaded=" << filename << " NextChunkDownloaded= " << NextChunkDownloaded << endl;
	cout << "Time forcasted and needed to load= " << timeNeededms << " " << elapsedms.count() << endl;
	return;

}

void DownLoadChunk(string filename, int chunkD, int fps, int timeNeededms)
{	
	
	auto start = std::chrono::high_resolution_clock::now();
	Mat frame;
	
	VideoCapture cap1(filename);
	if (!cap1.isOpened())
	{
		cout << "Cannot open the video file: "<<filename << endl;
		STOP;
	}

	for (int j = 1; j <= chunkD * fps; j++)
	{

		cap1 >> frame;
		fps = cap1.get(CAP_PROP_FPS);
		if (!frame.empty())
		{
			//cout<<"d: "<< j << endl;
			frameQ.pushframe(frame.clone());
		}
		else
		{
			cout << "Cannot open the video file: " << endl;
			STOP;
		}
	}
	cap1.release();
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedms = (finish - start) * 1000;
	cout << "Time forcasted and needed to load................=" << timeNeededms << " " << elapsedms.count() << endl;
	while (elapsedms.count() < timeNeededms)
	{
		finish = std::chrono::high_resolution_clock::now();
		elapsedms = (finish - start) * 1000;
		cout << "Time inside.......................=" << elapsedms.count() << endl;
	}

	
	decodeCount = 0;
	cout << "All frame Loaded for: " <<filename<< endl;
	return;

}

string getChunkNametoReqnRefCam(PPC &camera, string srcBaseAddr, int chunkN, V3 cameraDirection, int tiltAngle)
{
	ERI eri(10, 10); //size does not matter here
	float latt = eri.GetXYZ2Latitude(cameraDirection);
	float longg = eri.GetXYZ2Longitude(cameraDirection);
	
	float pann = longg - 180;
	float tiltt = 90-latt;
	int latAngle = 20;
	int temp = -90-latAngle/2;
	
	while ((int)tiltt >= temp)
	{
		temp = temp + latAngle;
		//cout << temp << endl;

	}
	int reqtilt = (2 * temp - latAngle) / 2;	
	int panAngle = (int)abs((20) / cos(3.1416*reqtilt / 180));   //argument in radian
	if (panAngle % 2 != 0)
	{
		panAngle += 1;
	}
	temp = -180 - panAngle / 2;
	//cout << temp << endl;
	//cout << "pan" << endl;
	while ((int)pann >= temp)
	{
		temp = temp + panAngle;
	}
	int reqpann = (2 * temp - panAngle) / 2;
	cout << latt << " orig tilt and pan:  " << longg << endl;
	cout << reqtilt << " req tilt and pan:  " << reqpann << endl;
	std::ostringstream oss1;
	//remove these two lines when you have chunk for all direction.
	reqtilt = -10;  //dlt 
	reqpann = 0;    //dlt

	oss1 <<srcBaseAddr<< "_"<<chunkN <<"_" << reqtilt << "_" << reqpann << ".avi";
	camera.Pan(reqpann);
	camera.Tilt(reqtilt);	   
	string ofilename = oss1.str();
	return ofilename;
}
string getChunkNametoReqnRefCamOptimized(PPC &camera1, M33 & reriCS, string srcBaseAddr, int chunkN, V3 cameraDirection, int tiltAngle)
{
	ERI eri(10, 10); //size does not matter here
	float latt = eri.GetXYZ2Latitude(cameraDirection);
	float longg = eri.GetXYZ2Longitude(cameraDirection);

	float pann = longg - 180;
	float tiltt = 90 - latt;
	int latAngle = 20;
	int temp = -90 - latAngle / 2;

	while ((int)tiltt >= temp)
	{
		temp = temp + latAngle;
		
	}
	int reqtilt = (2 * temp - latAngle) / 2;
	int panAngle = (int)abs((20) / cos(3.1416*reqtilt / 180));   //argument in radian
	if (panAngle % 2 != 0)
	{
		panAngle += 1;
	}
	temp = -180 - panAngle / 2;
	//cout << temp << endl;
	//cout << "pan" << endl;
	while ((int)pann >= temp)
	{
		temp = temp + panAngle;
	}
	int reqpann = (2 * temp - panAngle) / 2;
	//cout << latt << " orig tilt and pan:  " << longg << endl;
	//cout << reqtilt << " req tilt and pan:  " << reqpann << endl;
	std::ostringstream oss1;
	//remove these two lines when you have chunk for all direction.
	//reqtilt = -10;  //dlt 
	//reqpann = 0;    //dlt

	oss1 << srcBaseAddr << "_" << chunkN << "_" << reqtilt << "_" << reqpann << ".avi";
	PPC camera = camera1;
	//cout <<"ref cam direction"<< camera1.GetVD() << endl;
	camera.Pan(reqpann);
	camera.Tilt(reqtilt);

	V3 xaxis = camera.a.UnitVector();
	V3 yaxis = camera.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;

	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;

	string ofilename = oss1.str();
	return ofilename;
}

string getChunkNametoReq4mTiles(string srcBaseAddr, int chunkN, int tileN)
{
	std::ostringstream oss1;
	oss1 << srcBaseAddr << "_" << chunkN << "_" << tileN << ".avi";
	string ofilename = oss1.str();
	return ofilename;
}

void thredProcessFrame(PPC camera2, Path path1, int MxchunkN, int chunkD, int fps, int cf, struct samplingvar& svar, float var[10], vector <Mat>& conv)
{
	//waitKey(5000);
	cout << "here2" << endl;	
	int segi = 0;
	int chunkN = 0;
	Mat ret1;
	int frameN = 0;
	int segref = 0;
	int condition = 1;
	float time = 0;
	int last_cam_index = 0;
	while (condition)
	{
		// Get a next frame
		Mat frame;	
		if (frameQ.empty()) 
		{
			int cam_index = path1.GetCamIndexUsingTime(time, last_cam_index);
			V3 VD = path1.cams[cam_index].GetVD();
			int tiltAngle = 20;
			//string fn= getChunkNametoReq(VD, tiltAngle);
			
			//cout << fn << endl;
			
		}
		if (!frameQ.empty())
		{
			frame = frameQ.getframe();		
			//cout << "Frame queue not empty" << endl;
			// Process the frame
			if (!frame.empty())
			{
				frameN++;
				//print("frameN: " << frameN << endl);
				segi = path1.GetCamIndex(frameN, fps, segi);
				chunkN = frameN % (fps*chunkD);
				if (chunkN == 0)
				{
					segref = segi;
					print("segref: " << segref << endl);
				}

				Mat heatmap3c = Mat::zeros(camera2.h, camera2.w, frame.type());
				Mat heatmap = Mat::zeros(camera2.h, camera2.w, DataType<double>::type);
				ret1 = path1.CRERI2Conv(frame, var, cf, path1.cams[segi], path1.cams[segref], heatmap, &svar);
				//conv.push_back(ret1);
				
				if (frameN == chunkD * fps*MxchunkN)
				{
					condition = 0;
				}
			}
		}
	}
	int starting_frame = 0;
	int ending_frame = fps * chunkN*chunkD;
	//filename = "./Video/encodingtest/newmethod/rollerh264convtemp";
	//videowriterhelperx(1, fps, ret1.cols, ret1.rows, starting_frame, ending_frame, conv);
	return;
}

vector<float>timeVec;
vector<float>byteVec;

void LoadBWTraceData(char* filename)
{

	ifstream  file(filename);
	if (!file)
	{
		print("error: can't open file: " << filename << endl);
		system("pause");
	}
	string   line;
	while (getline(file, line))
	{
		stringstream  linestream(line);
		///// Mahi mahi trace has only one column thats why these are commented
		//int times;
		//linestream >> times;
		//times = ceil(times / 1000);
		//timeVec.push_back(times);
		float byte;
		linestream >> byte;
		//byte = byte*25;
		byteVec.push_back(byte);
		//cout <<"bwTrace: "<< times << " " << byte << endl;
		//cout <<"bwTrace: "<< byte << endl;
	}

}

void testDownloadVideoHttp4thSecVar(int singleOrVariableVD, int samplingValueCalculate, char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec,int extraSec)
{
	auto start = std::chrono::high_resolution_clock::now();
	vector <Mat> conv;
	vector <Mat> outputFrame2SAve;
	vector<Mat> outputSamplingRate2Save;
	struct samplingvar svar;
	Mat ret1;
	Path path1;
	float var[10];
	vector<float> frameRate4allFrames;
	vector <float>srVec;
	//string srcBaseAddr;

	float howManySecondsfor4thSec = extraSec;
	int extraFrameN =5*extraSec;
	int mxChunkN = 15;
	int chunkD = 4;//
	
	vector<float> chunkSizeKB;
	/*
	chunkSizeKB.push_back(100);
	chunkSizeKB.push_back(2414);
	chunkSizeKB.push_back(2400);
	chunkSizeKB.push_back(2600);
	chunkSizeKB.push_back(2600);
	chunkSizeKB.push_back(3000);
	chunkSizeKB.push_back(3000);
	chunkSizeKB.push_back(2000);
	chunkSizeKB.push_back(700);
	chunkSizeKB.push_back(600);
	chunkSizeKB.push_back(600);
	chunkSizeKB.push_back(900);
	chunkSizeKB.push_back(1000);
	chunkSizeKB.push_back(1000);
	chunkSizeKB.push_back(800);
	chunkSizeKB.push_back(1000);
	chunkSizeKB.push_back(1200);
	chunkSizeKB.push_back(1200);
	chunkSizeKB.push_back(1200);
	chunkSizeKB.push_back(2000);
	chunkSizeKB.push_back(1300); 
	chunkSizeKB.push_back(0);
	chunkSizeKB.push_back(132);
	chunkSizeKB.push_back(130);
	chunkSizeKB.push_back(150);
	chunkSizeKB.push_back(160);
	chunkSizeKB.push_back(200);
	chunkSizeKB.push_back(100);
	chunkSizeKB.push_back(70);
	chunkSizeKB.push_back(40);
	chunkSizeKB.push_back(600);
	chunkSizeKB.push_back(60);
	chunkSizeKB.push_back(40);
	chunkSizeKB.push_back(80);
	chunkSizeKB.push_back(90);
	chunkSizeKB.push_back(80);
	chunkSizeKB.push_back(60);
	chunkSizeKB.push_back(110);
	chunkSizeKB.push_back(100);
	chunkSizeKB.push_back(120);
	chunkSizeKB.push_back(100);
	chunkSizeKB.push_back(100); */


	chunkSizeKB.push_back(0);
	chunkSizeKB.push_back(538);
	chunkSizeKB.push_back(568);
	chunkSizeKB.push_back(575);
	chunkSizeKB.push_back(580);
	chunkSizeKB.push_back(764);
	chunkSizeKB.push_back(500);
	chunkSizeKB.push_back(170);
	chunkSizeKB.push_back(140);
	chunkSizeKB.push_back(140);
	chunkSizeKB.push_back(160);
	chunkSizeKB.push_back(140);
	chunkSizeKB.push_back(240);
	chunkSizeKB.push_back(280);
	chunkSizeKB.push_back(240);
	chunkSizeKB.push_back(260);
	chunkSizeKB.push_back(310);
	chunkSizeKB.push_back(320);
	chunkSizeKB.push_back(320);
	chunkSizeKB.push_back(330);
	chunkSizeKB.push_back(340);

	
	LoadBWTraceData(bwLog);
	string datax("./Video/source/ 4kDivingEncodingVariable.txt");// = fetchTextOverHttp("http://127.0.0.5:80/4thSecVar/diving/4kDivingEncodingVariable.txt");
	if (singleOrVariableVD == 1) {
		// srcBaseAddr = "http://127.0.0.5:80/3vid2crf3trace/cr40/rollerCR40";
	}
	else {
		// srcBaseAddr = "http://127.0.0.5:80/4thSecVar/diving/tempAndroid/4s/divingCR40";
	}
	

	//char* hmdFileName;
	if (singleOrVariableVD == 1) {
		//hmdFileName = "./Video/source/roller.txt";
	//	hmdFileName = "./Video/source/roller.txt";
	}
	else {
		//hmdFileName = "./Video/source/rollerSingleVD.txt";
	}
	
	
	vector <float> nonUniformList;
	temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrameN+1);  //n datapoint e n-1 gap hoy, ajonno +1 add koresi jate n ta gap e hoy

		
	/*IMP*/	
	ifstream  file("./Video/source/4kDivingEncodingVariable.txt");
	if (!file)
	{
		print("error: can't open file: " << filename << endl);
		system("pause");
	}
	string line;
	int i = 0;
	while (getline(file, line))
	{
		var[i] = stoi(line);
		cout << var[i] << '\n';
		i++;
	}

	int frameLen = var[0];
	int frameWidth = var[1];	
	float hfov = 90.0f;
	float corePredictionMargin = 0.9;//use 0.6 for perfect timing, thats when we get 30 fps, otherwise 0.8
	int w = frameLen * hfov / 360; //add frameLen and Width into the variable file also totalChunkN
	int h = frameWidth * hfov / 360;  //540 for perfect 2160 p but here we have 2048
	//int w = 640;
	//int h = 480;
	PPC camera2(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	PPC refCam(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	path1.LoadHMDTrackingData(hmdFileName, camera2);
	int compressionFactor = 5;// var[6];  //read from variable file

	int frameStarted = 0;
	
	int chunkN = 0;
	int fps = 30;
	
	Mat firstScene;
	upload_image("./Video/source/startScene.PNG", firstScene);

	Mat endScene;
	int condition = 1;
	int cam_index = 0;
	int tiltAngle = 20;
	string filename;
	V3 VD;
	int frameCount = 0;
	int startIndex = 0;
	int after90Index = -1;
	std::vector<std::future<void>> futures;
	Mat heatmap3c = Mat::zeros(camera2.h, camera2.w, DataType<double>::type);
	Mat heatmap = Mat::zeros(camera2.h, camera2.w, DataType<double>::type);
	int ERI_w = frameLen;   
	int ERI_h = frameWidth;    
	ERI eri(ERI_w, ERI_h);

	

	for (int chunk = 0; chunk <= mxChunkN+1; chunk++)
	{
		vector <Mat> temp;
		for (int i = 0; i < (fps*chunkD+extraFrameN); i++)
		{
			Mat m;
			temp.push_back(m);
			m.release();
		}
		frameQvec.push_back(temp);
	}

	eri.atanvalue();
	eri.xz2LonMap();
	eri.xz2LatMap();
	path1.nonUniformListInit(var);	
	path1.mapx(var);
	//path1.calculateAllSamplingRateOverCreri(var);

	
	cout << "Very first Chunk, no frame yet......" << endl;

	//*****future tiles name
	/*
	M33 reriCSx;
	ofstream output;
	output.open("./Video/source/chunkName.txt", std::ios::out | std::ios::app);
	output << "frameRequestTime:" << 120-nextDlChunkSec*30 << endl;
	chunkN = 1;
	frameCount = 0;
	cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
	VD = path1.cams[cam_index].GetVD();
	filename = getChunkNametoReqnRefCamOptimized(refCam, reriCSx, srcBaseAddr, chunkN, VD, tiltAngle);
	
	output << filename << endl;

	chunkN = 2;
	frameCount = 120 - nextDlChunkSec * 30;
	for (int i = 0; i < 50; i++)
	{
		cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
		VD = path1.cams[cam_index].GetVD();
		filename = getChunkNametoReqnRefCamOptimized(refCam, reriCSx, srcBaseAddr, chunkN, VD, tiltAngle);
		cout << filename << endl;
		chunkN++;
		frameCount = frameCount + 120;
		
		output << filename << endl;
	}
	output.close();
	STOP;
	//*/

	cam_index = path1.GetCamIndex(1, fps, cam_index);	
	VD = path1.cams[cam_index].GetVD();
	chunkN++;
	M33 reriCS;
	M33 reriCScopy;
	
	filename = getChunkNametoReqnRefCamOptimized(refCam, reriCS, srcBaseAddr, chunkN, VD, tiltAngle);
	cout <<"ReqF="<< filename << endl;
	NextChunkDownloaded = 0;
	auto finish1 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed1 = finish1 - start;
	int bwTimeNeeded = 200; //500 ms default value, without any trace
	int bwDuration;
	int bwIglobal;
	int packetNeeded;
	int bwShift = 0;

	
	//************* BW simulation****************//
	if (samplingValueCalculate == 1)
	{
		bwDuration = elapsed1.count() * 1000 + bwShift;
		 bwIglobal = 0;
		while (bwDuration > byteVec[bwIglobal]) {
			bwIglobal++;
		}

		cout << "pckSize: " << chunkSizeKB[chunkN] << endl;
		
		packetNeeded = chunkSizeKB[chunkN] * 1024 / 1500;  //500 KB file size approximate
		bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
		cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
		
		
	}

	//************* BW simulation****************//
	   
	cout << "First DL req Time:------>" << elapsed1.count() << " bwStartIndx---> " <<bwIglobal<<" bwEndIndex-->"<< bwIglobal+ bwTimeNeeded << endl;
	auto playStart = std::chrono::high_resolution_clock::now();
	
	DownLoadChunk4thSecVar(filename, chunkD, fps,extraFrameN, bwTimeNeeded, chunkN);
	
	while (NextChunkDownloaded == 0)
	{
		//displayImage(firstScene);
	}
	Mat convPixels(camera2.h, camera2.w, firstScene.type());
	Mat samplingPixels(camera2.h, camera2.w, firstScene.type());
	cout << camera2.h << " cam " << camera2.w << endl;
	frameCount = -1;

	int firstPlayTime;
	while (condition)
	{
		
		frameCount++; //0->
		if (frameCount == (fps*chunkD * mxChunkN-1+0)) //add 449 for last 5 second extension
		{
			condition = 0;
		}

		int currentChunk = (frameCount) / (fps*chunkD)+1;
		if (currentChunk > mxChunkN+1)
		{
			currentChunk = mxChunkN + 1;
		}
		int playIndex= frameCount-(currentChunk-1)* (fps*chunkD); //0-(fps*chunkD-1)
		cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex <<" "<<NextChunkDownloaded<< endl;
		//cout << frameCount << endl;

		if (playIndex==30 && currentChunk>1)
		{
			vector <Mat> temp;
			for (int i = 0; i < 2; i++)
			{
				Mat m;
				temp.push_back(m);
				m.release();
			}
			frameQvec[currentChunk - 1] = temp;
			cout << "memory erased..................." << endl;
		}

	

		if (currentChunk <= NextChunkDownloaded)
		{
			if (currentChunk == NextChunkDownloaded && currentChunk>1)
			{
				if (ChngReriCS == 1)
				{
					reriCS = reriCScopy;
					cout << "reriCS updated............................" << endl;
					ChngReriCS = 0;
				}
			}

		

			Mat frame;				
			frame = frameQvec[currentChunk][playIndex];	
			
			after90Index = -1;			
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			V3 vdTemp = path1.cams[cam_index].GetVD();
			//cout<<"longt: "<<eri.GetXYZ2Longitude(vdTemp)<<endl;
			
			
			if (samplingValueCalculate==1)
			{ 
				if (frameCount < 1)
				{
					auto firstFrame = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double> elapsedFirst = firstFrame - start;
					firstPlayTime = elapsedFirst.count() * 1000;
					playStart = std::chrono::high_resolution_clock::now();

				}
				path1.CRERI2ConvOptimizedWithSamplingRateVec(frame, var, eri, reriCS, convPixels, srVec, compressionFactor, path1.cams[cam_index], refCam);
				frameRate4allFrames.push_back(1);
				
			}
			else {
				path1.CRERI2ConvOptimized(frame, var, eri, reriCS, convPixels, compressionFactor, path1.cams[cam_index], refCam);
			
			}			
			
			//displayImage(convPixels);	
			displayImage33ms();
			//outputFrame2SAve.push_back(convPixels.clone());


			if ((playIndex) == fps*(chunkD-nextDlChunkSec))
			{
				int reqChunkN = currentChunk+1; 				
				cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
				reriCScopy = reriCS;
				VD = path1.cams[cam_index].GetVD();	
				
				filename = getChunkNametoReqnRefCamOptimized(refCam, reriCScopy, srcBaseAddr, reqChunkN, VD, tiltAngle);
				cout << "New Req. " << filename << endl;				
				finish1 = std::chrono::high_resolution_clock::now();
				elapsed1 = finish1 - start;
				if (samplingValueCalculate == 1)
				{
					//************* BW simulation****************//
					//bwDuration = elapsed1.count() * 1000+ bwShift; //real time spend
					bwDuration = frameCount * 33+firstPlayTime;   //forced assumption that all frame is 33ms play time, even though they are not.
					while (bwDuration > byteVec[bwIglobal]) {
						bwIglobal++;
					}
					packetNeeded = chunkSizeKB[reqChunkN] * 1024 / 1500;
					cout << "pckSize: " << chunkSizeKB[reqChunkN] << endl;
					bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms

				   //************* BW simulation****************//
				}
				
				cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
				futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps,extraFrameN, bwTimeNeeded, reqChunkN));

			}
		
		}	

		if (after90Index >= extraFrameN)
		{
			cout << "Network disconnected, try again...................................." << endl;
			auto playFinish = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> playElapsed = playFinish - playStart;
			cout << "Total Play Time:................> " << playElapsed.count()*1000 << endl;
			condition = 0;			
				
		}
		
		if(((currentChunk) > NextChunkDownloaded) && currentChunk>1)
		{
			after90Index++; //0->
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);			
			int index = nonUniformList[after90Index];
			int index2 = nonUniformList[after90Index + 1];
			
			int diffIndex = index2 - index;
			if (diffIndex>100 || diffIndex<0)
			{
				condition=0;
					break;
			}
			for (int i = 0; i < diffIndex; i++)
			{
				if (i > 0) {
					frameCount++;
				}
				currentChunk = (frameCount) / (fps*chunkD)+1; //current chunk 0 theke start, file chunk o 0 theke start krote hobe encoding er somoy
				playIndex = frameCount - (currentChunk-1) * (fps*chunkD);
				cout << "119CuntChunkN=" << currentChunk-1 << " frmcount=" << frameCount << " plCnt=" << playIndex <<" after90indx="<<index<<" "<<NextChunkDownloaded<< endl;
			//	cout << frameCount << endl;
				cam_index = path1.GetCamIndex(frameCount-1, fps, cam_index);
				Mat frame1;
				frame1 = frameQvec[currentChunk - 1][(fps*chunkD - 1) + after90Index];
				
				if (samplingValueCalculate == 1)
				{

					path1.CRERI2ConvOptimizedWithSamplingRateVec(frame1, var, eri, reriCS, convPixels, srVec, compressionFactor, path1.cams[cam_index], refCam);
					float frameRateTemp = diffIndex;
					frameRate4allFrames.push_back(frameRateTemp);
				}
				else {
					path1.CRERI2ConvOptimized(frame1, var, eri, reriCS, convPixels, compressionFactor, path1.cams[cam_index], refCam);
					
				}
				
				//cout<<frameQvec[currentChunk - 1].size() << endl;
				
				//displayImage(convPixels);
				displayImage33ms();
				//outputFrame2SAve.push_back(convPixels.clone());	
				if (playIndex == (fps*(chunkD-nextDlChunkSec)))
				{
					int reqChunkN = currentChunk + 1; //amader chunk 0 theke start hoy but download chunk start hoy 1 theke , apatoto			
					
					cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
					VD = path1.cams[cam_index].GetVD();
					reriCScopy = reriCS;
				
					filename = getChunkNametoReqnRefCamOptimized(refCam, reriCScopy, srcBaseAddr, reqChunkN, VD, tiltAngle);
					cout << "NewR: " << filename << endl;
					finish1 = std::chrono::high_resolution_clock::now();
					elapsed1 = finish1 - start;
					//************* BW simulation****************//
					if (samplingValueCalculate == 1)
					{
						//bwDuration = elapsed1.count() * 1000+bwShift;
						bwDuration = frameCount * 33+firstPlayTime;   //forced assumption that all frame is 33ms play time, even though they are not.
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}
						cout << "pckSize: " << chunkSizeKB[reqChunkN] << endl;
						packetNeeded = chunkSizeKB[reqChunkN] * 1024 / 1500;
						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
					}
					cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;

				   //************* BW simulation****************//
					//cout << elapsed1.count() << " third " << bwTimeNeeded << endl;
					futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));
					
					
				}

							
			}
			
		}
					   		

	}

	auto playFinishAll = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> playElapsed = playFinishAll - playStart;
	cout << "FPS:................> " << playElapsed.count() * 1000 / frameCount << endl;
	cout << "Frist frame Appear Time=..............>" << firstPlayTime << endl;


	if (samplingValueCalculate == 1)
	{

		float minTemp = 100;
		float totalTemp = 0;
		for (int i = 0; i < frameRate4allFrames.size(); i++)
		{
			float tempValue = 30 / frameRate4allFrames[i];
			totalTemp = totalTemp + tempValue;
			if (tempValue < minTemp)
			{
				minTemp = tempValue;
			}
		}

		cout << "Minimum Frame Rate: " << minTemp << " averageFrameRate: " << totalTemp / frameRate4allFrames.size() << endl;


		float overallMin = 10;
		float overallMax = 0;
		float ovrallAvgTotal = 0;
		double aggValue = 0;
		double value = 0;
		float min = 1000;
		float max = 0;
		float avg = 0;
		for (int i = 0; i < srVec.size(); i++)
		{
			value = srVec[i];
			aggValue = value + aggValue;
			cout << "frame: " << i << " SR value: " << value << endl;
			if (value > max)
			{
				max = value;
			}
			if (value < min)
			{
				min = value;
			}


			if (min < overallMin)
			{
				overallMin = min;
			}
			if (max > overallMax)
			{
				overallMax = max;
			}
			avg = aggValue / (srVec.size());

		}
		ofstream output;
		output.open("./Video/source/srHttpVarTest.txt", std::ios::out | std::ios::app);		
		output << "File: "<<srcBaseAddr << " min-> " << overallMin << " max-> " << overallMax << " avg->" << avg << endl;
		output.close();
		cout << "overall Pixel Sampling RAte: " << "min-> " << overallMin << " max-> " << overallMax << " avg->" << avg << endl;
	}


	int sf = 0;
	int ef = outputFrame2SAve.size();
	//cout << "SAving-> sf:ef " << sf << " " << ef << endl;	
	//videowriterhelperx(chunkN, 0, 0, fps, outputFrame2SAve[1].cols, outputFrame2SAve[1].rows, sf, ef, outputFrame2SAve);
	if (samplingValueCalculate == 1)
	{
		//videowriterhelperx(100, 0, 0, fps, outputSamplingRate2Save[0].cols, outputSamplingRate2Save[0].rows, sf, ef, outputSamplingRate2Save);
				//100 to differiantitate with regular video
	}
}

void testDownloadVideoHttp(int singleOrVariableVD, int samplingValueCalculate)
{	
	auto start = std::chrono::high_resolution_clock::now();
	vector <Mat> conv;
	struct samplingvar svar;
	vector <Mat> outputFrame2SAve;
	vector<Mat> outputSamplingRate2Save;
	int totalStallFrame = 0;
	Mat ret1;
	Path path1;	
	float var[10];
	
	/*IMP*/
	LoadBWTraceData("./bwLogs/AttLteDriving646KB2Min.txt"); //check inside for scaling up and down. There is a multiplication factor there.
	string datax;// = fetchTextOverHttp("http://127.0.0.5:80/4thSecVar/diving/4kDivingEncodingVariable.txt");
	string srcBaseAddr = "http://127.0.0.5:80/4thSecVar/diving/segment4rollerVD/Regular/crf40/divingCR40";
	std::istringstream f(datax);
	std::string line;
	int i = 0;
	while (std::getline(f, line)) 
	{
		var[i] = stoi(line);
		cout << var[i] << '\n';
		i++;
	}	

	int frameLen = var[0];
	int frameWidth = var[1]; 
	float hfov = 90.0f;
	float corePredictionMargin = 0.7;
	int w = frameLen * hfov / 360; //add frameLen and Width into the variable file also totalChunkN
	int h = frameWidth * hfov / 360;  //540 for perfect 2160 p but here we have 2048

	PPC camera2(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	PPC refCam(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);

	char* hmdFileName;
	if (singleOrVariableVD == 1) {
		hmdFileName = "./Video/source/roller.txt";
	}
	else {
		hmdFileName = "./Video/source/rollerSingleVD.txt";
	}

	path1.LoadHMDTrackingData(hmdFileName, camera2);

	int mxChunkN = 3;// var[4];
	int chunkD = 4;//var[5]; //read from variable	
	int compressionFactor = 5;// var[6];  //read from variable file

	int frameStarted = 0;
	Mat savedLastChunkFRame; //need attention
	int displayedFrameCount = 0;
	int chunkN=1;
	int fps = 30;
	int nextDlChunkSec = 3;
	Mat firstScene;	
	upload_image("./Video/source/startScene.PNG", firstScene);
	Mat endScene;
	int condition = 1;
	int cam_index=0;
	int tiltAngle = 20;
	string filename;
	V3 VD;
	int frameCount = 0;
	std::vector<std::future<void>> futures;
	Mat heatmap3c = Mat::zeros(camera2.h, camera2.w, DataType<double>::type);
	Mat heatmap = Mat::zeros(camera2.h, camera2.w, DataType<double>::type);

	M33 reriCS;
	int ERI_w = frameLen;   
	int ERI_h = frameWidth;    
	ERI eri(ERI_w, ERI_h);
	eri.atanvalue();
	eri.xz2LonMap();
	eri.xz2LatMap();
	path1.nonUniformListInit(var);
	path1.mapx(var);
	path1.calculateAllSamplingRateOverCreri(var);

	int framePlayed = 0;
	cout << "Very first Chunk, no frame yet......" << endl;

	int bwTimeNeeded = 200; //500 ms default value, without any trace
	int bwDuration;
	int bwIglobal;
	int packetNeeded;

	Mat convPixels(camera2.h, camera2.w, firstScene.type());
	Mat samplingPixels(camera2.h, camera2.w, firstScene.type());
	auto playStart = std::chrono::high_resolution_clock::now();
	while (condition)
	{
		if (framePlayed == mxChunkN * fps*chunkD)
		{
			cout << "conditon" << endl;
			condition = 0;

		}
		Mat frame;
		frameCount = framePlayed;
		if (frameQ.empty())
		{
			if (frameStarted == 0)
			{
				cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;
				displayImage(firstScene);
				cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
				VD = path1.cams[cam_index].GetVD();
				cout << "1" << endl;
				filename = getChunkNametoReqnRefCamOptimized(refCam, reriCS, srcBaseAddr, chunkN, VD, tiltAngle);
				cout << filename << endl;
				auto finish1 = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> elapsed1 = finish1 - start;
				if (samplingValueCalculate == 1)
				{
					bwDuration = elapsed1.count() * 1000;
					bwIglobal = 0;
					while (bwDuration > byteVec[bwIglobal]) {
						bwIglobal++;
					}

					packetNeeded = 500 * 1024 / 1500;  //500 KB file size approximate
					bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
					cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
					
				}

				//cout << elapsed1.count() << " " << bwTimeNeeded << endl; 
				cout << "New RequesFromEmpty: " << filename << endl;
				DownLoadChunk(filename, chunkD, fps, bwTimeNeeded);
				
				cout << "2" << endl;
				frameStarted = 1;
			}
			else
			{
				
				cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
				if (savedLastChunkFRame.empty())
				{
					cout << "Saved frame empty..... Red flag" << endl;
					waitKey(30);
				}
				else {
					if (samplingValueCalculate == 1)
					{
						path1.CRERI2ConvOptimizedWithSamplingRate(savedLastChunkFRame, var, eri, reriCS, convPixels, samplingPixels, compressionFactor, path1.cams[cam_index], refCam);
						outputSamplingRate2Save.push_back(samplingPixels.clone());
						
					}
					else {
						path1.CRERI2ConvOptimized(savedLastChunkFRame, var, eri, reriCS, convPixels, compressionFactor, path1.cams[cam_index], refCam);

					}
					outputFrame2SAve.push_back(convPixels.clone());
					displayImage(convPixels);
					framePlayed++;
					totalStallFrame++;
					cout <<"insideEmptyLoop-> frame: "<< framePlayed << "stalledFrame-> "<<totalStallFrame<<endl;
					
				}
			}
		}
		if (!frameQ.empty())
		{			
			frame = frameQ.getframe();
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			//cout << "not empty" << endl;

			if (samplingValueCalculate == 1)
			{
				path1.CRERI2ConvOptimizedWithSamplingRate(frame, var, eri, reriCS, convPixels, samplingPixels, compressionFactor, path1.cams[cam_index], refCam);
				outputSamplingRate2Save.push_back(samplingPixels.clone());
				if (frameCount < 2)
				{
					auto firstFrame = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double> elapsed = firstFrame - start;
					cout << "Time=..............>" << elapsed.count() * 1000 << endl;
				}
			}
			else {
				path1.CRERI2ConvOptimized(frame, var, eri, reriCS, convPixels, compressionFactor, path1.cams[cam_index], refCam);

			}
		
			
			outputFrame2SAve.push_back(convPixels.clone());
			displayImage(convPixels);

			frameCount++;
			framePlayed++;
			cout << framePlayed <<endl;
			if (frameCount%(chunkD * fps)==0)
			{
				frameCount = 0;
				savedLastChunkFRame = frame;
				
				cout << "Play reached current chunkEnd.........." << endl;
			}
			if (frameCount%(chunkD*fps) == nextDlChunkSec * fps)
			{
				chunkN = chunkN + 1;
				cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
				VD = path1.cams[cam_index].GetVD();
				filename = getChunkNametoReqnRefCamOptimized(refCam, reriCS, srcBaseAddr, chunkN, VD, tiltAngle);
				cout <<"New Reques: "<< filename << endl;	
				auto finish1 = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> elapsed1 = finish1 - start;
				if (samplingValueCalculate == 1)
				{
					bwDuration = elapsed1.count() * 1000;
					bwIglobal = 0;
					while (bwDuration > byteVec[bwIglobal]) {
						bwIglobal++;
					}

					packetNeeded = 500 * 1024 / 1500;  //500 KB file size approximate
					bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
					cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;

				}
			
				futures.push_back(std::async(std::launch::async, DownLoadChunk, filename, chunkD, fps, bwTimeNeeded));
				
				
			}	
			
						
		}

	}
	while (outputFrame2SAve.size() < fps*chunkD*mxChunkN)
	{
	}
	auto playFinish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> playElapsed = playFinish - playStart;
	cout << "Total Play Time:................> " << playElapsed.count() * 1000 << endl;

	vector<int>frameWiseMin;
	vector<int>frameWiseMax;
	vector<int>frameWiseAvg;

	if (samplingValueCalculate == 1)
	{	
		float overallMin = 10;
		float overallMax = 0;
		float ovrallAvgTotal = 0;
		for (int i = 0; i < outputSamplingRate2Save.size(); i++)
		{
			float min = 10;
			float max = 0;
			float value = 0;
			float aggValue = 0;

			for (int j = 0; j < outputSamplingRate2Save[0].rows; j++)
			{
				for (int k = 0; k < outputSamplingRate2Save[0].cols; k++)
				{
					value = outputSamplingRate2Save[i].at<Vec3b>(j, k)[0];

					value = 20 / value;
					//cout << j << " " << k << " " << value << endl;
					aggValue = aggValue + value;
					if (value > max)
					{
						max = value;
					}
					if (value < min)
					{
						min = value;
					}
				}
			}
			if (min < overallMin)
			{
				overallMin = min;
			}
			if (max > overallMax)
			{
				overallMax = max;
			}
			float avg = aggValue / (outputSamplingRate2Save[0].rows*outputSamplingRate2Save[0].cols);
			ovrallAvgTotal = ovrallAvgTotal + avg;
			//cout <<i<<" "<< min << " " << max << " " << avg << endl;
			frameWiseMin.push_back(min);
			frameWiseMax.push_back(max);
			frameWiseAvg.push_back(avg);

		}
		cout << "overall: " << "min-> " << overallMin << " max-> " << overallMax << " avg->" << ovrallAvgTotal / frameWiseAvg.size() << endl;
	}

	int sf = 0;
	int ef = outputFrame2SAve.size();
	cout << "SAving-> sf:ef " << sf << " " << ef << endl;
	cout << fps << outputSamplingRate2Save[0].cols << outputSamplingRate2Save[0].rows << " sf: " << sf << " ef: " << ef << endl;
	videowriterhelperx(chunkN, 2, 0, fps, outputFrame2SAve[1].cols, outputFrame2SAve[1].rows, sf, ef, outputFrame2SAve);
	if (samplingValueCalculate == 1)
	{
		videowriterhelperx(200, 0, 0, fps, outputSamplingRate2Save[0].cols, outputSamplingRate2Save[0].rows, sf, ef, outputSamplingRate2Save);
		//100 to differiantitate with regular video
	}



}

void getTilesNumber2req(vector<int> & tileBitMap, PPC camera, ERI & eri, int m, int n)
{


	int pixelI, pixelJ = 0;
	for (int v = 0; v < camera.h; v++)
	{
		for (int u = 0; u < camera.w; u++)
		{
			eri.EachPixelConv2ERI(camera, u, v, pixelI, pixelJ);
			int Xtile = floor(pixelJ*m / eri.w); //m*n col and row
			int Ytile = floor(pixelI*n / eri.h);
			int vectorindex = (Ytile)*m + Xtile;
			tileBitMap.at(vectorindex) = 1;

		}

	}

}

void testDownloadVideoHttpTile(int singleOrVariableVD, int samplingValueCalculate)
{
	auto start = std::chrono::high_resolution_clock::now();
	Path path1;
	float var[10];
	

	vector<float> chunkSizeKB;
	chunkSizeKB.push_back(0);
	chunkSizeKB.push_back(100*1.024);
	chunkSizeKB.push_back(110*1.024);

	LoadBWTraceData("./bwLogs/bw646KB.txt"); //check inside for scaling up and down. There is a multiplication factor there.
	string datax;// = fetchTextOverHttp("http://127.0.0.5:80/4thSecVar/diving/4kDivingEncodingVariable.txt");
	string srcBaseAddr = "http://127.0.0.5:80/3vid2crf3trace/tiles/diving";
	std::istringstream f(datax);
	std::string line;
	int i = 0;
	while (std::getline(f, line))
	{
		var[i] = stoi(line);
		cout << var[i] << '\n';
		i++;
	}

	int frameCol = var[0];
	int frameRow = var[1];
	float hfov = 90.0f;
	float corePredictionMargin = 0.7;
	int w = frameCol * hfov / 360; //add frameLen and Width into the variable file also totalChunkN
	int h = frameRow * hfov / 360;  //540 for perfect 2160 p but here we have 2048
	PPC camera2(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	char* hmdFileName;
	if (singleOrVariableVD == 1) {
		hmdFileName = "./Video/source/diving.txt";
	}
	else {
		hmdFileName = "./Video/source/rollerSingleVD.txt";
	}
	path1.LoadHMDTrackingData(hmdFileName, camera2);
	int mxChunkN = 3;// var[4];
	int chunkD = 4;//var[5]; //read from variable	
	int chunkN = 1;
	int fps = 30;
	Mat firstScene;
	upload_image("./Video/source/startScene.PNG", firstScene);
	Mat endScene;
	int condition = 1;
	int cam_index = 0;
	int tiltAngle = 20;
	string filename;
	int frameCount = 0;
	std::vector<std::future<void>> futures;

	int ERI_w = frameCol;
	int ERI_h = frameRow;
	ERI eri(ERI_w, ERI_h);
	int tilesRow = 4;
	int tilesCol = 6;

	for (int i = 0; i < tilesCol*tilesRow; i++)
	{
		vector<vector <Mat>> temp1;
		for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
		{
			vector <Mat> temp;
			for (int i = 0; i < (fps*chunkD); i++)
			{
				Mat m;
				temp.push_back(m);
				m.release();
			}
			temp1.push_back(temp);
		}
		frameQvecTiles.push_back(temp1);
	}
	int bwTimeNeeded = 200; //500 ms default value, without any trace
	int bwDuration;
	int bwIglobal;
	int packetNeeded;
	Mat convPixels(camera2.h, camera2.w, firstScene.type());
	

	Mat blankCanvas(frameRow, frameCol, firstScene.type());

	vector<vector <int>> reqTiles;
	for (int i = 0; i <= mxChunkN; i++)
	{

		vector<int> temp;
		int x = 0;
		temp.push_back(x);
		reqTiles.push_back(temp);
	}
	int tileDlSum = 0;
	int tileCond = 1;
	int tileRowN = 4;
	int tileColN = 6;

	cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;
	displayImage(firstScene);
	cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

	vector <int> tileBitMap;
	for (int i = 0; i < tileRowN*tileColN; i++)
	{
		tileBitMap.push_back(0);
	}

	getTilesNumber2req(tileBitMap, path1.cams[cam_index], eri, tileColN, tileRowN);

	auto playStart = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < tileRowN*tileColN; i++)
	{
		cout << i << "--" << tileBitMap[i] << endl;
		if (tileBitMap[i] == 1)
		{
			filename = getChunkNametoReq4mTiles(srcBaseAddr, chunkN, i);
			cout << filename << endl;
			reqTiles[chunkN].push_back(i);
			auto finish1 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed1 = finish1 - start;
			bwDuration = elapsed1.count() * 1000;
			bwIglobal = 0;
			while (bwDuration > byteVec[bwIglobal]) {
				bwIglobal++;
			}

			cout << "pckSize: " << chunkSizeKB[chunkN] << endl;
			packetNeeded = chunkSizeKB[chunkN]*4 * 1024 / 1500; 
			bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
			cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;

			futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, chunkN, chunkD, i, fps, bwTimeNeeded));
		}
	}
	Mat tileFrame;
	int tileColLen = frameCol / tileColN;
	int tileRowLen = frameRow / tileRowN;
	cout << "t: " << tileColLen << " " << tileRowLen << endl;
	cout << reqTiles[chunkN].size() << endl;
	vector<int> totalInside;
	while (NextChunkDownloaded < reqTiles[chunkN].size() - 1)
	{
		cout << "looping..." << endl;
	}
	NextChunkDownloaded = 0;
	cout << tileDlSum << endl;

	int tileCondition = 1;
	while (tileCondition)
	{

		for (int i = 0; i < fps*chunkD; i++)
		{
			cout << i << " " << chunkN << endl;
			cam_index = path1.GetCamIndex(i + (chunkN - 1)*chunkD*fps, fps, cam_index);
			if (i == 90)
			{
				vector <int> tileBitMap2;

				for (int i = 0; i < tileRowN*tileColN; i++)
				{
					tileBitMap2.push_back(0);
				}
				getTilesNumber2req(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);

				for (int i = 0; i < tileRowN*tileColN; i++)
				{
					//cout << i << "--" << tileBitMap2[i] << endl;
					int reqChunkN = chunkN + 1;
					if (reqChunkN <= mxChunkN)
					{
						if (tileBitMap2[i] == 1)
						{
							filename = getChunkNametoReq4mTiles(srcBaseAddr, reqChunkN, i);
							cout << filename << endl;
							reqTiles[reqChunkN].push_back(i);
							auto finish1 = std::chrono::high_resolution_clock::now();
							std::chrono::duration<double> elapsed1 = finish1 - start;
							bwDuration = elapsed1.count() * 1000;
							bwIglobal = 0;
							while (bwDuration > byteVec[bwIglobal]) {
								bwIglobal++;
							}

							cout << "pckSize: " << chunkSizeKB[chunkN] << endl;
							packetNeeded = chunkSizeKB[chunkN] * 4 * 1024 / 1500;
							bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
							cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
							futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded));
						}
					}
				}
			}

			for (int j = 1; j < reqTiles[chunkN].size(); j++)
			{
				int tileNo = reqTiles[chunkN][j];
				int startCol = tileColLen * (tileNo % tileColN);
				int startRow = tileRowLen * (tileNo / tileColN);
				//cout << reqTiles[j] << " " << startCol << " " << startRow << endl;
				Mat tileFrame = frameQvecTiles[reqTiles[chunkN][j]][chunkN][i];
				//displayImage(tileFrame);

				tileFrame.copyTo(blankCanvas(Rect(startCol, startRow, tileColLen, tileRowLen)));

			}

			Mat convPixelsTmp(camera2.h, camera2.w, firstScene.type());
			eri.ERI2Conv4tiles(convPixelsTmp, frameQvecTiles, reqTiles[chunkN], path1.cams[cam_index], tileColN, tileRowN, chunkN, i, totalInside);
			//eri.ERI2Conv(blankCanvas, convPixelsTmp, path1.cams[cam_index]);
			//	displayImage(blankCanvas);
			displayImage(convPixelsTmp);
			auto finish = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = finish - playStart;
			cout << "Time=" << elapsed.count() * 1000 << endl;
		

		}
		chunkN = chunkN + 1;
		if (chunkN > mxChunkN)
		{
			tileCondition = 0;
		}
	}
	long sumTotalInside;
	for (int i = 0; i <totalInside.size();  i++)
	{
		sumTotalInside = sumTotalInside + totalInside[i];
	}
	cout << sumTotalInside << endl;
	cout << totalInside.size()*camera2.w*camera2.h << endl;
	double avgTotalInside = sumTotalInside /(double) (totalInside.size()*camera2.w*camera2.h);
	cout << "avgTotalInside: " << avgTotalInside << endl;
}




Mat diffimgage(Mat backgroundImage, Mat currentImage) {
	cv::Mat diffImage;
	cv::absdiff(backgroundImage, currentImage, diffImage);

	cv::Mat foregroundMask = cv::Mat::zeros(diffImage.rows, diffImage.cols, CV_8UC1);

	float threshold = 30.0f;
	float dist;

	for (int j = 0; j < diffImage.rows; ++j)
	{
		for (int i = 0; i < diffImage.cols; ++i)
		{
			cv::Vec3b pix = diffImage.at<cv::Vec3b>(j, i);

			dist = (pix[0] * pix[0] + pix[1] * pix[1] + pix[2] * pix[2]);
			dist = sqrt(dist);

			if (dist > threshold)
			{
				foregroundMask.at<unsigned char>(j, i) = 255;
			}
		}
	}

	return diffImage;
}

void getcheckerboard()
{	
	ERI eri(3840, 2160);	
	Mat convImage;
	upload_image("./Image/checkerbigcolor1.PNG", convImage);   //Image must be square size
	Mat eriMat(eri.h, eri.w, convImage.type());
	float fov[2];
	fov[0] = 90.0f;
	fov[1] = 90.0f;
	PPC camera1(fov, convImage.cols);
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	
	camera1.Tilt(90);


	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.cols && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.rows)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}



	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);
	imshow("sample", eriMat);	
	waitKey(1000);

	upload_image("./Image/checkerbigcolor2.PNG", convImage);   //Image must be square size
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));

	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.cols && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.rows)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}

	imshow("sample", eriMat);	
	waitKey(1000);

	upload_image("./Image/checkerbigcolor3.PNG", convImage);   //Image must be square size
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	camera1.Pan(90);


	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
	
			if (pp[0] < convImage.cols && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.rows)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	camera1.Pan(180);

	imshow("sample", eriMat);	
	waitKey(1000);

	upload_image("./Image/checkerbigcolor4.PNG", convImage);   //Image must be square size

	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.cols && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.rows)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}

	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	camera1.Pan(270);

	imshow("sample", eriMat);	
	waitKey(1000);
	upload_image("./Image/checkerbigcolor5.PNG", convImage);   //Image must be square size

	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.cols && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.rows)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}
	camera1.PositionAndOrient(V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0));
	camera1.Tilt(-90);

	upload_image("./Image/checkerbigcolor6.PNG", convImage);   //Image must be square size

	for (int i = 0; i < eri.h; i++)
	{
		for (int j = 0; j < eri.w; j++)
		{
			V3 p = eri.Unproject(i, j);
			V3 pp;
			if (!camera1.Project(p, pp))
				continue;
			if (pp[0] < convImage.cols && pp[0] >= 0 && pp[1] >= 0 && pp[1] < convImage.rows)
			{
				//print(pp[1] << " " << pp[0] << " " << i << " " << j<<" "<<camera1.h<<" "<<camera1.w << endl);
				eriMat.at<Vec3b>(i, j) = convImage.at<Vec3b>((int)pp[1], (int)pp[0]);
			}
		}

	}

	
	imshow("sample", eriMat);
	img_write("./Image/checkerbigERI.PNG", eriMat);
	waitKey(10000);

}



void testforwardbackward()
{
	Mat eriPixels; 
	upload_image(IMAGE, eriPixels);  
	ERI eri(eriPixels.cols, eriPixels.rows); 
	int cfov=90;
	PPC camera1(cfov, cameraW, cameraH); 
	Mat convPixels = Mat::zeros(cameraH, cameraW, eriPixels.type()); 
	Mat convPixelsreverse = Mat::zeros(cameraH, cameraW, eriPixels.type()); 

	//camera1.Tilt(120);
	eri.ERI2Conv(eriPixels, convPixels, camera1);


	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 800, 400);
	imshow("sample", convPixels);


	//img_write("./Image/CONV_image.png", output_image_mat);// write an image
	eri.Conv2ERI(convPixels, convPixelsreverse, eriPixels, camera1);
	namedWindow("sample1", WINDOW_NORMAL);
	resizeWindow("sample1", 800, 400);

	imshow("sample1", convPixelsreverse);	
	waitKey(10000);
	
	
}

void img_write(const char *s1, cv::InputArray s2) {

	vector<int> compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
	string str1 ="\""+string(s1)+ "\"";
	imwrite(s1, s2, compression_params);
	waitKey(1);
}



void playstillmanually()
{
		Mat eriPixels; 
		upload_image(IMAGE, eriPixels);  
		ERI eri(eriPixels.cols, eriPixels.rows); 
		cout << eriPixels.type() << endl;
		STOP;
		PPC camera1(70.0f, 90.0f, 500); 
		Mat convPixels = Mat::zeros(cameraH, cameraW, eriPixels.type()); 
		
		
	for (int i = 0; i < 1500; i++)
	{
		namedWindow("sample", WINDOW_NORMAL);
		resizeWindow("sample", 800, 400);

		camera1.Pan(10);
		//V3 VD= camera1.GetVD();
		//float latt = eri.GetXYZ2Latitude(VD);
		//float longg = eri.GetXYZ2Longitude(VD);
		//cout << latt << "lat long" << longg << endl;
		eri.ERI2Conv(eriPixels, convPixels, camera1);
		cout << camera1.w << " " << camera1.h << endl;
		cout << convPixels.cols << " " << convPixels.rows << endl;
		//imshow("sample", convPixels);
		waitKey(1000);
		

	}
}
//*/



void check_interpolation() {
	
	PPC camera1(cFoV, cameraW, cameraH);
	PPC camera2(cFoV, cameraW, cameraH);
	camera1.Pan(90.0f);

	Mat source_image_mat;
	upload_image(IMAGE, source_image_mat);  //this function upload image of equirect form

	Mat output_image_mat = cv::Mat::zeros(cameraH, cameraW, source_image_mat.type());
	Mat output_image_mat_1 = cv::Mat::zeros(cameraH, cameraW, source_image_mat.type());
	ERI eri_image(source_image_mat.cols, source_image_mat.rows);
	eri_image.ERI2Conv(source_image_mat, output_image_mat, camera1);
	imshow("CONV_image", output_image_mat);
	waitKey(100);
	eri_image.ERI2Conv(source_image_mat, output_image_mat_1, camera2);
	imshow("CONV_image1", output_image_mat_1);
	waitKey(100);

	for (int i = 0; i < NUM_INTERP_frameN; i++)
	{
		
		//cout << i << endl;
		PPC interPPC;
		interPPC.SetInterpolated(&camera1,&camera2, i, NUM_INTERP_frameN);		
		eri_image.ERI2Conv(source_image_mat, output_image_mat, interPPC);
		imshow("CONV_imagex", output_image_mat);
		waitKey(10);
		
	}
	
}



int testPlayBackHMDPathStillImage()
{	
	Mat eriPixels; 
	char fname[] = "./Image/RollerCoasterFrame3to1.jpg";
	upload_image(fname, eriPixels);
	ERI eri(eriPixels.cols, eriPixels.rows);
	Path path1;
	PPC camera1(cFoV, cameraW, cameraH);
	Mat convPixels = cv::Mat::zeros(cameraH, cameraW, eriPixels.type());
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	path1.PlayBackPathStillImage(eriPixels, eri, convPixels);	
	return 0;

}



int testPlayBackHMDPathVideo()
{
	
	float hfov = 90.0f;
	float corePredictionMargin = 1;
	int w = 960;
	int h = 540;
	PPC camera1(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	Mat convPixels = cv::Mat::zeros(camera1.h, camera1.w, IMAGE_TYPE);	
	Path path1;
	int lastframe =100;
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	//path1.PlayBackPathVideo("./Video/roller.mkv", convPixels,lastframe);
	//path1.PlayBackPathVideo("http://128.10.120.229:80/RollerInput_1_10_0.avi", convPixels, lastframe);

	
	
	path1.PlayBackPathVideo("http://127.0.0.5:80/4thSecVar/rollerInput_1_10_40.avi", convPixels, lastframe);
	return 0;

}

int testPlayBackHMDPathVideoPixelInterval()
{
	PPC camera1(cFoV, cameraW, cameraH);
	Mat convPixels = cv::Mat::zeros(cameraH, cameraW, IMAGE_TYPE);
	Path path1;
	int lastframe = 1500;
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	path1.PlayBackPathVideoPixelInterval("./Video/roller_2000_1000.mp4", convPixels, lastframe);
	return 0;

}

int testViewDirectionAvgRotation() 
{
		PPC camera1(cFoV, cameraW, cameraH);
		Path path1;
		path1.LoadHMDTrackingData("./Video/diving.txt", camera1);
		path1.VDrotationAvg();
		return 0;
}


int Writeh264VideoUptoNframes() {

	Path path1;
	int lastframe = 900;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	path1.WriteH264("./Video/source/roller.mkv", lastframe, codec);
	path1.WriteH264("./Video/source/rhino.WEBM", lastframe, codec);
	path1.WriteH264("./Video/source/diving.mkv", lastframe, codec);

	return 0;
}



int testWriteh264tiles() {
	Path path1;
	int lastframe = 130;	
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	for (int i = 2; i < 20; i++)
	{
		path1.WriteH264tiles("./Video/source/diving.avi", i, 6, 4, codec);
	}
	
	
	return 0;

}

void testBoundingBox()
{
	
	Path path1;
	int lastFrame = 100;
	PPC camera1(110.0f, 800, 400);
	//cout << camera1.GetVD() << endl;
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	path1.DrawBoundinigBoxTemp("./video/roller.mkv", lastFrame);

}
	
void testbilinear()
{
	Path path1;
	Mat frame;	
	upload_image("./Image/check_rb5x5.png", frame);
	Mat outframe(10*frame.rows, 10*frame.cols, frame.type());

	for (int i = 0; i < outframe.rows; i++)
	{
		for (int j = 0; j < outframe.cols; j++)
		{
			float fi;
			float fj;
			fi = (float)(i * frame.rows)/ (float)outframe.rows;
			fj = (float)(j * frame.cols) / (float)outframe.cols;
			path1.bilinearinterpolation(outframe, frame, i, j, fi, fj);
			//outframe.at<Vec3b>(i, j) = frame.at<Vec3b>((int) fi, (int)fj);
		}

	}

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample",800,800);
	imshow("sample", outframe(Rect(outframe.rows/2-50,outframe.cols / 2 - 50,100,100)));
	waitKey(10000);

}

void getssim()
{
	int block_size = 10;
	compute_quality_metrics("./Image/test_conv_real.PNG", "./Image/test_conv_nl1.PNG", block_size);
	compute_quality_metrics("./Image/test_conv_real.PNG", "./Image/test_conv_ln1.PNG", block_size);

}

void testEncodingDecoding()
{
	int compressionfactor =10;
	Path path1;	
	Mat frame;
	Mat retencode;
	Mat retdecode;
	upload_image("./Image/eri_4k.jpg", frame);

	
	float hfov = 90.0f;
	float corePredictionMargin = 1;
	int w = 960;
	int h = 540;
	PPC corePPC(hfov*corePredictionMargin, w*corePredictionMargin , h*corePredictionMargin);
	PPC encodeRefPPC = corePPC;  //always next to corePPC before pan or tilt
	corePPC.Pan(50.0f);
	corePPC.Tilt(10.0f);

	PPC userPPC(hfov, w, h);
	userPPC.Pan(25.0f);
	userPPC.Tilt(20.0f);

	struct var encodevar;
	ERI eri(frame.cols,frame.rows);
	Vec3b linecolor(0, 0, 0);
	eri.ERI2ConvDrawBorderinERI(frame, corePPC, linecolor);
	Vec3b linecolor1(0, 100, 255);
	eri.ERI2ConvDrawBorderinERI(frame, userPPC, linecolor1);
	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 640, 360);
	imshow("sample", frame);	
	waitKey(1000);
	img_write("./Image/temp/test_original.PNG", frame);
	
	retencode = path1.EncodeNewNonLinV2(frame, &encodevar, corePPC, encodeRefPPC, compressionfactor);	
	//retencode = path1.EncodeLinear(frame, &encodevar, corePPC, compressionfactor);

	img_write("./Image/temp/test_encoded.PNG", retencode);	
	imshow("sample", retencode);
	waitKey(100);
	
	
	float var[10];
	var[0] = frame.cols;
	var[1] = frame.rows;
	var[2] = encodevar.We;
	var[3] = encodevar.Het;//*/	

	
	retdecode = path1.DecodeNewNonLinV2(retencode, var, compressionfactor, corePPC);
	//retdecode = path1.DecodeLinear(retencode, var, compressionfactor, corePPC);
	
	
	//imshow("sample",retdecode);
	//img_write("./Image/temp/test_decoded.PNG",retdecode);
	//waitKey(100);

	//Mat diff = diffimgage(frame, retdecode);
	//imshow("sample", diff);
	//img_write("./Image/test_diff.PNG", diff);
	//waitKey(100);

	Mat convPixels(userPPC.h, userPPC.w, frame.type());

	eri.ERI2Convtemp(retdecode, convPixels, userPPC);
	imshow("sample", convPixels);
	waitKey(100);
	img_write("./Image/temp/test_conv.PNG", convPixels);
/*
	eri.ERI2Conv(frame, convPixels, userPPC);
	imshow("sample", convPixels);
	waitKey(100);
	img_write("./Image/test_conv_real.PNG", convPixels);

*/

}


void testBugInSR(int singleOrVariableVD, int samplingValueCalculate, char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec)
{

	
		auto start = std::chrono::high_resolution_clock::now();
		vector <Mat> conv;
		vector <Mat> outputFrame2SAve;
		vector<Mat> outputSamplingRate2Save;
		struct samplingvar svar;
		Mat ret1;
		Path path1;
		float var[10];
		vector<float> frameRate4allFrames;
		
		float howManySecondsfor4thSec = extraSec;
		int extraFrameN = 5 * extraSec;
		int mxChunkN = 3;// var[4];
		int chunkD = 4;//var[5]; //read from variable	

		vector <float> nonUniformList;
		temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrameN + 1);  //n datapoint e n-1 gap hoy, ajonno +1 add koresi jate n ta gap e hoy

		ifstream  file("./Video/source/4kDivingEncodingVariable.txt");
		if (!file)
		{
			print("error: can't open file: " << filename << endl);
			system("pause");
		}
		string line;
		int i = 0;
		while (getline(file, line))
		{
			var[i] = stoi(line);
			cout << var[i] << '\n';
			i++;
		}

		int frameLen = var[0];
		int frameWidth = var[1];
		float hfov = 90.0f;
		float corePredictionMargin = 0.7;//use 0.6 for perfect timing, thats when we get 30 fps, otherwise 0.8
		int w = frameLen * hfov / 360; //add frameLen and Width into the variable file also totalChunkN
		int h = frameWidth * hfov / 360;  //540 for perfect 2160 p but here we have 2048
		
		PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
		PPC refCam(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
		path1.LoadHMDTrackingData(hmdFileName, camera2);
		int compressionFactor = 5;// var[6];  //read from variable file

		int frameStarted = 0;

		int chunkN = 0;
		int fps = 30;

		Mat firstScene;
		upload_image("./Video/source/startScene.PNG", firstScene);

		Mat endScene;
		int condition = 1;
		int cam_index = 0;
		int tiltAngle = 20;
		string filename;
		V3 VD;
		int frameCount = 0;
		int startIndex = 0;
		int after90Index = -1;
		std::vector<std::future<void>> futures;
		
		int ERI_w = frameLen;
		int ERI_h = frameWidth;
		ERI eri(ERI_w, ERI_h);
		vector <float>srVec;

		for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
		{
			vector <Mat> temp;
			for (int i = 0; i < (fps * chunkD + extraFrameN); i++)
			{
				Mat m;
				temp.push_back(m);
				
			}
			frameQvec.push_back(temp);
		}

		eri.atanvalue();
		eri.xz2LonMap();
		eri.xz2LatMap();
		path1.nonUniformListInit(var);
		path1.mapx(var);
		//path1.calculateAllSamplingRateOverCreri(var); //remove it final time;//ch

		cout << "Very first Chunk, no frame yet......" << endl;

				
		VD = camera2.GetVD();
		cout << "first VD: " << VD << endl;
		chunkN++;
		M33 reriCS;
		M33 reriCScopy;
		filename = getChunkNametoReqnRefCamOptimized(refCam, reriCS, srcBaseAddr, chunkN, VD, tiltAngle);
		cout << "ReqF=" << filename << endl;
		NextChunkDownloaded = 0;
		auto finish1 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed1 = finish1 - start;
		int bwTimeNeeded = 20; //500 ms default value, without any trace
		auto playStart = std::chrono::high_resolution_clock::now();

		DownLoadChunk4thSecVar(filename, chunkD, fps, extraFrameN, bwTimeNeeded, chunkN);
		int returningFromExtra = 0;
		while (NextChunkDownloaded == 0)
		{
			//displayImage(firstScene);
		}
		Mat convPixels(camera2.h, camera2.w, firstScene.type());
		Mat samplingPixels(camera2.h, camera2.w, firstScene.type());
		//cout << camera2.h << " cam " << camera2.w << endl;
		frameCount = -1;

		int firstPlayTime;
		while (condition)
		{
			cout << "loop" << endl;

			frameCount++; //0->
			if (frameCount == (fps * chunkD * mxChunkN - 1 + 0)) //add 449 for last 5 second extension
			{
				condition = 0;
			}

			int currentChunk = (frameCount) / (fps * chunkD) + 1;
			if (currentChunk > mxChunkN + 1)
			{
				currentChunk = mxChunkN + 1;
			}
			int playIndex = frameCount - (currentChunk - 1) * (fps * chunkD); //0-(fps*chunkD-1)
			cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex << " " << NextChunkDownloaded << endl;
			//cout << frameCount << endl;

			if (playIndex == 30 && currentChunk > 1)
			{
				vector <Mat> temp;
				for (int i = 0; i < 2; i++)
				{
					Mat m;
					temp.push_back(m);
					m.release();
				}
				frameQvec[currentChunk - 1] = temp;
				cout << "memory erased..................." << endl;
			}
			if (playIndex ==0 && currentChunk > 1)
			{
				reriCS = reriCScopy;
				cout << "reriCS updated" << endl;
			}

			if (currentChunk <= NextChunkDownloaded)
			{
				Mat frame;
				frame = frameQvec[currentChunk][playIndex];

				after90Index = -1;

				cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

				if (samplingValueCalculate == 1)
				{
					if (frameCount < 1)
					{
						auto firstFrame = std::chrono::high_resolution_clock::now();
						std::chrono::duration<double> elapsedFirst = firstFrame - start;
						firstPlayTime = elapsedFirst.count() * 1000;
						playStart = std::chrono::high_resolution_clock::now();

					}
					camera2.Pan(1.0f);
					//cout << "Second VD: " << camera2.GetVD() << endl;

					path1.CRERI2ConvOptimizedWithSamplingRateVec(frame, var, eri, reriCS, convPixels, srVec, compressionFactor, camera2, refCam);					
					frameRate4allFrames.push_back(1);

				}
				else {
					camera2.Pan(1.0f);
					cout << "Second VD: " << camera2.GetVD() << endl;
					path1.CRERI2ConvOptimized(frame, var, eri, reriCS, convPixels, compressionFactor, camera2, refCam);

				}

				//displayImage(convPixels);	
				displayImage33ms();
				outputFrame2SAve.push_back(convPixels.clone());

				
				if ((playIndex) == fps * (chunkD - nextDlChunkSec))
				{
					int reqChunkN = currentChunk + 1;

					cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
					VD = camera2.GetVD();
					 reriCScopy = reriCS;
					filename = getChunkNametoReqnRefCamOptimized(refCam, reriCScopy, srcBaseAddr, reqChunkN, VD, tiltAngle);
					cout << "New Req. " << filename << endl;
					finish1 = std::chrono::high_resolution_clock::now();
					elapsed1 = finish1 - start;
	
					futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));

				}
				

			}

			if (after90Index >= extraFrameN)
			{
				cout << "Network disconnected, try again...................................." << endl;
				auto playFinish = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> playElapsed = playFinish - playStart;
				cout << "Total Play Time:................> " << playElapsed.count() * 1000 << endl;
				condition = 0;

			}

			if (((currentChunk) > NextChunkDownloaded) && currentChunk > 1)
			{
				waitKey(1000);
							/*
				after90Index++; //0->
				cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
				int index = nonUniformList[after90Index];
				int index2 = nonUniformList[after90Index + 1];

				int diffIndex = index2 - index;
				if (diffIndex > 100 || diffIndex < 0)
				{
					condition = 0;
					break;
				}
				for (int i = 0; i < diffIndex; i++)
				{
					if (i > 0) {
						frameCount++;
					}
					currentChunk = (frameCount) / (fps * chunkD) + 1; //current chunk 0 theke start, file chunk o 0 theke start krote hobe encoding er somoy
					playIndex = frameCount - (currentChunk - 1) * (fps * chunkD);
					cout << "119CuntChunkN=" << currentChunk - 1 << " frmcount=" << frameCount << " plCnt=" << playIndex << " after90indx=" << index << " " << NextChunkDownloaded << endl;
					//	cout << frameCount << endl;
					cam_index = path1.GetCamIndex(frameCount - 1, fps, cam_index);
					Mat frame1;
					frame1 = frameQvec[currentChunk - 1][(fps * chunkD - 1) + after90Index];
					if (samplingValueCalculate == 1)
					{
						camera2.Pan(1.0f);
						//cout << "Second VD: " << camera2.GetVD() << endl;
						path1.CRERI2ConvOptimizedWithSamplingRateVec(frame1, var, eri, reriCS, convPixels, srVec, compressionFactor, camera2, refCam);
						
						float frameRateTemp = diffIndex;
						frameRate4allFrames.push_back(frameRateTemp);
					}
					else {
						camera2.Pan(1.0f);
						cout << "Second VD: " << camera2.GetVD() << endl;
						path1.CRERI2ConvOptimized(frame1, var, eri, reriCS, convPixels, compressionFactor, camera2, refCam);

					}

					//cout<<frameQvec[currentChunk - 1].size() << endl;

					//displayImage(convPixels);
					displayImage33ms();
					outputFrame2SAve.push_back(convPixels.clone());
					if (playIndex == (fps * (chunkD - nextDlChunkSec)))
					{
						int reqChunkN = currentChunk + 1; //amader chunk 0 theke start hoy but download chunk start hoy 1 theke , apatoto			

						cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
						VD = camera2.GetVD();
						cout << "Second req VD: " << VD << endl;
						filename = getChunkNametoReqnRefCamOptimized(refCam, reriCS, srcBaseAddr, reqChunkN, VD, tiltAngle);
						cout << "NewR: " << filename << endl;
						finish1 = std::chrono::high_resolution_clock::now();
						elapsed1 = finish1 - start;
						
						futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));


					}

			
				}      
				
				*/  

			}


	}

		auto playFinishAll = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> playElapsed = playFinishAll - playStart;
		cout << "FPS:................> " << playElapsed.count() * 1000 / frameCount << endl;
		cout << "Frist frame Appear Time=..............>" << firstPlayTime << endl;
		vector<int>frameWiseMin;
		vector<int>frameWiseMax;
		vector<int>frameWiseAvg;

		if (samplingValueCalculate == 1)
		{

			float minTemp = 100;
			float totalTemp = 0;
			for (int i = 0; i < frameRate4allFrames.size(); i++)
			{
				float tempValue = 30 / frameRate4allFrames[i];
				totalTemp = totalTemp + tempValue;
				//cout << tempValue << endl;
				if (tempValue < minTemp)
				{
					minTemp = tempValue;
				}
			}

			cout << "Minimum Frame Rate: " << minTemp << " averageFrameRate: " << totalTemp / frameRate4allFrames.size() << endl;


			float overallMin = 10;
			float overallMax = 0;
			float ovrallAvgTotal = 0;
			double aggValue = 0;
			double value = 0;
			float min = 1000;
			float max = 0;
			float avg = 0;
			for (int i = 0; i < srVec.size(); i++)
			{											
				value = srVec[i];
				aggValue = value + aggValue;		
				cout<<"frame: " <<i<< " SR value: "<<value<< endl;
				if (value > max)
				{
					max = value;
				}
				if (value < min)
				{
					min = value;
				}
					
				
				if (min < overallMin)
				{
					overallMin = min;
				}
				if (max > overallMax)
				{
					overallMax = max;
				}
				avg = aggValue / (srVec.size());			

			}
			ofstream output;
			output.open("./Video/source/srBugTest.txt", std::ios::out | std::ios::app);
			output << srcBaseAddr << ";" <<overallMax << ";" << overallMin << " ;"<< avg <<endl;
			output.close();
			cout << "overall Pixel Sampling RAte: " << "min-> " << overallMin << " max-> " << overallMax << " avg->" << avg << endl;
		}

}

void GenerateEncoding4sVarSpecificPanTiltChunk()
{	
	
	
float tilt = 10;
float pan = 40;
string fileName;
string chunkN;
ifstream  file("./Video/source/AllChunKname.txt");
if (!file)
{
	print("error: can't open file: " << filename << endl);
	system("pause");
}
string   line;
while (getline(file, line))
{
	stringstream  linestream(line);

	string lString, lString2;

	linestream >> lString;

	ostringstream oss2;
	oss2 << "http://127.0.0.5:80/3xid2crf3trace/1s/4s1s/" << lString << endl;
	lString = oss2.str();
	lString2 = lString;
	cout << lString << endl;
	
	

	unsigned first = lString.find("1s/");
	unsigned last = lString.find("_");
	string strNew = lString.substr(first, last - first);
	first = strNew.find("1s/");
	last = strNew.find("CR");
	strNew = strNew.substr(first, last - first);
	first = strNew.find("1s/");
	last = strNew.find("_");
	strNew = strNew.substr(first, last - first);
	strNew = (strNew.substr(8, strNew.length() - 8)).c_str();

	ostringstream oss1;
	oss1 << "\"" << "./Video/source/" << strNew << ".AVI" << "\"" << endl;
	cout << oss1.str() << endl;
	fileName = oss1.str();

	string strTilt;
	first = lString2.find("_");
	last = lString2.find(".");
	strTilt = lString2.substr(first, last - first);
	first = strTilt.find("_");
	last = strTilt.find(".");
	strTilt = strTilt.substr(first, last - first);


	if (strTilt[0] == '_')
	{
		strTilt.erase(0, 1);
	}
	chunkN = strTilt.substr(0, 3);
	first = chunkN.find("_");
	chunkN = chunkN.substr(0, first);
	cout << chunkN << endl;
	strTilt = strTilt.erase(0, 2);
	if (strTilt[0] == '_')
	{
		strTilt.erase(0, 1);
	}
	first = strTilt.find("_");

	string newTilt = strTilt.substr(0, first);
	string newPan = strTilt.substr(first + 1, strTilt.length() - first - 1);
	cout << strTilt << " " << newTilt << " " << newPan << endl;

	pan = stoi(newPan);
	tilt = stoi(newTilt);

	//.............

		char* fileName4s = "./Video/source/4sBase/diving";
		char* fileName1s = "./Video/source/extraSec/diving.AVI6";
		char* fileName2s = "./Video/source/extraSec/diving.AVI8";
		char* fileName3s = "./Video/source/extraSec/diving.AVI10"; 
		std::ostringstream oss11,oss22,oss3,oss4;
		

		oss11 << fileName4s << "_" << chunkN << "_" << tilt << "_" << pan << ".avi";
		oss22 << fileName1s << "_" << chunkN << "_" << tilt << "_" << pan << ".avi";
		oss3 << fileName2s << "_" << chunkN << "_" << tilt << "_" << pan << ".avi";
		oss4 << fileName3s << "_" << chunkN << "_" << tilt << "_" << pan << ".avi";	
		string tmp1 = oss11.str(); const char* Fn1 = tmp1.c_str();
		string tmp2 = oss22.str(); const char* Fn2 = tmp2.c_str();
		string tmp3 = oss3.str(); const char* Fn3 = tmp3.c_str();
		string tmp4 = oss4.str(); const char* Fn4 = tmp4.c_str();

	video4sBaseAndExtraSec((char*)Fn1, (char*)Fn2, (char*)Fn3, (char*)Fn4);




	///--------
	/*
		const char* c = fileName.c_str();
	
		int extraFrame;
		int extraSec;
	
		char* fileNamex = "./Video/source/diving.AVI";
		if (strchr(fileName.c_str(), 'v'))
		{
			fileNamex = "./Video/source/diving.AVI";
		
		}
		else if (strchr(fileName.c_str(), 'l'))
		{
			fileNamex = "./Video/source/roller.MKV";
		
		}
		else {
			fileNamex = "./Video/source/rhino.AVI";
		
		}
			for (int i = 0; i < 3; i++)
			{
				if (i == 0)
				{
					extraFrame = 30;
					extraSec = 6;
				}
				if (i == 1)
				{
					extraFrame = 40;
					extraSec = 8;
				}
				if (i == 2)
				{
					extraFrame = 50;
					extraSec = 10;

				}
			
				cout << fileNamex << endl;
					
				makeVideo4thSecVarSpecificPanTiltChunkNvariableLastSection(fileNamex, extraSec, pan, tilt, stoi(chunkN), extraFrame, extraSec);
			}
			*/
	
	}	
}




void GenerateEncodingRegularSpecificPanTiltChunk()
{
	//makeVideoRegularSpecificPanTiltChunkNAllSameFrame("./video/source/diving.AVI", 20, 10, 2, 400);
	//makeVideoRegularSpecificPanTiltChunkNAllSameFrame("./video/source/diving.AVI", 40, 10, 3, 400);
	//makeVideoRegularSpecificPanTiltChunkNAllSameFrame("./video/source/diving.AVI", 120, 10, 3, 400);
	STOP;

	float tilt = 10;
	float pan = 40;
	string fileName;
	string chunkN;
	ifstream  file("./Video/source/chunkNameCopy.txt");
	if (!file)
	{
		print("error: can't open file: " << filename << endl);
		system("pause");
	}
	string   line;
	while (getline(file, line))
	{
		stringstream  linestream(line);
		
		string lString, lString2;
		
		linestream >> lString;
		lString2 = lString;
		cout << lString << endl;

		unsigned first = lString.find("1s/");
		unsigned last = lString.find("_");
		string strNew = lString.substr(first, last - first);
		first = strNew.find("1s/");
		last = strNew.find("CR");
		strNew = strNew.substr(first, last - first);
		first = strNew.find("1s/");
		last = strNew.find("_");
		strNew = strNew.substr(first, last - first);
		strNew = (strNew.substr(8, strNew.length() - 8)).c_str();
		
		ostringstream oss1;
		oss1 <<"\""<< "./Video/source/" << strNew <<".AVI"<<"\""<< endl;
		cout << oss1.str() << endl;
		fileName = oss1.str();
		
		string strTilt;
		first = lString2.find("_");
		last = lString2.find(".");
		strTilt = lString2.substr(first, last - first);
		first = strTilt.find("_");
		last = strTilt.find(".");
		strTilt = strTilt.substr(first, last - first);
		
		
		if (strTilt[0]=='_')
		{
			strTilt.erase(0, 1);
		}
		chunkN = strTilt.substr(0, 3);
		first = chunkN.find("_");
		chunkN = chunkN.substr(0,first);
		cout << chunkN << endl;
		strTilt=strTilt.erase(0, 2);
		if (strTilt[0] == '_')
		{
			strTilt.erase(0, 1);
		}
		first = strTilt.find("_");
	
		string newTilt = strTilt.substr(0, first);
		string newPan = strTilt.substr(first+1, strTilt.length() - first-1);
		cout << strTilt <<" "<<newTilt<<" "<<newPan<< endl;
		
		pan = stoi(newPan);
		tilt = stoi(newTilt);

		
	
		const char* c = fileName.c_str();

		makeVideoRegularSpecificPanTiltChunkN("./video/source/diving.AVI", pan, tilt, stoi(chunkN),400);
		
		
		
	}
		
	

}

void makeVideoRegularSpecificPanTiltChunkNAllSameFrame(string fileName, float pan, float tilt, int chunkN, int beginFrame)
{
	cout << fileName << " " << tilt << " " << pan << endl;
	int extraFrame = 15;
	vector<Mat> downloadedFrame;
	vector <Mat> bufferedFrame;
	struct var encodevar;
	Path path1;
	float hfov = 90.0f;
	float corePredictionMargin = 1.2;
	int w = 960;
	int h = 512;
	PPC encodeRefPPC(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);  //always next to corePPC before pan or tilt
	PPC core1PPC(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	int cf = 5;
	VideoCapture cap(fileName);

	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fileName << endl;
		STOP;
	}
	int fps = 30;
	int segi = 0;
	Mat ret;
	float var[10];
	int chunDurationsec = 4;
	Mat frame;
	int discard = beginFrame;

	for (int fi = 0; fi < (chunkN)*fps * chunDurationsec + discard; fi++)
	{
		//cout << "loop:->" << endl;
		cap >> frame;
		if (frame.empty())
		{

			cout << "Can not read video frame: " << endl;
			break;
		}
		if (fi >= (chunkN - 1) * chunDurationsec * fps + discard && fi < (discard + chunkN * chunDurationsec * fps))
		{
			print("enFi: " << fi << endl);
			downloadedFrame.push_back(frame);
		}
		frame.release();
	}

	
	float addI;
	core1PPC.Pan(pan);
	core1PPC.Tilt(tilt);
	
	frame = downloadedFrame[1];
	ret = path1.EncodeNewNonLinV2(frame, &encodevar, core1PPC, encodeRefPPC, cf); //for all possible angle encodinig
	
	for (int i = 0; i < fps*chunDurationsec+extraFrame; i++)
	{
		bufferedFrame.push_back(ret);
	}
	
	
	ret.release();
	
	
	int sf = bufferedFrame.size() - chunDurationsec * fps - extraFrame;
	int ef = bufferedFrame.size();
	cout << "SAving-> sf:ef " << sf << " " << ef << endl;
	char* fn;
	if (discard == 400)
	{
		fn = "./Video/source/diving_";

	}
	if (discard == 1)
	{
		fn = "./Video/source/roller_";

	}
	if (discard == 100)
	{
		fn = "./Video/source/rhino_";

	}
		
	videowriterhelperxWithFileName(fn, chunkN, (int)pan, (int)tilt, fps, bufferedFrame[1].cols, bufferedFrame[1].rows, sf, ef, bufferedFrame);
	

}


void makeVideoRegularSpecificPanTiltChunkN(string fileName, float pan, float tilt, int chunkN, int beginFrame)
{
	cout << fileName << " " << tilt << " " << pan << endl;
	vector<Mat> downloadedFrame;
	vector <Mat> bufferedFrame;
	struct var encodevar;
	Path path1;
	float hfov = 90.0f;
	float corePredictionMargin = 1.2;
	int w = 960;
	int h = 512;
	PPC encodeRefPPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);  //always next to corePPC before pan or tilt
	PPC core1PPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	int cf = 5;
	VideoCapture cap(fileName);

	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fileName<<endl;
		STOP;
	}
	int fps = 30;
	int segi = 0;
	Mat ret;
	float var[10];
	int chunDurationsec = 4;
	Mat frame;
	int discard = beginFrame;

	for (int fi = 0; fi < (chunkN)*fps*chunDurationsec + discard; fi++)
	{
		//cout << "loop:->" << endl;
		cap >> frame;
		if (frame.empty())
		{

			cout << "Can not read video frame: " << endl;
			break;
		}
		if (fi >= (chunkN - 1)*chunDurationsec*fps + discard && fi < (discard + chunkN * chunDurationsec*fps))
		{
			print("enFi: " << fi << endl);
			//2displayImage(frame);
			downloadedFrame.push_back(frame);
		}
		frame.release();
	}



	float addI;
	core1PPC.Pan(pan);
	core1PPC.Tilt(tilt);

	for (int fi = 0; fi < chunDurationsec*fps; fi++)
	{
		cout << " fi: " << fi << endl;

		frame = downloadedFrame[fi];
		ret = path1.EncodeNewNonLinV2(frame, &encodevar, core1PPC, encodeRefPPC, cf); //for all possible angle encodinig
		bufferedFrame.push_back(ret);
	//	displayImage(ret);
		ret.release();
		if ((fi + 1) % (chunDurationsec*fps) == 0 && fi > 0)
		{			
			cout << chunkN << endl;
			int sf = bufferedFrame.size() - chunDurationsec * fps;
			int ef = bufferedFrame.size();
			cout << "SAving-> sf:ef " << sf << " " << ef << endl;
			char *fn;
			if (discard==400)
			{
				fn = "./Video/source/diving_";

			}
			if (discard == 1)
			{
				fn = "./Video/source/roller_";

			}
			if (discard == 100)
			{
				fn = "./Video/source/rhino_";

			}


			
			//cout << fps << ret.cols << ret.rows << " sf: " << sf << " ef: " << ef << endl;
			//videowriterhelperx(chunkN, (int)pan, (int)tilt, fps, bufferedFrame[1].cols, bufferedFrame[1].rows, sf, ef, bufferedFrame);
			videowriterhelperxWithFileName(fn, chunkN, (int)pan, (int)tilt, fps, bufferedFrame[1].cols, bufferedFrame[1].rows, sf, ef, bufferedFrame);

		}

		
	}
	
}

void makeVideo4thSecVarSpecificPanTiltChunkN(char* fileName,float pan, float tilt, int chunkN, int extraFrame, int howManySecondsfor4thSec)
{
	vector<Mat> downloadedFrame;
	vector <Mat> bufferedFrame;
	struct var encodevar;
	Path path1;
	float hfov = 90.0f;
	float corePredictionMargin = 1.2;
	int w = 960;
	int h = 512;
	PPC encodeRefPPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);  //always next to corePPC before pan or tilt
	PPC core1PPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	int cf = 5;
	VideoCapture cap(fileName);

	vector <float> nonUniformList;
	//float howManySecondsfor4thSec = 5;//5
	//int extraFrame = 30; //

	temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrame);

	
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		STOP;
	}
	int fps = 30;
	int lastframe = 910;
	int segi = 0;
	Mat ret;
	float var[10];
	int chunDurationsec = 4;
	Mat frame;
	for (int fi = 0; fi < (chunkN)*fps*chunkN*chunDurationsec+ nonUniformList[extraFrame-1]; fi++)
	{
		cap >> frame;
		if (frame.empty())
		{
			
			cout << "Can not read video frame: " << endl;
			break;
		}
		if (fi >= (chunkN - 1)*chunDurationsec*fps && fi <(chunkN*chunDurationsec*fps+nonUniformList[extraFrame - 1]))
		{
			print("enFi: " << fi << endl);
			displayImage(frame);
			downloadedFrame.push_back(frame);
		}
		frame.release();
	}

	

	float addI;
	core1PPC.Pan(pan);
	core1PPC.Tilt(tilt);

	for (int fi = 0; fi <chunDurationsec*fps; fi++)
	{
		cout <<" fi: " <<fi << endl;
	
		frame = downloadedFrame[fi];		
		ret = path1.EncodeNewNonLinV2(frame, &encodevar, core1PPC, encodeRefPPC, cf); //for all possible angle encodinig
		bufferedFrame.push_back(ret);
		ret.release();
		if ((fi + 1) % (chunDurationsec*fps) == 0 && fi > 0)
		{
			for (int i = 1; i < extraFrame+1; i++)
			{
				addI = fi + (int)nonUniformList[i - 1];				
				ret = path1.EncodeNewNonLinV2(downloadedFrame[addI], &encodevar, core1PPC, encodeRefPPC, cf);							
				bufferedFrame.push_back(ret);
				ret.release();
				cout << "; fi-" << fi << " addI-" << addI << endl;
			}			
			cout << chunkN << endl;
			int sf = bufferedFrame.size() - (fps*chunDurationsec+extraFrame);
			int ef = bufferedFrame.size();
			cout <<"SAving-> sf:ef "<< sf << " " << ef << endl;
			
			//cout << fps << ret.cols << ret.rows << " sf: " << sf << " ef: " << ef << endl;
			videowriterhelperxWithFileName(fileName, chunkN, (int)pan, (int)tilt, fps, bufferedFrame[1].cols, bufferedFrame[1].rows, sf, ef, bufferedFrame);

		}

		if (fi == 2)
		{
			var[0] = encodevar.colN;
			var[1] = encodevar.rowN;
			var[2] = encodevar.We;
			var[3] = encodevar.Het;
		}
	}
	ofstream output("./Video/encodingtest/4thSec/diving/diving_encoding_variable.txt");
	output << var[0] << endl;
	output << var[1] << endl;
	output << var[2] << endl;
	output << var[3] << endl;
	output.close();
}

void makeVideo4thSecVarSpecificPanTiltChunkNvariableLastSection(char* fileName, int extraSec, float pan, float tilt, int chunkN, int extraFrame, int howManySecondsfor4thSec)
{
	vector<Mat> downloadedFrame;
	vector <Mat> bufferedFrame;
	struct var encodevar;
	Path path1;
	float hfov = 90.0f;
	float corePredictionMargin = 1.2;
	int w = 960;
	int h = 512;
	PPC encodeRefPPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);  //always next to corePPC before pan or tilt
	PPC core1PPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	int cf = 5;
	VideoCapture cap(fileName);
	vector <float> nonUniformList;
	temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrame);
	int extraAdd4Diving = 0;
	cout << fileName << endl;
	if (strchr(fileName, 'g'))
	{
		extraAdd4Diving =400;
		cout << "extra 400" << endl;
	}
	else if (strchr(fileName, 'l'))
	{
		extraAdd4Diving = 1;
		cout << "extra 1" << endl;
	}
	else
	{
		extraAdd4Diving = 100;
		cout << "extra 100" << endl;
	}

	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		STOP;
	}
	int fps = 30;	
	int segi = 0;
	Mat ret;
	float var[10];
	int chunDurationsec = 4;
	Mat frame;
	for (int fi = 0; fi < (chunkN)*fps*chunDurationsec + nonUniformList[extraFrame - 1]+extraAdd4Diving; fi++)
	{
		//cout << "loop" << endl;
		cap >> frame;
		if (frame.empty())
		{

			cout << "Can not read video frame: " << endl;
			break;
		}
		if (fi >= (chunkN - 1)*chunDurationsec*fps+extraAdd4Diving && fi < (extraAdd4Diving+chunkN*chunDurationsec*fps + nonUniformList[extraFrame - 1]))
		{
			//print("enFi: " << fi << endl);
			//displayImage(frame);
			downloadedFrame.push_back(frame);
		}
		//frame.release();
	}
	
	cout << "in between" << endl;

	float addI;
	core1PPC.Pan(pan);
	core1PPC.Tilt(tilt);

	for (int fi = 0; fi < chunDurationsec*fps; fi++)
	{
		//cout << " fi: " << fi << endl;

		if ((fi + 1) % (chunDurationsec*fps) == 0 && fi > 0)
		{
			for (int i = 1; i < extraFrame + 1; i++)
			{
				addI = fi + (int)nonUniformList[i - 1];
				ret = path1.EncodeNewNonLinV2(downloadedFrame[addI], &encodevar, core1PPC, encodeRefPPC, cf);
				bufferedFrame.push_back(ret);
				//displayImage(ret);
				//ret.release();
				
				cout << "; fi-" << fi << " addI-" << addI << endl;
			}
			cout << chunkN << endl;
			int sf = bufferedFrame.size() - (extraFrame);
			int ef = bufferedFrame.size();
			cout << "SAving-> sf:ef " << sf << " " << ef << endl;			
			videowriterhelperxWithFileNameWithExtraSec(fileName, chunkN, extraSec, (int)pan, (int)tilt, fps, bufferedFrame[1].cols, bufferedFrame[1].rows, sf, ef, bufferedFrame);

		}


	}

}

void run10simulations()
{
	for (int simulation = 0; simulation < 1; simulation++)//dl//extraSec
	{
		char* bwLog = "./bwLogs/bw646kB.txt";
		char* videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf40/diving";
		char* hmdTrace = "./Video/source/diving.txt";

		for (int bw = 0; bw < 2; bw++)
		{
			if (bw == 0)
			{
				bwLog = "./bwLogs/bw646kB.txt";
			}
			else
			{
				bwLog = "./bwLogs/bw1962kB.txt";

			}
			for (int extraSec = 1; extraSec < 4; extraSec++)
			{
				for (int reqTime = 1; reqTime < 5; reqTime++)
				{
					for (int crF = 0; crF < 3; crF++)
					{

						for (int videoX = 0; videoX < 3; videoX++)
						{

							if (videoX == 0)
							{

								hmdTrace = "./Video/source/diving.txt";
								if (extraSec == 1 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf30/diving";
								}
								if (extraSec == 1 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf40/diving";
								}
								if (extraSec == 1 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf50/diving";
								}
								if (extraSec == 2 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf30/diving";
								}
								if (extraSec == 2 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf40/diving";
								}
								if (extraSec == 2 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf50/diving";
								}
								if (extraSec == 3 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving";
								}
								if (extraSec == 3 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf40/diving";
								}
								if (extraSec == 3 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf50/diving";
								}
								if (extraSec == 6 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf30/diving";
								}
								if (extraSec == 6 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf40/diving";
								}
								if (extraSec == 6 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf50/diving";
								}

							}
							if (videoX == 1)
							{
								hmdTrace = "./Video/source/roller.txt";
								if (extraSec == 1 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf30/roller";
								}
								if (extraSec == 1 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf40/roller";
								}
								if (extraSec == 1 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf50/roller";
								}
								if (extraSec == 2 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf30/roller";
								}
								if (extraSec == 2 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf40/roller";
								}
								if (extraSec == 2 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf50/roller";
								}
								if (extraSec == 3 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/roller";
								}
								if (extraSec == 3 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf40/roller";
								}
								if (extraSec == 3 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf50/roller";
								}
								if (extraSec == 6 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf30/roller";
								}
								if (extraSec == 6 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf40/roller";
								}
								if (extraSec == 6 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf50/roller";
								}
							}
							if (videoX == 20)
							{
								hmdTrace = "./Video/source/rhino.txt";
								if (extraSec == 1 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf30/rhino";
								}
								if (extraSec == 1 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf40/rhino";
								}
								if (extraSec == 1 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf50/rhino";
								}
								if (extraSec == 2 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf30/rhino";
								}
								if (extraSec == 2 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf40/rhino";
								}
								if (extraSec == 2 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf50/rhino";
								}
								if (extraSec == 3 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/rhino";
								}
								if (extraSec == 3 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf40/rhino";
								}
								if (extraSec == 3 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf50/rhino";
								}
								if (extraSec == 6 & crF == 0)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf30/rhino";
								}
								if (extraSec == 6 & crF == 1)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf40/rhino";
								}
								if (extraSec == 6 & crF == 2)
								{
									videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf50/rhino";
								}

							}

							testDownloadVideoHttp4thSecVar(1, 1, videoFile, bwLog, hmdTrace, reqTime, extraSec);
						}
					}
				}
			}

		}




	}

	   
}

void testCPUtime(string fileName)
{
	struct samplingvar svar;
	vector <Mat> outputFrames;	
	vector <Mat> creriFrames;
	Mat ret1;
	Path path1;
	float var[10];
	ifstream  f("./Video/source/4kDivingEncodingVariable.txt");
	std::string line;
	int i = 0;
	string srcBaseAddr = "http://127.0.0.5:80/3vid2crf3trace/1s/4s1s/roller";
	while (std::getline(f, line))
	{
		var[i] = stoi(line);
		cout << var[i] << '\n';
		i++;
	}

	int frameLen = var[0];
	int frameWidth = var[1];
	float hfov = 90.0f;
	float corePredictionMargin = 0.6;
	int w = frameLen * hfov / 360; 
	int h = frameWidth * hfov / 360;  

	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	PPC refCam(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);

	char* hmdFileName;
	hmdFileName = "./Video/source/diving.txt";
	path1.LoadHMDTrackingData(hmdFileName, camera2);

	int compressionFactor = 5;// var[6];  //read from variable file	
	int chunkN = 1;
	int fps = 30;
	
	Mat firstScene;
	upload_image("./Video/source/startScene.PNG", firstScene);
	Mat endScene;
	int condition = 1;
	int cam_index = 0;
	int tiltAngle = 20;
	string filename;
	V3 VD;
	int frameCount = 0;
	

	M33 reriCS;
	int ERI_w = frameLen;
	int ERI_h = frameWidth;
	ERI eri(ERI_w, ERI_h);
	eri.atanvalue();
	eri.xz2LonMap();
	eri.xz2LatMap();
	path1.nonUniformListInit(var);
	path1.mapx(var);
	path1.calculateAllSamplingRateOverCreri(var);

	Mat convPixels(camera2.h, camera2.w, firstScene.type());
	float decodeTime;
	float creriTime;
	
	VideoCapture cap(fileName.c_str());
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		waitKey(100000);
	}

	int lastframe = 720;	
	int segi = 0;
	Mat ret; 
	filename = getChunkNametoReqnRefCamOptimized(refCam, reriCS, srcBaseAddr, chunkN, VD, tiltAngle);
	auto playS = chrono::high_resolution_clock::now();
	
	for (int fi = 0; fi < 120; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			STOP;
		}
		creriFrames.push_back(frame);	

	}

	auto playFinish = chrono::high_resolution_clock::now();
	chrono::duration<double> playElapsed = playFinish - playS;
	decodeTime = playElapsed.count();
	cout << "H264 decode Time:................> " << playElapsed.count() * 1000 << endl;
	playS= chrono::high_resolution_clock::now();

	for (int fi = 0; fi < 120; fi++)
	{
		frameCount = fi;
		cam_index = 5; //path1.GetCamIndex(frameCount, fps, cam_index);
		VD = path1.cams[cam_index].GetVD();
		path1.CRERI2ConvOptimized(creriFrames[fi], var, eri, reriCS, convPixels, compressionFactor, path1.cams[cam_index], refCam);
		//displayImage(convPixels);
	}
	   	

	playFinish = chrono::high_resolution_clock::now();
	playElapsed = playFinish - playS;
	creriTime = playElapsed.count();
	cout << "CRERI decode Time:................> " << playElapsed.count() * 1000 << endl;
	ofstream output;
	output.open("./Video/source/cpuTime.txt", std::ios::out | std::ios::app);
	output << fileName<<";" <<  decodeTime <<";"<<creriTime<< endl;
	output.close();
}

void testCPUtimeTiles(string fileName) 
{

	struct samplingvar svar;
	vector <Mat> outputFrames;	Mat ret; Mat ret1;  vector <Mat> tileFrames;	
	Path path1;
	float var[10];
	ifstream  f("./Video/source/4kDivingEncodingVariable.txt");
	std::string line;
	int i = 0;
	
	while (std::getline(f, line))
	{
		var[i] = stoi(line);
		cout << var[i] << '\n';
		i++;
	}
	int frameCol = var[0];	int frameRow = var[1];
	int frameLen = var[0];	int frameWidth = var[1];	float hfov = 90.0f;	float corePredictionMargin = 0.6;
	int w = frameLen * hfov / 360;	int h = frameWidth * hfov / 360;

	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	PPC refCam(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);

	char* hmdFileName;	hmdFileName = "./Video/source/diving.txt";
	path1.LoadHMDTrackingData(hmdFileName, camera2);

	int compressionFactor = 5;	int chunkN = 1; int chunkD = 4; int fps = 30;

	Mat firstScene;	upload_image("./Video/source/startScene.PNG", firstScene); Mat convPixels(camera2.h, camera2.w, firstScene.type());
	
	int condition = 1;	int cam_index = 0;	int tiltAngle = 20;
	string filename;
	V3 VD;	int frameCount = 0;

	M33 reriCS;
	int ERI_w = frameLen;	int ERI_h = frameWidth;	ERI eri(ERI_w, ERI_h);
	eri.atanvalue();	eri.xz2LonMap();	eri.xz2LatMap();	path1.nonUniformListInit(var);	path1.mapx(var);

		
	float decodeTime;	float creriTime;

	VideoCapture cap("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/diving.avi_1_9.avi");
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fileName<<endl;
		waitKey(100000);
	}
	
	auto playS = chrono::high_resolution_clock::now();
	for (int fi = 0; fi < 120; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			STOP;
		}
		tileFrames.push_back(frame);

	}

	auto playFinish = chrono::high_resolution_clock::now();
	chrono::duration<double> playElapsed = playFinish - playS;	decodeTime = playElapsed.count();
	cout << "Tile H264 decode Time:................> " << playElapsed.count() * 1000 << endl;

	int tileRowN = 4;	int tileColN = 6;	int tileColLen = frameCol / tileColN;	int tileRowLen = frameRow / tileRowN;
	for (int i = 0; i < 24; i++)
	{
		vector<vector <Mat>> temp1;
		for (int chunk = 0; chunk <= 5; chunk++)
		{
			vector <Mat> temp;
			for (int i = 0; i < (fps * chunkD); i++)
			{
				Mat m;
				temp.push_back(m);
				m.release();
			}
			temp1.push_back(temp);
		}
		frameQvecTiles.push_back(temp1);
	}

	int tileN1 = 8; int tileN2 = 9; int tileN3 = 14; int tileN4 = 15;
	string fileTemp = "C:/inetpub/wwwroot/3vid2crf3trace/Tiles/diving.avi_1_8.avi";
	DownLoadTilesChunk(fileTemp, chunkN, chunkD, tileN1, fps, 10);
	fileTemp = "C:/inetpub/wwwroot/3vid2crf3trace/Tiles/diving.avi_1_9.avi";
	DownLoadTilesChunk(fileTemp, chunkN, chunkD, tileN2, fps, 10);
	fileTemp = "C:/inetpub/wwwroot/3vid2crf3trace/Tiles/diving.avi_1_12.avi";
	DownLoadTilesChunk(fileTemp, chunkN, chunkD, tileN3, fps, 10);
	fileTemp = "C:/inetpub/wwwroot/3vid2crf3trace/Tiles/diving.avi_1_13.avi";
	DownLoadTilesChunk(fileTemp, chunkN, chunkD, tileN4, fps, 10);
	


	vector<vector <int>> reqTiles; vector<int> totalInside;
	for (int i = 0; i <= 4; i++)
	{	vector<int> temp;
		int x = 0;
		temp.push_back(x);
		reqTiles.push_back(temp);
	}
	reqTiles[1].push_back(8); reqTiles[1].push_back(9); reqTiles[1].push_back(14); reqTiles[1].push_back(15);

	playS = chrono::high_resolution_clock::now();
	for (int i = 0; i < fps * chunkD; i++)
	{
		
		cam_index = path1.GetCamIndex(4, fps, cam_index);	
		Mat convPixelsTmp(camera2.h, camera2.w, firstScene.type());		
		eri.ERI2Conv4tiles(convPixelsTmp, frameQvecTiles, reqTiles[1], camera2, tileColN, tileRowN, chunkN, i, totalInside);	//ch camera2	
		//displayImage(convPixelsTmp);

	}

	playFinish = chrono::high_resolution_clock::now();
	playElapsed = playFinish - playS;
	creriTime = playElapsed.count();
	cout << "tile decode Time:................> " << playElapsed.count() * 1000 << endl;
	ofstream output;
	output.open("./Video/source/cpuTime.txt", std::ios::out | std::ios::app);
	output << fileName << ";" << decodeTime << ";" << creriTime << endl;
	output.close();

}


void GenerateEncoding4AllDirection()
{
	int tiltseperaton = 20;
	int chunkDurationsec = 4;
	for (int tilt = -90; tilt <= 90; tilt=tilt+tiltseperaton)//
	{
		
		int panAngle = (int)abs((20) / (cos(3.1416*tilt/180)));   //argument in radian
		if (panAngle % 2 != 0)
		{
			panAngle += 1;
		}
		for (int pan = -180; pan <= 180; pan=pan+panAngle)
		{	
			cout << "tilt: " << tilt << " pan: " << pan <<" angle: "<< panAngle<< endl;
			//testvideoEncodeNew4s(chunkDurationsec, pan, tilt);			
			makeVideo4thSecVar(pan, tilt);			
			//STOP;
		}
	}
	cout << "came" << endl;
	STOP;
}

void GenerateEncoding4AllDirectiontemp()
{
	int tiltseperaton = 20;
	int chunkDurationsec = 4;
	int pan = 0;
	int tilt = -10;
	int chunkstart = 1;

	cout << "tilt: " << tilt << " pan: " << pan<< endl;
	//testvideoEncodeNew4stemp(chunkDurationsec, pan, tilt, chunkstart);
	makeVideo4thSecVar(pan, tilt);
		
	STOP;
}

void temp_calc(vector <float>& nonUniformList, float n, int extraFrame)
{
	float finalValue = n * 30;
	float finalIndex = extraFrame;
	for (int col = 1; col < extraFrame+1; col++)
	{
		float b = 1;
		float x=col;
		float a = float((finalValue-finalIndex*b)/(finalIndex*finalIndex));
		int u_x = a * x*x+b*x;
		cout<<"Nonuniform list->" <<"after90Index= "<< x << "  NonuniformIndex=" << u_x<<endl;
		nonUniformList.push_back(u_x);

	}
}

void makeVideo4thSecVar(float pan, float tilt)
{
	vector <Mat> encoded;
	vector <Mat> bufferedFrame;
	struct var encodevar;
	Path path1;
	float hfov = 90.0f;
	float corePredictionMargin = 1.2;
	int w = 960;
	int h = 512;	
	PPC encodeRefPPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);  //always next to corePPC before pan or tilt
	PPC core1PPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	int cf = 5;
	VideoCapture cap("./Video/roller.MKV");

	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		waitKey(100000);
	}
	int fps = 30;
	int lastframe = 910;
	int segi = 0;
	Mat ret;
	float var[10];
	int chunDurationsec = 3;
	Mat frame;
	core1PPC.Pan(pan);
	core1PPC.Tilt(tilt);


	for (int fi = 0; fi < lastframe; fi++)
	{
		//print("frame Saving: " << fi << endl);		
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}
			
		print("enFi: " << fi << endl);					
		ret = path1.EncodeNewNonLinV2(frame, &encodevar, core1PPC, encodeRefPPC, cf); //for all possible angle encodinig
		encoded.push_back(ret);
		frame.release();
		ret.release();
		
	}


	vector <float> nonUniformList;
	float howManySecondsfor4thSec = 5;
	int extraFrame = 30;
	temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrame);
	float addI;
	for (int fi = 0; fi < 640; fi++)
	{
		cout << fi << endl;		
		bufferedFrame.push_back(encoded[fi]);
		if ((fi + 1) % (chunDurationsec*fps) == 0 && fi > 0)
		{
			for (int i = 1; i < 31; i++)
			{ 
				addI = fi +(int)nonUniformList[i-1];
				bufferedFrame.push_back(encoded[addI]);
				cout <<"fi-"<<fi<< " addI-" << addI << endl;
			}

			int chunkN = (fi + 1) / (chunDurationsec*fps);
			cout << chunkN << endl;
			int sf = bufferedFrame.size()-120;
			int ef = bufferedFrame.size();
			cout <<sf<< " sf:ef " << ef << endl;
			filename = "./Video/encodingtest/4thSec/rollerFul/roller_";
			cout << fps << ret.cols << ret.rows << " sf: " << sf << " ef: " << ef << endl;
			videowriterhelperx(chunkN, (int)pan, (int)tilt, fps, encoded[1].cols, encoded[1].rows, sf, ef, bufferedFrame);
			
		}

		if (fi == 2)
		{
			var[0] = encodevar.colN;
			var[1] = encodevar.rowN;
			var[2] = encodevar.We;
			var[3] = encodevar.Het;
		}
	}
	ofstream output("./Video/encodingtest/4thSec/rollerFul/Roller_encoding_variable.txt");
	output << var[0] << endl;
	output << var[1] << endl;
	output << var[2] << endl;
	output << var[3] << endl;
	output.close();

}


void testvideoEncodeNew4stemp(int chunDurationsec, float pan, float tilt, int chunkstart) {
	vector <Mat> encoded;
	struct var encodevar;
	Path path1;

	float hfov = 90.0f;
	float corePredictionMargin = 1;
	int w = 500;
	int h = 250;
	PPC corePPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	PPC encodeRefPPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);  //always next to corePPC before pan or tilt

	int cf = 5;

	path1.LoadHMDTrackingData("./Video/roller.txt", corePPC);
	VideoCapture cap("./Video/roller_2000_1000.mp4");
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		waitKey(100000);
	}
	int fps = cap.get(CAP_PROP_FPS);
	int lastframe = 120;
	int segi = 0;
	Mat ret;
	float var[10];

	for (int fi = 0; fi < lastframe; fi++)
	{		
		float hfov = 90.0f;
		float corePredictionMargin = 1;
		int w = 960;
		int h = 512;
		PPC core1PPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
		cout << endl;
		print("fi: " << fi << endl);
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}
		if (fi >= chunDurationsec*fps*(chunkstart - 1))
		{
			if (fi % (chunDurationsec*fps) == 0)
			{
				segi = path1.GetCamIndex(fi, fps, segi);
				print("segi: " << segi << endl);
			}

			core1PPC.Pan(pan);
			core1PPC.Tilt(tilt);
			ret = path1.EncodeNewNonLinV2(frame, &encodevar, core1PPC, encodeRefPPC, cf); //for all possible angle encodinig
			//ret = frame;
			encoded.push_back(ret);

			if ((fi + 1) % (chunDurationsec*fps) == 0 && fi > 0)
			{
				int chunkN = (fi + 1) / (chunDurationsec*fps);
				cout << chunkN << endl;
				int sf = 0;
				int ef = chunDurationsec * fps;
				filename = "./Video/encodingtest/newmethod/RollerInput2_";
				cout << fps << ret.cols << ret.rows << " sf: " << sf << " ef: " << ef << endl;
				videowriterhelperx(chunkN, (int)pan, (int)tilt, fps, ret.cols, ret.rows, sf, ef, encoded);
			}

			if (fi == 117)
			{
				var[0] = encodevar.colN;
				var[1] = encodevar.rowN;
				var[2] = encodevar.We;
				var[3] = encodevar.Het;
			}
		}
	}

	ofstream output("./Video/encodingtest/newmethod/Roller_encoding_variable.txt");
	output << var[0] << endl;
	output << var[1] << endl;
	output << var[2] << endl;
	output << var[3] << endl;
	output.close();


}


void testvideoEncodeNew4s(int chunDurationsec, float pan, float tilt) {
	vector <Mat> encoded;	
	struct var encodevar;
	Path path1;


	float hfov = 90.0f;
	float corePredictionMargin = 1;
	int w = 960;
	int h = 512;
	PPC corePPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	PPC encodeRefPPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);  //always next to corePPC before pan or tilt
	   	
	int cf = 5;

	path1.LoadHMDTrackingData("./Video/roller.txt", corePPC);
	VideoCapture cap("./Video/roller.MKV");
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		waitKey(100000);
	}
	int fps = cap.get(CAP_PROP_FPS);
	int lastframe = 480;	
	int segi = 0;
	Mat ret; 
	float var[10];
	
	for (int fi = 0; fi < lastframe; fi++)
	{
		float hfov = 90.0f;
		float corePredictionMargin = 1;
		int w = 960;
		int h = 512;
		PPC core1PPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);

		print("fi: "<<fi<<endl);
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}
		if (fi % (chunDurationsec*fps) == 0)
		{
			segi = path1.GetCamIndex(fi, fps, segi);
			print("segi: "<< segi << endl);
		}

		//ret = path1.EncodeNewNonLinV2(frame, &encodevar, path1.cams[segi], encodeRefPPC, cf);
		core1PPC.Pan(pan);
		core1PPC.Tilt(tilt);
		ret = path1.EncodeNewNonLinV2(frame, &encodevar, core1PPC, encodeRefPPC, cf); //for all possible angle encodinig
		encoded.push_back(ret);
		
		if ((fi + 1) % (chunDurationsec*fps) == 0 && fi > 0)		
		{
			int chunkN = (fi + 1) / (chunDurationsec*fps);
			cout << chunkN << endl;
			int sf = chunDurationsec * fps*(chunkN - 1);
			int ef = chunDurationsec * fps*chunkN;
			filename = "./Video/encodingtest/newmethod/RollerInput_";
			cout << fps << ret.cols << ret.rows <<" sf: "<< sf<<" ef: " << ef << endl;
			
			videowriterhelperx(chunkN, (int)pan, (int)tilt, fps, ret.cols, ret.rows, sf, ef, encoded);
		}
		if (fi == 2)
		{			
			var[0] = encodevar.colN;
			var[1] = encodevar.rowN;
			var[2] = encodevar.We;
			var[3] = encodevar.Het;			
		}
		
	}  

	ofstream output("./Video/encodingtest/newmethod/Roller_encoding_variable.txt");
	output << var[0] << endl;
	output << var[1] << endl;
	output << var[2] << endl;
	output << var[3] << endl;
	output.close();
	

}



void videowriterhelperx(int chunkN , float pan, float tilt, int fps,int cols, int rows, int starting_frame,int ending_frame, vector<Mat> file2wr )
{
	
	
	std::ostringstream oss1;
	oss1 << chunkN <<"_"<<tilt<<"_"<<pan<< ".avi";
	string ofilename = oss1.str();
	VideoWriter writer1;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	writer1.set(VIDEOWRITER_PROP_QUALITY, 10);
	cout << writer1.get(VIDEOWRITER_PROP_QUALITY);
	cout << "Writing videofile: " << ofilename << codec << endl;	
	writer1.open(ofilename, codec, fps, Size(cols, rows), true);

	if (!writer1.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return;
	}
	for (int i = starting_frame; i < ending_frame; i++)
	{
		writer1.write(file2wr[i]);

	}
	writer1.release();


}

void videowriterhelperxWithFileName(char* filename, int chunkN, float pan, float tilt, int fps, int cols, int rows, int starting_frame, int ending_frame, vector<Mat> file2wr)
{


	std::ostringstream oss1;
	oss1 << filename << chunkN << "_" << tilt << "_" << pan << ".avi";
	string ofilename = oss1.str();
	VideoWriter writer1;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	writer1.set(VIDEOWRITER_PROP_QUALITY, 10);
	cout << writer1.get(VIDEOWRITER_PROP_QUALITY);
	cout << "Writing videofile: " << ofilename << endl;
	writer1.open(ofilename, codec, fps, Size(cols, rows), true);

	if (!writer1.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return;
	}
	for (int i = starting_frame; i < ending_frame; i++)
	{
		writer1.write(file2wr[i]);

	}
	writer1.release();


}

void videowriterhelperxWithFileNameWithExtraSec(char* filename, int extraSec, int chunkN, float pan, float tilt, int fps, int cols, int rows, int starting_frame, int ending_frame, vector<Mat> file2wr)
{

	std::ostringstream oss1;
	oss1 << filename << chunkN << "_" << extraSec<<"_"<< tilt << "_" << pan << ".avi";
	string ofilename = oss1.str();
	VideoWriter writer1;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	writer1.set(VIDEOWRITER_PROP_QUALITY, 10);
	cout << writer1.get(VIDEOWRITER_PROP_QUALITY);
	cout << "Writing videofile: " << ofilename <<endl;
	writer1.open(ofilename, codec, fps, Size(cols, rows), true);

	if (!writer1.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return;
	}
	for (int i = starting_frame; i < ending_frame; i++)
	{
		writer1.write(file2wr[i]);

	}
	writer1.release();


}


void testrotationxyframe()
{
	Mat frame;
	upload_image("./Image/360_equirectangular_800_400.JPG", frame);
	//upload_image("http://127.0.0.5:80/1.JPG", frame);
	if (!frame.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return;
	}


	float hfov = 110.0f;
	PPC camera(hfov, 800, 400);

	Path path1;
	
	Mat newERI = Mat::zeros(frame.rows, frame.cols, frame.type());

	V3 p = camera.GetVD();
	camera.Pan(0.0f);
	camera.Tilt(45.0f);
	V3 p1 = camera.GetVD();

	// build local coordinate system of RERI
	V3 xaxis = camera.a.UnitVector();
	V3 yaxis = camera.b.UnitVector()*-1.0f;
	V3 zaxis = xaxis ^ yaxis;
	M33 reriCS;
	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;


	path1.RotateXYaxisERI2RERI(frame, newERI, p, p1, reriCS);	
	

	namedWindow("sample", WINDOW_NORMAL);
	resizeWindow("sample", 400, 400);

	imshow("sample", newERI);
	waitKey(10000);
	

}



void testMousecontrol()
{
	ERI_INIT;
	int cond = 1;
	while (cond == 1)
	{

		eri.ERI2Conv(eriPixels, convPixels, camera1);
		imshow("CONV_image", convPixels);
		waitKey(1);

		int ch = getch();
		cout << ch << endl;
		switch (ch)
		{
		case 77:
			camera1.Pan(5.0f);
			break;
		case 75:
			camera1.Pan(-5.0f);
			break;
		case 72:
			camera1.Tilt(5.0f);
			break;
		case 80:
			camera1.Tilt(-5.0f);
			break;
		case 113:
			cond = 0;
			break;

		}
	}

}



/***************************************/
/**************Additional code*************************/

int upload_image(string path, Mat &image) {
	Mat image_temp;
	string path_temp = path;
	image_temp = imread(path_temp.c_str(), IMREAD_COLOR);
	if (!image_temp.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	image = image_temp;


	return 0;
}




void testvideodecodeNqual()
{
	vector <Mat> conv;
	vector <Mat> hmap;
	vector<float> min;
	vector<float> avg;
	struct samplingvar svar;
	Mat ret1;
	Path path1;
	PPC camera2(120.0f, 800, 400);
	path1.LoadHMDTrackingData("./Video/rollerSingleVD.txt", camera2);
	int lastframe = 1800;
	int segi = 0;

	int cf = 5;
	float var[10];
	float x;
	string datax;// = fetchTextOverHttp("http://127.0.0.5:80/testQual/diving_encoding_variable.txt");

	std::istringstream f(datax);
	std::string line;
	int i = 0;
	while (std::getline(f, line))
	{
		var[i] = stoi(line);
		cout << var[i] << '\n';
		i++;
	}


	int chunkD = 4;
	int chunkN;
	int fps;
	int MxchunkN = 4;
	for (int i = 1; i <= MxchunkN; i++)
	{
		chunkN = i;
		string fn = "http://127.0.0.5:80/testQual/diving_";
		std::ostringstream oss;
		oss << fn << (chunkN)<<"_10_40.mp4";
		string filename = oss.str();

		VideoCapture cap1(filename);
		if (!cap1.isOpened())
		{
			cout << "Cannot open the video file: " << endl;
			waitKey(100000);
		}
		fps = cap1.get(CAP_PROP_FPS);
		int segref = 0;

		for (int j = fps * (chunkN - 1)*chunkD; j < fps*chunkN*chunkD; j++)
		{

			Mat encodframe;
			cap1 >> encodframe;
			if (encodframe.empty())
			{
				cout << j << endl;
				cout << "Can not read video frame: " << endl;
				break;
			}
			segi = path1.GetCamIndex(j, fps, segi);
			print("chunN: " << i << " frame: " << j << " segi: " << segi << endl;);
			if (j%fps == 0)
			{
				segref = segi;
				print("segref: " << segref << endl);
			}

			V3 p = path1.cams[segi].GetVD() - path1.cams[segref].GetVD();
			//print(p << " prev: "<< path1.cams[segi].GetVD()<<endl<<endl);
			p = V3(0, 0, -1) + p;
			p = p.UnitVector();

			Mat heatmap3c = Mat::zeros(camera2.h, camera2.w, encodframe.type());
			Mat heatmap = Mat::zeros(camera2.h, camera2.w, DataType<double>::type);
			//camera2.PositionAndOrient(V3(0, 0, 0), p, V3(0, 1, 0));
			ret1 = path1.CRERI2ConvQual(encodframe, var, cf, path1.cams[segi], path1.cams[segref], heatmap, &svar);

			//ret1 is the conventional image, heatmap is the heatmap of quality and svar has vto and vtin parameter: max and min sampling interval
			//generate color for heatmap//

			float vtin = svar.vtin;
			float vto = svar.vto;
			min.push_back(svar.min);
			avg.push_back(svar.avg);
			float bb;
			for (int i = 0; i < camera2.h; i++)
			{
				for (int j = 0; j < camera2.w; j++)
				{
					bb = heatmap.at<float>(i, j);
					float factor = (float)bb / (float)(vtin);
					int colora = (float)255 / (float)factor;
					colora = (colora <= 255) ? colora : 255;
					Vec3b insidecolorx(colora, colora, 255);
					heatmap3c.at<Vec3b>(i, j) = insidecolorx;
					//cout << heatmap3c.at<Vec3b>(i, j) << endl;
				}
			}

			hmap.push_back(heatmap3c);
			conv.push_back(ret1);
		}
	}

	ofstream output("./Video/encodingtest/min.txt");
	for (int i = 0; i < min.size(); i++)
	{
		output << min[i] << "\n";
	}
	output.close();

	ofstream output1("./Video/encodingtest/avg.txt");
	for (int i = 0; i < avg.size(); i++)
	{
		output1 << avg[i] << "\n";
	}
	output1.close();

	int starting_frame = 0;
	int ending_frame = fps * chunkN*chunkD;
	filename = "./Video/encodingtest/rollerh264conv";
//	videowriterhelperx(111, fps, ret1.cols, ret1.rows, starting_frame, ending_frame, conv);
	filename = "./Video/encodingtest/rollerh264Hmap";
	//videowriterhelperx(111, fps, ret1.cols, ret1.rows, starting_frame, ending_frame, hmap);


}
void createSmallTilesFromLongTiles(char* fileName)
{

	vector <Mat> frameS;
	
	int fps = 30;
	int chunkD = 4;
	
	VideoCapture cap(fileName);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		waitKey(100000);
	}

	for (int fi = 0; fi < 900; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}
		if (fi > 0)
		{
			frameS.push_back(frame);
		}

	}

	for (int i = 0; i < frameS.size(); i++)
	{
		if ((i+1)%(fps*chunkD)==0 && i>0)
		{
			int sf = i+1 - 120;
			int ef =i+1;
			int chunkN = (i + 1) / (fps * chunkD);
			cout << "SAving-> sf:ef " << sf << " " << ef << endl;
			videowriterhelperxWithFileName(fileName, chunkN, 0, 0, fps, frameS[1].cols, frameS[1].rows, sf, ef, frameS);

		}

	}
}
int testTiling() {
	PPC camera1(cFoV, cameraW, cameraH);
	PPC camera2(cFoV, cameraW, cameraH);
	PPC camera3(cFoV, cameraW, cameraH);
	PPC camera4(cFoV, cameraW, cameraH);
	PPC camera5(cFoV, cameraW, cameraH);
	Path path1;
	Path path2;
	Path path3;
	Path path4;
	Path path5;
	int lastframe = 121;

	path5.LoadHMDTrackingData("./Video/roller.txt", camera1);
	
	int m = 1;
	int n = 1;
	int t = 1;
	
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 2;
	n = 2;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);
	
	m = 4;
	n = 6;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 8;
	n = 8;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 10;
	n = 10;
	t = 1;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);
	STOP;
	m = 20;
	n = 20;
	t = 4;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);

	m = 30;
	n = 30;
	t = 4;
	path5.ConvPixel2ERITile("./Video/roller.mkv", lastframe, m, n, t);
	return 0;

}

// load  one frame and load one camera from path file and display that frame 

int out_video_file()//(Mat &output_image_mat, ERI eri_image, Path path1)
{
	//VideoCapture cap("http://www.engr.colostate.edu/me/facil/dynamics/files/drop.avi");

	VideoCapture cap("http://98.223.107.255:80/RollerInput_1_10_0.avi");
	if (!cap.isOpened()) {
		cout << "Cannot open the video file" << endl;
		system("pause");

	}

	namedWindow("MyVideo", WINDOW_NORMAL);

	int fi = 1;
	while (1)
	{
		Mat frame;
		cap >> frame;

		if (frame.empty())
		{
			cout << "empty" << endl;
			break;
		}
		//path1.cams[fi].PositionAndOrient(V3(0, 0, 0), V3(0, 0, 1), V3(0, 1, 0));
		//eri_image.ERI2Conv(frame, output_image_mat, path1.cams[fi]);
		//imshow("MyVideo", output_image_mat);
		//fi++;
		imshow("window",frame);
		waitKey(20);
	}
	cout << "finish" << endl;
	return 0;
}

//check wether interpolation works between two position of a still image. 

int testPlayBackManualPathStillImage() {
	ERI_INIT;

	int framesN = 30;
	path1.AppendCamera(camera1, framesN);
	camera1.Pan(90);
	path1.AppendCamera(camera1, framesN);
	camera1.Pan(-90);
	path1.AppendCamera(camera1, framesN);
	path1.PlayBackPathStillImage(eriPixels, eri, convPixels);
	return 0;

}


void testvideoendecodenew() {
	vector <Mat> encoded;
	vector <Mat> conv;
	struct var encodevar;
	Path path1;
	PPC camera1(90.0f, 800, 400);
	int cf = 5;

	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	VideoCapture cap("./Video/roller.MKV");
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		waitKey(100000);
	}

	int lastframe = 720;
	int fps = 30;
	int segi = 0;
	Mat ret; Mat ret1;

	for (int fi = 0; fi < lastframe; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}
		segi = path1.GetCamIndex(fi, fps, segi);

		ret = path1.EncodeNewNonLinV2(frame, &encodevar, path1.cams[segi], camera1, cf);
		encoded.push_back(ret);
		//namedWindow("sample", WINDOW_NORMAL);
		//resizeWindow("sample", 800, 800);
		//imshow("sample", ret);
		//waitKey(1000);
		float var[10];
		var[0] = frame.cols;
		var[1] = frame.rows;
		var[2] = encodevar.We;
		var[3] = encodevar.Het;//*/	

		PPC camera2(120.0f, 800, 400);
		camera2.Pan(120.0f);
		//camera2.Tilt(25.0f);
		//ret1 =  path1.CRERI2Conv(encodframe, var, cf, camera2, heatmap, &svar);
		//conv.push_back(ret1);
		//imshow("sample", ret1);
		waitKey(1000);
	}

	VideoWriter writer;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	writer.set(VIDEOWRITER_PROP_QUALITY, 30);
	string filename = "./Video/encodingtest/rollerh264encod.MKV";
	cout << "Writing videofile: " << filename << codec << endl;
	writer.open(filename, codec, fps, Size(ret.cols, ret.rows), true);

	if (!writer.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return;
	}
	for (int i = 0; i < encoded.size(); i++)
	{
		writer.write(encoded[i]);
	}
	writer.release();

	VideoWriter writer1;
	writer1.set(VIDEOWRITER_PROP_QUALITY, 30);
	filename = "./Video/encodingtest/rollerh264conv.MKV";
	cout << "Writing videofile: " << filename << codec << endl;
	writer1.open(filename, codec, fps, Size(ret1.cols, ret1.rows), true);

	if (!writer1.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return;
	}
	for (int i = 0; i < conv.size(); i++)
	{
		writer1.write(conv[i]);
	}
	writer1.release();
}

void video4sBaseAndExtraSec(char* fileName4s, char* fileName1s, char* fileName2s, char* fileName3s) {

	vector <Mat> frame4s1s;
	vector <Mat> frame4s2s;
	vector <Mat> frame4s3s;
	int fps = 30;
	int chunkD = 4;
	int extra1sLength = 30;
	int extra2sLength = 40;
	int extra3sLength = 50;


	VideoCapture cap(fileName4s);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " <<fileName4s<< endl;
		waitKey(100000);
	}
	   
	for (int fi = 0; fi < fps*chunkD; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}
		
		frame4s1s.push_back(frame);
		frame4s2s.push_back(frame);
		frame4s3s.push_back(frame);
		
	}

	VideoCapture cap1s(fileName1s);
	for (int fi = 0; fi < extra1sLength; fi++)
	{
		Mat frame;
		cap1s >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}

		frame4s1s.push_back(frame);

	}

	VideoCapture cap2s(fileName2s);
	for (int fi = 0; fi < extra2sLength; fi++)
	{
		Mat frame;
		cap2s >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}

		frame4s2s.push_back(frame);

	}

	VideoCapture cap3s(fileName3s);
	for (int fi = 0; fi < extra3sLength; fi++)
	{
		Mat frame;
		cap3s >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;
			break;
		}

		frame4s3s.push_back(frame);

	}

	
	int sf = frame4s1s.size() - (fps*chunkD+extra1sLength);
	int ef = frame4s1s.size();
	cout << "SAving-> sf:ef " << sf << " " << ef << endl;
	videowriterhelperxWithFileNameWithExtraSec(fileName4s, 600, extra1sLength, 0, 0, fps, frame4s1s[1].cols, frame4s1s[1].rows, sf, ef, frame4s1s);

	sf = frame4s2s.size() - (fps*chunkD + extra2sLength);
	ef = frame4s2s.size();
	cout << "SAving-> sf:ef " << sf << " " << ef << endl;
	videowriterhelperxWithFileNameWithExtraSec(fileName4s, 800, extra2sLength, 0, 0, fps, frame4s1s[2].cols, frame4s1s[2].rows, sf, ef, frame4s2s);


	sf = frame4s3s.size() - (fps*chunkD + extra3sLength);
	ef = frame4s3s.size();
	cout << "SAving-> sf:ef " << sf << " " << ef << endl;
	videowriterhelperxWithFileNameWithExtraSec(fileName4s, 1000, extra3sLength, 0, 0, fps, frame4s1s[3].cols, frame4s1s[3].rows, sf, ef, frame4s3s);

	
}


int testvideoencodedecode() {

	vector<float> We;
	vector<float> Het;
	vector<float>Heb;
	vector<float> R0x;
	vector<float>R0y;
	vector<float> R0R1;
	vector<float>R0R4;
	int compressionfactor = 5;
	Path path1;
	vector <Mat> encodedbuffer;
	PPC camera1(110.0f, 800, 400);
	path1.LoadHMDTrackingData("./Video/roller.txt", camera1);
	int lastframe = 5;
	encodedbuffer = path1.videoencode("./Video/roller_2000_1000.mp4", lastframe, We, Het, Heb, R0x, R0y, R0R1, R0R4, compressionfactor);
	path1.videodecode("./Video/encodingtest/rollerh264encod.MKV", lastframe, 2000, 1000, We, Het, Heb, R0x, R0y, R0R1, R0R4, compressionfactor);
	//change the frame height and width in this funciton
	return 0;

}

void testRotationxy()
{
	Mat eriPixels;
	upload_image("./Image/360_equirectangular_800_400.JPG", eriPixels);
	if (!eriPixels.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return;
	}


	float hfov = 110.0f;
	PPC camera(hfov, 800, 400);

	Path path1;
	path1.LoadHMDTrackingData("./Video/roller.txt", camera);
	path1.BuildERI2RERIVideo(eriPixels, camera);



}

