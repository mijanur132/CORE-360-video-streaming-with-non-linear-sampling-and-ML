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
#include<math.h>

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
int timePassed = 0;
float nextDlChunkSecVar=3000;
float DlTimeVariation = 0;

float maxF(float a, float b) {return (a > b) ? a : b; }

//float samplingRateFrame[2000][2000];

int NextChunkDownloaded=0;
int requestedChunk2DL = 0;
int decodeCount=0;
void makeVideo4thSecVarSpecificPanTiltChunkN(char* fileName, float pan, float tilt, int ChnkN, int extraFrame, int extraSec);
void makeVideo4thSecVarSpecificPanTiltChunkNvariableLastSection(char* fileName, int extraSec, float pan, float tilt, int chunkN, int extraFrame, int howManySecondsfor4thSec);

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

void timeInterval(std::chrono::time_point<std::chrono::high_resolution_clock> start) {

	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	cout << "time: " << elapsed.count() * 1000 << endl;
}

void SaveAtRandomText(int argc, int argv[])
{
	ofstream os1;
	os1.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/RandomText.txt", std::ios::out | std::ios::app);
	for (int i = 0; i < argc; i++)
	{
		os1 << argv[i] << ";";
	}
	os1 << endl;
	os1.close();

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
	//cout << "displayingTim......................................e=" << elapsed.count()*1000 << endl;
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
	
 	VideoCapture cap1(filename);  /// just for android test, must replace with cap1(filename)

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
	float elapsedTimeMS = elapsedms.count();
	DlTimeVariation = (1 - 0.5) * DlTimeVariation + 0.5 * (elapsedTimeMS-nextDlChunkSecVar );
	nextDlChunkSecVar = 0.5* nextDlChunkSecVar+ 0.5* elapsedTimeMS;
	nextDlChunkSecVar = nextDlChunkSecVar +  DlTimeVariation;

	//nextDlChunkSecVar = 0.1*nextDlChunkSecVar+0.9*elapsedTimeMS;

	if (nextDlChunkSecVar > 4000) {
		nextDlChunkSecVar = 4000;
	}
	
	cout << " Downloaded=" << filename << " NextChunkDownloaded= "<< NextChunkDownloaded<<endl;
	cout << "Time forcasted and needed to load= " << timeNeededms << " " << elapsedms.count()   << endl;
	cout << "nextDlChunkSecvar..............................................................................: " << nextDlChunkSecVar <<" "<<DlTimeVariation<< endl;
	ChngReriCS = 1;
	return;
}

/*
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

	while (timePassed < timeNeededms)	{
		Mat firstScene;
		upload_image("./Video/source/startScene.PNG", firstScene);
		//displayImage(firstScene);		
		//cout << timePassed << endl;		
	}
	NextChunkDownloaded = NextChunkDownloaded + 1;
	decodeCount = 0;
	cout << timePassed << " <-passed:needed-> " << timeNeededms << endl;
	cout << " Downloaded=" << filename << " NextChunkDownloaded= " << NextChunkDownloaded << endl;
	return;

}
*/

void DownLoadTilesChunk(string filename, int chunkN, int chunkD, int tileN, int fps, int timeNeededms)
{
	auto start = std::chrono::high_resolution_clock::now();
	Mat frame;
	cout << endl;
	VideoCapture cap1(filename);

	if (!cap1.isOpened())
	{
		cout << "**********************************" << endl;
		cout << "Cannot open the video file: " << filename << endl;
		waitKey(100);
		STOP;
	}

	for (int j = 0; j < fps * chunkD; j++)
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
	cap1.release();
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
	//cout << "Time forcasted and needed to load= " << timeNeededms << " " << elapsedms.count() << endl;
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
string getChunkNametoReqAndroid(PPC& camera1, M33& reriCS, string srcBaseAddr, int chunkN, int pan)
{

	std::ostringstream oss1;
	oss1 << srcBaseAddr << "_" << chunkN << "_" << 0 << "_" << pan << ".avi";
	filename = oss1.str();
	cout << "ReqF=" << filename << endl;

	PPC camera = camera1;
	//cout <<"ref cam direction"<< camera1.GetVD() << endl;
	camera.Pan(pan);
	camera.Tilt(0);

	V3 xaxis = camera.a.UnitVector();
	V3 yaxis = camera.b.UnitVector() * -1.0f;
	V3 zaxis = xaxis ^ yaxis;

	reriCS[0] = xaxis;
	reriCS[1] = yaxis;
	reriCS[2] = zaxis;

	return filename;
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
	//	cout <<"bwTrace: "<< byte << endl;
	}

}

void testDownloadVideoHttp4thSecVarDelayFRmeasurement(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec, int Size)
{
	int singleOrVariableVD = 1;
	int samplingValueCalculate = 1;
	auto start = std::chrono::high_resolution_clock::now();
	vector <Mat> conv;	vector <Mat> outputFrame2SAve;	vector<Mat> outputSamplingRate2Save;
	vector<float> frameRate4allFrames;	vector <float>srVec; 	vector <float> nonUniformList;
	struct samplingvar svar; Mat ret1;	Path path1;
	M33 reriCS;	M33 reriCScopy;
	int compressionFactor = 5;	int frameStarted = 0;		int chunkN = 0;	int fps = 30;
	float var[10];	float howManySecondsfor4thSec = extraSec;	int extraFrameN = 5 * extraSec;	int mxChunkN = 15;	int chunkD = 4;//

	float minFR = 30.0f;

	vector<float> chunkSizeKB; vector<string>chunkName;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/4s3s/crf30/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0; 
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr,'\\');
		int start = int(positionMarker - nameChr)+1;
		name = name.substr(start, name.size()-start);
		float KB;
		linestream >> KB;
		if (extraSec==1)		{KB = KB / 1024.0f * (0.93 * 1.05);	}
		else if (extraSec == 2) { KB = KB / 1024.0f * (0.93 * 1.12); }
		else if (extraSec == 3) { KB = KB / 1024.0f * (0.93 * 1.17); }
		else if (extraSec == 6) { KB = KB / 1024.0f * (0.93 * 1.33); }
		else { KB = KB / 1024.0f * (0.93 * 1.52); }
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);
		
		//cout << name << " " << KB << endl;
		
	}

	ifstream  file("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	if (!file)
	{
		print("error: can't open file: " << filename << endl);		system("pause");
	}
	string line;
	int ii = 0;
	while (getline(file, line))
	{
		var[ii] = stoi(line);		cout << var[ii] << '\n';		ii++;
	}


	int frameLen = var[0];	int frameWidth = var[1];		float hfov = 90.0f;
	float corePredictionMargin = 0.9;	int w = frameLen * hfov / 360; 	int h = frameWidth * hfov / 360;

	int ERI_w = frameLen;	int ERI_h = frameWidth;	ERI eri(ERI_w, ERI_h);

	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	PPC refCam(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	path1.LoadHMDTrackingData(hmdFileName, camera2);


	Mat firstScene;	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
	Mat convPixels(camera2.h, camera2.w, firstScene.type());
	Mat samplingPixels(camera2.h, camera2.w, firstScene.type());
	cout << camera2.h << " cam " << camera2.w << endl;

	int firstPlayTime;	int condition = 1;	int cam_index = 0;	int tiltAngle = 20;
	string filename;
	V3 VD;
	int frameCount = 0;	int startIndex = 0;	int after90Index = -1; int noFind = 0;
	int realFrameCount = 0;;
	int minChunkAvgFR = 30;
	std::vector<std::future<void>> futures;
	int delayTotal = 0;

	for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
	{
		vector <Mat> temp;
		for (int i = 0; i < (fps * chunkD + extraFrameN); i++)
		{
			Mat m;	temp.push_back(m);	m.release();
		}
		frameQvec.push_back(temp);
	}


	temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrameN + 1);
	LoadBWTraceData(bwLog);	eri.atanvalue();	eri.xz2LonMap();	eri.xz2LatMap();
	path1.nonUniformListInit(var);		path1.mapx(var);
	

	cout << "Very first Chunk, no frame yet......" << endl;


	cam_index = path1.GetCamIndex(1, fps, cam_index);		VD = path1.cams[cam_index].GetVD();
	chunkN++;
	filename = getChunkNametoReqnRefCamOptimized(refCam, reriCS, srcBaseAddr, chunkN, VD, tiltAngle);
	cout << "ReqF=" << filename << endl;
	
	NextChunkDownloaded = 0;

	auto finish1 = std::chrono::high_resolution_clock::now();	std::chrono::duration<double> elapsed1 = finish1 - start;
	
	int bwTimeNeeded = 200; int bwDuration;	int bwIglobal;	int packetNeeded;	int bwShift = 0;
	//************* BW simulation****************//
	if (samplingValueCalculate == 1)
	{
		bwDuration = elapsed1.count() * 1000 + bwShift;
		bwIglobal = 0;
		while (bwDuration > byteVec[bwIglobal]) {
			bwIglobal++;
		}
		string name = filename;
		cout << filename << endl;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '/');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		cout << name <<" "<< chunkName[0]<< endl;
		int namePos = 0;
		while((name.compare(chunkName[namePos])) && namePos <=152)
		{
			namePos++;
		}
		//packetNeeded = chunkSizeKB[namePos] *1024 / 1500; //1500 for diving
		if (namePos==153)
		{
			noFind++;
			cout << "noFind.............>>>"<<noFind << endl;
			packetNeeded = 1620 * 1024 / 1500;
			filename="http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving_10_-10_-80.avi";

		}
		
		bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
		cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;

	}
	//************* BW simulation****************//

	cout << "First DL req Time:------>" << elapsed1.count() << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
	auto playStart = std::chrono::high_resolution_clock::now();

	DownLoadChunk4thSecVar(filename, chunkD, fps, extraFrameN, bwTimeNeeded, chunkN);

	while (NextChunkDownloaded == 0)
	{
		displayImage(firstScene);
	}

	frameCount = -1;
	while (condition)
	{

		frameCount++; 
		realFrameCount++;
		if (frameCount == (fps * chunkD * mxChunkN - 1 + 0))
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

		if (playIndex == 30 && currentChunk > 1)
		{
			vector <Mat> temp;
			for (int i = 0; i < 2; i++)
			{
				Mat m;		temp.push_back(m);	m.release();
			}
			frameQvec[currentChunk - 1] = temp;
			cout << "memory erased..................." << endl;
		}


		if (currentChunk <= NextChunkDownloaded)
		{
			if (currentChunk == NextChunkDownloaded && currentChunk > 1)
			{
				if (ChngReriCS == 1)
				{
					reriCS = reriCScopy;cout << "reriCS updated............................" << endl;	ChngReriCS = 0;
				}
			}

			Mat frame;	frame = frameQvec[currentChunk][playIndex];

			after90Index = -1;
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			V3 vdTemp = path1.cams[cam_index].GetVD();			//cout<<"longt: "<<eri.GetXYZ2Longitude(vdTemp)<<endl;
			
			
			if (frameCount < 1)
			{
				auto firstFrame = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> elapsedFirst = firstFrame - start; firstPlayTime = elapsedFirst.count() * 1000;
				playStart = std::chrono::high_resolution_clock::now();

			}
			frameRate4allFrames.push_back(1);
			Mat mx;
			frameQvec[currentChunk][playIndex] = mx;
		
		

			
			displayImage33ms();


			if ((playIndex) == fps * (chunkD - nextDlChunkSec))
			{
				int reqChunkN = currentChunk + 1;
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
					bwDuration = frameCount * 33 + firstPlayTime;   //forced assumption that all frame is 33ms play time, even though they are not.
					while (bwDuration > byteVec[bwIglobal]) {
						bwIglobal++;
					}
					string name = filename;
					const char* nameChr = name.c_str();
					const char* positionMarker = strrchr(nameChr, '/');
					int start = int(positionMarker - nameChr) + 1;
					name = name.substr(start, name.size() - start);
					int namePos = 0;
				
					while ((name.compare(chunkName[namePos])) && namePos <= 152)
					{
						namePos++;
					}
				//	packetNeeded = chunkSizeKB[namePos] * 1024 / 1500;
					if (namePos == 153)
					{
						noFind++;
						cout << "noFind.............>>>" << noFind << endl;
						packetNeeded = 1620 * 1024 / 1500;
						filename = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving_10_-10_-80.avi";

					}
					
					bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms

					//************* BW simulation****************//
				}

				cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
				futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));

			}

		}

		if (after90Index >=( extraFrameN-1))
		{
			frameCount--;  //reverse framecount here, cause no frame came
			currentChunk = (frameCount) / (fps * chunkD) + 1;
			auto delayStart = std::chrono::high_resolution_clock::now();
			auto delayEnd = std::chrono::high_resolution_clock::now();;
			std::chrono::duration<double> delayElapsed = delayEnd - delayStart;
			int duration=delayElapsed.count()*1000;
			while (duration < 33)
			{
				delayEnd = std::chrono::high_resolution_clock::now();;
				delayElapsed = delayEnd - delayStart;
				duration = delayElapsed.count() * 1000;
			}
			delayTotal = delayTotal + 33;
			cout << "delayTotal: " << delayTotal << endl;
		}

		if (((currentChunk) > NextChunkDownloaded) && currentChunk > 1 && after90Index <extraFrameN-1)
		{
			after90Index++; //0->
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			int index = nonUniformList[after90Index];
			int index2 = nonUniformList[after90Index + 1];

			int diffIndex = index2 - index;
			if (diffIndex > 100 || diffIndex < 0)
			{
				cout <<"diffIndex: "<< diffIndex << endl;
				condition = 0;
				break;
			}
			
			if (30.0f/diffIndex<minFR)
			{
				minFR = 30.0f / diffIndex;
			}
			
			realFrameCount++;
			int chunkWiseFRMin = 30.0f*realFrameCount / frameCount;
			if (chunkWiseFRMin < minChunkAvgFR)
			{
				minChunkAvgFR =chunkWiseFRMin;
			}
			cout << "diffIndex: " << diffIndex << "minFR: " << minFR <<"CWM: "<<chunkWiseFRMin<< endl;
			
			frameRate4allFrames.push_back(1);
			for (int i = 0; i < diffIndex; i++)
			{
				if (i > 0) {
					frameCount++;
				}
				currentChunk = (frameCount) / (fps * chunkD) + 1; //current chunk 0 theke start, file chunk o 0 theke start krote hobe encoding er somoy
				playIndex = frameCount - (currentChunk - 1) * (fps * chunkD);
				cout << "119CuntChunkN=" << currentChunk - 1 << " frmcount=" << frameCount << " plCnt=" << playIndex << " after90indx=" << after90Index << " crntChnk: " << NextChunkDownloaded << endl;
				//	cout << frameCount << endl;
				cam_index = path1.GetCamIndex(frameCount - 1, fps, cam_index);
				Mat frame1;
				frame1 = frameQvec[currentChunk - 1][(fps * chunkD - 1) + after90Index];
				float frameRateTemp = diffIndex;

				displayImage33ms();
				Mat mx1;
				frameQvec[currentChunk - 1][(fps * chunkD - 1) + after90Index]=mx1;
				if (playIndex == (fps * (chunkD - nextDlChunkSec)))
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
						bwDuration = frameCount * 33 + firstPlayTime;   //forced assumption that all frame is 33ms play time, even though they are not.
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}
						string name = filename;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						int namePos = 0;
						while ((name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = chunkSizeKB[namePos] * 1024 / 1500;
						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
					}
					cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;

					futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));


				}


			}

		}
		
	}

	auto playFinishAll = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> playElapsed = playFinishAll - playStart;
	cout << "FPS:................> " << playElapsed.count() * 1000 / frameCount << endl;
	cout << "Frist frame Appear Time=..............>" << firstPlayTime << endl;
	float delayOldWay = playElapsed.count() * 1000 - frameCount * 33;


	float totalTemp = 0;
	for (int i = 0; i < frameRate4allFrames.size(); i++)
	{
		float tempValue = frameRate4allFrames[i];
		totalTemp = totalTemp + tempValue;
		
	}

	cout << totalTemp << " " << realFrameCount <<" "<<noFind<< endl;

	float avgFrameRate = 30*totalTemp / frameCount;

	ofstream output;
	output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/CoREfDelay.txt", std::ios::out | std::ios::app);
	output << "File," << srcBaseAddr << ",MinFR,"<<minFR<<",avgFR,"<< avgFrameRate<< ",Delay,"<<delayTotal<< ",firstFrame,"<< firstPlayTime <<endl;
	output.close();
	cout << "File," << srcBaseAddr << ",MinFR," << minFR << ",chunkWiseMin," << minChunkAvgFR << ",avgFrameRate," << avgFrameRate << ",totDely," << delayTotal << ",firstFrame," << firstPlayTime << endl;


}

