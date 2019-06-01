#include "ppc.h"
#include "m33.h"
#include"Image.h"
#include"path.h"

#include <C:\opencv\build\include\opencv2/opencv.hpp>
#include <C:\opencv\build\include\opencv2\core\core.hpp>
#include <C:\opencv\build\include\opencv2\highgui\highgui.hpp>
#include<math.h>
#include<string.h>

using namespace cv;

Path::Path(int camsN) {

	for (int i = 0; i < camsN; i++) 
	{
		cams.push_back(PPC(60, 320, 240));
		segmentFramesN.push_back(10);
	}


}

void Path::AppendCamera(PPC newCam, int framesN)
{
	cams.push_back(newCam);
	segmentFramesN.push_back(framesN);
}


PPC* Path::GetView(int segi, int framei) {

	//PPC interPPC;
	//interPPC.SetInterpolated(cams[segi], cams[segi + 1], framei, segmentFramesN[segi]);

	return 0;
}

void Path::LoadIMT(char *fileName) {

}