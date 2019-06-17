#include "pixelCalculation.h"

PIXELCALCULATION::PIXELCALCULATION(int totalpixelnumber) {
	totalPixelNumber = totalpixelnumber;
	for (int i = 0; i < totalpixelnumber; i++)
	{
		vector<int>s(1, 0);
		pixelInterval.push_back(s);

	}	
	for (int i = 0; i < totalpixelnumber; i++)
	{
		InsideLastTimeVector.push_back(0);	

	}
	currentPixelFrame = 0;

}

void PIXELCALCULATION::NewPixelInterval(int pixel, int frame)
{
	pixelInterval[pixel].push_back(frame);
}

void PIXELCALCULATION::EndPixelInterval(int pixel, int frame) {
	pixelInterval[pixel].push_back(frame);

}

void PIXELCALCULATION::GetFramePixelInterval(ERI eri, Mat erivis, PPC *ppc) 
{
	int wscalefactor = eri.w / erivis.cols;
	int hscalefactor =eri.h / erivis.rows;
	for (int row = 0; row < erivis.rows; row++)
	{
		for (int col = 0; col < erivis.cols; col++)
		{	
			int erow = row * hscalefactor;
			int ecol = col * wscalefactor;
			int pixelNo = (row)*erivis.cols + col;
			if (eri.ERIPixelInsidePPC(erow, ecol, ppc))
			{
				//cout << "inside for..................................... pixel:" << pixelNo << endl;
				
				if (!(InsideLastTimeVector.at(pixelNo)))
				{
					NewPixelInterval(pixelNo, currentPixelFrame);
					
					
				}
				InsideLastTimeVector.at(pixelNo) = 1;				
			}

			else 
			{
				//cout << "outside for pixel:" << pixelNo << endl;
				if (InsideLastTimeVector[pixelNo])
				{
					EndPixelInterval(pixelNo, currentPixelFrame-1);
					
				}
				InsideLastTimeVector.at(pixelNo) = 0;
				
			}//end if inside else outside


		}//end column
	}//end row

	currentPixelFrame++;

}//endmethod

void PIXELCALCULATION::PrintPixelInterval() 
{
	cout << "Pixel: " << totalPixelNumber;
	for (int pixelseq = 0; pixelseq < totalPixelNumber; pixelseq++)
	{
		cout << "Pixel: " << pixelseq;
		for (int intervalseq = 0; intervalseq < pixelInterval[pixelseq].size(); intervalseq++)
		{
			cout << pixelInterval[pixelseq][intervalseq] << ",";

		}//endintervalseqfor
		cout << "||";
	}//endpixelseqforlooop
}//endprintpixelinterval


void PIXELCALCULATION::SaveIntervalTxt() {
	ofstream output("Interval.csv");
	
	for (int pixelseq = 0; pixelseq < 80000; pixelseq++)
	{
		output << "P:" << pixelseq<<":";
		for (int intervalseq = 0; intervalseq < pixelInterval[pixelseq].size(); intervalseq++)
		{
			output << pixelInterval[pixelseq][intervalseq] << "-";

		}//endintervalseqfor
		output <<endl;
	}//endpixelseqfor
	
	output.close();
}