void testDownloadVideoHttp4thSecVarAndroid(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec, int Size)
{
	int singleOrVariableVD = 1;
	int samplingValueCalculate = 1;
	auto start = std::chrono::high_resolution_clock::now();
	vector <Mat> conv;	vector <Mat> outputFrame2SAve;	vector<Mat> outputSamplingRate2Save; vector <int> panVec;
	vector<float> frameRate4allFrames;	vector <float>srVec; vector <float>srVecMin;	vector <float> nonUniformList;
	vector <int> srRealT;	vector <int> srMinRealT; vector <int> frRealT;
	struct samplingvar svar; Mat ret1;	Path path1;
	M33 reriCS;	M33 reriCScopy;
	int compressionFactor = 5;	int frameStarted = 0;		int chunkN = 0;	int fps = 30;
	float var[10];	float howManySecondsfor4thSec = extraSec;	int extraFrameN = 5 * extraSec;	int mxChunkN = 7;	int chunkD = 4;//
	Mat tempP;

	vector<float> chunkSizeKB; vector<string>chunkName;
	int mahimahiPackSize = 1400;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/4s3s/crf30/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		if (extraSec == 1) { KB = KB / 1024.0f * (0.93 * 1.05); }
		else if (extraSec == 2) { KB = KB / 1024.0f * (0.93 * 1.12); }
		else if (extraSec == 3) { KB = KB / 1024.0f * (0.83 * 1.17); }
		else if (extraSec == 6) { KB = KB / 1024.0f * (1 * 1.33); }
		else { KB = KB / 1024.0f * (0.93 * 1.52); }
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);

		//cout << name << " " << KB << endl;

	}
	Size = Size * 1;
	ifstream  file("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	if (!file)
	{
		print("error: can't open file: " << filename << endl);		system("pause");
	}
	string line;
	int ii = 0;
	while (getline(file, line))
	{
		var[ii] = stoi(line);		cout << var[ii] << '\n';		ii++;
	}


	int frameLen = var[0];	int frameWidth = var[1];		float hfov = 90.0f;
	float corePredictionMargin = 0.6;	int w = frameLen * hfov / 360; 	int h = frameWidth * hfov / 360;

	int ERI_w = frameLen;	int ERI_h = frameWidth;	ERI eri(ERI_w, ERI_h);

	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	PPC refCam(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	//path1.LoadHMDTrackingData(hmdFileName, camera2);


	Mat firstScene;	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
	Mat convPixels(camera2.h, camera2.w, firstScene.type());
	Mat samplingPixels(camera2.h, camera2.w, firstScene.type());
	cout << camera2.h << " cam " << camera2.w << endl;

	int firstPlayTime;	int condition = 1;	int cam_index = 0;	int tiltAngle = 20;
	string filename;
	V3 VD;
	int frameCount = 0;	int startIndex = 0;	int after90Index = -1;
	std::vector<std::future<void>> futures;
	int delayTotal = 0;

	for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
	{
		vector <Mat> temp;
		for (int i = 0; i < (fps * chunkD + extraFrameN); i++)
		{
			Mat m;	temp.push_back(m);	m.release();
		}
		frameQvec.push_back(temp);
	}

	/*android hmd loaded*/

	ifstream  filex(hmdFileName);
	if (!filex)
	{
		print("error: can't open file: " << hmdFileName << endl);
		system("pause");
	}
	string   line2;
	while (getline(filex, line2))
	{
		stringstream  linestream(line2);		
		int byte;
		linestream >> byte;
		panVec.push_back(byte);
		//cout << byte << endl;
	}

	/* android hmd loaded done*/
	
	temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrameN + 1);
	LoadBWTraceData(bwLog);	
	eri.atanvalue();	eri.xz2LonMap();	eri.xz2LatMap();
	path1.nonUniformListInit(var);		path1.mapx(var);


	cout << "Very first Chunk, no frame yet......" << endl;



	int minFR = 30.0f;
	//cam_index = path1.GetCamIndex(1, fps, cam_index);		VD = path1.cams[cam_index].GetVD();
	chunkN++;
	filename = getChunkNametoReqAndroid(refCam, reriCS, srcBaseAddr, chunkN, panVec[0]);
		

	NextChunkDownloaded = 0;

	auto finish1 = std::chrono::high_resolution_clock::now();	std::chrono::duration<double> elapsed1 = finish1 - start;

	int bwTimeNeeded = 200; int bwDuration;	int bwIglobal;	int packetNeeded;	int bwShift = 0;


	cout << "First DL req Time:------>" << elapsed1.count() << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
	auto playStart = std::chrono::high_resolution_clock::now();

	DownLoadChunk4thSecVar(filename, chunkD, fps, extraFrameN, bwTimeNeeded, chunkN);

	while (NextChunkDownloaded == 0)
	{
		displayImage(firstScene);
	}

	frameCount = -1;
	while (condition)
	{

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

		if (playIndex == 30 && currentChunk > 1)
		{
			vector <Mat> temp;
			for (int i = 0; i < 2; i++)
			{
				Mat m;		temp.push_back(m);	m.release();
			}
			frameQvec[currentChunk - 1] = temp;
			cout << "memory erased..................." << endl;
		}


		if (currentChunk <= NextChunkDownloaded)
		{
			if (currentChunk == NextChunkDownloaded && currentChunk > 1)
			{
				if (ChngReriCS == 1)
				{
					reriCS = reriCScopy; cout << "reriCS updated............................" << endl;	ChngReriCS = 0;
				}
			}

			Mat frame;	frame = frameQvec[currentChunk][playIndex];

			after90Index = -1;
			PPC camerax = refCam;
			int pan = panVec[frameCount];
			camerax.Pan(pan);
			cout << "119CuntChunkN=" << currentChunk - 1 << " frmcount=" << frameCount << " plCnt=" << playIndex << " after90indx=" << after90Index << " crntChnk: " << NextChunkDownloaded << "pan: " << pan << endl;
			path1.CRERI2ConvOptimizedWithSamplingRateVecMinAvg(frame, var, eri, reriCS, convPixels, srVec, srVecMin, compressionFactor, camerax, refCam);
			

			displayImage(convPixels);
			cout << "SR:........ >" << srVec[srVec.size() - 1] << endl;
			srRealT.push_back(srVec[srVec.size() - 1]);
			srMinRealT.push_back(srVecMin[srVecMin.size() - 1]);
		
			//displayImage33ms();
			//outputFrame2SAve.push_back(convPixels.clone());


			if ((playIndex) == fps * (chunkD - nextDlChunkSec))
			{
				int reqChunkN = currentChunk + 1;
				//cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
				reriCScopy = reriCS;
				//VD = path1.cams[cam_index].GetVD();

				filename = getChunkNametoReqAndroid(refCam, reriCScopy, srcBaseAddr, reqChunkN, pan);
				//outputFileName << filename << endl;
				cout << "New Req. " << filename << endl;
				finish1 = std::chrono::high_resolution_clock::now();
				elapsed1 = finish1 - start;
			

				cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
				futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));

			}

		}

		if (after90Index >= (extraFrameN - 1))
		{
			frameCount--;  //reverse framecount here, cause no frame came
			frRealT.push_back(0);
			srRealT.push_back(0);
			srMinRealT.push_back(0);
			currentChunk = (frameCount) / (fps * chunkD) + 1;
			//cout << currentChunk<<" currentChunk: Next " <<NextChunkDownloaded<< endl;
			auto delayStart = std::chrono::high_resolution_clock::now();
			auto delayEnd = std::chrono::high_resolution_clock::now();;
			std::chrono::duration<double> delayElapsed = delayEnd - delayStart;
			int duration = delayElapsed.count() * 1000;
			while (duration < 30)
			{
				delayEnd = std::chrono::high_resolution_clock::now();;
				delayElapsed = delayEnd - delayStart;
				duration = delayElapsed.count() * 1000;
			}
			delayTotal = delayTotal + 30;
			cout << "delayTotal: " << delayTotal << endl;
		}

		if (((currentChunk) > NextChunkDownloaded) && currentChunk > 1 && after90Index < extraFrameN - 1)
		{
			after90Index++; //0->
			int index = nonUniformList[after90Index];
			int index2 = nonUniformList[after90Index + 1];

			int diffIndex = index2 - index;

			float currentFR = 30.0f / diffIndex;
			if (currentFR < minFR)
			{
				minFR = 30.0f / diffIndex;
			}

			cout << "diffIndex: " << diffIndex << "minFR: " << minFR << endl;

			frameRate4allFrames.push_back(1);

			if (diffIndex > 100 || diffIndex < 0)
			{
				cout << "diffIndex: " << diffIndex << endl;
				condition = 0;
				break;
			}
			cout << "diffIndex: " << diffIndex << endl;
			for (int i = 0; i < diffIndex; i++)
			{
				if (i > 0) {
					frameCount++;
				}
				currentChunk = (frameCount) / (fps * chunkD) + 1; //current chunk 0 theke start, file chunk o 0 theke start krote hobe encoding er somoy
				playIndex = frameCount - (currentChunk - 1) * (fps * chunkD);
				
				//	cout << frameCount << endl;
				int pan=panVec[frameCount - 1];
				Mat frame1;
	
				frame1 = tempP;
				float frameRateTemp = diffIndex;

				PPC camerax = refCam;
				camerax.Pan(pan);
				cout << "119CuntChunkN=" << currentChunk - 1 << " frmcount=" << frameCount << " plCnt=" << playIndex << " after90indx=" << after90Index << " crntChnk: " << NextChunkDownloaded << "pan: "<<pan<<endl;
				path1.CRERI2ConvOptimizedWithSamplingRateVec(frame1, var, eri, reriCS, convPixels, srVec, compressionFactor, camerax, refCam);

				cout << "SR:........ >" << srVec[srVec.size() - 1] << endl;
				srRealT.push_back(srVec[srVec.size() - 1]);
				srMinRealT.push_back(srVecMin[srVecMin.size() - 1]);
				displayImage(convPixels);
				displayImage33ms();


				if (playIndex == (fps * (chunkD - nextDlChunkSec)))
				{
					int reqChunkN = currentChunk + 1; //amader chunk 0 theke start hoy but download chunk start hoy 1 theke , apatoto			

					int pan = panVec[frameCount];
					reriCScopy = reriCS;

					filename = getChunkNametoReqAndroid(refCam, reriCScopy, srcBaseAddr, reqChunkN, pan);
					//outputFileName << filename << endl;
					cout << "NewR: " << filename << endl;
					finish1 = std::chrono::high_resolution_clock::now();
					elapsed1 = finish1 - start;
					//************* BW simulation****************//
		
					cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;

					//************* BW simulation****************//
					futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));


				}


			}

		}

	}

	auto playFinishAll = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> playElapsed = playFinishAll - playStart;
	cout << "FPS:................> " << playElapsed.count() * 1000 / frameCount << endl;
	cout << "Frist frame Appear Time=..............>" << firstPlayTime << endl;
		   
	double aggValue = 0;
	double value = 0;
	float min = 1000;
	float max = 0;
	float avg = 0;
	for (int i = 0; i < frameRate4allFrames.size(); i++)
	{
		value = srVec[i];
		aggValue = value + aggValue;

		if (value > max)
		{
			max = value;
		}
		if (value < min)
		{
			min = value;
		}		
		avg = aggValue / (srVec.size());

	}
	ofstream outputSR;
	outputSR.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/androidCore_FR_SR_all_D_646.txt", std::ios::out | std::ios::app);
	outputSR << srcBaseAddr << hmdFileName << bwLog << endl;
	for (int i = 0; i < srRealT.size(); i++)
	{
		outputSR << "file:" << srcBaseAddr <<  srRealT[i] << ",srMinRealTime:" << srMinRealT[i]  <<  endl;

	}
	outputSR.close();

	ofstream output;
	output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/AndroidCoREallValues.txt", std::ios::out | std::ios::app);
	outputSR << srcBaseAddr << hmdFileName << bwLog << endl;
	output << "File," << srcBaseAddr << ",avgSR," << avg << ",minSR," << min<< endl;
	output.close();
	cout << "File," << srcBaseAddr << bwLog << nextDlChunkSec << ",avgSR," << avg << ",minSR," << min << endl;
	
}


void testDownloadVideoHttp4thSecVar(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec, int Size)
{
	
	int samplingValueCalculate = 1;
	auto start = std::chrono::high_resolution_clock::now();
	vector <Mat> conv;	vector <Mat> outputFrame2SAve;	vector<Mat> outputSamplingRate2Save;
	vector<float> frameRate4allFrames;	vector <float>srVec; vector <float>srVecMin;	vector <float> nonUniformList; vector<float>reqTime;
	vector <int> srRealT;	vector <int> srMinRealT;vector <int> frRealT;
	struct samplingvar svar; Mat ret1;	Path path1;
	M33 reriCS;	M33 reriCScopy;
	int compressionFactor = 5;	int frameStarted = 0;		int chunkN = 0;	int fps = 30;
	float var[10];	float howManySecondsfor4thSec = extraSec;	int extraFrameN = 5 * extraSec;	int mxChunkN = 15;	int chunkD = 4;  //
	Mat tempP;

	vector<float> chunkSizeKB; vector<string>chunkName;
	int mahimahiPackSize = 1400;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/4s6s/mobisys/crf30/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		KB = KB / 1024.0f;
		
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);

		//cout << name << " " << KB << endl;

	}
	Size = Size * 1;
	ifstream  file("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	if (!file)
	{
		print("error: can't open file: " << filename << endl);		system("pause");
	}
	string line;
	int ii = 0;
	while (getline(file, line))
	{
		var[ii] = stoi(line);		cout << var[ii] << '\n';		ii++;
	}


	int frameLen = var[0];	int frameWidth = var[1];		float hfov = 90.0f;
	float corePredictionMargin = 0.6;	int w = frameLen * hfov / 360; 	int h = frameWidth * hfov / 360;

	int ERI_w = frameLen;	int ERI_h = frameWidth;	ERI eri(ERI_w, ERI_h);

	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	PPC refCam(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	path1.LoadHMDTrackingData(hmdFileName, camera2);


	Mat firstScene;	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
	Mat convPixels(camera2.h, camera2.w, firstScene.type());
	Mat samplingPixels(camera2.h, camera2.w, firstScene.type());
	cout << camera2.h << " cam " << camera2.w << endl;

	int firstPlayTime;	int condition = 1;	int cam_index = 0;	int tiltAngle = 20;
	string filename;
	V3 VD;
	int frameCount = 0;	int startIndex = 0;	int after90Index = -1;
	std::vector<std::future<void>> futures;
	int delayTotal = 0;

	for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
	{
		vector <Mat> temp;
		for (int i = 0; i < (fps * chunkD + extraFrameN); i++)
		{
			Mat m;	temp.push_back(m);	m.release();
		}
		frameQvec.push_back(temp);
	}


	temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrameN + 1);
	LoadBWTraceData(bwLog);	eri.atanvalue();	eri.xz2LonMap();	eri.xz2LatMap();
	path1.nonUniformListInit(var);		path1.mapx(var);


	cout << "Very first Chunk, no frame yet......" << endl;


	int minFR = 30.0f;
	cam_index = path1.GetCamIndex(1, fps, cam_index);		VD = path1.cams[cam_index].GetVD();
	chunkN++;
	filename = getChunkNametoReqnRefCamOptimized(refCam, reriCS, srcBaseAddr, chunkN, VD, tiltAngle);
	cout << "ReqF=" << filename << endl;

	NextChunkDownloaded = 0;

	auto finish1 = std::chrono::high_resolution_clock::now();	std::chrono::duration<double> elapsed1 = finish1 - start;

	int bwTimeNeeded = 200; int bwDuration;	int bwIglobal;	int packetNeeded;	int bwShift = 0;
	//************* BW simulation****************//
	if (samplingValueCalculate == 1)
	{
		bwDuration = elapsed1.count() * 1000 + bwShift;
		bwIglobal = 0;
		while (bwDuration > byteVec[bwIglobal]) {
			bwIglobal++;
		}
		string name = filename;
		cout << filename << endl;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '/');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		cout << "partName: " << name << endl;
		int namePos = 0;
		while (namePos<=chunkName.size() && (name.compare(chunkName[namePos])))
		{
			namePos++;
		}
		if (namePos<=chunkName.size())
		{
			cout << namePos << " chunkName: " << chunkName[namePos] << " " << chunkSizeKB[namePos]<<" size: " << chunkName.size() << endl;
			packetNeeded = chunkSizeKB[namePos] * 1024 / mahimahiPackSize;				
			bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms

		}
		else {
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << " " << chunkName.size() << endl;
			packetNeeded = 2000 * 1024 / mahimahiPackSize;
			cout<<"file Not found: ......................................................................................................................."<<filename<<endl;
			filename = "http://127.0.0.5:80/3vid2crf3trace/4s6s/mobisys/crf30/30_diving.AVI6_1_10_0.avi";
			bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
		}		

	}
	
	//************* BW simulation****************//

	cout << "First DL req Time:------>" << elapsed1.count() << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
	auto playStart = std::chrono::high_resolution_clock::now();

	DownLoadChunk4thSecVar(filename, chunkD, fps, extraFrameN, bwTimeNeeded, chunkN);
	requestedChunk2DL = requestedChunk2DL + 1;

	while (NextChunkDownloaded == 0)
	{
		displayImage(firstScene);
	}

	frameCount = -1;
	while (condition)
	{
		
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
		cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex << " DownloadedChunk:" << NextChunkDownloaded << endl;
		
		if (currentChunk > requestedChunk2DL)
		{
			nextDlChunkSecVar = 4000;
			reqTime.push_back(4000);
			int reqChunkN = currentChunk;
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			reriCScopy = reriCS;
			VD = path1.cams[cam_index].GetVD();

			filename = getChunkNametoReqnRefCamOptimized(refCam, reriCScopy, srcBaseAddr, reqChunkN, VD, tiltAngle);
			//outputFileName << filename << endl;
			cout << "New Req. " << filename << endl;
			finish1 = std::chrono::high_resolution_clock::now();
			elapsed1 = finish1 - start;
			if (samplingValueCalculate == 1)
			{
				//************* BW simulation****************//
				//bwDuration = elapsed1.count() * 1000+ bwShift; //real time spend
				bwDuration = frameCount * 33 + firstPlayTime;   //forced assumption that all frame is 33ms play time, even though they are not.
				while (bwDuration > byteVec[bwIglobal]) {
					bwIglobal++;
				}
				string name = filename;
				const char* nameChr = name.c_str();
				const char* positionMarker = strrchr(nameChr, '/');
				int start = int(positionMarker - nameChr) + 1;
				name = name.substr(start, name.size() - start);
				cout << "partName: " << name << endl;
				int namePos = 0;

				while (namePos <= chunkName.size() && (name.compare(chunkName[namePos])))
				{
					namePos++;
				}
				if (namePos <= chunkName.size())
				{
					cout << " namePos:" << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
					packetNeeded = chunkSizeKB[namePos] * 1024 / mahimahiPackSize;
					bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms

				}
				else {
					cout << "NamePos: " << namePos << " Name:" << chunkName[namePos] << " size:" << chunkSizeKB[namePos] << " chunKNamesize:" << chunkName.size() << endl;
					packetNeeded = 2000 * 1024 / mahimahiPackSize;
					cout << "file Not found:........................................................................................................................................... " << filename << endl;
					filename = "http://127.0.0.5:80/3vid2crf3trace/4s6s/mobisys/crf30/30_diving.AVI6_1_10_0.avi";
					bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
				}
				//************* BW simulation****************//
			}

			cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
			requestedChunk2DL = requestedChunk2DL + 1;
			futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));
		}

		if (currentChunk > 1 & playIndex == 89)
		{
			cout << "erase................................................................................................................." << endl;
			for (int j = 0; j < currentChunk; j++) 
			{
				for (int i = 0; i < 150; i++)				{
					Mat tempx;
					frameQvec[j][i] = tempx;
				}
			}

		}
	

		if (currentChunk <= NextChunkDownloaded)
		{
			if (currentChunk == NextChunkDownloaded && currentChunk > 1)
			{
				if (ChngReriCS == 1)
				{
					reriCS = reriCScopy; cout << "reriCS updated............................" << endl;	ChngReriCS = 0;
				}
			}

			Mat frame;	frame = frameQvec[currentChunk][playIndex];

			after90Index = -1;
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			V3 vdTemp = path1.cams[cam_index].GetVD();			//cout<<"longt: "<<eri.GetXYZ2Longitude(vdTemp)<<endl;

			if (samplingValueCalculate == 1)
			{
				if (frameCount < 1)
				{
					auto firstFrame = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double> elapsedFirst = firstFrame - start; firstPlayTime = elapsedFirst.count() * 1000;
					playStart = std::chrono::high_resolution_clock::now();
					tempP = frame.clone();

				}
				path1.CRERI2ConvOptimizedWithSamplingRateVecMinAvg(frame, var, eri, reriCS, convPixels, srVec, srVecMin, compressionFactor, path1.cams[cam_index], refCam);
			}
			else {
				path1.CRERI2ConvOptimized(frame, var, eri, reriCS, convPixels, compressionFactor, path1.cams[cam_index], refCam);

			}
			frameRate4allFrames.push_back(1);

	

			//displayImage(convPixels);
			//cout << "SR:........ >" << srVec[srVec.size() - 1] << endl;
			srRealT.push_back(srVec[srVec.size() - 1]);
			srMinRealT.push_back(srVecMin[srVecMin.size() - 1]);
			frRealT.push_back(30);
			//displayImage33ms();
			//outputFrame2SAve.push_back(convPixels.clone());
			
			
			if (nextDlChunkSec!=0){
				nextDlChunkSecVar = nextDlChunkSec*1000;
			}
			if ((playIndex) == (int)(fps * (chunkD - nextDlChunkSecVar/1000.0f)))
			{	
				reqTime.push_back(nextDlChunkSecVar);
				int reqChunkN = currentChunk + 1;
				cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
				reriCScopy = reriCS;
				VD = path1.cams[cam_index].GetVD();

				filename = getChunkNametoReqnRefCamOptimized(refCam, reriCScopy, srcBaseAddr, reqChunkN, VD, tiltAngle);
				//outputFileName << filename << endl;
				cout << "New Req. " << filename << endl;
				finish1 = std::chrono::high_resolution_clock::now();
				elapsed1 = finish1 - start;
				if (samplingValueCalculate == 1)
				{
					//************* BW simulation****************//
					//bwDuration = elapsed1.count() * 1000+ bwShift; //real time spend
					bwDuration = frameCount * 33 + firstPlayTime;   //forced assumption that all frame is 33ms play time, even though they are not.
					while (bwDuration > byteVec[bwIglobal]) {
						bwIglobal++;
					}
					string name = filename;
					const char* nameChr = name.c_str();
					const char* positionMarker = strrchr(nameChr, '/');
					int start = int(positionMarker - nameChr) + 1;
					name = name.substr(start, name.size() - start);
					cout << "partName: " << name << endl;
					int namePos = 0;

					while (namePos <= chunkName.size() && (name.compare(chunkName[namePos])))
					{
						namePos++;
					}
					if (namePos <= chunkName.size())
					{
						cout <<" namePos:"<< namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = chunkSizeKB[namePos] * 1024 / mahimahiPackSize;						
						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms

					}
					else {
						cout <<"NamePos: "<< namePos << " Name:" << chunkName[namePos] << " size:" << chunkSizeKB[namePos] <<" chunKNamesize:"<<chunkName.size()<< endl;
						packetNeeded = 2000 * 1024 / mahimahiPackSize;
						cout << "file Not found:........................................................................................................................................... " << filename << endl;						
						filename = "http://127.0.0.5:80/3vid2crf3trace/4s6s/mobisys/crf30/30_diving.AVI6_1_10_0.avi";
						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
					}
					//************* BW simulation****************//
				}
				requestedChunk2DL = requestedChunk2DL + 1;
				cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
				futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));

			}

		}

		if (after90Index >= (extraFrameN - 1))
		{
			frameCount--;  //reverse framecount here, cause no frame came
			frRealT.push_back(0);
			srRealT.push_back(0);
			srMinRealT.push_back(0);
			currentChunk = (frameCount) / (fps * chunkD) + 1;
			//cout << currentChunk<<" currentChunk: Next " <<NextChunkDownloaded<< endl;
			auto delayStart = std::chrono::high_resolution_clock::now();
			auto delayEnd = std::chrono::high_resolution_clock::now();;
			std::chrono::duration<double> delayElapsed = delayEnd - delayStart;
			int duration = delayElapsed.count() * 1000;
			while (duration < 30)
			{
				delayEnd = std::chrono::high_resolution_clock::now();;
				delayElapsed = delayEnd - delayStart;
				duration = delayElapsed.count() * 1000;
			}
			delayTotal = delayTotal + 30;
			cout << "delayTotal: " << delayTotal << endl;
		}

		if (((currentChunk) > NextChunkDownloaded) && currentChunk > 1 && after90Index < extraFrameN - 1)
		{
			after90Index++; //0->
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			int index = nonUniformList[after90Index];
			int index2 = nonUniformList[after90Index + 1];

			int diffIndex = index2 - index;

			float currentFR = 30.0f / diffIndex;
			if (currentFR < minFR)
			{
				minFR = 30.0f / diffIndex;
			}
						
			cout << "diffIndex: " << diffIndex << "minFR: " << minFR<< endl;

			frameRate4allFrames.push_back(1);
					   
			if (diffIndex > 100 || diffIndex < 0)
			{
				cout << "diffIndex: " << diffIndex << endl;
				condition = 0;
				break;
			}
			cout << "diffIndex: " << diffIndex << endl;
			for (int i = 0; i < diffIndex; i++)
			{
				if (i > 0) {
					frameCount++;
				}
				currentChunk = (frameCount) / (fps * chunkD) + 1; //current chunk 0 theke start, file chunk o 0 theke start krote hobe encoding er somoy
				playIndex = frameCount - (currentChunk - 1) * (fps * chunkD);

				
				//frameQvec[currentChunk - 1][i] = 0;
				

				cout << "119CuntChunkN=" << currentChunk - 1 << " frmcount=" << frameCount << " plCnt=" << playIndex << " after90indx=" << after90Index << " crntChnk: " << NextChunkDownloaded << endl;
				//	cout << frameCount << endl;
				cam_index = path1.GetCamIndex(frameCount - 1, fps, cam_index);
				Mat frame1;				
				frame1 = tempP;
				float frameRateTemp = diffIndex;
				
				if (samplingValueCalculate == 1)
				{

					path1.CRERI2ConvOptimizedWithSamplingRateVec(frame1, var, eri, reriCS, convPixels, srVec, compressionFactor, path1.cams[cam_index], refCam);

				}
				else {
					path1.CRERI2ConvOptimized(frame1, var, eri, reriCS, convPixels, compressionFactor, path1.cams[cam_index], refCam);

				}			

				//displayImage(convPixels);
				//cout << "SR:........ >" << srVec[srVec.size() - 1] << endl;
				srRealT.push_back(srVec[srVec.size() - 1]);
				srMinRealT.push_back(srVecMin[srVecMin.size() - 1]);
				frRealT.push_back(currentFR);
				displayImage33ms();
				
				if (nextDlChunkSec !=0) {
					nextDlChunkSecVar = nextDlChunkSec*1000;
				}
				if (playIndex ==(int)( (fps * (chunkD - nextDlChunkSecVar/1000.0f))))
				{
					reqTime.push_back(nextDlChunkSecVar);
					int reqChunkN = currentChunk + 1; //amader chunk 0 theke start hoy but download chunk start hoy 1 theke , apatoto			

					cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
					VD = path1.cams[cam_index].GetVD();
					reriCScopy = reriCS;

					filename = getChunkNametoReqnRefCamOptimized(refCam, reriCScopy, srcBaseAddr, reqChunkN, VD, tiltAngle);
					//outputFileName << filename << endl;
					cout << "NewR: " << filename << endl;
					finish1 = std::chrono::high_resolution_clock::now();
					elapsed1 = finish1 - start;
					//************* BW simulation****************//
					if (samplingValueCalculate == 1)
					{
						//bwDuration = elapsed1.count() * 1000+bwShift;
						bwDuration = frameCount * 33 + firstPlayTime;   //forced assumption that all frame is 33ms play time, even though they are not.
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}

						string name = filename;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						cout << "partName: " << name << endl;
						int namePos = 0;

						/*
						string name = filename;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						int namePos = 0; */

						while (namePos < chunkName.size() && (name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						if (namePos < chunkName.size())
						{
							cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
							packetNeeded = chunkSizeKB[namePos] * 1024 / mahimahiPackSize;								
							bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms

						}
						else {
							cout << "NamePos: " << namePos << " Name:" << chunkName[namePos] << " size:" << chunkSizeKB[namePos] << " chunKNamesize:" << chunkName.size() << endl;
							packetNeeded = 1000 * 1024 / mahimahiPackSize;
							cout << "fileName not found: ................................................................................................" << filename << endl;
							filename = "http://127.0.0.5:80/3vid2crf3trace/4s6s/mobisys/crf30/30_diving.AVI6_1_10_0.avi";
							bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
							
						}

					}
					requestedChunk2DL = requestedChunk2DL + 1;
					cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;

					//************* BW simulation****************//
					futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));


				}


			}

		}

	}

	auto playFinishAll = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> playElapsed = playFinishAll - playStart;
	cout << "FPS:................> " << playElapsed.count() * 1000 / frameCount << endl;
	cout << "Frist frame Appear Time=..............>" << firstPlayTime << endl;

	//outputFileName.close();
	if (samplingValueCalculate == 1)
	{

		
		float totalTemp = 0;
		for (int i = 0; i < frameRate4allFrames.size(); i++)
		{
			float tempValue = frameRate4allFrames[i];
			totalTemp = totalTemp + tempValue;

		}
		
		
		float avgFR = 30 * totalTemp / frameCount;
		if (avgFR>30)
		{
			avgFR =30;
		}
			
		double aggValue = 0;
		double value = 0;
		float min = 1000;
		float max = 0;
		float avg = 0;
		for (int i = 0; i < frameRate4allFrames.size(); i++)
		{
			value = srVec[i];
			aggValue = value + aggValue;

			if (value > max)
			{
				max = value;
			}
			if (value < min)
			{
				min = value;
			}
			//cout << "frame: " << i << " SR avg value: " << value << " min: " << min << endl;
			avg = aggValue / (srVec.size());

		}
		ofstream outputSR;		
		outputSR.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/Core_FR_SR_all_D_646.txt", std::ios::out | std::ios::app);
		outputSR << srcBaseAddr << hmdFileName << bwLog << endl;
		for (int i = 0; i < srRealT.size(); i++)
		{
			outputSR<<"file:" <<srcBaseAddr<<",srAAvgRealTime:"<<srRealT[i]<<",srMinRealTime:"<<srMinRealT[i]<<",frRealTime:" <<frRealT[i]<< endl;

		}		
		outputSR.close();
		
		ofstream output;
		output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/CoREallValues_avg.txt", std::ios::out | std::ios::app);
		output << srcBaseAddr << hmdFileName << bwLog << endl;
		output << "avgSR," << avg << ",minSR," << min << ",maxSR," << max << ",MinFR," << minFR << ",avgFR," << avgFR << ",totalDely," << delayTotal << endl;
		output.close();
		cout << "File," << srcBaseAddr << bwLog << nextDlChunkSec << ",avgSR," << avg << ",minSR," << min << ",MinFR," << minFR << ",avgFR," << avgFR << ",totalDelay," << delayTotal << endl;

		ofstream outputDlReq;
		outputDlReq.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/DlReqTime.txt", std::ios::out | std::ios::app);
		outputDlReq << srcBaseAddr << hmdFileName << bwLog<<"_" <<nextDlChunkSec<< endl;
		for (int i = 0; i < reqTime.size(); i++)
		{
			outputDlReq << i << "," << reqTime[i] << endl;
		}
		
		outputDlReq.close();
		

	}


}

