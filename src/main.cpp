
#include <getopt.h>
#include <iostream>
#include <cmath>
#include <string>
#include "utilities/lodepng.h"
#include "utilities/rgba.hpp"
#include "utilities/num.hpp"
#include <complex>
#include <cassert>
#include <limits>
#include <deque>
#include <mutex>
#include <atomic>
#include <condition_variable>

static std::vector<std::pair<double,rgb>> colourGradient = {
	{ 0.0		, { 0  , 0  , 0   } },
	{ 0.03		, { 0  , 7  , 100 } },
	{ 0.16		, { 32 , 107, 203 } },
	{ 0.42		, { 237, 255, 255 } },
	{ 0.64		, { 255, 170, 0   } },
	{ 0.86		, { 0  , 2  , 0   } },
	{ 1.0		, { 0  , 0  , 0   } }
};

static unsigned int blockDim = 16;
static unsigned int subDiv = 4;

static unsigned int res = 1024;
static unsigned int maxDwell = 512;
static bool mark = false;

static constexpr const int  dwellFill = std::numeric_limits<int>::max();
static constexpr const int  dwellCompute = std::numeric_limits<int>::max()-1;
static constexpr const rgba borderFill(255,255,255,255);
static constexpr const rgba borderCompute(255,0,0,255);
static std::vector<rgba> colours;

void createColourMap(unsigned int const maxDwell) {
	rgb colour(0,0,0);
	double pos = 0.0;

	//Adding the last value if its not there...
	if (colourGradient.size() == 0 || colourGradient.back().first != 1.0) {
		colourGradient.push_back({ 1.0, {0,0,0}});
	}

	for (auto &gradient : colourGradient) {
		int r = (int) gradient.second.r - colour.r;
		int g = (int) gradient.second.g - colour.g;
		int b = (int) gradient.second.b - colour.b;
		unsigned int const max = std::ceil((double) maxDwell * (gradient.first - pos));
		for (unsigned int i = 0; i < max; i++) {
			double blend = (double) i / max;
			rgba newColour(
				colour.r + (blend * r),
				colour.g + (blend * g),
				colour.b + (blend * b),
				255
			);
			colours.push_back(newColour);
		}
		pos = gradient.first;
		colour = gradient.second;
	}
}



rgba const &dwellColor(std::complex<double> const z, unsigned int const dwell) {
	static constexpr const double log2 = 0.693147180559945309417232121458176568075500134360255254120;
	assert(colours.size() > 0);
	switch (dwell) {
		case dwellFill:
			return borderFill;
		case dwellCompute:
			return borderCompute;
	}
	unsigned int index = dwell + 1 - std::log(std::log(std::abs(z))/log2);
	return colours.at(index % colours.size());
}

unsigned int pixelDwell(std::complex<double> const &cmin,
						std::complex<double> const &dc,
						unsigned int const y,
						unsigned int const x)
{
	double const fy = (double)y / res;
	double const fx = (double)x / res;
	std::complex<double> const c = cmin + std::complex<double>(fx * dc.real(), fy * dc.imag());
	std::complex<double> z = c;
	unsigned int dwell = 0;

	while(dwell < maxDwell && std::abs(z) < (2 * 2)) {
		z = z * z + c;
		dwell++;
	}

	return dwell;
}

int commonBorder(std::vector<std::vector<int>> &dwellBuffer,
				 std::complex<double> const &cmin,
				 std::complex<double> const &dc,
				 unsigned int const atY,
				 unsigned int const atX,
				 unsigned int const blockSize)
{
	unsigned int const yMax = (res > atY + blockSize - 1) ? atY + blockSize - 1 : res - 1;
	unsigned int const xMax = (res > atX + blockSize - 1) ? atX + blockSize - 1 : res - 1;
	int commonDwell = -1;
	for (unsigned int i = 0; i < blockSize; i++) {
		for (unsigned int s = 0; s < 4; s++) {
			unsigned const int y = s % 2 == 0 ? atY + i : (s == 1 ? yMax : atY);
			unsigned const int x = s % 2 != 0 ? atX + i : (s == 0 ? xMax : atX);
			if (y < res && x < res) {
				if (dwellBuffer.at(y).at(x) < 0) {
					dwellBuffer.at(y).at(x) = pixelDwell(cmin, dc, y, x);
				}
				if (commonDwell == -1) {
					commonDwell = dwellBuffer.at(y).at(x);
				} else if (commonDwell != dwellBuffer.at(y).at(x)) {
					return -1;
				}
			}
		}
	}
	return commonDwell;
}

