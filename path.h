#pragma once
#include "ppc.h"
#include <vector> 


using namespace cv;


class Path {

public:
	vector<PPC> cams;  // these define the views (i.e. cameras) in between which the Path interpolates		
	vector<int> segmentFramesN;  // array of same length as cams minus 1, i.e. camsN-1, defining the number of frames for each segment; segmentFramesN[0] is the number of frames from cams[0] to cams[1];
	
	Path(int camsN);	
	
	int cam_array_size();
	void AppendCamera(PPC newCam, int framesN); // appends camera to cams array, connected by segment with framesN frames to the previously last camera; has to reallocate array of cams to have size increased by one, copy old data, append new data

	PPC* GetView(int segi, int framei); // returns ppc defining view on segment segi, by interpolating cams[segi] and cams[segi+1], at fractional step framei/segmentFramesN[segi]; use Interpolate method of PPC class

	void LoadIMT(char *fileName); // loads a path from file of French people, with one cam per frame, i.e. all segments are of length 1
	// recover axis v and angle theta, then rotate default camera (i.e. initial camera) about axis v
	// for this add to the PPC class a rotate about arbitrary axis method that rotates the ppc components (a, b, c)

	//PPC interPPC;
	//interPPC.SetInterpolated(cams[segi], cams[segi + 1], framei, segmentFramesN[segi]);

}

;