void testSrAccrossSpecificFrame(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int extraSec, int Size)
{
	int singleOrVariableVD = 1;
	int samplingValueCalculate = 1;
	auto start = std::chrono::high_resolution_clock::now();
	vector <Mat> conv;	vector <Mat> outputFrame2SAve;	vector<Mat> outputSamplingRate2Save;
	vector<float> frameRate4allFrames;	vector <float>srVec; vector <float>srVecMin;	vector <float> nonUniformList;
	vector <int> srRealT;	vector <int> srMinRealT; vector <int> frRealT;
	struct samplingvar svar; Mat ret1;	Path path1;
	M33 reriCS;	M33 reriCScopy;
	int compressionFactor = 5;	int frameStarted = 0;		int chunkN = 0;	int fps = 30;
	float var[10];	float howManySecondsfor4thSec = extraSec;	int extraFrameN = 5 * extraSec;	int mxChunkN = 14;	int chunkD = 4;//
	Mat tempP;

	vector<float> chunkSizeKB; vector<string>chunkName;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/4s3s/crf30/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		if (extraSec == 1) { KB = KB / 1024.0f * (0.93 * 1.05); }
		else if (extraSec == 2) { KB = KB / 1024.0f * (0.93 * 1.12); }
		else if (extraSec == 3) { KB = KB / 1024.0f * (0.83 * 1.17); }
		else if (extraSec == 6) { KB = KB / 1024.0f * (0.93 * 1.33); }
		else { KB = KB / 1024.0f * (0.93 * 1.52); }
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);

		//cout << name << " " << KB << endl;

	}
	Size = Size * 1;
	ifstream  file("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	if (!file)
	{
		print("error: can't open file: " << filename << endl);		system("pause");
	}
	string line;
	int ii = 0;
	while (getline(file, line))
	{
		var[ii] = stoi(line);		cout << var[ii] << '\n';		ii++;
	}


	int frameLen = var[0];	int frameWidth = var[1];		float hfov = 90.0f;
	float corePredictionMargin = 0.7;	int w = frameLen * hfov / 360; 	int h = frameWidth * hfov / 360;

	int ERI_w = frameLen;	int ERI_h = frameWidth;	ERI eri(ERI_w, ERI_h);

	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	PPC refCam(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	path1.LoadHMDTrackingData(hmdFileName, camera2);


	Mat firstScene;	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
	Mat convPixels(camera2.h, camera2.w, firstScene.type());
	Mat samplingPixels(camera2.h, camera2.w, firstScene.type());
	cout << camera2.h << " cam " << camera2.w << endl;

	int firstPlayTime;	int condition = 1;	int cam_index = 0;	int tiltAngle = 20;
	string filename;
	V3 VD;
	int frameCount = 0;	int startIndex = 0;	int after90Index = -1;
	std::vector<std::future<void>> futures;
	int delayTotal = 0;

	for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
	{
		vector <Mat> temp;
		for (int i = 0; i < (fps * chunkD + extraFrameN); i++)
		{
			Mat m;	temp.push_back(m);	m.release();
		}
		frameQvec.push_back(temp);
	}


	temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrameN + 1);
	LoadBWTraceData(bwLog);	eri.atanvalue();	eri.xz2LonMap();	eri.xz2LatMap();
	path1.nonUniformListInit(var);		path1.mapx(var);


	cout << "Very first Chunk, no frame yet......" << endl;

	
	int minFR = 30.0f;
	cam_index = path1.GetCamIndex(1, fps, cam_index);		VD = path1.cams[cam_index].GetVD();
	chunkN++;
	filename = getChunkNametoReqnRefCamOptimized(refCam, reriCS, srcBaseAddr, chunkN, VD, tiltAngle);
	cout << "ReqF=" << filename << endl;

	NextChunkDownloaded = 0;

	auto finish1 = std::chrono::high_resolution_clock::now();	std::chrono::duration<double> elapsed1 = finish1 - start;

	int bwTimeNeeded = 200; int bwDuration;	int bwIglobal;	int packetNeeded;	int bwShift = 0;
	//************* BW simulation****************//
	if (samplingValueCalculate == 1)
	{
		bwDuration = elapsed1.count() * 1000 + bwShift;
		bwIglobal = 0;
		while (bwDuration > byteVec[bwIglobal]) {
			bwIglobal++;
		}
		string name = filename;
		//outputFileName << filename << endl;
		cout << filename << endl;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '/');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);

		int namePos = 0;
		while (namePos < 262 && (name.compare(chunkName[namePos])))
		{
			namePos++;
		}
		if (namePos < 262)
		{
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
			packetNeeded = chunkSizeKB[namePos] * 1024 / 1500;
			//filename = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving_3_10_40.avi";
			//packetNeeded = Size * 1024 / 1500;		
			bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms

		}
		else {
			packetNeeded = 1000 * 1024 / 1500;
			filename = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving_3_10_40.avi";
			bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
		}

	}
	//************* BW simulation****************//

	cout << "First DL req Time:------>" << elapsed1.count() << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
	auto playStart = std::chrono::high_resolution_clock::now();

	DownLoadChunk4thSecVar(filename, chunkD, fps, extraFrameN, bwTimeNeeded, chunkN);

	while (NextChunkDownloaded == 0)
	{
		displayImage(firstScene);
	}

	frameCount = -1;
	while (condition)
	{

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

		if (playIndex == 30 && currentChunk > 1)
		{
			vector <Mat> temp;
			for (int i = 0; i < 2; i++)
			{
				Mat m;		temp.push_back(m);	m.release();
			}
			frameQvec[currentChunk - 1] = temp;
			cout << "memory erased..................." << endl;
		}


		if (currentChunk <= NextChunkDownloaded)
		{
			if (currentChunk == NextChunkDownloaded && currentChunk > 1)
			{
				if (ChngReriCS == 1)
				{
					reriCS = reriCScopy; cout << "reriCS updated............................" << endl;	ChngReriCS = 0;
				}
			}

			Mat frame;	frame = frameQvec[currentChunk][playIndex];

			after90Index = -1;
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			V3 vdTemp = path1.cams[cam_index].GetVD();			//cout<<"longt: "<<eri.GetXYZ2Longitude(vdTemp)<<endl;

			if (samplingValueCalculate == 1)
			{
				if (frameCount < 1)
				{
					auto firstFrame = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double> elapsedFirst = firstFrame - start; firstPlayTime = elapsedFirst.count() * 1000;
					playStart = std::chrono::high_resolution_clock::now();
					tempP = frame.clone();

				}
				path1.CRERI2ConvOptimizedWithPerFrameSamplingRateVecMinAvg(frame, var, eri, reriCS, convPixels, srVec, srVecMin, compressionFactor, path1.cams[cam_index], refCam);


			}
			else {
				path1.CRERI2ConvOptimized(frame, var, eri, reriCS, convPixels, compressionFactor, path1.cams[cam_index], refCam);

			}
			
			displayImage(convPixels);
			

			if ((playIndex) == fps * (chunkD - nextDlChunkSec))
			{
				int reqChunkN = currentChunk + 1;
				cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
				reriCScopy = reriCS;
				VD = path1.cams[cam_index].GetVD();

				filename = getChunkNametoReqnRefCamOptimized(refCam, reriCScopy, srcBaseAddr, reqChunkN, VD, tiltAngle);
				//outputFileName << filename << endl;
				cout << "New Req. " << filename << endl;
				finish1 = std::chrono::high_resolution_clock::now();
				elapsed1 = finish1 - start;
				if (samplingValueCalculate == 1)
				{
					//************* BW simulation****************//
					//bwDuration = elapsed1.count() * 1000+ bwShift; //real time spend
					bwDuration = frameCount * 33 + firstPlayTime;   //forced assumption that all frame is 33ms play time, even though they are not.
					while (bwDuration > byteVec[bwIglobal]) {
						bwIglobal++;
					}
					string name = filename;
					const char* nameChr = name.c_str();
					const char* positionMarker = strrchr(nameChr, '/');
					int start = int(positionMarker - nameChr) + 1;
					name = name.substr(start, name.size() - start);

					int namePos = 0;
					while (namePos < 262 && (name.compare(chunkName[namePos])))
					{
						namePos++;
					}
					if (namePos < 262)
					{
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = chunkSizeKB[namePos] * 1024 / 1500;
						//filename = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving_3_10_40.avi";
						//packetNeeded = Size * 1024 / 1500;		
						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms

					}
					else {
						packetNeeded = 1000 * 1024 / 1500;
						filename = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving_3_10_40.avi";
						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
					}
					//************* BW simulation****************//
				}

				cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;
				futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));

			}

		}

		if (after90Index >= (extraFrameN - 1))
		{
			frameCount--;  //reverse framecount here, cause no frame came
			frRealT.push_back(0);
			srRealT.push_back(0);
			srMinRealT.push_back(0);
			currentChunk = (frameCount) / (fps * chunkD) + 1;
			//cout << currentChunk<<" currentChunk: Next " <<NextChunkDownloaded<< endl;
			auto delayStart = std::chrono::high_resolution_clock::now();
			auto delayEnd = std::chrono::high_resolution_clock::now();;
			std::chrono::duration<double> delayElapsed = delayEnd - delayStart;
			int duration = delayElapsed.count() * 1000;
			while (duration < 30)
			{
				delayEnd = std::chrono::high_resolution_clock::now();;
				delayElapsed = delayEnd - delayStart;
				duration = delayElapsed.count() * 1000;
			}
			delayTotal = delayTotal + 30;
			cout << "delayTotal: " << delayTotal << endl;
		}

		if (((currentChunk) > NextChunkDownloaded) && currentChunk > 1 && after90Index < extraFrameN - 1)
		{
			after90Index++; //0->
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			int index = nonUniformList[after90Index];
			int index2 = nonUniformList[after90Index + 1];

			int diffIndex = index2 - index;

			float currentFR = 30.0f / diffIndex;
			if (currentFR < minFR)
			{
				minFR = 30.0f / diffIndex;
			}

			cout << "diffIndex: " << diffIndex << "minFR: " << minFR << endl;

			frameRate4allFrames.push_back(1);

			if (diffIndex > 100 || diffIndex < 0)
			{
				cout << "diffIndex: " << diffIndex << endl;
				condition = 0;
				break;
			}
			cout << "diffIndex: " << diffIndex << endl;
			for (int i = 0; i < diffIndex; i++)
			{
				if (i > 0) {
					frameCount++;
				}
				currentChunk = (frameCount) / (fps * chunkD) + 1; //current chunk 0 theke start, file chunk o 0 theke start krote hobe encoding er somoy
				playIndex = frameCount - (currentChunk - 1) * (fps * chunkD);
				cout << "119CuntChunkN=" << currentChunk - 1 << " frmcount=" << frameCount << " plCnt=" << playIndex << " after90indx=" << after90Index << " crntChnk: " << NextChunkDownloaded << endl;
				//	cout << frameCount << endl;
				cam_index = path1.GetCamIndex(frameCount - 1, fps, cam_index);
				Mat frame1;
				//frame1 = frameQvec[currentChunk - 1][(fps * chunkD - 1) + after90Index];
				//frame1 = frameQvec[currentChunk - 1][(fps * chunkD - 1)];
				frame1 = tempP;
				float frameRateTemp = diffIndex;

				if (samplingValueCalculate == 1)
				{

					path1.CRERI2ConvOptimizedWithPerFrameSamplingRateVecMinAvg(frame1, var, eri, reriCS, convPixels, srVec, srVecMin, compressionFactor, path1.cams[cam_index], refCam);
				}
				else {
					path1.CRERI2ConvOptimized(frame1, var, eri, reriCS, convPixels, compressionFactor, path1.cams[cam_index], refCam);

				}

				displayImage(convPixels);
				


				if (playIndex == (fps * (chunkD - nextDlChunkSec)))
				{
					int reqChunkN = currentChunk + 1; //amader chunk 0 theke start hoy but download chunk start hoy 1 theke , apatoto			

					cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
					VD = path1.cams[cam_index].GetVD();
					reriCScopy = reriCS;

					filename = getChunkNametoReqnRefCamOptimized(refCam, reriCScopy, srcBaseAddr, reqChunkN, VD, tiltAngle);
					//outputFileName << filename << endl;
					cout << "NewR: " << filename << endl;
					finish1 = std::chrono::high_resolution_clock::now();
					elapsed1 = finish1 - start;
					//************* BW simulation****************//
					if (samplingValueCalculate == 1)
					{
						//bwDuration = elapsed1.count() * 1000+bwShift;
						bwDuration = frameCount * 33 + firstPlayTime;   //forced assumption that all frame is 33ms play time, even though they are not.
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}
						string name = filename;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						int namePos = 0;
						while (namePos < 262 && (name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						if (namePos < 262)
						{
							cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
							packetNeeded = chunkSizeKB[namePos] * 1024 / 1500;
							//filename = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving_3_10_40.avi";
							//packetNeeded = Size * 1024 / 1500;		
							bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms

						}
						else {
							packetNeeded = 1000 * 1024 / 1500;
							filename = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf30/diving_3_10_40.avi";
							bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
						}

					}
					cout << "Later DLs req Time:------>" << bwDuration << " bwStartIndx---> " << bwIglobal << " bwEndIndex-->" << bwIglobal + bwTimeNeeded << endl;

					//************* BW simulation****************//
					futures.push_back(std::async(std::launch::async, DownLoadChunk4thSecVar, filename, chunkD, fps, extraFrameN, bwTimeNeeded, reqChunkN));


				}


			}

		}

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

	PPC cameraTemp = camera;
	cameraTemp.Pan(72.0f);

	for (int v = 0; v < camera.h; v++)
	{
		for (int u = 0; u < camera.w; u++)
		{
			eri.EachPixelConv2ERI(cameraTemp, u, v, pixelI, pixelJ);
			int Xtile = floor(pixelJ * m / eri.w); //m*n col and row
			int Ytile = floor(pixelI * n / eri.h);
			int vectorindex = (Ytile)*m + Xtile;
			tileBitMap.at(vectorindex) = 1;

		}

	}

	PPC cameraTemp1 = camera;
	cameraTemp1.Pan(-72.0f);

	for (int v = 0; v < camera.h; v++)
	{
		for (int u = 0; u < camera.w; u++)
		{
			eri.EachPixelConv2ERI(cameraTemp1, u, v, pixelI, pixelJ);
			int Xtile = floor(pixelJ * m / eri.w); //m*n col and row
			int Ytile = floor(pixelI * n / eri.h);
			int vectorindex = (Ytile)*m + Xtile;
			tileBitMap.at(vectorindex) = 1;

		}

	}


	PPC cameraTemp2 = camera;
	cameraTemp2.Tilt(36.0f);

	for (int v = 0; v < camera.h; v++)
	{
		for (int u = 0; u < camera.w; u++)
		{
			eri.EachPixelConv2ERI(cameraTemp2, u, v, pixelI, pixelJ);
			int Xtile = floor(pixelJ * m / eri.w); //m*n col and row
			int Ytile = floor(pixelI * n / eri.h);
			int vectorindex = (Ytile)*m + Xtile;
			tileBitMap.at(vectorindex) = 1;

		}

	}


	PPC cameraTemp3 = camera;
	cameraTemp.Tilt(-36.0f);

	for (int v = 0; v < camera.h; v++)
	{
		for (int u = 0; u < camera.w; u++)
		{
			eri.EachPixelConv2ERI(cameraTemp3, u, v, pixelI, pixelJ);
			int Xtile = floor(pixelJ * m / eri.w); //m*n col and row
			int Ytile = floor(pixelI * n / eri.h);
			int vectorindex = (Ytile)*m + Xtile;
			tileBitMap.at(vectorindex) = 1;
		}

	}



}
void getTilesNumber2reqFovOnly(vector<int>& tileBitMap, PPC camera, ERI& eri, int m, int n)
{
	int pixelI, pixelJ = 0;
	for (int v = 0; v < camera.h; v++)
	{
		for (int u = 0; u < camera.w; u++)
		{
			eri.EachPixelConv2ERI(camera, u, v, pixelI, pixelJ);
			int Xtile = floor(pixelJ * m / eri.w); //m*n col and row
			int Ytile = floor(pixelI * n / eri.h);
			int vectorindex = (Ytile)*m + Xtile;
			tileBitMap.at(vectorindex) = 1;
		}

	}

}

