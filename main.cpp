/* This file is part of the EPPM source code package.
 *
 * Copyright (c) 2013-2016 Linchao Bao (linchaobao@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "bao_basic.h"
#include "bao_flow_patchmatch_multiscale_cuda.h"
#include "bao_flow_tools.h"
#include <fstream>
#include <string>

using namespace std;

char* str_to_c(string & s)
{
	char * c = new char[s.size() + 1];
	std::copy(s.begin(), s.end(), c);
	c[s.size()] = '\0';
	return c;
}

int main(int argc, char*argv[])
{
	int h = 1080 / 2;//
	int w = 1920 / 2;//

	//alloc memory
	unsigned char***img1 = bao_alloc<unsigned char>(h, w, 3);
	unsigned char***img2 = bao_alloc<unsigned char>(h, w, 3);
	float**disp1_x = bao_alloc<float>(h, w);
	float**disp1_y = bao_alloc<float>(h, w);
	float**disp2_x = bao_alloc<float>(h, w);
	float**disp2_y = bao_alloc<float>(h, w);
	memset(&(disp1_x[0][0]), 0, sizeof(float)*h*w);
	memset(&(disp1_y[0][0]), 0, sizeof(float)*h*w);
	memset(&(disp2_x[0][0]), 0, sizeof(float)*h*w);
	memset(&(disp2_y[0][0]), 0, sizeof(float)*h*w);

	string baseDir = R"(G:\ProRail\Assen-Zwolle\)";
	string path = baseDir + R"(frames\)";
	int maxFrames = 4;
	int nextFrames = 4;

	for (int i = 1; i < maxFrames; i++)
	{
		cout << "Loading image..." << endl;

		int nchannels = 0;
		
		// Load image 1.
		std::string si1 = to_string(i);
		std::string padding1(8 - si1.size(), '0');
		si1 = padding1 + si1;
		string file1 = path + si1 + ".ppm";
		bao_loadimage_ppm(str_to_c(file1), img1[0][0], h, w, &nchannels);

		for (int j = 1; j < nextFrames; j++)
		{
			std::string si2 = to_string(i + j);
			std::string padding2(8 - si2.size(), '0');
			si2 = padding2 + si2;
			string file2 = path + si2 + ".ppm";

			// Load image.
			bao_loadimage_ppm(str_to_c(file2), img2[0][0], h, w, &nchannels);

			cout << "Processing " << i << " - " << j << " (image size " << w << " * " << h << " * " << nchannels << ")...\n";
			bao_timer_gpu_cpu timer;
			bao_flow_patchmatch_multiscale_cuda eppm;

			// Compute optical flow.
			timer.start();
			eppm.init(img1, img2, h, w);
			eppm.compute_flow(disp1_x, disp1_y);
			timer.time_display("GPU");

			cout << "Saving flo file..." << h << "*" << w << endl;

			string floFile = baseDir + R"(flow\)" + si1 + "-" + si2 + ".flo";
			bao_save_flo_file(floFile.c_str(), disp1_x, disp1_y, h, w);
		}
	}

	bao_free(img1);
	bao_free(img2);
	bao_free(disp1_x);
	bao_free(disp1_y);
	bao_free(disp2_x);
	bao_free(disp2_y);

	return 0;
}