void markBorder(std::vector<std::vector<int>> &dwellBuffer,
				int const dwell,
				unsigned int const atY,
				unsigned int const atX,
				unsigned int const blockSize)
{
	unsigned int const yMax = (res > atY + blockSize - 1) ? atY + blockSize - 1 : res - 1;
	unsigned int const xMax = (res > atX + blockSize - 1) ? atX + blockSize - 1 : res - 1;
	for (unsigned int i = 0; i < blockSize; i++) {
		for (unsigned int s = 0; s < 4; s++) {
			unsigned const int y = s % 2 == 0 ? atY + i : (s == 1 ? yMax : atY);
			unsigned const int x = s % 2 != 0 ? atX + i : (s == 0 ? xMax : atX);
			if (y < res && x < res) {
				dwellBuffer.at(y).at(x) = dwell;
			}
		}
	}
}

// Currently the same as computeBlock
void threadedComputeBlock(std::vector<std::vector<int>> &dwellBuffer,
	std::complex<double> const &cmin,
	std::complex<double> const &dc,
	unsigned int const atY,
	unsigned int const atX,
	unsigned int const blockSize,
	unsigned int const omitBorder = 0)
{
	unsigned int const yMax = (res > atY + blockSize) ? atY + blockSize : res;
	unsigned int const xMax = (res > atX + blockSize) ? atX + blockSize : res;
	for (unsigned int y = atY + omitBorder; y < yMax - omitBorder; y++) {
		for (unsigned int x = atX + omitBorder; x < xMax - omitBorder; x++) {
			dwellBuffer.at(y).at(x) = pixelDwell(cmin, dc, y, x);
		}
	}
}

void computeBlock(std::vector<std::vector<int>> &dwellBuffer,
	std::complex<double> const &cmin,
	std::complex<double> const &dc,
	unsigned int const atY,
	unsigned int const atX,
	unsigned int const blockSize,
	unsigned int const omitBorder = 0)
{
	unsigned int const yMax = (res > atY + blockSize) ? atY + blockSize : res;
	unsigned int const xMax = (res > atX + blockSize) ? atX + blockSize : res;
	for (unsigned int y = atY + omitBorder; y < yMax - omitBorder; y++) {
		for (unsigned int x = atX + omitBorder; x < xMax - omitBorder; x++) {
			dwellBuffer.at(y).at(x) = pixelDwell(cmin, dc, y, x);
		}
	}
}

void fillBlock(std::vector<std::vector<int>> &dwellBuffer,
			   int const dwell,
			   unsigned int const atY,
			   unsigned int const atX,
			   unsigned int const blockSize,
			   unsigned int const omitBorder = 0)
{
	unsigned int const yMax = (res > atY + blockSize) ? atY + blockSize : res;
	unsigned int const xMax = (res > atX + blockSize) ? atX + blockSize : res;
	for (unsigned int y = atY + omitBorder; y < yMax - omitBorder; y++) {
		for (unsigned int x = atX + omitBorder; x < xMax - omitBorder; x++) {
			if (dwellBuffer.at(y).at(x) < 0) {
				dwellBuffer.at(y).at(x) = dwell;
			}
		}
	}
}


// define job data type here

typedef struct job {
   std::vector<std::vector<int>> &dwellBuffer;
   std::complex<double> &cmin;
   std::complex<double> &dc;
   unsigned int const atY;
   unsigned int const atX;
   unsigned int const blockSize;
} job;