void testDownloadVideoHttpTileBlankPixelMeasurementFoVPlus(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN)
{
	float corePredictionMargin =1;
	int mxChunkN = 55;
	auto start = std::chrono::high_resolution_clock::now();
	Path path1;
	float var[10];

	LoadBWTraceData(bwLog);
	vector<float> chunkSizeKB; vector<string>chunkName;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/crf30/1s/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		KB = KB / 1024.0f;
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);
	//	cout << "kb: " << KB << endl;

	}
	ifstream  f("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	string line;
	int ii = 0;
	while (std::getline(f, line))
	{
		var[ii] = stoi(line);
		cout << var[ii] << '\n';
		ii++;
	}

	int frameCol = var[0];
	int frameRow = var[1];
	float hfov = 90.0f;
	
	int w = frameCol * hfov / 360; 
	int h = frameRow * hfov / 360; 
	PPC camera2(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
		
	path1.LoadHMDTrackingData(hmdFileName, camera2);
	
	int chunkD = 1;
	int chunkN = 1;
	int fps = 30;
	Mat firstScene;
	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
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
	int bwTimeNeeded = 200; 
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
	int tileRowN = rowM;
	int tileColN = colN;
	

	cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;
	
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

			string name = filename;
			cout << filename << endl;
			const char* nameChr = name.c_str();
			const char* positionMarker = strrchr(nameChr, '/');
			int start = int(positionMarker - nameChr) + 1;
			name = name.substr(start, name.size() - start);
			cout << name << " " << chunkName[0] << endl;
			int namePos = 0;
			while ((name.compare(chunkName[namePos])))
			{
				namePos++;
			}
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
			packetNeeded = 12*chunkSizeKB[namePos] * 1024 / 1500;			
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
	vector<int> totalInSide;
	vector<int>totalChunkReq;
	int totalTileLoaded = 0;
	int totalDelay = 0;
	totalChunkReq.push_back(reqTiles[chunkN].size());
	while (NextChunkDownloaded < reqTiles[chunkN].size() - 1)
	{
		displayImage33ms();
		//displayImage(firstScene);
		timePassed = timePassed + 30.0f;
		waitKey(20);
		cout << "in first while" << endl;
	}

	totalTileLoaded = totalTileLoaded + NextChunkDownloaded;
	NextChunkDownloaded = 0;
	cout << tileDlSum << endl;

	int playStartTime = 0;
	int tileCondition = 1;
	frameCount = -1;
	while (tileCondition)
	{		
		frameCount++;
		timePassed = timePassed + 30.0f;

		if (frameCount == (fps * chunkD * mxChunkN - 1))
		{
			tileCondition = 0;
		}
		if (frameCount == 0)
		{
			auto firstFrame = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsedFirst = firstFrame - playStart; 
			playStartTime = elapsedFirst.count() * 1000;			
		}

		int currentChunk = (frameCount) / (fps * chunkD) + 1;
		int playIndex = frameCount - (currentChunk - 1) * (fps * chunkD); //0-(fps*chunkD-1)
		cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex  <<"passedTime: ->"<<timePassed<< endl;

		if ((playIndex) == fps * (chunkD - nextDlChunkSec))
		{			
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			vector <int> tileBitMap2;
			for (int i = 0; i < tileRowN*tileColN; i++)
			{
				tileBitMap2.push_back(0);
			}
			getTilesNumber2req(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
			int reqChunkN = currentChunk + 1;
			for (int i = 0; i < tileRowN*tileColN; i++)
			{
				//cout << i << "--" << tileBitMap2[i] << endl;
				
				if (reqChunkN <= mxChunkN)
				{
					if (tileBitMap2[i] == 1)
					{
						filename = getChunkNametoReq4mTiles(srcBaseAddr, reqChunkN, i);
						cout << filename << endl;
						reqTiles[reqChunkN].push_back(i);
						auto finish1 = std::chrono::high_resolution_clock::now();
						bwDuration = bwDuration = frameCount * 33 +playStartTime;
						bwIglobal = 0;
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}

						string name = filename;
						cout << filename << endl;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						int namePos = 0;
						while ((name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = 4 * chunkSizeKB[namePos] * 1024 / 1500;

						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
						timePassed = 0;
						cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
						futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded));
					}
				}
			}

			totalChunkReq.push_back(reqTiles[reqChunkN].size());
		}

		if (frameCount == (fps * chunkD - 1))
		{
			auto delayStart = std::chrono::high_resolution_clock::now();
			while (NextChunkDownloaded < reqTiles[currentChunk + 1].size() - 1)
			{
				timePassed = timePassed + 30.0f;
				displayImage33ms();
				totalDelay = totalDelay + 33;
			}
			currentChunk = currentChunk + 1;
			auto delayFinish = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> delayElapsed = delayFinish - delayStart;
			

			cout << "Delay Added.................: " << totalDelay << endl;
		}

		cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

		cout << "just before:: " << currentChunk << " " << reqTiles[currentChunk].size() << endl;
		
		eri.ERI2Conv4tiles(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex, totalInSide);
		displayImage(convPixels);
		//displayImage33ms();

	
	}
	long sumtotalInSide;
	for (int i = 0; i < totalInSide.size(); i++)
	{
		cout << i << " " << totalInSide[i] << endl;
		sumtotalInSide = sumtotalInSide + totalInSide[i];
	}

	int totalChunk = 0;
	for (int i = 0; i < totalChunkReq.size() - 1; i++)
	{
		totalChunk = totalChunk + totalChunkReq[i];
		cout << i << " " << totalChunkReq[i] << endl;
	}

	cout << sumtotalInSide << endl;

	double avgtotalOutSide = 100 - sumtotalInSide / (double)(totalInSide.size());
	ofstream output;
	output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/tileBlankTotalDataFoVPlus.txt", std::ios::out | std::ios::app);
	output<< srcBaseAddr << " blank%: " << avgtotalOutSide << "totalChunk2req: " << totalChunk << endl;
	output.close();

	cout << srcBaseAddr << " blank%: " << avgtotalOutSide << "totalChunk2req: " << totalChunk << endl;
	

}

void testDownloadVideoHttpTileBlankPixelMeasurementFoV(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN)
{
	int mxChunkN = 55;
	float corePredictionMargin = 1;
	auto start = std::chrono::high_resolution_clock::now();
	Path path1;
	float var[10];

	LoadBWTraceData(bwLog);
	vector<float> chunkSizeKB; vector<string>chunkName;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/crf30/1s/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		KB = KB / 1024.0f;
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);
		//cout << "kb: " << KB << endl;

	}
	ifstream  f("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	string line;
	int ii = 0;
	while (std::getline(f, line))
	{
		var[ii] = stoi(line);
		cout << var[ii] << '\n';
		ii++;
	}

	int frameCol = var[0];
	int frameRow = var[1];
	float hfov = 90.0f;
	
	int w = frameCol * hfov / 360;
	int h = frameRow * hfov / 360;
	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);

	path1.LoadHMDTrackingData(hmdFileName, camera2);
	
	int chunkD = 1;
	int chunkN = 1;
	int fps = 30;
	Mat firstScene;
	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
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

	for (int i = 0; i < tilesCol * tilesRow; i++)
	{
		vector<vector <Mat>> temp1;
		for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
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
	int bwTimeNeeded = 200;
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
	int tileRowN = rowM;
	int tileColN = colN;


	cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;

	cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

	vector <int> tileBitMap;
	for (int i = 0; i < tileRowN * tileColN; i++)
	{
		tileBitMap.push_back(0);
	}

	getTilesNumber2reqFovOnly(tileBitMap, path1.cams[cam_index], eri, tileColN, tileRowN);



	auto playStart = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < tileRowN * tileColN; i++)
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

			string name = filename;
			cout << filename << endl;
			const char* nameChr = name.c_str();
			const char* positionMarker = strrchr(nameChr, '/');
			int start = int(positionMarker - nameChr) + 1;
			name = name.substr(start, name.size() - start);
			cout << name << " " << chunkName[0] << endl;
			int namePos = 0;
			while ((name.compare(chunkName[namePos])))
			{
				namePos++;
			}
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
			packetNeeded = 12 * chunkSizeKB[namePos] * 1024 / 1500;
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
	vector<int> totalInSide;
	vector<int>totalChunkReq;
	int totalTileLoaded = 0;
	int totalDelay = 0;
	totalChunkReq.push_back(reqTiles[chunkN].size());
	while (NextChunkDownloaded < reqTiles[chunkN].size() - 1)
	{
		displayImage33ms();
		//displayImage(firstScene);
		timePassed = timePassed + 30.0f;
		waitKey(20);
		cout << "in first while" << endl;
	}

	totalTileLoaded = totalTileLoaded + NextChunkDownloaded;
	NextChunkDownloaded = 0;
	cout << tileDlSum << endl;

	int playStartTime = 0;
	int tileCondition = 1;
	frameCount = -1;
	while (tileCondition)
	{
		frameCount++;
		timePassed = timePassed + 30.0f;

		if (frameCount == (fps * chunkD * mxChunkN - 1))
		{
			tileCondition = 0;
		}
		if (frameCount == 0)
		{
			auto firstFrame = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsedFirst = firstFrame - playStart;
			playStartTime = elapsedFirst.count() * 1000;
		}

		int currentChunk = (frameCount) / (fps * chunkD) + 1;
		int playIndex = frameCount - (currentChunk - 1) * (fps * chunkD); //0-(fps*chunkD-1)
		cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex << "passedTime: ->" << timePassed << endl;

		if ((playIndex) == fps * (chunkD - nextDlChunkSec))
		{
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			vector <int> tileBitMap2;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				tileBitMap2.push_back(0);
			}
			getTilesNumber2reqFovOnly(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
			int reqChunkN = currentChunk + 1;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				//cout << i << "--" << tileBitMap2[i] << endl;

				if (reqChunkN <= mxChunkN)
				{
					if (tileBitMap2[i] == 1)
					{
						filename = getChunkNametoReq4mTiles(srcBaseAddr, reqChunkN, i);
						cout << filename << endl;
						reqTiles[reqChunkN].push_back(i);
						auto finish1 = std::chrono::high_resolution_clock::now();
						bwDuration = bwDuration = frameCount * 33 + playStartTime;
						bwIglobal = 0;
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}

						string name = filename;
						cout << filename << endl;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						int namePos = 0;
						while ((name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = 4 * chunkSizeKB[namePos] * 1024 / 1500;

						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
						timePassed = 0;
						cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
						futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded));
					}
				}
			}

			totalChunkReq.push_back(reqTiles[reqChunkN].size());
		}

		if (frameCount == (fps * chunkD - 1))
		{
			auto delayStart = std::chrono::high_resolution_clock::now();
			while (NextChunkDownloaded < reqTiles[currentChunk + 1].size() - 1)
			{
				timePassed = timePassed + 30.0f;
				displayImage33ms();
				totalDelay = totalDelay + 33;
			}
			currentChunk = currentChunk + 1;
			auto delayFinish = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> delayElapsed = delayFinish - delayStart;


			cout << "Delay Added.................: " << totalDelay << endl;
		}

		cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

		
		eri.ERI2Conv4tiles(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex, totalInSide);
		//displayImage(convPixels);
		displayImage33ms();


	}
	long sumtotalInSide;
	for (int i = 0; i < totalInSide.size(); i++)
	{
		cout << i << " " << totalInSide[i] << endl;
		sumtotalInSide = sumtotalInSide + totalInSide[i];
	}

	int totalChunk = 0;
	for (int i = 0; i < totalChunkReq.size() - 1; i++)
	{
		totalChunk = totalChunk + totalChunkReq[i];
		cout << i << " " << totalChunkReq[i] << endl;
	}

	cout << sumtotalInSide << endl;

	double avgtotalOutSide = 100-sumtotalInSide/ (double)(totalInSide.size());
	ofstream output;
	output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/tileBlankTotalDataFoV.txt", std::ios::out | std::ios::app);
	output << srcBaseAddr << " blank%: " << avgtotalOutSide << "totalChunk2req: " << totalChunk << endl;
	output.close();
	
	cout <<srcBaseAddr<< " blank%: " <<avgtotalOutSide << "totalChunk2req: " << totalChunk << endl;
}


