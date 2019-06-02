
#include"path.h"


using namespace cv;

Path::Path(int camsN) {

	for (int i = 0; i < camsN; i++) 
	{
		cams.push_back(PPC(90, 1200, 800));
		segmentFramesN.push_back(10);
	}


}



int Path::cam_array_size() {
	return cams.size();
}



void Path::AppendCamera(PPC newCam, int framesN)
{
	cams.push_back(newCam);
	segmentFramesN.push_back(framesN);
}


PPC* Path::GetView(int segi, int framei) {

	PPC interPPC;
	interPPC.SetInterpolated(&cams[segi], &cams[segi + 1], framei, segmentFramesN[segi]);

	return &interPPC;
}

void Path::LoadIMT(char *fileName) {

}