// define mutex, condition variable and deque here

void addWork(/* parameters */)
{

}

void marianiSilver( std::vector<std::vector<int>> &dwellBuffer,
					std::complex<double> const &cmin,
					std::complex<double> const &dc,
					unsigned int const atY,
					unsigned int const atX,
					unsigned int const blockSize)
{
	int dwell = commonBorder(dwellBuffer, cmin, dc, atY, atX, blockSize);
	if ( dwell >= 0 ) {
		fillBlock(dwellBuffer, dwell, atY, atX, blockSize);
		if (mark)
			markBorder(dwellBuffer, dwellFill, atY, atX, blockSize);
	} else if (blockSize <= blockDim) {
		computeBlock(dwellBuffer, cmin, dc, atY, atX, blockSize);
		if (mark)
			markBorder(dwellBuffer, dwellCompute, atY, atX, blockSize);
	} else {
		// Subdivision
		unsigned int newBlockSize = blockSize / subDiv;
		for (unsigned int ydiv = 0; ydiv < subDiv; ydiv++) {
			for (unsigned int xdiv = 0; xdiv < subDiv; xdiv++) {
				marianiSilver(dwellBuffer, cmin, dc, atY + (ydiv * newBlockSize), atX + (xdiv * newBlockSize), newBlockSize);
			}
		}
	}
}

void help() {
	std::cout << "Mandelbrot Set Renderer" << std::endl;
	std::cout << std::endl;
	std::cout << "\t" << "-x [0;1]" << "\t" << "Center of Re[-1.5;0.5] (default=0.5)" << std::endl;
	std::cout << "\t" << "-y [0;1]" << "\t" << "Center of Im[-1;1] (default=0.5)" << std::endl;
	std::cout << "\t" << "-s (0;1]" << "\t" << "Inverse scaling factor (default=1)" << std::endl;
	std::cout << "\t" << "-r [pixel]" << "\t" << "Image resolution (default=1024)" << std::endl;
	std::cout << "\t" << "-i [iterations]" << "\t" << "Iterations or max dwell (default=512)" << std::endl;
	std::cout << "\t" << "-c [colours]" << "\t" << "colour map iterations (default=1)" << std::endl;
	std::cout << "\t" << "-b [block dim]" << "\t" << "min block dimension for subdivision (default=16)" << std::endl;
	std::cout << "\t" << "-d [subdivison]" << "\t" << "subdivision of blocks (default=4)" << std::endl;
	std::cout << "\t" << "-m" << "\t" << "mark Mariani-Silver borders" << std::endl;
	std::cout << "\t" << "-t" << "\t" << "traditional computation (no Mariani-Silver)" << std::endl;
}

void worker(void) {
	// Currently I'm doing nothing
}