void testDownloadVideoHttpTileDelayMeasurementFoVPlus(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN)
{
	int fovPlusOrno = 1;   //1 for fovPlus, 0 for fov
	float corePredictionMargin = 0.6;
	int mxChunkN = 56;
	float accuracy = 100.0f;  //prediction accuracy
	int FrameN2PredictAhead = 0;

	auto start = std::chrono::high_resolution_clock::now();
	Path path1;
	float var[10];

	LoadBWTraceData(bwLog);
	int mahimahiPackSize = 1400;
	vector<float> chunkSizeKB; vector<string>chunkName;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/crf30/1s/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		KB = KB / 1024.0f;
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);
		//cout << "kb: " << KB << endl;

	}
	ifstream  f("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	string line;
	int ii = 0;
	while (std::getline(f, line))
	{
		var[ii] = stoi(line);
		cout << var[ii] << '\n';
		ii++;
	}

	int frameCol = var[0];
	int frameRow = var[1];
	float hfov = 90.0f;
	
	int w = frameCol * hfov / 360;
	int h = frameRow * hfov / 360;
	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);

	path1.LoadHMDTrackingData(hmdFileName, camera2);
	
	int chunkD = 1;
	int chunkN = 1;
	int fps = 30;
	Mat firstScene;
	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
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

	for (int i = 0; i < tilesCol * tilesRow; i++)
	{
		vector<vector <Mat>> temp1;
		for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
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
	int bwTimeNeeded = 200;
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
	int tileRowN = rowM;
	int tileColN = colN;

	int chunkBwDivider = 12;

	if (fovPlusOrno==0)
	{
		chunkBwDivider = 5;
	}

	cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;

	
	//select accuracy of prediction//
	int frameCountForAccuracy = frameCount + 0;
	cam_index = path1.GetCamIndex(frameCountForAccuracy, fps, cam_index);
	PPC camera4req = path1.cams[cam_index];
	camera4req.Pan(180 * (100 - accuracy) / 100.0f);
	camera4req.Tilt(90 * (100 - accuracy) / 100.0f);


	vector <int> tileBitMap;
	for (int i = 0; i < tileRowN * tileColN; i++)
	{
		tileBitMap.push_back(0);
	}
	if (fovPlusOrno==0)
	{
		getTilesNumber2reqFovOnly(tileBitMap, camera4req, eri, tileColN, tileRowN);
	}
	
	else
	{
		getTilesNumber2req(tileBitMap, camera4req, eri, tileColN, tileRowN);
	}
	
	auto playStart = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < tileRowN * tileColN; i++)
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

			string name = filename;
			cout << filename << endl;
			const char* nameChr = name.c_str();
			const char* positionMarker = strrchr(nameChr, '/');
			int start = int(positionMarker - nameChr) + 1;
			name = name.substr(start, name.size() - start);
			cout << name << " " << chunkName[0] << endl;
			int namePos = 0;
			while ((name.compare(chunkName[namePos])))
			{
				namePos++;
			}
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
			packetNeeded = chunkBwDivider * chunkSizeKB[namePos] * 1024 / mahimahiPackSize;
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
	vector<int> totalInSide;
	vector<float>blankPixels; 
	vector<float>frameRate;
	vector<float> samplingRateAvg;
	vector<float> samplingRateMin;
	vector<int>totalChunkReq;
	vector<int>T;
	float tileProcessT = 0;
	int totalTileLoaded = 0;
	int totalDelay = 0;
	long totalProcessT=0;
	
	auto chunkStarT = std::chrono::high_resolution_clock::now();
	totalChunkReq.push_back(reqTiles[chunkN].size());
	while(0)// (NextChunkDownloaded < reqTiles[chunkN].size() - 1)
	{
		displayImage33ms();
		//displayImage(firstScene);
		timePassed = timePassed + 30.0f;
		waitKey(20);
		cout << "in first while" << endl;
	}

	totalTileLoaded = totalTileLoaded + NextChunkDownloaded;
	NextChunkDownloaded = 0;
	cout << tileDlSum << endl;

	int playStartTime = 0;
	int tileCondition = 1;
	frameCount = -1;

	auto delayMeasureStart = std::chrono::high_resolution_clock::now();
	auto reqSend = std::chrono::high_resolution_clock::now();
	int maxDelay = 0;

	while (tileCondition)
	{
		frameCount++;
		timePassed = timePassed + 30.0f;

		if (frameCount == (fps * chunkD * mxChunkN - 2))
		{
			tileCondition = 0;
		}
		if (frameCount == 0)
		{
			auto firstFrame = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsedFirst = firstFrame - playStart;
			playStartTime = elapsedFirst.count() * 1000;
		}

		int currentChunk = (frameCount) / (fps * chunkD) + 1;
		int playIndex = frameCount - (currentChunk - 1) * (fps * chunkD); //0-(fps*chunkD-1)
		cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex << "passedTime: ->" << timePassed << endl;
		cout << frameCount;
		if (playIndex==1)
		{
			chunkStarT = std::chrono::high_resolution_clock::now();

		}
		if ((playIndex) == fps * (chunkD - nextDlChunkSec))
		{
			frameCountForAccuracy = frameCount + FrameN2PredictAhead;
			cam_index = path1.GetCamIndex(frameCountForAccuracy, fps, cam_index);
			camera4req = path1.cams[cam_index];
			camera4req.Pan(180 * (100 - accuracy) / 100.0f);
			camera4req.Tilt(90 * (100 - accuracy) / 100.0f);

			vector <int> tileBitMap2;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				tileBitMap2.push_back(0);
			}

			if (fovPlusOrno == 0)
			{
				getTilesNumber2reqFovOnly(tileBitMap2, camera4req, eri, tileColN, tileRowN);
			}

			else
			{
				getTilesNumber2req(tileBitMap2, camera4req, eri, tileColN, tileRowN);
			}


			//getTilesNumber2req(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
			int reqChunkN = currentChunk + 1;
			NextChunkDownloaded = 0;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				//cout << i << "--" << tileBitMap2[i] << endl;

				if (reqChunkN <= mxChunkN)
				{					
					if (tileBitMap2[i] == 1)
					{
						filename = getChunkNametoReq4mTiles(srcBaseAddr, reqChunkN, i);
						//cout << filename << endl;
						reqTiles[reqChunkN].push_back(i);
						auto finish1 = std::chrono::high_resolution_clock::now();
						bwDuration = bwDuration = frameCount * 33 + playStartTime+totalDelay;
						bwIglobal = 0;
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}

						string name = filename;
						cout << filename << endl;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						int namePos = 0;
						while ((name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded =chunkBwDivider * chunkSizeKB[namePos] * 1024 / mahimahiPackSize;

						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
						timePassed = 0;
						cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
						if (maxDelay<bwTimeNeeded)
						{
							maxDelay = bwTimeNeeded;
						}
						reqSend = std::chrono::high_resolution_clock::now();
						//futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded));
						std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded);
					}
					
				}
			}
			float totalDlTime = maxDelay;
			float tpTime = reqTiles[reqChunkN].size() * 49;
			tileProcessT = totalDlTime + tpTime;
			totalProcessT = totalProcessT + tileProcessT/100.0f;
			maxDelay = 0;
			cout << "tileProcessT: ---------------------------------------->" << tileProcessT << endl;
			totalChunkReq.push_back(reqTiles[reqChunkN].size());
		}

		if (playIndex == (fps * chunkD - 1))
		{			
			if(33*chunkD*fps<tileProcessT)
			{
				int f = abs(33 * chunkD * fps - tileProcessT) / 33; 
				totalDelay = totalDelay + 33*f;
				for (int i = 0; i < f; i++)
				{
					//blankPixels.push_back(1);
					//frameRate.push_back(0);
					samplingRateMin.push_back(0);
					samplingRateAvg.push_back(0);

				}

				cout << "Delay Added........................................................................: " << totalDelay <<",tileProcesT:"<<tileProcessT<<",added Now:"<<33*f<< endl;

			}
			
		}

		cam_index = path1.GetCamIndex(frameCount, fps, cam_index);	
		//eri.ERI2Conv4tilesDoNothing(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex, totalOutSide);
		float blankP, srmiN, sravG = 0;
		eri.ERI2Conv4tilesWithSrFrDelay(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex, blankP, sravG, srmiN);
		//displayImage(convPixels);
		blankPixels.push_back(blankP);
		frameRate.push_back(30);
		samplingRateAvg.push_back(sravG);
		samplingRateMin.push_back(srmiN);
		displayImage33ms();


	}
	auto delayMeasurementTimeEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedDelay = delayMeasurementTimeEnd-delayMeasureStart;
	
	ofstream output;
	output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/tile_BP_FR_SR_Fov_detailed.txt", std::ios::out | std::ios::app);
	output << hmdFileName << "," << bwLog << endl;
	int totalbP = 0;
	int totalFR = 0;
	for (int i = 0; i < blankPixels.size(); i++)
	{
		totalbP = totalbP + blankPixels[i];
		output << filename << ","<< "blankP:" << blankPixels[i] << ",frameR:" << frameRate[i] << ", srAvg:" << samplingRateAvg[i] << ",srMin:" << samplingRateMin[i] << endl;
		
		totalFR = totalFR + frameRate[i];
	}
	float avgFR = 30*totalFR / frameRate.size();
	float bp = totalbP / blankPixels.size();
	output.close();

	int totalTile = 0;
	for (int i = 0; i <totalChunkReq.size(); i++)
	{
		totalTile = totalChunkReq[i] + totalTile;
	}

	ofstream output1;
	output1.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/tile_FoV_concise.txt", std::ios::out | std::ios::app);
	cout<< "Delay OldWay: " << totalDelay <<",bp:"<<bp<<endl;
	
	output1 <<srcBaseAddr<<bwLog<<hmdFileName<<": "<< "Delay: " << totalDelay << ",bp:" << bp<<", totalTileChunk:"<<totalTile<<" Fravg:"<<avgFR<< endl;
	output1.close();
}

void testDownloadVideoHttpTileDelayMeasurementFoVOnlyAndroid(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN)
{
	int fovPlusOrno = 0;   //1 for fovPlus, 0 for fov
	float corePredictionMargin = 0.7;
	int mxChunkN = 56;

	auto start = std::chrono::high_resolution_clock::now();
	Path path1;
	float var[10];

	LoadBWTraceData(bwLog);
	int mahimahiPackSize = 1400;
	vector<float> chunkSizeKB; vector<string>chunkName;  vector <int> panVec;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/crf30/1s/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		KB = KB / 1024.0f;
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);
		//cout << "kb: " << KB << endl;

	}
	ifstream  f("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	string line;
	int ii = 0;
	while (std::getline(f, line))
	{
		var[ii] = stoi(line);
		cout << var[ii] << '\n';
		ii++;
	}

	int frameCol = var[0];
	int frameRow = var[1];
	float hfov = 90.0f;

	int w = frameCol * hfov / 360;
	int h = frameRow * hfov / 360;
	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	PPC refCam(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);

	//path1.LoadHMDTrackingData(hmdFileName, camera2);

	/*android hmd loaded*/

	ifstream  filex(hmdFileName);
	if (!filex)
	{
		print("error: can't open file: " << hmdFileName << endl);
		system("pause");
	}
	string   line2;
	while (getline(filex, line2))
	{
		stringstream  linestream(line2);
		int byte;
		linestream >> byte;
		panVec.push_back(byte);
		//cout << byte << endl;
	}

	/* android hmd loaded done*/

	int chunkD = 1;
	int chunkN = 1;
	int fps = 30;
	Mat firstScene;
	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
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

	for (int i = 0; i < tilesCol * tilesRow; i++)
	{
		vector<vector <Mat>> temp1;
		for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
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
	int bwTimeNeeded = 200;
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
	int tileRowN = rowM;
	int tileColN = colN;

	int chunkBwDivider = 12;

	if (fovPlusOrno == 0)
	{
		chunkBwDivider = 5;
	}



	cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;

	PPC camex=refCam;
	camex.Pan(panVec[frameCount]);

	vector <int> tileBitMap;
	for (int i = 0; i < tileRowN * tileColN; i++)
	{
		tileBitMap.push_back(0);
	}
	if (fovPlusOrno == 0)
	{
		getTilesNumber2reqFovOnly(tileBitMap, camex, eri, tileColN, tileRowN);
	}

	else
	{
		getTilesNumber2req(tileBitMap, camex, eri, tileColN, tileRowN);
	}



	auto playStart = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < tileRowN * tileColN; i++)
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

			string name = filename;
			cout << filename << endl;
			const char* nameChr = name.c_str();
			const char* positionMarker = strrchr(nameChr, '/');
			int start = int(positionMarker - nameChr) + 1;
			name = name.substr(start, name.size() - start);
			cout << name << " " << chunkName[0] << endl;
			int namePos = 0;
			while ((name.compare(chunkName[namePos])))
			{
				namePos++;
			}
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
			packetNeeded = chunkBwDivider * chunkSizeKB[namePos] * 1024 / mahimahiPackSize;
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
	vector<int> totalInSide;
	vector<float>blankPixels;
	vector<float>frameRate;
	vector<float> samplingRateAvg;
	vector<float> samplingRateMin;
	vector<int>totalChunkReq;
	vector<int>T;
	float tileProcessT = 0;
	int totalTileLoaded = 0;
	int totalDelay = 0;
	long totalProcessT = 0;

	auto chunkStarT = std::chrono::high_resolution_clock::now();
	totalChunkReq.push_back(reqTiles[chunkN].size());
	while (0)// (NextChunkDownloaded < reqTiles[chunkN].size() - 1)
	{
		displayImage33ms();
		//displayImage(firstScene);
		timePassed = timePassed + 30.0f;
		waitKey(20);
		cout << "in first while" << endl;
	}

	totalTileLoaded = totalTileLoaded + NextChunkDownloaded;
	NextChunkDownloaded = 0;
	cout << tileDlSum << endl;

	int playStartTime = 0;
	int tileCondition = 1;
	frameCount = -1;

	auto delayMeasureStart = std::chrono::high_resolution_clock::now();
	auto reqSend = std::chrono::high_resolution_clock::now();
	int maxDelay = 0;

	while (tileCondition)
	{
		frameCount++;
		timePassed = timePassed + 30.0f;

		if (frameCount == (fps * chunkD * mxChunkN - 2))
		{
			tileCondition = 0;
		}
		if (frameCount == 0)
		{
			auto firstFrame = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsedFirst = firstFrame - playStart;
			playStartTime = elapsedFirst.count() * 1000;
		}

		int currentChunk = (frameCount) / (fps * chunkD) + 1;
		int playIndex = frameCount - (currentChunk - 1) * (fps * chunkD); //0-(fps*chunkD-1)
		cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex << "passedTime: ->" << timePassed << endl;
		cout << frameCount;
		if (playIndex == 1)
		{
			chunkStarT = std::chrono::high_resolution_clock::now();

		}
		if ((playIndex) == fps * (chunkD - nextDlChunkSec))
		{
			PPC camex = refCam;
			camex.Pan(panVec[frameCount]);
			vector <int> tileBitMap2;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				tileBitMap2.push_back(0);
			}

			if (fovPlusOrno == 0)
			{
				getTilesNumber2reqFovOnly(tileBitMap2, camex, eri, tileColN, tileRowN);
			}

			else
			{
				getTilesNumber2req(tileBitMap2, camex, eri, tileColN, tileRowN);
			}
			
			int reqChunkN = currentChunk + 1;
			NextChunkDownloaded = 0;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				//cout << i << "--" << tileBitMap2[i] << endl;

				if (reqChunkN <= mxChunkN)
				{
					if (tileBitMap2[i] == 1)
					{
						filename = getChunkNametoReq4mTiles(srcBaseAddr, reqChunkN, i);
						//cout << filename << endl;
						reqTiles[reqChunkN].push_back(i);
						auto finish1 = std::chrono::high_resolution_clock::now();
						bwDuration = bwDuration = frameCount * 33 + playStartTime + totalDelay;
						bwIglobal = 0;
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}

						string name = filename;
						cout << filename << endl;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						int namePos = 0;
						while ((name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = chunkBwDivider * chunkSizeKB[namePos] * 1024 / mahimahiPackSize;

						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
						timePassed = 0;
						cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
						if (maxDelay < bwTimeNeeded)
						{
							maxDelay = bwTimeNeeded;
						}
						reqSend = std::chrono::high_resolution_clock::now();
						//futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded));
						std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded);
					}

				}
			}
			float totalDlTime = maxDelay;
			float tpTime = reqTiles[reqChunkN].size() * 49;
			tileProcessT = totalDlTime + tpTime;
			totalProcessT = totalProcessT + tileProcessT / 100.0f;
			maxDelay = 0;
			cout << "tileProcessT: ---------------------------------------->" << tileProcessT << endl;
			totalChunkReq.push_back(reqTiles[reqChunkN].size());
		}

		if (playIndex == (fps * chunkD - 1))
		{
			if (33 * chunkD * fps < tileProcessT)
			{
				int f = abs(33 * chunkD * fps - tileProcessT) / 33;
				totalDelay = totalDelay + 33 * f;
				for (int i = 0; i < f; i++)
				{
					//blankPixels.push_back(100);
					//frameRate.push_back(0);
					samplingRateMin.push_back(0);
					samplingRateAvg.push_back(0);

				}

				cout << "Delay Added........................................................................: " << totalDelay << ",tileProcesT:" << tileProcessT << ",added Now:" << 33 * f << endl;

			}
		
		}
		PPC camex = refCam;
		camex.Pan(panVec[frameCount]);
		//eri.ERI2Conv4tilesDoNothing(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex, totalOutSide);
		float blankP, srmiN, sravG = 0;
		eri.ERI2Conv4tilesWithSrFrDelay(convPixels, frameQvecTiles, reqTiles[currentChunk], camex, tileColN, tileRowN, currentChunk, playIndex, blankP, sravG, srmiN);
		displayImage(convPixels);
		blankPixels.push_back(blankP);
		frameRate.push_back(30);
		samplingRateAvg.push_back(sravG);
		samplingRateMin.push_back(srmiN);
		displayImage33ms();


	}
	auto delayMeasurementTimeEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedDelay = delayMeasurementTimeEnd - delayMeasureStart;

	ofstream output;
	output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/android_tile_Diving_BP_FR_SRminFov_D_646_FoVonly.txt", std::ios::out | std::ios::app);
	output << hmdFileName << "," << bwLog << endl;
	int totalbP = 0;
	
	int totalFR = 0;
	for (int i = 0; i < blankPixels.size(); i++)
	{
		totalbP = totalbP + blankPixels[i];
		output << filename << "," << "blankP:" << blankPixels[i] << ",frameR:" << frameRate[i] << ", srAvg:" << samplingRateAvg[i] << ",srMin:" << samplingRateMin[i] << endl;
		totalFR = totalFR + frameRate[i];
	}
	float avgFR = 30 * totalFR / frameRate.size();
	float bp = totalbP / blankPixels.size();
	output.close();

	int totalTile = 0;
	for (int i = 0; i < totalChunkReq.size(); i++)
	{
		totalTile = totalChunkReq[i] + totalTile;
	}

	ofstream output1;
	output1.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/AndroidTileFoVonly.txt", std::ios::out | std::ios::app);
	cout << "Delay OldWay: " << totalDelay << ",bp:" << bp << endl;

	output1 << srcBaseAddr << bwLog << hmdFileName << ": " << "Delay: " << totalDelay << ",bp:" << bp << ", totalTileChunk:" << totalTile << " Fravg:" << avgFR << endl;
	output1.close();

}


