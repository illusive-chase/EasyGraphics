#if defined(__linux) || defined(__linux__)
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <cstdint>
#include <linux/input.h>
#include "LinuxRender.hh"


struct LinuxRender {
public:
	friend class RenderImpl;	
private:
	fb_var_screeninfo vinfo = {};
	fb_fix_screeninfo finfo = {};
	long screensize = 0;
	long width = 0, height = 0;
	char* fbp = nullptr;
	char* mbp = nullptr;
	int fp = 0;
	int fevent0 = 0;
	int fmouse0 = 0;
	int mouse_click_status[3] = {};
	int mouse_move_status[3] = {};

	LinuxRender() {
		fp = open("/dev/fb0", O_RDWR);
		if (fp < 0) printf("Error: Fail to open device\n"), exit(1);
		fevent0 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
		if (fevent0 < 0) printf("Error: Fail to open device\n"), exit(1);
		fmouse0 = open("/dev/input/mouse0", O_RDONLY | O_NONBLOCK);
		if (fmouse0 < 0) printf("Error: Fail to open device\n"), exit(1);
		if (ioctl(fp, FBIOGET_FSCREENINFO, &finfo)) printf("Error: Fail to read fixed infor\n"), exit(1);
		if (ioctl(fp, FBIOGET_VSCREENINFO, &vinfo)) printf("Error: Fail to read variable info\n"), exit(1);
		screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
		width = vinfo.xres;
		height = vinfo.yres;
		if (width != 800 || height != 480) printf("Error: Unexpected screeb size (%u, %u) instead of (800, 480)\n", width, height), exit(1);
		if (vinfo.bits_per_pixel != 32) printf("Error: Unexpected bits per pixel %u instead of 32\n", vinfo.bits_per_pixel), exit(1);
		fbp = (char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
		if (fbp == MAP_FAILED) printf("Error: Failed to map framebuffer to memory\n"), exit(1);
		mbp = new char[screensize];
	}

	~LinuxRender() {
		munmap(fbp, screensize);
		close(fp);
		close(fevent0);
		close(fmouse0);
		delete[] mbp;
	}

	static LinuxRender& instance() {
		static LinuxRender content;
		return content;
	}

	void sync() {
		memcpy(fbp, mbp, screensize);
		memset(mbp, 0, screensize);
	}

	int* check_mouse_click() {
		input_event ts = {};
		mouse_click_status[0] = 0;
		while (read(fevent0, &ts, sizeof(ts)) >= 0) {
			if (ts.type == EV_ABS) {
				if (ts.code == ABS_X) mouse_click_status[1] = ts.value * 800 / 1024;
				else if (ts.code == ABS_Y) mouse_click_status[2] = ts.value * 480 / 600;
			} else if (ts.type == EV_KEY && ts.code == BTN_TOUCH) {
				mouse_click_status[0] = ts.value + 1;
			}
		}
		return mouse_click_status;
	}

	int* check_mouse_move() {
		uint8_t buff[3] = {};
		memset(mouse_move_status, 0, sizeof(mouse_move_status));
		while (read(fmouse0, buff, sizeof(buff)) >= 0) {
			mouse_move_status[0] = 1;
			mouse_move_status[1] += static_cast<int>(static_cast<int8_t>(buff[1]));
			mouse_move_status[2] += static_cast<int>(static_cast<int8_t>(buff[2]));
		}
		return mouse_move_status;
	}


};


int RenderImpl::Width() {
	return LinuxRender::instance().width;
}

int RenderImpl::Height() {
	return LinuxRender::instance().height;
}

uint8_t* RenderImpl::Data() {
	return reinterpret_cast<uint8_t*>(LinuxRender::instance().mbp);
}

void RenderImpl::Render() {
	LinuxRender::instance().sync();
}

int* RenderImpl::MouseClick() {
	return LinuxRender::instance().check_mouse_click();
}

int* RenderImpl::MouseMove() {
	return LinuxRender::instance().check_mouse_move();
}
#endif