int main( int argc, char *argv[] )
{
	std::string output = "output.png";
	double x = 0.5, y = 0.5;
	double scale = 1;
	unsigned int colourIterations = 1;
	bool mariani = true;
	bool quiet = false;

	{
		char c;
		while((c = getopt(argc,argv,"x:y:s:r:o:i:c:b:d:mthq"))!=-1) {
			switch(c) {
				case 'x':
					x = num::clamp(atof(optarg),0.0,1.0);
					break;
				case 'y':
					y = num::clamp(atof(optarg),0.0,1.0);
					break;
				case 's':
					scale = num::clamp(atof(optarg),0.0,1.0);
					if (scale == 0) scale = 1;
					break;
				case 'r':
					res = std::max(1,atoi(optarg));
					break;
				case 'i':
					maxDwell = std::max(1,atoi(optarg));
					break;
				case 'c':
					colourIterations = std::max(1,atoi(optarg));
					break;
				case 'b':
					blockDim = std::max(4,atoi(optarg));
					break;
				case 'd':
					subDiv = std::max(2,atoi(optarg));
					break;
				case 'm':
					mark = true;
					break;
				case 't':
					mariani = false;
					break;
				case 'q':
					quiet = true;
					break;
				case 'o':
					output = optarg;
					break;
				case 'h':
					help();
					exit(0);
					break;
				default:
					std::cerr << "Unknown argument '" << c << "'" << std::endl << std::endl;
					help();
					exit(1);
			}
		}
	}

	double const xmin = -3.5 + (2 * 2 * x);
	double const xmax = -1.5 + (2 * 2 * x);
	double const ymin = -3.0 + (2 * 2 * y);
	double const ymax = -1.0 + (2 * 2 * y);
	double const xlen = std::abs(xmin - xmax);
	double const ylen = std::abs(ymin - ymax);

	std::complex<double> const cmin(xmin + (0.5 * (1 - scale) * xlen),ymin + (0.5 * (1 - scale) * ylen));
	std::complex<double> const cmax(xmax - (0.5 * (1 - scale) * xlen),ymax - (0.5 * (1 - scale) * ylen));
	std::complex<double> const dc = cmax - cmin;

	if (!quiet) {
		std::cout << std::fixed;
		std::cout << "Center:      [" << x << "," << y << "]" << std::endl;
		std::cout << "Zoom:        " << (unsigned long long) (1/scale) * 100 << "%" <<  std::endl;
		std::cout << "Iterations:  " << maxDwell  << std::endl;
		std::cout << "Window:      Re[" << cmin.real() << ", " << cmax.real() << "], Im[" << cmin.imag() << ", " << cmax.imag() << "]" << std::endl;
		std::cout << "Output:      " << output << std::endl;
		std::cout << "Block dim:   " << blockDim << std::endl;
		std::cout << "Subdivision: " << subDiv << std::endl;
		std::cout << "Borders:     " << ((mark) ? "marking" : "not marking") << std::endl;
	}

	std::vector<std::vector<int>> dwellBuffer(res, std::vector<int>(res, -1));

	if (mariani) {
		// Scale the blockSize from res up to a subdividable value
		// Number of possible subdivisions:
		unsigned int const numDiv = std::ceil(std::log((double) res/blockDim)/std::log((double) subDiv));
		// Calculate a dividable resolution for the blockSize:
		unsigned int const correctedBlockSize = std::pow(subDiv,numDiv) * blockDim;
		// Mariani-Silver subdivision algorithm
		marianiSilver(dwellBuffer, cmin, dc, 0, 0, correctedBlockSize);
	} else {
		// Traditional Mandelbrot-Set computation or the 'Escape Time' algorithm
		computeBlock(dwellBuffer, cmin, dc, 0, 0, res, 0);
		if (mark)
			markBorder(dwellBuffer, dwellCompute, 0, 0, res);
	}

	// Add here the worker for Task 2

	// The colour iterations defines how often the colour gradient will
	// be seen on the final picture. Basically the repetitive factor
	createColourMap(maxDwell / colourIterations);
	std::vector<unsigned char> frameBuffer(res * res * 4, 0);
	unsigned char *pixel = &(frameBuffer.at(0));

	// Map the dwellBuffer to the frameBuffer
	for (unsigned int y = 0; y < res; y++) {
		for (unsigned int x = 0; x < res; x++) {
			// Getting a colour from the map depending on the dwell value and
			// the coordinates as a complex number. This  method is responsible
			// for all the nice colours you see
			rgba const &colour = dwellColor(std::complex<double>(x,y), dwellBuffer.at(y).at(x));
			// class rgba provides a method to directly write a colour into a
			// framebuffer. The address to the next pixel is hereby returned
			pixel = colour.putFramebuffer(pixel);
		}
	}

	unsigned int const error = lodepng::encode(output, frameBuffer, res, res);
	if (error) {
		std::cout << "An error occurred while writing the image file: " << error << ": " << lodepng_error_text(error) << std::endl;
		return 1;
	}

	return 0;
}