void testDownloadVideoHttpTileDelayMeasurementFoVMobisysBoth(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN, int fovOnly, int accuracy1)
{
	int fovPlusOrno = fovOnly;   //1 for fovPlus, 0 for fov
	float corePredictionMargin = 0.6;
	int mxChunkN = 56;
	float accuracy = accuracy1;  //prediction accuracy
	int FrameN2PredictAhead = 0;

	auto start = std::chrono::high_resolution_clock::now();
	Path path1;
	float var[10];

	LoadBWTraceData(bwLog);
	int mahimahiPackSize = 1400;
	vector<float> chunkSizeKB; vector<string>chunkName;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/crf30/1s/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		KB = KB / 1024.0f;
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);
		//cout << "kb: " << KB << endl;

	}
	ifstream  f("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	string line;
	int ii = 0;
	while (std::getline(f, line))
	{
		var[ii] = stoi(line);
		cout << var[ii] << '\n';
		ii++;
	}

	int frameCol = var[0];
	int frameRow = var[1];
	float hfov = 90.0f;

	int w = frameCol * hfov / 360;
	int h = frameRow * hfov / 360;
	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);

	path1.LoadHMDTrackingData(hmdFileName, camera2);

	int chunkD = 1;
	int chunkN = 1;
	int fps = 30;
	Mat firstScene;
	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
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

	for (int i = 0; i < tilesCol * tilesRow; i++)
	{
		vector<vector <Mat>> temp1;
		for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
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
	int bwTimeNeeded = 200;
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
	int tileRowN = rowM;
	int tileColN = colN;

	int chunkBwDivider = 12;

	if (fovPlusOrno == 0)
	{
		chunkBwDivider = 5;
	}



	cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;

	//cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
	int frameCountForAccuracy = frameCount + 0;
	cam_index = path1.GetCamIndex(frameCountForAccuracy, fps, cam_index);
	PPC camera4req = path1.cams[cam_index];
	camera4req.Pan(180 * (100 - accuracy) / 100.0f);
	camera4req.Tilt(90 * (100 - accuracy) / 100.0f);


	vector <int> tileBitMap;
	for (int i = 0; i < tileRowN * tileColN; i++)
	{
		tileBitMap.push_back(0);
	}
	if (fovPlusOrno == 0)
	{
		//getTilesNumber2reqFovOnly(tileBitMap, path1.cams[cam_index], eri, tileColN, tileRowN);
		getTilesNumber2reqFovOnly(tileBitMap, camera4req, eri, tileColN, tileRowN);
	}

	else
	{
		//getTilesNumber2req(tileBitMap, path1.cams[cam_index], eri, tileColN, tileRowN);
		getTilesNumber2req(tileBitMap, camera4req, eri, tileColN, tileRowN);
	}



	auto playStart = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < tileRowN * tileColN; i++)
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

			string name = filename;
			cout << filename << endl;
			const char* nameChr = name.c_str();
			const char* positionMarker = strrchr(nameChr, '/');
			int start = int(positionMarker - nameChr) + 1;
			name = name.substr(start, name.size() - start);
			cout << name << " " << chunkName[0] << endl;
			int namePos = 0;
			while ((name.compare(chunkName[namePos])))
			{
				namePos++;
			}
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
			packetNeeded = chunkBwDivider * chunkSizeKB[namePos] * 1024 / mahimahiPackSize;
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
	vector<int> totalInSide;
	vector<float>blankPixels;
	vector<float>frameRate;
	vector<float> samplingRateAvg;
	vector<float> samplingRateMin;
	vector<int>totalChunkReq;
	vector<int>T;
	float tileProcessT = 0;
	int totalTileLoaded = 0;
	int totalDelay = 0;
	long totalProcessT = 0;

	auto chunkStarT = std::chrono::high_resolution_clock::now();
	totalChunkReq.push_back(reqTiles[chunkN].size());
	while (0)// (NextChunkDownloaded < reqTiles[chunkN].size() - 1)
	{
		displayImage33ms();
		//displayImage(firstScene);
		timePassed = timePassed + 30.0f;
		waitKey(20);
		cout << "in first while" << endl;
	}

	totalTileLoaded = totalTileLoaded + NextChunkDownloaded;
	NextChunkDownloaded = 0;
	cout << tileDlSum << endl;

	int playStartTime = 0;
	int tileCondition = 1;
	frameCount = -1;

	auto delayMeasureStart = std::chrono::high_resolution_clock::now();
	auto reqSend = std::chrono::high_resolution_clock::now();
	int maxDelay = 0;

	while (tileCondition)
	{
		frameCount++;
		timePassed = timePassed + 30.0f;

		if (frameCount == (fps * chunkD * mxChunkN - 2))
		{
			tileCondition = 0;
		}
		if (frameCount == 0)
		{
			auto firstFrame = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsedFirst = firstFrame - playStart;
			playStartTime = elapsedFirst.count() * 1000;
		}

		int currentChunk = (frameCount) / (fps * chunkD) + 1;
		int playIndex = frameCount - (currentChunk - 1) * (fps * chunkD); //0-(fps*chunkD-1)
		cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex << "passedTime: ->" << timePassed << endl;
		cout << frameCount;
		if (playIndex == 1)
		{
			chunkStarT = std::chrono::high_resolution_clock::now();

		}
		if ((playIndex) == fps * (chunkD - nextDlChunkSec))
		{
			//cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

			int frameCountForAccuracy = frameCount + FrameN2PredictAhead;
			cam_index = path1.GetCamIndex(frameCountForAccuracy, fps, cam_index);
			PPC camera4req = path1.cams[cam_index];
			camera4req.Pan(180 * (100 - accuracy) / 100.0f);
			camera4req.Tilt(90 * (100 - accuracy) / 100.0f);

			vector <int> tileBitMap2;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				tileBitMap2.push_back(0);
			}

			if (fovPlusOrno == 0)
			{
				//getTilesNumber2reqFovOnly(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
				getTilesNumber2reqFovOnly(tileBitMap2, camera4req, eri, tileColN, tileRowN);
			}

			else
			{
				//getTilesNumber2req(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
				getTilesNumber2req(tileBitMap2, camera4req, eri, tileColN, tileRowN);
			}


			//getTilesNumber2req(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
			int reqChunkN = currentChunk + 1;
			NextChunkDownloaded = 0;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				//cout << i << "--" << tileBitMap2[i] << endl;

				if (reqChunkN <= mxChunkN)
				{
					if (tileBitMap2[i] == 1)
					{
						filename = getChunkNametoReq4mTiles(srcBaseAddr, reqChunkN, i);
						//cout << filename << endl;
						reqTiles[reqChunkN].push_back(i);
						auto finish1 = std::chrono::high_resolution_clock::now();
						bwDuration = bwDuration = frameCount * 33 + playStartTime + totalDelay;
						bwIglobal = 0;
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}

						string name = filename;
						cout << filename << endl;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						int namePos = 0;
						while ((name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = chunkBwDivider * chunkSizeKB[namePos] * 1024 / mahimahiPackSize;

						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
						timePassed = 0;
						cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
						if (maxDelay < bwTimeNeeded)
						{
							maxDelay = bwTimeNeeded;
						}
						reqSend = std::chrono::high_resolution_clock::now();
						//futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded));
						std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded);
					}

				}
			}
			float totalDlTime = maxDelay;
			float tpTime = reqTiles[reqChunkN].size() * 49;
			tileProcessT = totalDlTime + tpTime;
			totalProcessT = totalProcessT + tileProcessT / 100.0f;
			maxDelay = 0;
			cout << "tileProcessT: ---------------------------------------->" << tileProcessT << endl;
			totalChunkReq.push_back(reqTiles[reqChunkN].size());
		}

		if (playIndex == (fps * chunkD - 1))
		{
			if (33 * chunkD * fps < tileProcessT)
			{
				int f = abs(33 * chunkD * fps - tileProcessT) / 33;
				totalDelay = totalDelay + 33 * f;
				for (int i = 0; i < f; i++)
				{
					//blankPixels.push_back(100);
					//frameRate.push_back(0);
					samplingRateMin.push_back(0);
					samplingRateAvg.push_back(0);

				}

				cout << "Delay Added........................................................................: " << totalDelay << ",tileProcesT:" << tileProcessT << ",added Now:" << 33 * f << endl;

			}
			
		}

		cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
		//eri.ERI2Conv4tilesDoNothing(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex, totalOutSide);
		float blankP, srmiN, sravG = 0;
		eri.ERI2Conv4tilesWithSrFrDelay(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex, blankP, sravG, srmiN);
		//displayImage(convPixels);
		blankPixels.push_back(blankP);
		frameRate.push_back(30);
		samplingRateAvg.push_back(sravG);
		samplingRateMin.push_back(srmiN);
		displayImage33ms();


	}
	auto delayMeasurementTimeEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedDelay = delayMeasurementTimeEnd - delayMeasureStart;

	ofstream output;
	ofstream output1;
	if (fovPlusOrno == 1)
	{
		output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/tile_allframe_alldataFoVP.txt", std::ios::out | std::ios::app);
		output1.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/tile_allframe_avgdataFoVP.txt", std::ios::out | std::ios::app);
	}
	if (fovPlusOrno == 0)
	{
		output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/tile_allframe_alldataFoV.txt", std::ios::out | std::ios::app);
		output1.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/mobisys/tile_allframe_avgdataFoV.txt", std::ios::out | std::ios::app);
	}
	output << hmdFileName << "," << bwLog << endl;
	int totalbP = 0;
	int frameWithNonZeroBP = 0;
	int maxBP = 0;

	int totalFR = 0;
	for (int i = 0; i < blankPixels.size(); i++)
	{
		if (blankPixels[i] > 0) {
			frameWithNonZeroBP++;
		}
		if (blankPixels[i] > maxBP) {
			maxBP = blankPixels[i];
		}
		totalbP = totalbP + blankPixels[i];
		output << filename << "," << "blankP:" << blankPixels[i] << ",frameR:" << frameRate[i] << ", srAvg:" << samplingRateAvg[i] << ",srMin:" << samplingRateMin[i] << endl;
		totalFR = totalFR + frameRate[i];
	}
	float avgFR = 30 * totalFR / frameRate.size();
	float bp = totalbP / blankPixels.size();
	float frameWithNonZeroBPAvg = 100*frameWithNonZeroBP / blankPixels.size();
	output.close();

	int totalTile = 0;
	for (int i = 0; i < totalChunkReq.size(); i++)
	{
		totalTile = totalChunkReq[i] + totalTile;
	}

	

	cout << "Delay OldWay: " << totalDelay << ",bp:" << bp << endl;

	output1 << srcBaseAddr << bwLog << hmdFileName << ": " << "Delay: " << totalDelay << ",bp(%):" << bp <<" frameWithBp>0=  "<<frameWithNonZeroBPAvg<<" ,maxBPinAframe= "<<maxBP<<", totalTileChunk:" << totalTile << endl;
	output1.close();

}




void testDownloadVideoHttpTileDelayMeasurementFoV(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN)
{

	int mxChunkN = 55;
	float corePredictionMargin = 0.7;
	auto start = std::chrono::high_resolution_clock::now();
	Path path1;
	float var[10];

	LoadBWTraceData(bwLog);
	vector<float> chunkSizeKB; vector<string>chunkName;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/crf30/1s/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		KB = KB / 1024.0f;
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);
		cout << "kb: " << KB << endl;

	}
	ifstream  f("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	string line;
	int ii = 0;
	while (std::getline(f, line))
	{
		var[ii] = stoi(line);
		cout << var[ii] << '\n';
		ii++;
	}

	int frameCol = var[0];
	int frameRow = var[1];
	float hfov = 90.0f;
	
	int w = frameCol * hfov / 360;
	int h = frameRow * hfov / 360;
	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);

	path1.LoadHMDTrackingData(hmdFileName, camera2);

	int chunkD = 1;
	int chunkN = 1;
	int fps = 30;
	Mat firstScene;
	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
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

	for (int i = 0; i < tilesCol * tilesRow; i++)
	{
		vector<vector <Mat>> temp1;
		for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
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
	int bwTimeNeeded = 200;
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
	int tileRowN = rowM;
	int tileColN = colN;


	cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;

	cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

	vector <int> tileBitMap;
	for (int i = 0; i < tileRowN * tileColN; i++)
	{
		tileBitMap.push_back(0);
	}

	getTilesNumber2reqFovOnly(tileBitMap, path1.cams[cam_index], eri, tileColN, tileRowN);



	auto playStart = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < tileRowN * tileColN; i++)
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

			string name = filename;
			cout << filename << endl;
			const char* nameChr = name.c_str();
			const char* positionMarker = strrchr(nameChr, '/');
			int start = int(positionMarker - nameChr) + 1;
			name = name.substr(start, name.size() - start);
			cout << name << " " << chunkName[0] << endl;
			int namePos = 0;
			while ((name.compare(chunkName[namePos])))
			{
				namePos++;
			}
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
			packetNeeded = 12 * chunkSizeKB[namePos] * 1024 / 1500;
			bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
			cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
			//futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, chunkN, chunkD, i, fps, bwTimeNeeded));
		}
	}

	Mat tileFrame;
	int tileColLen = frameCol / tileColN;
	int tileRowLen = frameRow / tileRowN;
	cout << "t: " << tileColLen << " " << tileRowLen << endl;
	cout << reqTiles[chunkN].size() << endl;
	vector<int> totalInSide;
	vector<int>totalChunkReq;
	int totalTileLoaded = 0;
	int totalDelay = 0;
	totalChunkReq.push_back(reqTiles[chunkN].size());
	while (0)// (NextChunkDownloaded < reqTiles[chunkN].size() - 1)
	{
		displayImage33ms();
		//displayImage(firstScene);
		timePassed = timePassed + 30.0f;
		waitKey(20);
		cout << "in first while" << endl;
	}

	totalTileLoaded = totalTileLoaded + NextChunkDownloaded;
	NextChunkDownloaded = 0;
	

	int playStartTime = 0;
	int tileCondition = 1;
	frameCount = -1;

	auto delayMeasureStart = std::chrono::high_resolution_clock::now();
	auto reqSend = std::chrono::high_resolution_clock::now();
	int maxDelay = 0;

	while (tileCondition)
	{
		frameCount++;
		timePassed = timePassed + 30.0f;

		if (frameCount == (fps * chunkD * mxChunkN - 2))
		{
			tileCondition = 0;
		}
		if (frameCount == 0)
		{
			auto firstFrame = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsedFirst = firstFrame - playStart;
			playStartTime = elapsedFirst.count() * 1000;
		}

		int currentChunk = (frameCount) / (fps * chunkD) + 1;
		int playIndex = frameCount - (currentChunk - 1) * (fps * chunkD); //0-(fps*chunkD-1)
		//cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex << "passedTime: ->" << timePassed << endl;
		cout << frameCount;
		if ((playIndex) == fps * (chunkD - nextDlChunkSec))
		{
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			vector <int> tileBitMap2;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				tileBitMap2.push_back(0);
			}
			getTilesNumber2reqFovOnly(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
			int reqChunkN = currentChunk + 1;
			NextChunkDownloaded = 0;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				//cout << i << "--" << tileBitMap2[i] << endl;

				if (reqChunkN <= mxChunkN)
				{
					if (tileBitMap2[i] == 1)
					{
						filename = getChunkNametoReq4mTiles(srcBaseAddr, reqChunkN, i);
						//cout << filename << endl;
						reqTiles[reqChunkN].push_back(i);
						auto finish1 = std::chrono::high_resolution_clock::now();
						bwDuration = bwDuration = frameCount * 33 + playStartTime;
						bwIglobal = 0;
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}

						string name = filename;
						cout << filename << endl;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						int namePos = 0;
						while ((name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = 12 * chunkSizeKB[namePos] * 1024 / 1500;

						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
						timePassed = 0;
						cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
						if (maxDelay < bwTimeNeeded)
						{
							maxDelay = bwTimeNeeded;
						}
						reqSend = std::chrono::high_resolution_clock::now();
						//futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded));
						//std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded);
					}
				}
			}

			totalChunkReq.push_back(reqTiles[reqChunkN].size());
		}

		if (playIndex == (fps * chunkD - 1))
		{
			auto delayStart = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> delayElapsed = delayStart - reqSend;
			while (delayElapsed.count() * 1000 < bwTimeNeeded)
			{
				timePassed = timePassed + 33.0f;
				displayImage33ms();
				totalDelay = totalDelay + 33;
				delayStart = std::chrono::high_resolution_clock::now();
				delayElapsed = delayStart - reqSend;
				cout << "TimePassed: " << delayElapsed.count() * 1000 << " bwTimeNeeded: " << bwTimeNeeded << endl;
			}
			currentChunk = currentChunk;
			cout << "Delay Added.................: " << totalDelay << endl;
		}

		cam_index = path1.GetCamIndex(frameCount, fps, cam_index);


		//eri.ERI2Conv4tilesDoNothing(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex, totalOutSide);
		//displayImage(convPixels);
		displayImage33ms();


	}
	auto delayMeasurementTimeEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedDelay = delayMeasurementTimeEnd - delayMeasureStart;

	ofstream output;
	output.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/tileDelayFoVPlus.txt", std::ios::out | std::ios::app);
	output << srcBaseAddr << " ,Delay: " << (elapsedDelay.count() * 1000) - 33 * frameCount << endl;
	output.close();


	cout << "Extra Time needed/delay: " << (elapsedDelay.count() * 1000) - 33 * frameCount << endl;
	cout << "Delay OldWay: " << totalDelay << endl;
}

void testDownloadVideoHttpTileQualChngLineMeasurementFoVPlus(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN)
{
	int fovOnly = 1;
	int mxChunkN = 59;
	float corePredictionMargin = 1;

	auto start = std::chrono::high_resolution_clock::now();
	Path path1;
	float var[10];

	LoadBWTraceData(bwLog);
	vector<float> chunkSizeKB; vector<string>chunkName;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/crf30/1s/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		KB = KB / 1024.0f;
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);
		//cout << "kb: " << KB << endl;

	}
	ifstream  f("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/4kDivingEncodingVariable.txt");
	string line;
	int ii = 0;
	while (std::getline(f, line))
	{
		var[ii] = stoi(line);
		cout << var[ii] << '\n';
		ii++;
	}

	int frameCol = var[0];
	int frameRow = var[1];
	float hfov = 90.0f;

	int w = frameCol * hfov / 360;
	int h = frameRow * hfov / 360;
	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);

	path1.LoadHMDTrackingData(hmdFileName, camera2);

	int chunkD = 1;
	int chunkN = 1;
	int fps = 30;
	Mat firstScene;
	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", firstScene);
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

	for (int i = 0; i < tilesCol * tilesRow; i++)
	{
		vector<vector <Mat>> temp1;
		for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
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
	int bwTimeNeeded = 200;
	int bwDuration;
	int bwIglobal;
	int packetNeeded;

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
	int tileRowN = rowM;
	int tileColN = colN;


	cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;

	cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

	vector <int> tileBitMap, tileBitMapCopy;
	for (int i = 0; i < tileRowN * tileColN; i++)
	{
		tileBitMap.push_back(0);
	}

	for (int i = 0; i < tileRowN * tileColN; i++)
	{
		tileBitMapCopy.push_back(0);
	}

	if (fovOnly==1)
	{
		getTilesNumber2reqFovOnly(tileBitMap, path1.cams[cam_index], eri, tileColN, tileRowN);
	}
	else {
		getTilesNumber2req(tileBitMap, path1.cams[cam_index], eri, tileColN, tileRowN);
	}

	auto playStart = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < tileRowN * tileColN; i++)
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

			string name = filename;
			cout << filename << endl;
			const char* nameChr = name.c_str();
			const char* positionMarker = strrchr(nameChr, '/');
			int start = int(positionMarker - nameChr) + 1;
			name = name.substr(start, name.size() - start);
			cout << name << " " << chunkName[0] << endl;
			int namePos = 0;
			while ((name.compare(chunkName[namePos])))
			{
				namePos++;
			}
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
			packetNeeded = 12 * chunkSizeKB[namePos] * 1024 / 1500;
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
	vector<int> totalQualBorder, totalQualPopescu;

	vector<int>totalChunkReq;
	int totalTileLoaded = 0;
	int totalDelay = 0;
	totalChunkReq.push_back(reqTiles[chunkN].size());
	while (NextChunkDownloaded < reqTiles[chunkN].size() - 1)
	{
		displayImage33ms();
		//displayImage(firstScene);
		timePassed = timePassed + 30.0f;
		waitKey(20);
		cout << "in first while" << endl;
	}

	totalTileLoaded = totalTileLoaded + NextChunkDownloaded;
	NextChunkDownloaded = 0;
	cout << tileDlSum << endl;

	int playStartTime = 0;
	int tileCondition = 1;
	frameCount = -1;
	int totalBorderingPixel = 0; int totalCount = 0;

	auto delayMeasureStart = std::chrono::high_resolution_clock::now();
	PPC tempCam = path1.cams[cam_index];

	ofstream outputFrameSR;
	outputFrameSR.open("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/TileQualPerFrameBorderingPixel.txt", std::ios::out | std::ios::app);

	while (tileCondition)
	{
		frameCount++;
		timePassed = timePassed + 30.0f;

		if (frameCount == (fps * chunkD * mxChunkN - 2))
		{
			tileCondition = 0;
		}
		if (frameCount == 0)
		{
			auto firstFrame = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsedFirst = firstFrame - playStart;
			playStartTime = elapsedFirst.count() * 1000;
		}

		int currentChunk = (frameCount) / (fps * chunkD) + 1;
		int playIndex = frameCount - (currentChunk - 1) * (fps * chunkD); //0-(fps*chunkD-1)
		cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex << "passedTime: ->" << timePassed << endl;

		if ((playIndex) == fps * (chunkD - nextDlChunkSec))
		{
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			vector <int> tileBitMap2;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				tileBitMap2.push_back(0);
			}

			if (fovOnly==1)
			{
				getTilesNumber2reqFovOnly(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
			}
			else
			{
				getTilesNumber2req(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
			}
			
			int reqChunkN = currentChunk + 1;
			NextChunkDownloaded = 0;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				//cout << i << "--" << tileBitMap2[i] << endl;

				if (reqChunkN <= mxChunkN)
				{
					if (tileBitMap2[i] == 1)
					{
						filename = getChunkNametoReq4mTiles(srcBaseAddr, reqChunkN, i);
						//cout << filename << endl;
						reqTiles[reqChunkN].push_back(i);
						auto finish1 = std::chrono::high_resolution_clock::now();
						bwDuration = bwDuration = frameCount * 33 + playStartTime;
						bwIglobal = 0;
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}

						string name = filename;
						cout << filename << endl;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						int namePos = 0;
						while ((name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = 12 * chunkSizeKB[namePos] * 1024 / 1500;

						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
						timePassed = 0;
						cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
						futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded));
						//std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded);
					}
				}
			}

			totalChunkReq.push_back(reqTiles[reqChunkN].size());
		}

		if (playIndex == (fps * chunkD - 1))
		{
			auto delayStart = std::chrono::high_resolution_clock::now();
			while (NextChunkDownloaded < reqTiles[currentChunk + 1].size() - 1)
			{
				timePassed = timePassed + 30.0f;
				displayImage33ms();
				totalDelay = totalDelay + 33;
			}
			currentChunk = currentChunk + 1;
			auto delayFinish = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> delayElapsed = delayFinish - delayStart;


			cout << "Delay Added.................: " << totalDelay << endl;
		}

		cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

		
		vector<vector<float>> dummyVec;
		for (int i = 0; i < camera2.h+1; i++)
		{
			vector<float> dummyDummy;
			for (int ii = 0; ii < camera2.w+1; ii++)
			{
				dummyDummy.push_back(100);
			}
			dummyVec.push_back(dummyDummy);
		}
		Mat convPixels(camera2.h, camera2.w, firstScene.type());
		Mat convPixels1(camera2.h, camera2.w, firstScene.type());
		int borderingPixel=eri.ERI2Conv4tilesTotalPixelBorderingQualChange(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex,dummyVec);
		//int borderingPixel=eri.ERI2Conv4tiles(convPixels, frameQvecTiles, reqTiles[currentChunk], path1.cams[cam_index], tileColN, tileRowN, currentChunk, playIndex,totalQualPopescu);
		totalBorderingPixel = totalBorderingPixel + borderingPixel;
		outputFrameSR << srcBaseAddr << "," << hmdFileName << ",BorderingPixel," <<borderingPixel<< endl;
		totalCount++;
		//displayImage(convPixels);
		convPixels.release();
		
		
	}		
	float borderingPixelAvg = totalBorderingPixel / totalCount;
	//outputFrameSR << srcBaseAddr << "," << hmdFileName << ",BorderingPixel," << borderingPixelAvg << endl;	
	outputFrameSR.close();

}

