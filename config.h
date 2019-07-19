#pragma once
#define		IMAGE "./Image/cb_color_eri.jpg"
#define		IMAGE_TYPE 16
#define		cameraW  1200
#define     cameraH  800
#define		eriVw	 1600
#define		cFoV	 110     //hfov

#define		VIDEO    "./Video/roller_2000_1000.mkv"
#define		HMD_DATA  "./Video/roller.txt"

//function check_interpolation()
#define NUM_INTERP_frameN	0 // 0 means no interpolation... each frame one seperate camera. value 1 is not permitted... gives error


// function out_video_file_interpolated
#define NUM_FRAME_LOAD	290
#define FRAME_START		0

//main

#define INIT_CAM_N					0

#define TEST_IMAGE_INTERPOLATION	check_interpolation()
#define	TEST_VIDEO_INTERPOLATION	out_video_file_interpolated(output_image_mat, eri_video_image, path1)
#define	PLAY_ANIMATION_ON_IMAGE		play(source_image_mat, output_image_mat, eri_image, path1)
#define TEST_MOUSE_CONTROL			mouse_control(source_image_mat, output_image_mat, eri_image, camera1)
#define  TEST_FORWARD_BACKWARD		forward_backward(source_image_mat, output_image_mat, output_image_mat_reverse, eri_image, camera1)

#define ERI_INIT	Mat eriPixels;\
					upload_image(IMAGE, eriPixels);  \
					ERI eri(eriPixels.cols, eriPixels.rows);\
					ERI erivideoimage(eriVw, eriVw/2);\
					PPC camera1(cFoV, cameraW, cameraH);\
					Mat convPixels = Mat::zeros(cameraH, cameraW, eriPixels.type());\
					Mat convPixelsreverse = Mat::zeros(cameraH, cameraW, eriPixels.type());\
					Path path1;