void testDownloadVideoHttpTile(char* srcBaseAddr, char* bwLog, char* hmdFileName, int nextDlChunkSec, int rowM, int colN)
{
	auto start = std::chrono::high_resolution_clock::now();
	Path path1;
	float var[10];

	LoadBWTraceData(bwLog);
	vector<float> chunkSizeKB; vector<string>chunkName;
	ifstream  file1("C:/inetpub/wwwroot/3vid2crf3trace/Tiles/chunkSizes.txt");
	if (!file1)
	{
		print("error: can't open file: " << "chunkSizes" << endl);		system("pause");
	}
	string line1;	int i = 0;
	while (getline(file1, line1))
	{
		string name;
		stringstream linestream(line1);
		linestream >> name;
		const char* nameChr = name.c_str();
		const char* positionMarker = strrchr(nameChr, '\\');
		int start = int(positionMarker - nameChr) + 1;
		name = name.substr(start, name.size() - start);
		float KB;
		linestream >> KB;
		KB = KB / 1024.0f;
		chunkSizeKB.push_back(KB);
		chunkName.push_back(name);

	}
	ifstream  f("./Video/source/4kDivingEncodingVariable.txt");
	string line;
	int ii = 0;
	while (std::getline(f, line))
	{
		var[ii] = stoi(line);
		cout << var[ii] << '\n';
		ii++;
	}

	int frameCol = var[0];
	int frameRow = var[1];
	float hfov = 90.0f;
	float corePredictionMargin = 0.6;
	int w = frameCol * hfov / 360;
	int h = frameRow * hfov / 360;
	PPC camera2(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);

	path1.LoadHMDTrackingData(hmdFileName, camera2);
	int mxChunkN = 15;
	int chunkD = 4;
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

	for (int i = 0; i < tilesCol * tilesRow; i++)
	{
		vector<vector <Mat>> temp1;
		for (int chunk = 0; chunk <= mxChunkN + 1; chunk++)
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
	int bwTimeNeeded = 200;
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
	int tileRowN = rowM;
	int tileColN = colN;

	cout << "Very first Chunk, no frame yet.....More than once is red flag." << endl;

	cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

	vector <int> tileBitMap;
	for (int i = 0; i < tileRowN * tileColN; i++)
	{
		tileBitMap.push_back(0);
	}

	getTilesNumber2req(tileBitMap, path1.cams[cam_index], eri, tileColN, tileRowN);

	auto playStart = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < tileRowN * tileColN; i++)
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

			string name = filename;
			cout << filename << endl;
			const char* nameChr = name.c_str();
			const char* positionMarker = strrchr(nameChr, '/');
			int start = int(positionMarker - nameChr) + 1;
			name = name.substr(start, name.size() - start);
			cout << name << " " << chunkName[0] << endl;
			int namePos = 0;
			while ((name.compare(chunkName[namePos])))
			{
				namePos++;
			}
			cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
			packetNeeded = 4 * chunkSizeKB[namePos] * 1024 / 1500;
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
	int totalTileLoaded = 0;
	while (NextChunkDownloaded < reqTiles[chunkN].size() - 1)
	{
		displayImage(firstScene);
	}
	totalTileLoaded = totalTileLoaded + NextChunkDownloaded;
	NextChunkDownloaded = 0;
	cout << tileDlSum << endl;

	int playStartTime = 0;
	int tileCondition = 1;
	frameCount = -1;
	while (tileCondition)
	{
		frameCount++;

		if (frameCount == (fps * chunkD * mxChunkN - 1))
		{
			tileCondition = 0;
		}

		if (frameCount == 0)
		{
			auto firstFrame = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsedFirst = firstFrame - playStart;
			playStartTime = elapsedFirst.count() * 1000;

		}

		int currentChunk = (frameCount) / (fps * chunkD) + 1;
		int playIndex = frameCount - (currentChunk - 1) * (fps * chunkD); //0-(fps*chunkD-1)
		cout << " CurrentChunkN=" << currentChunk << " frmcount=" << frameCount << " plCnt=" << playIndex << endl;

		if (chunkN > 1 && playIndex == 10)
		{
			for (int i = 0; i < tilesCol * tilesRow; i++)
			{
				vector<vector <Mat>> temp1;
				vector <Mat> temp;
				for (int i = 0; i < 2; i++)
				{
					Mat m;
					temp.push_back(m);
					m.release();
				}
				temp1.push_back(temp);
				frameQvecTiles[chunkN - 2] = temp1;
			}
			cout << "memory freed................." << endl;
		}
		if ((playIndex) == fps * (chunkD - nextDlChunkSec))
		{
			cam_index = path1.GetCamIndex(frameCount, fps, cam_index);
			vector <int> tileBitMap2;
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				tileBitMap2.push_back(0);
			}
			getTilesNumber2req(tileBitMap2, path1.cams[cam_index], eri, tileColN, tileRowN);
			for (int i = 0; i < tileRowN * tileColN; i++)
			{
				//cout << i << "--" << tileBitMap2[i] << endl;
				int reqChunkN = currentChunk + 1;
				if (reqChunkN <= mxChunkN)
				{
					if (tileBitMap2[i] == 1)
					{
						filename = getChunkNametoReq4mTiles(srcBaseAddr, reqChunkN, i);
						cout << filename << endl;
						reqTiles[reqChunkN].push_back(i);
						auto finish1 = std::chrono::high_resolution_clock::now();
						bwDuration = bwDuration = frameCount * 33 + playStartTime;
						bwIglobal = 0;
						while (bwDuration > byteVec[bwIglobal]) {
							bwIglobal++;
						}

						string name = filename;
						cout << filename << endl;
						const char* nameChr = name.c_str();
						const char* positionMarker = strrchr(nameChr, '/');
						int start = int(positionMarker - nameChr) + 1;
						name = name.substr(start, name.size() - start);
						cout << name << " " << chunkName[0] << endl;
						int namePos = 0;
						while ((name.compare(chunkName[namePos])))
						{
							namePos++;
						}
						cout << namePos << " " << chunkName[namePos] << " " << chunkSizeKB[namePos] << endl;
						packetNeeded = 4 * chunkSizeKB[namePos] * 1024 / 1500;

						bwTimeNeeded = byteVec[bwIglobal + packetNeeded] - byteVec[bwIglobal]; //in ms
						cout << "time: " << bwDuration << " Index4Time= " << bwIglobal << " reqTime=" << bwTimeNeeded << endl;
						futures.push_back(std::async(std::launch::async, DownLoadTilesChunk, filename, reqChunkN, chunkD, i, fps, bwTimeNeeded));
					}
				}
			}
		}

		if (frameCount == (fps * chunkD - 1))
		{
			while (NextChunkDownloaded < reqTiles[chunkN + 1].size() - 1)
			{
				cout << "looping..." << endl;
			}
			chunkN = chunkN + 1;

		}

		cam_index = path1.GetCamIndex(frameCount, fps, cam_index);

		eri.ERI2Conv4tiles(convPixels, frameQvecTiles, reqTiles[chunkN], path1.cams[cam_index], tileColN, tileRowN, chunkN, playIndex, totalInside);

		displayImage(convPixels);



	}
	long sumTotalInside;
	for (int i = 0; i < totalInside.size(); i++)
	{
		sumTotalInside = sumTotalInside + totalInside[i];
	}
	cout << sumTotalInside << endl;
	cout << totalInside.size() * camera2.w * camera2.h << endl;
	double avgTotalInside = sumTotalInside / (double)(totalInside.size() * camera2.w * camera2.h);
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
	int lastframe = 1800;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	path1.WriteH264("./Video/source/roller.avi", lastframe, codec);
	//path1.WriteH264("./Video/source/rhino.avi", lastframe, codec);
	//path1.WriteH264("./Video/source/diving.avi", lastframe, codec);

	return 0;
}



int testWriteh264tiles() {
	Path path1;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');

	for (int i = 60; i < 206; i++)
	{
		path1.WriteH264tiles("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/rhino.AVI", i, 6, 4, codec);
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
	upload_image("./Image/ERI.png", frame);

	
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

void GenerateEncoding4sVarSpecificPanTiltChunkAddextra()
{


	float tilt = 10;
	float pan = 40;
	string fileName;
	string chunkN;
	ifstream  file("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/All4s6sChunkDataBase/FatCoRE/6S/chunkNames.txt");
	if (!file)
	{
		print("error: can't open file: " << filename << endl);
		system("pause");
	}
	string   line;
	
	while (getline(file, line))
	{
		stringstream  linestream(line);

		string lString;

		linestream >> lString;

		
		char* fileName4s = "./Video/source/All4s6sChunkDataBase/FatCoRE/6S/";
		char* fileName1s = "./Video/source/All4s6sChunkDataBase/FatCoRE/9S/";
	
		std::ostringstream oss11, oss22;


		oss11 << fileName4s <<lString;
		oss22 << fileName1s <<lString;
		
		string tmp1 = oss11.str(); const char* Fn1 = tmp1.c_str();
		string tmp2 = oss22.str(); const char* Fn2 = tmp2.c_str();
		int d=tmp2.find("AVI6");
		tmp2.replace(d+3, 1, "9");
		cout << tmp2 <<" "<<d<< endl;
		cout << tmp1 << endl;
		video4sBaseAndExtraSec((char*)Fn1, (char*)Fn2);

	}
}

void video4sBaseAndExtraSec(char* fileName4s, char* fileName1s) {

	vector <Mat> frame4s1s;
	vector <Mat> frame4s2s;
	vector <Mat> frame4s3s;
	int fps = 30;
	int chunkD = 4;
	int extra1sLength = 45;

	VideoCapture cap(fileName4s);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fileName4s << endl;
		waitKey(100000);
	}

	for (int fi = 0; fi < fps * chunkD; fi++)
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
		//cout << frame.rows << " " << frame.cols;
		//displayImage(frame);

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
		//cout << frame.rows << " " << frame.cols;
		frame4s1s.push_back(frame);
		//displayImage(frame);

	}

	

	int sf = frame4s1s.size() - (fps * chunkD + extra1sLength);
	int ef = frame4s1s.size();
	cout << "SAving-> sf:ef " << sf << " " << ef << endl;
	videowriterhelperxWithFileNameWithExtraSec(fileName1s, 900, extra1sLength, 0, 0, fps, frame4s1s[1].cols, frame4s1s[1].rows, sf, ef, frame4s1s); //check this function, ofilename has been replaced to filename1s
	
}


void GenerateEncoding4sVarSpecificPanTiltChunk(int pan, int tilt, int chunkN)
{	
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
	ERI eri(var[0], var[1]);
	eri.atanvalue();	eri.xz2LonMap();	eri.xz2LatMap();
	path1.EncodingNonUniformListInit(var); //palasx
	path1.mapxEncoding(var);  //change prediction margin here//*************//
	
	int extraFrame=45;
	int extraSec=9;	
	char* fileNamex = "./Video/source/roller.AVI";
			
	cout << fileNamex << endl;					
	EncodeVideoWithExtensionSpecificPanTiltChunkN(fileNamex, extraSec, pan, tilt, chunkN, extraFrame, extraSec);    //change prediction margin inside//*************//
	
}


void EncodeVideoWithExtensionSpecificPanTiltChunkN(char* fileName, int extraSec, float pan, float tilt, int chunkN, int extraFrame, int howManySecondsfor4thSec)
{
	chunkN = chunkN + 10;
	vector<Mat> downloadedFrame;
	vector <Mat> bufferedFrame;
	vector <Mat> bufferedCoREFrame;
	struct var encodevar;
	Path path1;
	float hfov = 90.0f;
	float corePredictionMargin = 1.2;
	int w = 960;
	int h = 512;
	PPC encodeRefPPC(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);  //always next to corePPC before pan or tilt  //if you change it must update the MapxEncoding Function
	PPC core1PPC(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
	int cf = 5;
	VideoCapture cap(fileName);
	vector <float> nonUniformList;
	temp_calc(nonUniformList, howManySecondsfor4thSec, extraFrame);
	int extraAdd4Diving = 0;
	cout << fileName << endl;
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
	int extraIndex = 0;
	
	for (int fi = 0; fi <= (chunkN)*fps * chunDurationsec + nonUniformList[extraFrame - 1]; fi++)
	{
		cap >> frame;
		
		if (frame.empty())
		{
			cout << "Can not read video frame: " << endl;
			break;
		}
		else if (fi >= (chunkN - 1) * chunDurationsec * fps && fi < (chunkN * chunDurationsec * fps))
		{
			//cout << "fi: " << fi << endl;
			//ret = path1.EncodeNewNonLinV2Optimized(frame, &encodevar, core1PPC, encodeRefPPC, cf, (fi - (chunkN - 1) * chunDurationsec * fps));
			//bufferedFrame.push_back(frame.clone());
			Mat m;
			//bufferedCoREFrame.push_back(m);
			//displayImage(ret);
		}

		else if (fi >= chunkN * chunDurationsec * fps && fi <= chunkN * chunDurationsec * fps + nonUniformList[extraFrame - 1])
		{
			
			if (fi == chunkN * chunDurationsec * fps + nonUniformList[extraIndex])
			{
				cout << "fi: " << fi << endl;
				//ret = path1.EncodeNewNonLinV2Optimized(frame, &encodevar, core1PPC, encodeRefPPC, cf, (fi - (chunkN - 1) * chunDurationsec * fps));
				bufferedFrame.push_back(frame.clone());
				Mat m;
				bufferedCoREFrame.push_back(m);
				//displayImage(ret);
				extraIndex++;		
			}
		}
	
	}

	for (int tilt = -90; tilt <= 90; tilt = tilt + 20)//
	{
		int panAngle = (int)abs((20) / (cos(3.1416 * tilt / 180)));   //argument in radian
		if (panAngle % 2 != 0)
		{
			panAngle += 1;
		}
		for (int pan = -180; pan <= 180; pan = pan + panAngle)
		{
			PPC core1PPC(hfov * corePredictionMargin, w * corePredictionMargin, h * corePredictionMargin);
			core1PPC.Pan(pan);
			core1PPC.Tilt(tilt);
			cout << "tilt: " << tilt << " pan: " << pan << "chunkN: " << chunkN << endl;
			for (int i = 0; i < bufferedFrame.size(); i++)
			{
				frame = bufferedFrame[i];
				ret = path1.EncodeNewNonLinV2Optimized(frame, &encodevar, core1PPC, encodeRefPPC, cf, i);
				bufferedCoREFrame[i]=ret.clone();
			}
			int sf = 0;
			int ef = bufferedCoREFrame.size();
			cout << "SAving-> sf:ef " << sf << " " << ef << endl;
			videowriterhelperxWithFileNameWithExtraSec(fileName, chunkN, extraSec, (int)pan, (int)tilt, fps, bufferedCoREFrame[1].cols, bufferedCoREFrame[1].rows, sf, ef, bufferedCoREFrame);
		}

	}
			

}



void GenerateEncoding4sVarSpecificPanTiltChunk4mFileName()
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

		char* fileName4s = "./Video/source/4sBase/rhino";
		char* fileName1s = "./Video/source/extraSec/rhino.AVI6";
		char* fileName2s = "./Video/source/extraSec/rhino.AVI8";
		char* fileName3s = "./Video/source/extraSec/rhino.AVI10";
		std::ostringstream oss11, oss22, oss3, oss4;


		oss11 << fileName4s << "_" << chunkN << "_" << tilt << "_" << pan << ".avi";
		oss22 << fileName1s << "_" << chunkN << "_" << tilt << "_" << pan << ".avi";
		oss3 << fileName2s << "_" << chunkN << "_" << tilt << "_" << pan << ".avi";
		oss4 << fileName3s << "_" << chunkN << "_" << tilt << "_" << pan << ".avi";
		string tmp1 = oss11.str(); const char* Fn1 = tmp1.c_str();
		string tmp2 = oss22.str(); const char* Fn2 = tmp2.c_str();
		string tmp3 = oss3.str(); const char* Fn3 = tmp3.c_str();
		string tmp4 = oss4.str(); const char* Fn4 = tmp4.c_str();

		//video4sBaseAndExtraSec((char*)Fn1, (char*)Fn2, (char*)Fn3, (char*)Fn4);




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
	makeVideoRegularSpecificPanTiltChunkNAllSameFrame("./video/source/diving.AVI", 20, 10, 2, 400);
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
	float corePredictionMargin = 1.4;
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
	float corePredictionMargin = 1.4;
	int w = 960;
	int h = 512;
	PPC encodeRefPPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);  //always next to corePPC before pan or tilt
	PPC core1PPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	int cf = 5;
	int fps = 30;
	int segi = 0;
	Mat ret;
	float var[10];
	int chunDurationsec = 4;
	Mat frame;
	int discard = beginFrame;
	   
	VideoCapture cap(fileName);

	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << fileName<<endl;
		STOP;
	}


	for (int fi = 0; fi < (chunkN)*fps*chunDurationsec +discard; fi++)
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

	for (int fi = 0; fi < chunDurationsec*fps; fi++)
	{
		cout << " fi: " << fi << endl;

		frame = downloadedFrame[fi];
		//ret = path1.EncodeNewNonLinV2(frame, &encodevar, core1PPC, encodeRefPPC, cf); //for all possible angle encodinig
		bufferedFrame.push_back(frame.clone());
		
		
		if ((fi + 1) % (chunDurationsec*fps) == 0 && fi > 0)
		{			
			cout << chunkN << endl;
			int sf = bufferedFrame.size() - chunDurationsec * fps;
			int ef = bufferedFrame.size();
			cout << "SAving-> sf:ef " << sf << " " << ef << endl;
			char *fn;
			if (discard == 0)
			{
				fn = "C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/rollerFrame4s2_";

			}
			if (discard==400)
			{
				fn = "C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/divingFrame4s_";

			}
			
			if (discard == 100)
			{
				fn = "C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/rhinoFrame4s_";

			}
			
			videowriterhelperxWithFileName(fn, chunkN, (int)pan, (int)tilt, fps, bufferedFrame[1].cols, bufferedFrame[1].rows, sf, ef, bufferedFrame);

		}

		
	}
	for (int i = 0; i < downloadedFrame.size(); i++)
	{
		Mat m;
		downloadedFrame[i] = m;
		bufferedFrame[i] = m;
	}

	downloadedFrame.clear();
	bufferedFrame.clear();
	
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


void run10simulations()
{
	for (int simulation = 0; simulation < 1; simulation++)//dl//extraSec
	{
		char* bwLog = "./bwLogs/bw646kB.txt";
		char* videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf40/roller"; //ch
		char* hmdTrace = "./Video/source/roller.txt";
		int extraSec = 3;

		
		if (extraSec == 1)
		{
			videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s1s/crf40/roller";
		}
		else if (extraSec == 2)
		{
			videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s2s/crf40/roller";
		}
		else if (extraSec == 3)
		{
			videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s3s/crf40/roller";
		}
		else if (extraSec == 6)
		{
			videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s6s/crf40/roller";
		}
		else if (extraSec == 8)
		{
			videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s8s/crf40/roller";
		}
		else 
		{
			videoFile = "http://127.0.0.5:80/3vid2crf3trace/4s10s/crf40/roller";
		}
	
		int reqTime =1;

		//bwLog = "./bwLogs/bw1962kB.txt";
		testDownloadVideoHttp4thSecVar(videoFile, bwLog, hmdTrace, reqTime, extraSec,10000); //ch


	}
   
}

void testCPUtime(string fileName)
{
	struct samplingvar svar;	vector <Mat> outputFrames;		vector <Mat> creriFrames;
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
	cout << fileName << endl;
	auto playS = chrono::high_resolution_clock::now();
	
	for (int fi = 0; fi < 120; fi++)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << fi << endl;
			cout << "Can not read video frame: " << endl;			
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
	GenerateEncoding4sVarSpecificPanTiltChunkAddextra();
	STOP;
	for (int chunkN = 1; chunkN < 16; chunkN++)
	{
		GenerateEncoding4sVarSpecificPanTiltChunk(0, 0, chunkN);
		/*
		for (int tilt = -90; tilt <= 90; tilt = tilt + tiltseperaton)//
		{
			int panAngle = (int)abs((20) / (cos(3.1416 * tilt / 180)));   //argument in radian
			if (panAngle % 2 != 0)
			{
				panAngle += 1;
			}
			for (int pan = -180; pan <= 180; pan = pan + panAngle)
			{
				cout << "tilt: " << tilt << " pan: " << pan << "chunkN: " << chunkN << endl;
				GenerateEncoding4sVarSpecificPanTiltChunk(pan, tilt, chunkN);
			}
			
		} */
	}
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
		//cout<<"Nonuniform list->" <<"after90Index= "<< x << "  NonuniformIndex=" << u_x<<endl;
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
	float corePredictionMargin = 1.4;
	int w = 960;
	int h = 512;	
	PPC encodeRefPPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);  //always next to corePPC before pan or tilt
	PPC core1PPC(hfov*corePredictionMargin, w*corePredictionMargin, h*corePredictionMargin);
	int cf = 5;
	VideoCapture cap("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Video/source/diving.avi");

	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		waitKey(100000);
	}
	int fps = 30;
	int lastframe = 120;
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
		var[0] = encodevar.colN;
		var[1] = encodevar.rowN;
		var[2] = encodevar.We;
		var[3] = encodevar.Het;
		ofstream output("./Video/source/Roller_encoding_variable.txt");
		output << var[0] << endl;
		output << var[1] << endl;
		output << var[2] << endl;
		output << var[3] << endl;
		output.close();
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
	cout << "Writing videofile: " << filename <<endl;
	writer1.open(filename, codec, fps, Size(cols, rows), true);

	if (!writer1.isOpened())
	{
		cerr << "Could not open the output video file for write\n";
		return;
	}
	for (int i = starting_frame; i < ending_frame; i++)
	{
		writer1.write(file2wr[i]);
		//displayImage(file2wr[i]);

	}
	writer1.release();


}


void testrotationxyframe()
{
	Mat frame;
	//upload_image("C:/Users/pmija/OneDrive - purdue.edu/lab129/EdgeVR/Code/Image/360_equirectangular_800_400.JPG", frame);
	//upload_image("http://127.0.0.5:80/1.JPG", frame);
	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", frame);
	if (!frame.data)                              // Check for invalid input
	{
		cout << "Testoratationxyframe:Could not open or find the image" << std::endl;
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
		cout << "upload: Could not open or find the image" << std::endl;
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
	int chunkD = 1;
	
	VideoCapture cap(fileName);
	if (!cap.isOpened())
	{
		cout << "Cannot open the video file: " << endl;
		waitKey(100000);
	}
	Mat frame;
	upload_image("./Video/source/startScene.PNG", frame);
	while(!frame.empty())
	{	
		cap >> frame;	
		frameS.push_back(frame.clone());
		

	}

	for (int i = 0; i < frameS.size(); i++)
	{
		if ((i+1)%(fps*chunkD)==0 && i>0)
		{
			int sf = i+1 - fps*chunkD;
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
	upload_image("C:/inetpub/wwwroot/3vid2crf3trace/360_equirectangular_800_400.JPG", eriPixels);
	if (!eriPixels.data)                              // Check for invalid input
	{
		cout << "textrotationxy:Could not open or find the image" << std::endl;
		return;
	}


	float hfov = 110.0f;
	PPC camera(hfov, 800, 400);

	Path path1;
	path1.LoadHMDTrackingData("./Video/roller.txt", camera);
	path1.BuildERI2RERIVideo(eriPixels, camera);

	
}

void testCombineMultipleTileIntoSingleFrame()
{
	for (int i = 1; i < 16; i++)
	{

		String fileName1, fileName2, fileName3, fileName4;
		ostringstream oss1;
		oss1 << "./Video/source/tileS/roller.MKV_" << i << "_8.avi";
		fileName1 = oss1.str();
		ostringstream oss2;
		oss2 << "./Video/source/tileS/roller.MKV_" << i << "_9.avi";
		fileName2 = oss2.str();
		//fileName1 = "./Video/source/tileS/roller.MKV_10_8.avi";

		//fileName2 = "./Video/source/tileS/roller.MKV_10_9.avi";
		ostringstream oss3;
		oss3 << "./Video/source/tileS/roller.MKV_" << i << "_14.avi";
		fileName3 = oss3.str();
		//fileName3 ="./Video/source/tileS/roller.MKV_10_14.avi";
		ostringstream oss4;
		oss4 << "./Video/source/tileS/roller.MKV_" << i << "_15.avi";
		fileName4 = oss4.str();
		//fileName4 = "./Video/source/tileS/roller.MKV_10_15.avi";
		VideoCapture cap1(fileName1);
		VideoCapture cap2(fileName2);
		VideoCapture cap3(fileName3);
		VideoCapture cap4(fileName4);

		int tileW = 640; int tileH = 512;

		Mat frame;
		upload_image("./Video/Source/startScene.PNG", frame);
		Mat largeFrame(2 * tileH, 2 * tileW, frame.type());
		int fi = 0;

		VideoWriter writer1;
		int codec = VideoWriter::fourcc('H', '2', '6', '4');
		writer1.set(VIDEOWRITER_PROP_QUALITY, 10);
		cout << writer1.get(VIDEOWRITER_PROP_QUALITY);
		ostringstream ossOut;
		ossOut << "./Video/source/tileS/rollerCombined891415_" << i << ".avi";
		string ofileName = ossOut.str();
		//string ofileName = "./Video/source/tileS/divingCombined781415.avi";
		cout << "Writing videofile: " << ofileName << endl;
		writer1.open(ofileName, codec, 30, Size(2 * tileW, 2 * tileH), true);


		while (!frame.empty())
		{
			cap1 >> frame;
			if ((!frame.empty()))
			{
				//cout << "1" << endl;
				frame.copyTo(largeFrame(Rect(0, 0, tileW, tileH)));
			}
			
			cap2 >> frame;
			if ((!frame.empty()))
			{
				//cout << "11" << endl;
				//displayImage(frame);
				//displayImage(largeFrame);
				frame.copyTo(largeFrame(Rect(tileW, 0, tileW, tileH)));
			}
			cap3 >> frame;
			if ((!frame.empty()))
			{
				//cout << "111" << endl;
				frame.copyTo(largeFrame(Rect(0, tileH, tileW, tileH)));
			}
			cap4 >> frame;
			if ((!frame.empty()))
			{
				//cout << "1111" << endl;
				frame.copyTo(largeFrame(Rect(tileW, tileH, tileW, tileH)));
			}
			fi++;
			cout << fi << endl;
			writer1.write(largeFrame);
		}

		writer1.release();
	}
}

void testNtileDecodeTimeSeperateVsOnAsingleFrame()
{
	vector<float> timeCore, chunk1,chunk2,chunk3,chunk4,combined;
	double chunk1Total = 0;
	double chunk2Total = 0;
	double chunk3Total = 0;
	double chunk4Total = 0;
	double combinedt = 0;
	double core = 0;
	for (int i = 0; i < 100; i++)
	{


		for (int xi = 10; xi < 11; xi++)
		{
			String fileName1, fileName2, fileName3, fileName4, fileName5;
			//	fileName1 = "./Video/source/tileS/diving.avi_10_8.avi";
				//fileName2 = "./Video/source/tileS/diving.avi_10_9.avi";
				//fileName3 = "./Video/source/tileS/diving.avi_10_14.avi";
				//fileName4 = "./Video/source/tileS/diving.avi_10_15.avi";
				//fileName1 = "./Video/Tiles/diving/diving_1x.avi";
				//fileName2 = "./Video/Tiles/diving/diving_2x.avi";
				//fileName3 = "./Video/Tiles/diving/diving_3x.avi";
				//fileName4 = "./Video/Tiles/diving/diving_4x.avi";
			Mat frame;

			stringstream oss1, oss2, oss3, oss4, oss5;
			oss1 << "./Video/source/tileS/1sTiles/crf30/diving.avi_" << xi << "_8.avi1_0_0.avi_30.avi";
			oss2 << "./Video/source/tileS/1sTiles/crf30/diving.avi_" << xi << "_8.avi2_0_0.avi_30.avi";
			oss3 << "./Video/source/tileS/1sTiles/crf30/diving.avi_" << xi << "_8.avi3_0_0.avi_30.avi";
			oss4 << "./Video/source/tileS/1sTiles/crf30/diving.avi_" << xi << "_8.avi4_0_0.avi_30.avi";
			oss5 << "./Video/source/tileS/1sTiles/crf30/diving.avi_" << xi << "_8.avi_30.avi";
			upload_image("./Video/source/startScene.PNG", frame);
			int i = 0;

			fileName1 = oss1.str();
			fileName2 = oss2.str();
			fileName3 = oss3.str();
			fileName4 = oss4.str();
			fileName5 = oss5.str();

			cout << fileName1 << endl;
			cout << fileName5 << endl;

			auto startDecoding = std::chrono::high_resolution_clock::now();
			VideoCapture capx(fileName5);
			while (!frame.empty())
			{
				i++;
				capx >> frame;
				if (i > 120)
				{
					break;
				}
			}
			auto endDecoding = std::chrono::high_resolution_clock::now();
			chrono::duration<double> elapsed = endDecoding - startDecoding;
			cout << "Time for combined: " << elapsed.count() * 1000 << endl;
			combined.push_back(elapsed.count() * 1000);
			combinedt = combinedt + elapsed.count() * 1000;
			cout << i << endl;
			i = 0;
			upload_image("./Video/source/startScene.PNG", frame);
			startDecoding = std::chrono::high_resolution_clock::now();
			VideoCapture cap1(fileName1);
			while (!frame.empty())
			{
				i++;
				if (i > 30)
				{
					break;
				}
				cap1 >> frame;
			}

			endDecoding = std::chrono::high_resolution_clock::now();
			elapsed = endDecoding - startDecoding;
			cout << "Time for tile1: " << elapsed.count() * 1000 << endl;
			chunk1Total = chunk1Total + elapsed.count() * 1000;
			cout << i << endl;
			i = 0;
			chunk1.push_back(elapsed.count() * 1000);
			upload_image("./Video/source/startScene.PNG", frame);
			startDecoding = std::chrono::high_resolution_clock::now();
			VideoCapture cap2(fileName2);
			while (!frame.empty())
			{
				i++;
				if (i > 30)
				{
					break;
				}
				cap2 >> frame;
			}
			endDecoding = std::chrono::high_resolution_clock::now();
			elapsed = endDecoding - startDecoding;
			cout << "Time for tile2: " << elapsed.count() * 1000 << endl;
			cout << i << endl;
			chunk2.push_back(elapsed.count() * 1000);
			chunk2Total = chunk2Total + elapsed.count() * 1000;
			i = 0;
			upload_image("./Video/source/startScene.PNG", frame);
			startDecoding = std::chrono::high_resolution_clock::now();
			VideoCapture cap3(fileName3);
			while (!frame.empty())
			{
				i++;
				if (i > 30)
				{
					break;
				}
				cap3 >> frame;
			}
			cout << i << endl;
			endDecoding = std::chrono::high_resolution_clock::now();
			elapsed = endDecoding - startDecoding;
			cout << "Time for tile3: " << elapsed.count() * 1000 << endl;
			chunk3.push_back(elapsed.count() * 1000);
			chunk3Total = chunk3Total + elapsed.count() * 1000;
			i = 0;
			upload_image("./Video/source/startScene.PNG", frame);
			startDecoding = std::chrono::high_resolution_clock::now();
			VideoCapture cap4(fileName4);
			while (!frame.empty())
			{
				i++;
				if (i > 30)
				{
					break;
				}
				cap4 >> frame;
			}
			endDecoding = std::chrono::high_resolution_clock::now();
			elapsed = endDecoding - startDecoding;
			cout << "Time for tile4: " << elapsed.count() * 1000 << endl;
			cout << i << endl;
			chunk4.push_back(elapsed.count() * 1000);
			chunk4Total = chunk4Total + elapsed.count() * 1000;
			i = 0;
			startDecoding = std::chrono::high_resolution_clock::now();
			//	VideoCapture capxe("./Video/Tiles/CoRE/diving_6_10_40.avi");
			VideoCapture capxe("./Video/Tiles/diving/diving_1_10_-180.avi");
			while (!frame.empty())
			{
				i++;
				if (i > 30)
				{
					break;
				}
				capxe >> frame;
			}
			endDecoding = std::chrono::high_resolution_clock::now();
			elapsed = endDecoding - startDecoding;
			//cout << "Time for CoRE: " << elapsed.count() * 1000 << endl;
			core = core + elapsed.count() * 1000;
			timeCore.push_back(elapsed.count() * 1000);
			cout << i << endl;
		}
		int sumChunk1 = 0; int sumChunk2 = 0; int sumChunk3 = 0; int sumChunk4; int sumCore = 0; int SumCombined = 0;
		for (int ki = 0; ki < timeCore.size(); ki++)
		{
			sumChunk1 = sumChunk1 + chunk1[ki];
			sumChunk2 = sumChunk2 + chunk2[ki];
			sumChunk3 = sumChunk3 + chunk3[ki];
			sumChunk4 = sumChunk4 + chunk4[ki];
			SumCombined = SumCombined + combined[ki];
			sumCore = sumCore + timeCore[ki];
			
		}
	}
		cout << "sum 1 " << chunk1Total / chunk1.size() << endl;
		cout << "sum 2 " << chunk2Total / chunk2.size() << endl;
		cout << "sum 3  " << chunk3Total / chunk3.size() << endl;
		cout << "sum 4 " << chunk4Total / chunk4.size() << endl;
		//cout << "core " << core / timeCore.size();
		cout << " comb " << combinedt / combined.size();
		//cout << "combined " << SumCombined / timeCore.size() << endl;
		//cout << "core " << sumCore / timeCore.size() << endl;
	

}

void testSeperateCoREinto4tiles()
{
	String fileName, fileName1, fileName2, fileName3, fileName4;
	fileName = "./Video/Tiles/diving/diving_1_10_-180.avi";

	fileName1 = "./Video/Tiles/diving/diving_1x.avi";
	fileName2 = "./Video/Tiles/diving/diving_2x.avi";
	fileName3 = "./Video/Tiles/diving/diving_3x.avi";
	fileName4 = "./Video/Tiles/diving/diving_4x.avi";
	VideoCapture cap1(fileName);	

	Mat frame;
	upload_image("./Video/Source/startScene.PNG", frame);
	
	int fi = 0;

	VideoWriter writer1, writer2, writer3, writer4;
	int codec = VideoWriter::fourcc('H', '2', '6', '4');
	writer1.set(VIDEOWRITER_PROP_QUALITY, 10);
	writer2.set(VIDEOWRITER_PROP_QUALITY, 10);
	writer3.set(VIDEOWRITER_PROP_QUALITY, 10);
	writer4.set(VIDEOWRITER_PROP_QUALITY, 10);
	cout << writer1.get(VIDEOWRITER_PROP_QUALITY);
	
	int tileFrameW = 844;
	int tileFrameH = 534;

	cout << "Writing videofile: " << fileName1 << endl;
	writer1.open(fileName1, codec, 30, Size(tileFrameW, tileFrameH), true);
	writer2.open(fileName2, codec, 30, Size(tileFrameW, tileFrameH), true);
	writer3.open(fileName3, codec, 30, Size(tileFrameW, tileFrameH), true);
	writer4.open(fileName4, codec, 30, Size(tileFrameW, tileFrameH), true);

	Mat frame1,frame2,frame3,frame4;
	while (!frame.empty())
	{
		cap1 >> frame;
		if ((!frame.empty()))
		{
			frame(Rect(0, 0, tileFrameW, tileFrameH)).copyTo(frame1);
			frame(Rect(tileFrameW, 0, tileFrameW, tileFrameH)).copyTo(frame2);
			frame(Rect(0, tileFrameH, tileFrameW, tileFrameH)).copyTo(frame3);
			frame(Rect(tileFrameW, tileFrameH, tileFrameW, tileFrameH)).copyTo(frame4);
		}
		writer1.write(frame1);
		writer2.write(frame2);
		writer3.write(frame3);
		writer4.write(frame4);
		
	}

	writer1.release();
	writer2.release();
	writer3.release();
	writer4.release();
	   
}

void testCoREtileDecodeTimeSeperateVsOnAsingleFrame()
{
	int sum1 = 0; int sum2 = 0; int sum3 = 0; int sum4 = 0; int comb = 0;
	
	String fileName1, fileName2, fileName3, fileName4;
	fileName1 = "./Video/Tiles/diving/diving_1x.avi";
	fileName2 = "./Video/Tiles/diving/diving_2x.avi";
	fileName3 = "./Video/Tiles/diving/diving_3x.avi";
	fileName4 = "./Video/Tiles/diving/diving_4x.avi";
	
	Mat frame;

	for (int ix = 0; ix < 10; ix++)
	{
		upload_image("./Video/source/startScene.PNG", frame);
		int i = 0;

		auto startDecoding = std::chrono::high_resolution_clock::now();
		VideoCapture capx("./Video/Tiles/diving/diving_1_10_-180.avi");
		while (!frame.empty())
		{
			capx >> frame;
		}
		auto endDecoding = std::chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed = endDecoding - startDecoding;
		cout << "Time for combined: " << elapsed.count() * 1000 << endl;
		comb = comb + elapsed.count() * 1000;	
		upload_image("./Video/source/startScene.PNG", frame);
		startDecoding = std::chrono::high_resolution_clock::now();
		VideoCapture cap1(fileName1);

		while (!frame.empty())
		{
			cap1 >> frame;

		}
		endDecoding = std::chrono::high_resolution_clock::now();
		elapsed = endDecoding - startDecoding;
		cout << "Time for tile1: " << elapsed.count() * 1000 << endl;
		sum1 = sum1 + elapsed.count() * 1000;
		upload_image("./Video/source/startScene.PNG", frame);
		startDecoding = std::chrono::high_resolution_clock::now();
		VideoCapture cap2(fileName2);

		while (!frame.empty())
		{
			cap2 >> frame;
		}
		endDecoding = std::chrono::high_resolution_clock::now();
		elapsed = endDecoding - startDecoding;
		cout << "Time for tile2: " << elapsed.count() * 1000 << endl;
		sum2 = sum2 + elapsed.count() * 1000;
		upload_image("./Video/source/startScene.PNG", frame);
		startDecoding = std::chrono::high_resolution_clock::now();
		VideoCapture cap3(fileName3);

		while (!frame.empty())
		{
			cap3 >> frame;
		}
		endDecoding = std::chrono::high_resolution_clock::now();
		elapsed = endDecoding - startDecoding;
		cout << "Time for tile3: " << elapsed.count() * 1000 << endl;
		sum3 = sum3 + elapsed.count() * 1000;
		upload_image("./Video/source/startScene.PNG", frame);
		startDecoding = std::chrono::high_resolution_clock::now();
		VideoCapture cap4(fileName4);
		while (!frame.empty())
		{
			cap4 >> frame;
		}
		endDecoding = std::chrono::high_resolution_clock::now();
		elapsed = endDecoding - startDecoding;
		cout << "Time for tile4: " << elapsed.count() * 1000 << endl;
		sum4 = sum4 + elapsed.count() * 1000;
	}

	cout << "avg Chunk1: " << sum1 / 100 << endl;
	cout << "avg Chunk2: " << sum2 / 100 << endl;
	cout << "avg Chunk3: " << sum3 / 100 << endl;
	cout << "avg Chunk4: " << sum4 / 100 << endl;
	cout << "avg com: " << comb / 100 << endl;

	STOP;
}