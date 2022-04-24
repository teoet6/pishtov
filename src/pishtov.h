/*
Copyright (c) 2021 Theodor Totev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
Pishtov is a C header that mimics Yashu's JS Pishtov. It tries not only to be
faster by being written in C but to also be more powerful.

The point of the JS Pishtov is to remove the boilerplate from game programming
and thus make it easy to teach children programming. The JS Pishtov is very
successful in this domain. Many young programmers were and are taught
programming using it. Despite this the JS Pishtov has flaws - it is near
impossible to make a performance-intensive game in it, it fails at teaching
important material like types, files, threads, and sometimes even the idea that
code is executed top to bottom. The JS Pishtov frequently conflicts with the
other most prevalent programming material in Bulgaria - competitive
programming - it's hard to teach games to competitive programmers and even
harder to teach cp to game programmers.

Pishtov tries to retain the ease-of-use for new programmers, but also to
allow skilled programmers to create faster and bigger games and simulations. By
virtue of being written in a systems lannguage it expands the toolkit of the
programmer with files, threading, stricter typechecking, pointers and much
more.

This project's purpouse is currently ill-defined. The line between feature
creep and actual feature is very blurry. As such I frequently move code between
the actual header file and the example usage.

TODO In no particular order
	[ ] Have a build system that expands `#include ""` and creates the final header file instead of writing everything in one file.
	[X] Make Pishtov a C library instead of a C++ one
	[X] Rectangles
	[X] Circles and Ellipses
	[ ] Arcs i.e. semicircles and semiellipses
	[X] Lines
	[X] Images
	[ ] Text
	[X] Translation, scaling and rotation of the canvas
	[ ] OS-independent network-programming
	[ ] 3D graphics
	[ ] Shaders
	[ ] An optional define stating whether or not you want a main game loop or just the library functions
	[X] Default Code::Blocks Windows compile
	[ ] WASM compile
	[ ] Android compile
*/

#ifndef PISHTOV_H
#define PISHTOV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__);

// User side stuff
void keydown(int key);
void keyup(int key);
void mousedown(int button);
void mouseup(int button);
void init();
void update();
void draw();

// OS-specific stuff. This includes opening a window, making an OpenGL context
// for said window, processing keyboard and mouse input, swapping the buffers,
// providing functions that aren't in the C standard library and more. You need
// to have the following functions defined for each OS:
void pshtv_open_window(const char *name, int w, int h); // Open a window.
void pshtv_handle_events(); // Ask the OS for new events and react accordingly.
void pshtv_swap_buffers(); // Swap the OpenGL buffers. Can be a noop.
void *pshtv_load_gl(const char *name); // Dynamically load an OpenGL function

float mouse_x, mouse_y; // Automatically set to the mouse position on the window
float window_w, window_h; // Automatically set to the window dimensions

// The other part of the pishtov deals with actually drawing using OpenGL
// It defines the following functions and variables:
float pshtv_transform_matrix[4][4];
float pshtv_fill_color[4];

void fill_color(uint32_t c);
void fill_rect(float x, float y, float w, float h);
void fill_line(float x1, float y1, float x2, float y2, float w);
void fill_ellipse(float x, float y, float rx, float ry);
void fill_triangle(float x1, float y1, float x2, float y2, float x3, float y3);
void draw_image_buffer(uint8_t *buffer, uint32_t img_w, uint32_t img_h, float x, float y, float w, float h);
void translate(float x, float y);
void scale(float x, float y);
void rotate(float a);
void pshtv_redraw();

// All of the functions not mentioned above are implementation details and are
// subject to change and/or removal at any time. Functions mentioned above are
// more or less stable.

#if defined(_WIN32) // Windows 32 or 64 bit

#include <windows.h>
#include <windowsx.h>

HWND pshtv_hwnd;
HDC pshtv_hdc;

// These are all functions from wingdi so you have to link to them. In
// order to retain our goal of compiling with default settings on
// Code::Blocks we dynamically load them.
typedef int (*PSHTVPROC_CHOOSEPIXELFORMAT) (HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);
typedef BOOL (*PSHTVPROC_SETPIXELFORMAT) (HDC hdc, int format, const PIXELFORMATDESCRIPTOR *ppfd);
typedef HGLRC (*PSHTVPROC_WGLCREATECONTEXT) (HDC hdc);
typedef BOOL (*PSHTVPROC_WGLMAKECURRENT) (HDC hdc, HGLRC hglrc);
typedef BOOL (*PSHTVPROC_SWAPBUFFERS) (HDC hdc);
typedef PROC (*PSHTVPROC_WGLGETPROCADDRESS) (LPCSTR unnamedParam1);

PSHTVPROC_CHOOSEPIXELFORMAT pshtv_ChoosePixelFormat;
PSHTVPROC_SETPIXELFORMAT    pshtv_SetPixelFormat;
PSHTVPROC_SWAPBUFFERS       pshtv_SwapBuffers;
PSHTVPROC_WGLCREATECONTEXT  pshtv_wglCreateContext;
PSHTVPROC_WGLMAKECURRENT    pshtv_wglMakeCurrent;
PSHTVPROC_WGLGETPROCADDRESS pshtv_wglGetProcAddress;

void *pshtv_load_gdi(const char *name) {
	static HMODULE gdi_handle;
	if (!gdi_handle)
		gdi_handle = LoadLibrary("gdi32.dll");
	return (void*)GetProcAddress(gdi_handle, name);
}

void *pshtv_load_wgl(const char *name) {
	static HMODULE gl_handle;
	if (!gl_handle)
		gl_handle = LoadLibrary("opengl32.dll");
	return (void*)GetProcAddress(gl_handle, name);
}

void *pshtv_load_gl(const char* name) {
	void *ret = (void*)pshtv_wglGetProcAddress(name);
	// wglGetProcAddress returs NULL for non-extension functions, so we
	// need to load the function from opengl32.dll directly.
	if (!ret)
		ret = pshtv_load_wgl(name);
	return ret;
}

LRESULT CALLBACK pshtv_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_MOUSEMOVE:
		mouse_x = GET_X_LPARAM(lparam);
		mouse_y = GET_Y_LPARAM(lparam);
		return 0;
	case WM_LBUTTONDOWN: mousedown(1); return 0;
	case WM_MBUTTONDOWN: mousedown(2); return 0;
	case WM_RBUTTONDOWN: mousedown(3); return 0;
	case WM_LBUTTONUP: mouseup(1); return 0;
	case WM_MBUTTONUP: mouseup(2); return 0;
	case WM_RBUTTONUP: mouseup(3); return 0;
	case WM_KEYDOWN: keydown(wparam); return 0;
	case WM_KEYUP:   keyup  (wparam); return 0;
	case WM_SIZE:
		window_w = LOWORD(lparam);
		window_h = HIWORD(lparam);
		return 0;
	case WM_QUIT: exit(0);
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void pshtv_open_window(const char *name, int w, int h) {
	pshtv_ChoosePixelFormat = (PSHTVPROC_CHOOSEPIXELFORMAT) pshtv_load_gdi("ChoosePixelFormat");
	pshtv_SetPixelFormat    = (PSHTVPROC_SETPIXELFORMAT)    pshtv_load_gdi("SetPixelFormat");
	pshtv_SwapBuffers       = (PSHTVPROC_SWAPBUFFERS)       pshtv_load_gdi("SwapBuffers");
	pshtv_wglCreateContext  = (PSHTVPROC_WGLCREATECONTEXT)  pshtv_load_wgl("wglCreateContext");
	pshtv_wglMakeCurrent    = (PSHTVPROC_WGLMAKECURRENT)    pshtv_load_wgl("wglMakeCurrent");
	pshtv_wglGetProcAddress = (PSHTVPROC_WGLGETPROCADDRESS) pshtv_load_wgl("wglGetProcAddress");

	HINSTANCE hinstance = GetModuleHandle(NULL);

	WNDCLASS wc = {};
	wc.lpfnWndProc = pshtv_window_proc;
	wc.hInstance   = hinstance;
	wc.lpszClassName = name;
	wc.style = CS_OWNDC;
	wc.hCursor = LoadCursor(hinstance, IDC_ARROW);
	RegisterClass(&wc);

	pshtv_hwnd = CreateWindowEx(0, name, name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, NULL, NULL, hinstance, NULL);
	if (pshtv_hwnd == NULL) {
		eprintf("Could not get window handle\n");
		exit(-1);
	}

	pshtv_hdc = GetDC(pshtv_hwnd);
	if (pshtv_hdc == NULL) {
		eprintf("Could not get device context\n");
		exit(-1);
	}

	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW, PFD_SUPPORT_OPENGL, PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int pixel_format = pshtv_ChoosePixelFormat(pshtv_hdc, &pfd);
	if (!pixel_format) {
		eprintf("Could not choose pixel format\n");
		exit(-1);
	}

	pshtv_SetPixelFormat(pshtv_hdc, pixel_format, &pfd);

	HGLRC context = pshtv_wglCreateContext(pshtv_hdc);
	pshtv_wglMakeCurrent(pshtv_hdc, context);

	ShowWindow(pshtv_hwnd, SW_NORMAL);

	window_w = w;
	window_h = h;
}

void pshtv_handle_events() {
	MSG msg = {};
	while(PeekMessage(&msg, pshtv_hwnd, 0, 0, PM_NOREMOVE)) {
		if(GetMessage(&msg, pshtv_hwnd, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			exit(0);
		}
	}
}

void pshtv_swap_buffers() {
	pshtv_SwapBuffers(pshtv_hdc);
}

#elif defined(__APPLE__)
#error Apple not supported
#elif defined(__ANDROID__)
#error Andoird not supported
#else // Assume X11 on Linux
// https://github.com/gamedevtech/X11OpenGLWindow

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <dlfcn.h>
#include <time.h>

Display *pshtv_display;
Window pshtv_window;
Atom pshtv_atom_wm_delete_window;

void pshtv_open_window(const char *name, int w, int h) {
	int screen_id;
	GLXContext context;

	pshtv_display = XOpenDisplay(NULL);
	if (pshtv_display == NULL) {
		eprintf("Error opening X11 display\n")
			exit(-1);
	}

	screen_id = DefaultScreen(pshtv_display);

	GLint majorGLX = 0, minorGLX = 0;
	glXQueryVersion(pshtv_display, &majorGLX, &minorGLX);
	if (majorGLX < 1 && minorGLX < 2) {
		eprintf("glx version is %d.%d\n", majorGLX, minorGLX);
		eprintf("Minimum supported glx version is 1.2\n");
		exit(-1);
	}

	GLint glx_attribs[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		// GLX_SAMPLE_BUFFERS, 0,
		// GLX_SAMPLES, 0,
		None,
	};
	XVisualInfo *visual_info = glXChooseVisual(pshtv_display, screen_id, glx_attribs);
	if (visual_info == NULL) {
		eprintf("Could not create correct visual window\n");
		exit(-1);
	}

	XSetWindowAttributes attributes;
	attributes.border_pixel = BlackPixel(pshtv_display, screen_id);
	attributes.background_pixel = WhitePixel(pshtv_display, screen_id);
	attributes.override_redirect = True;
	attributes.colormap = XCreateColormap(pshtv_display, RootWindow(pshtv_display, screen_id), visual_info->visual, AllocNone);
	attributes.event_mask = ExposureMask;

	pshtv_window = XCreateWindow(pshtv_display, RootWindow(pshtv_display, screen_id), 0, 0, w, h, 0, visual_info->depth, InputOutput, visual_info->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &attributes);

	pshtv_atom_wm_delete_window = XInternAtom(pshtv_display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(pshtv_display, pshtv_window, &pshtv_atom_wm_delete_window, 1);

	context = glXCreateContext(pshtv_display, visual_info, NULL, GL_TRUE);
	glXMakeCurrent(pshtv_display, pshtv_window, context);

	XClearWindow(pshtv_display, pshtv_window);

	XSelectInput(pshtv_display, pshtv_window,
		PointerMotionMask |
		ButtonPressMask |
		ButtonReleaseMask |
		KeyPressMask |
		KeyReleaseMask |
		KeymapStateMask |
		StructureNotifyMask);

	XStoreName(pshtv_display, pshtv_window, name);

	XMapWindow(pshtv_display, pshtv_window);

	window_w = w;
	window_h = h;
}

// TODO use KF86 keys as well
int pshtv_translate_key(int native) {
	switch (native) {
	                                   // VK_LBUTTON 0x01 Left mouse button
	                                   // VK_RBUTTON 0x02 Right mouse button
	                                   // VK_CANCEL 0x03 Control-break processing
	                                   // VK_MBUTTON 0x04 Middle mouse button (three-button mouse)
	                                   // VK_XBUTTON1 0x05 X1 mouse button
	                                   // VK_XBUTTON2 0x06 X2 mouse button
	                                   // --- 0x07 Undefined
	case XK_BackSpace:    return 0x08; // VK_BACK 0x08 BACKSPACE key
	case XK_Tab:          return 0x09; // VK_TAB 0x09 TAB key
	                                   // --- 0x0A-0B Reserved
	case XK_Clear:        return 0x0c; // VK_CLEAR 0x0C CLEAR key
	case XK_KP_Enter:
	case XK_Return:       return 0x0d; // VK_RETURN 0x0D ENTER key
	                                   // --- 0x0E-0F Undefined
	case XK_Shift_L:      return 0x10; // VK_SHIFT 0x10 SHIFT key
	case XK_Control_L:    return 0x11; // VK_CONTROL 0x11 CTRL key
	case XK_Alt_L:
	case XK_Alt_R:        return 0x12; // VK_MENU 0x12 ALT key
	case XK_Pause:        return 0x13; // VK_PAUSE 0x13 PAUSE key
	case XK_Caps_Lock:    return 0x14; // VK_CAPITAL 0x14 CAPS LOCK key
	                                   // TODO I don't even know what these buttons are supposed to do
	                                   // VK_KANA 0x15 IME Kana mode
	                                   // VK_HANGUEL 0x15 IME Hanguel mode (maintained for compatibility; use VK_HANGUL)
	                                   // VK_HANGUL 0x15 IME Hangul mode
	                                   // VK_IME_ON 0x16 IME On
	                                   // VK_JUNJA 0x17 IME Junja mode
	                                   // VK_FINAL 0x18 IME final mode
	                                   // VK_HANJA 0x19 IME Hanja mode
	                                   // VK_KANJI 0x19 IME Kanji mode
	                                   // VK_IME_OFF 0x1A IME Off
	case XK_Escape:       return 0x1b; // VK_ESCAPE 0x1B ESC key
	                                   // VK_CONVERT 0x1C IME convert VK_NONCONVERT 0x1D IME nonconvert
	                                   // VK_ACCEPT 0x1E IME accept
	                                   // VK_MODECHANGE 0x1F IME mode change request
	case XK_space:        return 0x20; // VK_SPACE 0x20 SPACEBAR
	case XK_Page_Up:      return 0x21; // VK_PRIOR 0x21 PAGE UP key
	case XK_Page_Down:    return 0x22; // VK_NEXT 0x22 PAGE DOWN key
	case XK_End:          return 0x23; // VK_END 0x23 END key
	case XK_Begin:
	case XK_Home:         return 0x24; // VK_HOME 0x24 HOME key
	case XK_Left:         return 0x25; // VK_LEFT 0x25 LEFT ARROW key
	case XK_Up:           return 0x26; // VK_UP 0x26 UP ARROW key
	case XK_Right:        return 0x27; // VK_RIGHT 0x27 RIGHT ARROW key
	case XK_Down:         return 0x28; // VK_DOWN 0x28 DOWN ARROW key
	case XK_Select:       return 0x29; // VK_SELECT 0x29 SELECT key
	                                   // VK_PRINT 0x2A PRINT key
	case XK_Execute:      return 0x2b; // VK_EXECUTE 0x2B EXECUTE key
	case XK_Print:        return 0x2c; // VK_SNAPSHOT 0x2C PRINT SCREEN key
	case XK_Insert:       return 0x2d; // VK_INSERT 0x2D INS key
	case XK_Delete:       return 0x2e; // VK_DELETE 0x2E DEL key
	case XK_Help:         return 0x2f; // VK_HELP 0x2F HELP key
	case XK_0:            return 0x30; // 0x30 0 key
	case XK_1:            return 0x31; // 0x31 1 key
	case XK_2:            return 0x32; // 0x32 2 key
	case XK_3:            return 0x33; // 0x33 3 key
	case XK_4:            return 0x34; // 0x34 4 key
	case XK_5:            return 0x35; // 0x35 5 key
	case XK_6:            return 0x36; // 0x36 6 key
	case XK_7:            return 0x37; // 0x37 7 key
	case XK_8:            return 0x38; // 0x38 8 key
	case XK_9:            return 0x39; // 0x39 9 key
	                                   // --- 0x3A-40 Undefined
	case XK_a:
	case XK_A:            return 0x41; // 0x41 A key
	case XK_b:
	case XK_B:            return 0x42; // 0x42 B key
	case XK_c:
	case XK_C:            return 0x43; // 0x43 C key
	case XK_d:
	case XK_D:            return 0x44; // 0x44 D key
	case XK_e:
	case XK_E:            return 0x45; // 0x45 E key
	case XK_f:
	case XK_F:            return 0x46; // 0x46 F key
	case XK_g:
	case XK_G:            return 0x47; // 0x47 G key
	case XK_h:
	case XK_H:            return 0x48; // 0x48 H key
	case XK_i:
	case XK_I:            return 0x49; // 0x49 I key
	case XK_j:
	case XK_J:            return 0x4a; // 0x4A J key
	case XK_k:
	case XK_K:            return 0x4b; // 0x4B K key
	case XK_l:
	case XK_L:            return 0x4c; // 0x4C L key
	case XK_m:
	case XK_M:            return 0x4d; // 0x4D M key
	case XK_n:
	case XK_N:            return 0x4e; // 0x4E N key
	case XK_o:
	case XK_O:            return 0x4f; // 0x4F O key
	case XK_p:
	case XK_P:            return 0x50; // 0x50 P key
	case XK_q:
	case XK_Q:            return 0x51; // 0x51 Q key
	case XK_r:
	case XK_R:            return 0x52; // 0x52 R key
	case XK_s:
	case XK_S:            return 0x53; // 0x53 S key
	case XK_t:
	case XK_T:            return 0x54; // 0x54 T key
	case XK_u:
	case XK_U:            return 0x55; // 0x55 U key
	case XK_v:
	case XK_V:            return 0x56; // 0x56 V key
	case XK_w:
	case XK_W:            return 0x57; // 0x57 W key
	case XK_x:
	case XK_X:            return 0x58; // 0x58 X key
	case XK_y:
	case XK_Y:            return 0x59; // 0x59 Y key
	case XK_z:
	case XK_Z:            return 0x5a; // 0x5A Z key
	case XK_Super_L:      return 0x5b; // VK_LWIN 0x5B Left Windows key (Natural keyboard)
	case XK_Super_R:      return 0x5c; // VK_RWIN 0x5C Right Windows key (Natural keyboard)
	                                   // VK_APPS 0x5D Applications key (Natural keyboard)
	                                   // --- 0x5E Reserved
	                                   // VK_SLEEP 0x5F Computer Sleep key
	case XK_KP_0:
	case XK_KP_Insert:    return 0x60; // VK_NUMPAD0 0x60 Numeric keypad 0 key
	case XK_KP_1:
	case XK_KP_End:       return 0x61; // VK_NUMPAD1 0x61 Numeric keypad 1 key
	case XK_KP_2:
	case XK_KP_Down:      return 0x62; // VK_NUMPAD2 0x62 Numeric keypad 2 key
	case XK_KP_3:
	case XK_KP_Page_Down: return 0x63; // VK_NUMPAD3 0x63 Numeric keypad 3 key
	case XK_KP_4:
	case XK_KP_Left:      return 0x64; // VK_NUMPAD4 0x64 Numeric keypad 4 key
	case XK_KP_5:
	case XK_KP_Begin:     return 0x65; // VK_NUMPAD5 0x65 Numeric keypad 5 key
	case XK_KP_6:
	case XK_KP_Right:     return 0x66; // VK_NUMPAD6 0x66 Numeric keypad 6 key
	case XK_KP_7:
	case XK_KP_Home:      return 0x67; // VK_NUMPAD7 0x67 Numeric keypad 7 key
	case XK_KP_8:
	case XK_KP_Up:        return 0x68; // VK_NUMPAD8 0x68 Numeric keypad 8 key
	case XK_KP_9:
	case XK_KP_Page_Up:   return 0x69; // VK_NUMPAD9 0x69 Numeric keypad 9 key
	case XK_KP_Multiply:  return 0x6a; // VK_MULTIPLY 0x6A Multiply key
	case XK_KP_Add:       return 0x6b; // VK_ADD 0x6B Add key
	case XK_KP_Separator: return 0x6c;
	case XK_KP_Subtract:  return 0x6d; // VK_SUBTRACT 0x6D Subtract key
	case XK_KP_Decimal:
	case XK_KP_Delete:    return 0x6e; // VK_DECIMAL 0x6E Decimal key
	case XK_KP_Divide:    return 0x6f; // VK_DIVIDE 0x6F Divide key
	case XK_F1:           return 0x70; // VK_F1 0x70 F1 key
	case XK_F2:           return 0x71; // VK_F2 0x71 F2 key
	case XK_F3:           return 0x72; // VK_F3 0x72 F3 key
	case XK_F4:           return 0x73; // VK_F4 0x73 F4 key
	case XK_F5:           return 0x74; // VK_F5 0x74 F5 key
	case XK_F6:           return 0x75; // VK_F6 0x75 F6 key
	case XK_F7:           return 0x76; // VK_F7 0x76 F7 key
	case XK_F8:           return 0x77; // VK_F8 0x77 F8 key
	case XK_F9:           return 0x78; // VK_F9 0x78 F9 key
	case XK_F10:          return 0x79; // VK_F10 0x79 F10 key
	case XK_F11:          return 0x7a; // VK_F11 0x7A F11 key
	case XK_F12:          return 0x7b; // VK_F12 0x7B F12 key
	case XK_F13:          return 0x7c; // VK_F13 0x7C F13 key
	case XK_F14:          return 0x7d; // VK_F14 0x7D F14 key
	case XK_F15:          return 0x7e; // VK_F15 0x7E F15 key
	case XK_F16:          return 0x7f; // VK_F16 0x7F F16 key
	case XK_F17:          return 0x80; // VK_F17 0x80 F17 key
	case XK_F18:          return 0x81; // VK_F18 0x81 F18 key
	case XK_F19:          return 0x82; // VK_F19 0x82 F19 key
	case XK_F20:          return 0x83; // VK_F20 0x83 F20 key
	case XK_F21:          return 0x84; // VK_F21 0x84 F21 key
	case XK_F22:          return 0x85; // VK_F22 0x85 F22 key
	case XK_F23:          return 0x86; // VK_F23 0x86 F23 key
	case XK_F24:          return 0x87; // VK_F24 0x87 F24 key
	                                   // --- 0x88-8F Unassigned
	case XK_Num_Lock:     return 0x90; // VK_NUMLOCK 0x90 NUM LOCK key
	case XK_Scroll_Lock:  return 0x91; // VK_SCROLL 0x91 SCROLL LOCK key
	                                   // --- 0x92-96 OEM specific
	                                   // --- 0x97-9F Unassigned
	// case XK_Shift_L:      return 0xa0; // VK_LSHIFT 0xA0 Left SHIFT key
	case XK_Shift_R:      return 0xa1; // VK_RSHIFT 0xA1 Right SHIFT key
	// case XK_Control_L:    return 0xa2; // VK_LCONTROL 0xA2 Left CONTROL key
	case XK_Control_R:    return 0xa3; // VK_RCONTROL 0xA3 Right CONTROL key
	                                   // VK_LMENU 0xA4 Left MENU key
	case XK_Menu:         return 0xa5; // VK_RMENU 0xA5 Right MENU key
	                                   // VK_BROWSER_BACK 0xA6 Browser Back key
	                                   // VK_BROWSER_FORWARD 0xA7 Browser Forward key
	                                   // VK_BROWSER_REFRESH 0xA8 Browser Refresh key
	                                   // VK_BROWSER_STOP 0xA9 Browser Stop key
	                                   // VK_BROWSER_SEARCH 0xAA Browser Search key
	                                   // VK_BROWSER_FAVORITES 0xAB Browser Favorites key
	                                   // VK_BROWSER_HOME 0xAC Browser Start and Home key
	                                   // VK_VOLUME_MUTE 0xAD Volume Mute key
	                                   // VK_VOLUME_DOWN 0xAE Volume Down key
	                                   // VK_VOLUME_UP 0xAF Volume Up key
	                                   // VK_MEDIA_NEXT_TRACK 0xB0 Next Track key
	                                   // VK_MEDIA_PREV_TRACK 0xB1 Previous Track key
	                                   // VK_MEDIA_STOP 0xB2 Stop Media key
	                                   // VK_MEDIA_PLAY_PAUSE 0xB3 Play/Pause Media key
	                                   // VK_LAUNCH_MAIL 0xB4 Start Mail key
	                                   // VK_LAUNCH_MEDIA_SELECT 0xB5 Select Media key
	                                   // VK_LAUNCH_APP1 0xB6 Start Application 1 key
	                                   // VK_LAUNCH_APP2 0xB7 Start Application 2 key
	                                   // --- 0xB8-B9 Reserved
	                                   // VK_OEM_1 0xBA Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ';:' key
	case XK_equal:        return 0xbb; // VK_OEM_PLUS 0xBB For any country/region, the '+' key
	case XK_comma:        return 0xbc; // VK_OEM_COMMA 0xBC For any country/region, the ',' key
	case XK_minus:        return 0xbd; // VK_OEM_MINUS 0xBD For any country/region, the '-' key
	case XK_period:       return 0xbe; // VK_OEM_PERIOD 0xBE For any country/region, the '.' key
	case XK_slash:        return 0xbf; // VK_OEM_2 0xBF Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the '/?' key
	case XK_grave:        return 0xc0; // VK_OEM_3 0xC0 Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the '`~' key
	                                   // --- 0xC1-D7 Reserved
	                                   // --- 0xD8-DA Unassigned
	case XK_bracketleft:  return 0xdb; // VK_OEM_4 0xDB Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the '[{' key
	case XK_backslash:    return 0xdc; // VK_OEM_5 0xDC Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the '\|' key
	case XK_bracketright: return 0xdd; // VK_OEM_6 0xDD Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ']}' key
	case XK_apostrophe:   return 0xde; // VK_OEM_7 0xDE Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the 'single-quote/double-quote' key
	                                   // VK_OEM_8 0xDF Used for miscellaneous characters; it can vary by keyboard.
	                                   // --- 0xE0 Reserved
	                                   // --- 0xE1 OEM specific
	                                   // VK_OEM_102 0xE2 Either the angle bracket key or the backslash key on the RT 102-key keyboard
	                                   // --- 0xE3-E4 OEM specific
	                                   // VK_PROCESSKEY 0xE5 IME PROCESS key
	                                   // --- 0xE6 OEM specific
	                                   // VK_PACKET 0xE7 Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP
	                                   // --- 0xE8 Unassigned
	                                   // --- 0xE9-F5 OEM specific
	                                   // VK_ATTN 0xF6 Attn key
	                                   // VK_CRSEL 0xF7 CrSel key
	                                   // VK_EXSEL 0xF8 ExSel key
	                                   // VK_EREOF 0xF9 Erase EOF key
	                                   // VK_PLAY 0xFA Play key
	                                   // VK_ZOOM 0xFB Zoom key
	                                   // VK_NONAME 0xFC Reserved
	                                   // VK_PA1 0xFD PA1 key
	                                   // VK_OEM_CLEAR 0xFE Clear key
	}
	return 0;
}

void pshtv_handle_events() {
	while (XPending(pshtv_display)) {
		XEvent ev;
		XNextEvent(pshtv_display, &ev);
		switch (ev.type) {
		case MotionNotify:
			mouse_x = ev.xmotion.x;
			mouse_y = ev.xmotion.y;
			break;
		case ButtonPress:
			mousedown(ev.xbutton.button);
			break;
		case ButtonRelease:
			mouseup(ev.xbutton.button);
			break;
		case KeyPress:
			keydown(pshtv_translate_key(XLookupKeysym(&ev.xkey, 0)));
			break;
		case KeyRelease:
			keyup(pshtv_translate_key(XLookupKeysym(&ev.xkey, 0)));
			break;
		case KeymapNotify:
			XRefreshKeyboardMapping(&ev.xmapping);
			break;
		case ConfigureNotify:
			window_w = ev.xconfigure.width;
			window_h = ev.xconfigure.height;
			break;
		case ClientMessage:
			if ((Atom)ev.xclient.data.l[0] == pshtv_atom_wm_delete_window)
				exit(0);
			break;
		case DestroyNotify:
			exit(0);
			break;
		}
	}
}

void pshtv_swap_buffers() {
	glXSwapBuffers(pshtv_display, pshtv_window);
}

void *pshtv_load_gl(const char *name) {
	static void *gl_handle;
	if (!gl_handle)
		gl_handle = dlopen("libGL.so", RTLD_LAZY);
	return dlsym(gl_handle, name);
}

#endif

#include <GL/gl.h>
#include <GL/glext.h>

typedef void (*glAttachShader_t) (GLuint program, GLuint shader);
typedef void (*glBindBuffer_t) (GLenum target, GLuint buffer);
typedef void (*glBindVertexArray_t) (GLuint array);
typedef void (*glBlendFunc_t) (GLenum sfactor, GLenum dfactor);
typedef void (*glBufferData_t) (GLenum target, GLsizeiptr size, const void * data, GLenum usage);
typedef void (*glClear_t) (GLbitfield mask);
typedef void (*glClearColor_t) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (*glClearDepth_t) (GLdouble depth);
typedef void (*glCompileShader_t) (GLuint shader);
typedef GLuint (*glCreateProgram_t) (void);
typedef GLuint (*glCreateShader_t) (GLenum shaderType);
typedef void (*glDeleteBuffers_t) (GLsizei n, const GLuint * buffers);
typedef void (*glDeleteShader_t) (GLuint shader);
typedef void (*glDeleteVertexArrays_t) (GLsizei n, const GLuint *arrays);
typedef void (*glDepthFunc_t) (GLenum func);
typedef void (*glDrawArrays_t) (GLenum mode, GLint first, GLsizei count);
typedef void (*glEnable_t) (GLenum cap);
typedef void (*glEnableVertexAttribArray_t) (GLuint index);
typedef void (*glFlush_t) (void);
typedef void (*glGenBuffers_t) (GLsizei n, GLuint * buffers);
typedef void (*glGenVertexArrays_t) (GLsizei n, GLuint *arrays);
typedef GLint (*glGetAttribLocation_t) (GLuint program, const GLchar *name);
typedef void (*glGetProgramInfoLog_t) (GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void (*glGetProgramiv_t) (GLuint program, GLenum pname, GLint *params);
typedef void (*glGetShaderInfoLog_t) (GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void (*glGetShaderiv_t) (GLuint shader, GLenum pname, GLint *params);
typedef GLint (*glGetUniformLocation_t) (GLuint program, const GLchar *name);
typedef void (*glLinkProgram_t) (GLuint program);
typedef void (*glShaderSource_t) (GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void (*glUniformMatrix4fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (*glUseProgram_t) (GLuint program);
typedef void (*glVertexAttribPointer_t) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
typedef void (*glViewport_t) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (*glGenTextures_t) (GLsizei n, GLuint *textures);
typedef void (*glBindTexture_t) (GLenum target, GLuint texture);
typedef void (*glTexImage2D_t) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * data);
typedef void (*glDeleteTextures_t) (GLsizei n, const GLuint * textures);
typedef void (*glTexParameteri_t) (GLenum target, GLenum pname, GLint param);

#define PSHTV_DECLARE_GL(X) X ## _t p ## X
PSHTV_DECLARE_GL(glAttachShader);
PSHTV_DECLARE_GL(glBindBuffer);
PSHTV_DECLARE_GL(glBindVertexArray);
PSHTV_DECLARE_GL(glBlendFunc);
PSHTV_DECLARE_GL(glBufferData);
PSHTV_DECLARE_GL(glClear);
PSHTV_DECLARE_GL(glClearColor);
PSHTV_DECLARE_GL(glClearDepth);
PSHTV_DECLARE_GL(glCompileShader);
PSHTV_DECLARE_GL(glCreateProgram);
PSHTV_DECLARE_GL(glCreateShader);
PSHTV_DECLARE_GL(glDeleteBuffers);
PSHTV_DECLARE_GL(glDeleteShader);
PSHTV_DECLARE_GL(glDeleteVertexArrays);
PSHTV_DECLARE_GL(glDepthFunc);
PSHTV_DECLARE_GL(glDrawArrays);
PSHTV_DECLARE_GL(glEnable);
PSHTV_DECLARE_GL(glEnableVertexAttribArray);
PSHTV_DECLARE_GL(glFlush);
PSHTV_DECLARE_GL(glGenBuffers);
PSHTV_DECLARE_GL(glGenVertexArrays);
PSHTV_DECLARE_GL(glGetAttribLocation);
PSHTV_DECLARE_GL(glGetProgramInfoLog);
PSHTV_DECLARE_GL(glGetProgramiv);
PSHTV_DECLARE_GL(glGetShaderInfoLog);
PSHTV_DECLARE_GL(glGetShaderiv);
PSHTV_DECLARE_GL(glGetUniformLocation);
PSHTV_DECLARE_GL(glLinkProgram);
PSHTV_DECLARE_GL(glShaderSource);
PSHTV_DECLARE_GL(glUniformMatrix4fv);
PSHTV_DECLARE_GL(glUseProgram);
PSHTV_DECLARE_GL(glVertexAttribPointer);
PSHTV_DECLARE_GL(glViewport);
PSHTV_DECLARE_GL(glGenTextures);
PSHTV_DECLARE_GL(glBindTexture);
PSHTV_DECLARE_GL(glTexImage2D);
PSHTV_DECLARE_GL(glDeleteTextures);
PSHTV_DECLARE_GL(glTexParameteri);

#define PSHTV_LOAD_GL(X) p ## X = (X ## _t)pshtv_load_gl(#X)
void pshtv_load_gls() {
	PSHTV_LOAD_GL(glAttachShader);
	PSHTV_LOAD_GL(glBindBuffer);
	PSHTV_LOAD_GL(glBindVertexArray);
	PSHTV_LOAD_GL(glBlendFunc);
	PSHTV_LOAD_GL(glBufferData);
	PSHTV_LOAD_GL(glClear);
	PSHTV_LOAD_GL(glClearColor);
	PSHTV_LOAD_GL(glClearDepth);
	PSHTV_LOAD_GL(glCompileShader);
	PSHTV_LOAD_GL(glCreateProgram);
	PSHTV_LOAD_GL(glCreateShader);
	PSHTV_LOAD_GL(glDeleteBuffers);
	PSHTV_LOAD_GL(glDeleteShader);
	PSHTV_LOAD_GL(glDeleteVertexArrays);
	PSHTV_LOAD_GL(glDepthFunc);
	PSHTV_LOAD_GL(glDrawArrays);
	PSHTV_LOAD_GL(glEnable);
	PSHTV_LOAD_GL(glEnableVertexAttribArray);
	PSHTV_LOAD_GL(glFlush);
	PSHTV_LOAD_GL(glGenBuffers);
	PSHTV_LOAD_GL(glGenVertexArrays);
	PSHTV_LOAD_GL(glGetAttribLocation);
	PSHTV_LOAD_GL(glGetProgramInfoLog);
	PSHTV_LOAD_GL(glGetProgramiv);
	PSHTV_LOAD_GL(glGetShaderInfoLog);
	PSHTV_LOAD_GL(glGetShaderiv);
	PSHTV_LOAD_GL(glGetUniformLocation);
	PSHTV_LOAD_GL(glLinkProgram);
	PSHTV_LOAD_GL(glShaderSource);
	PSHTV_LOAD_GL(glUniformMatrix4fv);
	PSHTV_LOAD_GL(glUseProgram);
	PSHTV_LOAD_GL(glVertexAttribPointer);
	PSHTV_LOAD_GL(glViewport);
	PSHTV_LOAD_GL(glGenTextures);
	PSHTV_LOAD_GL(glBindTexture);
	PSHTV_LOAD_GL(glTexImage2D);
	PSHTV_LOAD_GL(glDeleteTextures);
	PSHTV_LOAD_GL(glTexParameteri);
}

GLuint pshtv_compile_shader(const char *src, GLenum type) {
	GLuint shader = pglCreateShader(type);

	pglShaderSource(shader, 1, &src, NULL);
	pglCompileShader(shader);

	int success;
	pglGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info[512];
		pglGetShaderInfoLog(shader, 512, NULL, info);
		eprintf("%s\n%s\n", src, info);
	}

	return shader;
}

GLuint pshtv_make_shader_prog(const char *vertex_src, const char *fragment_src) {
	GLuint prog = pglCreateProgram();

	GLuint vertex_shader = pshtv_compile_shader(vertex_src, GL_VERTEX_SHADER);
	GLuint fragment_shader = pshtv_compile_shader(fragment_src, GL_FRAGMENT_SHADER);

	pglAttachShader(prog, vertex_shader);
	pglAttachShader(prog, fragment_shader);
	pglLinkProgram(prog);

	GLint success;
	pglGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success) {
		char info[512];
		pglGetProgramInfoLog(prog, 512, NULL, info);
		eprintf("%s\n", info);
	}

	pglDeleteShader(vertex_shader);
	pglDeleteShader(fragment_shader);

	return prog;
}

float pshtv_z;

#define PSHTV_PASS_FIELD_AS_ATTRIBUTE(SHADER, ATTRIBUTE, SIZE, TYPE, NORMALIZED, STRUCT, FIELD) { \
	pglVertexAttribPointer(ATTRIBUTE, SIZE, TYPE, NORMALIZED, sizeof(STRUCT), (void*)offsetof(STRUCT, FIELD)); \
	pglEnableVertexAttribArray(ATTRIBUTE); \
}

struct Pshtv_Quad_Vert {
	float pos[2];
	float col[4];
	float z;
};

#define PSHTV_QUAD_VERTS_CAP (4096 * 4)
size_t pshtv_quad_verts_len;
struct Pshtv_Quad_Vert pshtv_quad_verts[PSHTV_QUAD_VERTS_CAP];

void pshtv_flush_quads() {
	if (!pshtv_quad_verts_len) return;

	static GLuint shader_prog;
	static GLint in_pos, in_col, in_z, u_transform;
	if (!shader_prog) {
		shader_prog = pshtv_make_shader_prog(
			"#version 130\n"

			"in vec2 in_pos;\n"
			"in vec4 in_col;\n"
			"in float in_z;\n"

			"out vec4 ex_col;\n"

			"uniform mat4 u_transform;\n"

			"void main() {\n"
			"    gl_Position = u_transform * vec4(in_pos, in_z / 1000000.0, 1.0);\n"
			"    ex_col = in_col;\n"
			"}\n",


			"#version 130\n"

			"in vec4 ex_col;\n"

			"void main() {\n"
			"    gl_FragColor = ex_col;\n"
			"}\n"
		);

		in_pos = pglGetAttribLocation(shader_prog, "in_pos");
		in_col = pglGetAttribLocation(shader_prog, "in_col");
		in_z   = pglGetAttribLocation(shader_prog, "in_z");
		u_transform = pglGetUniformLocation(shader_prog, "u_transform");
	}

	pglUseProgram(shader_prog);

	GLuint vao, vbo;

	pglGenVertexArrays(1, &vao);
	pglBindVertexArray(vao);

	pglGenBuffers(1, &vbo);
	pglBindBuffer(GL_ARRAY_BUFFER, vbo);
	pglBufferData(GL_ARRAY_BUFFER, sizeof(struct Pshtv_Quad_Vert) * pshtv_quad_verts_len, pshtv_quad_verts, GL_STREAM_DRAW);

	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_pos, 2, GL_FLOAT, GL_FALSE, struct Pshtv_Quad_Vert, pos);
	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_col, 4, GL_FLOAT, GL_FALSE, struct Pshtv_Quad_Vert, col);
	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_z,   1, GL_FLOAT, GL_FALSE, struct Pshtv_Quad_Vert, z);

	pglUniformMatrix4fv(u_transform, 1, GL_TRUE, (const float*)pshtv_transform_matrix);
	pglEnableVertexAttribArray(u_transform);

	pglDrawArrays(GL_QUADS, 0, pshtv_quad_verts_len);

	pglDeleteBuffers(1, &vbo);
	pglDeleteVertexArrays(1, &vao);

	pshtv_quad_verts_len = 0;
}

struct Pshtv_Ellipse_Vert {
	float pos[2];
	float col[4];
	float corner[2];
	float z;
};

#define PSHTV_ELLIPSE_VERTS_CAP (4096 * 4)
size_t pshtv_ellipse_verts_len;
struct Pshtv_Ellipse_Vert pshtv_ellipse_verts[PSHTV_ELLIPSE_VERTS_CAP];

void pshtv_flush_ellipses() {
	if (!pshtv_ellipse_verts_len) return;

	static GLuint shader_prog;
	static GLint in_pos, in_col, in_corner, in_z, u_transform;
	if (!shader_prog) {
		shader_prog = pshtv_make_shader_prog(
			"#version 130\n"

			"in vec2 in_pos;\n"
			"in vec2 in_corner;\n"
			"in vec4 in_col;\n"
			"in float in_z;\n"

			"out vec2 ex_corner;\n"
			"out vec4 ex_col;\n"

			"uniform mat4 u_transform;\n"

			"void main() {\n"
			"    gl_Position = u_transform * vec4(in_pos, in_z / 1000000.f, 1.0);\n"
			"    ex_corner = in_corner;\n"
			"    ex_col = in_col;\n"
			"}\n",


			"#version 130\n"

			"in vec2 ex_corner;\n"
			"in vec4 ex_col;\n"

			"void main() {\n"
			"    if (dot(ex_corner, ex_corner) > 1.0)\n"
			"        discard;\n"
			"    gl_FragColor = ex_col;\n"
			"}\n"
		);

		in_pos    = pglGetAttribLocation(shader_prog, "in_pos");
		in_col    = pglGetAttribLocation(shader_prog, "in_col");
		in_corner = pglGetAttribLocation(shader_prog, "in_corner");
		in_z      = pglGetAttribLocation(shader_prog, "in_z");
		u_transform = pglGetUniformLocation(shader_prog, "u_transform");
	}

	pglUseProgram(shader_prog);

	GLuint vao, vbo;

	pglGenVertexArrays(1, &vao);
	pglBindVertexArray(vao);

	pglGenBuffers(1, &vbo);
	pglBindBuffer(GL_ARRAY_BUFFER, vbo);
	pglBufferData(GL_ARRAY_BUFFER, sizeof(struct Pshtv_Ellipse_Vert) * pshtv_ellipse_verts_len, pshtv_ellipse_verts, GL_STREAM_DRAW);

	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_pos,    2, GL_FLOAT, GL_FALSE, struct Pshtv_Ellipse_Vert, pos);
	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_col,    4, GL_FLOAT, GL_FALSE, struct Pshtv_Ellipse_Vert, col);
	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_corner, 2, GL_FLOAT, GL_FALSE, struct Pshtv_Ellipse_Vert, corner);
	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_z,      1, GL_FLOAT, GL_FALSE, struct Pshtv_Ellipse_Vert, z);

	pglUniformMatrix4fv(u_transform, 1, GL_TRUE, (const float*)pshtv_transform_matrix);
	pglEnableVertexAttribArray(u_transform);

	pglDrawArrays(GL_QUADS, 0, pshtv_ellipse_verts_len);

	pglDeleteBuffers(1, &vbo);
	pglDeleteVertexArrays(1, &vao);

	pshtv_ellipse_verts_len = 0;
}

struct Pshtv_Triangle_Vert {
	float pos[2];
	float col[4];
	float z;
};

#define PSHTV_TRIANGLE_VERTS_CAP (4096 * 3)
size_t pshtv_triangle_verts_len;
struct Pshtv_Triangle_Vert pshtv_triangle_verts[PSHTV_TRIANGLE_VERTS_CAP];

void pshtv_flush_triangles() {
	if (!pshtv_triangle_verts_len) return;

	static GLuint shader_prog;
	static GLint in_pos, in_col, in_z, u_transform;
	if (!shader_prog) {
		shader_prog = pshtv_make_shader_prog(
			"#version 130\n"

			"in vec2 in_pos;\n"
			"in vec4 in_col;\n"
			"in float in_z;\n"

			"out vec4 ex_col;\n"

			"uniform mat4 u_transform;\n"

			"void main() {\n"
			"    gl_Position = u_transform * vec4(in_pos, in_z / 1000000.0, 1.0);\n"
			"    ex_col = in_col;\n"
			"}\n",


			"#version 130\n"

			"in vec4 ex_col;\n"

			"void main() {\n"
			"    gl_FragColor = ex_col;\n"
			"}\n"
		);

		in_pos    = pglGetAttribLocation(shader_prog, "in_pos");
		in_col    = pglGetAttribLocation(shader_prog, "in_col");
		in_z      = pglGetAttribLocation(shader_prog, "in_z");
		u_transform = pglGetUniformLocation(shader_prog, "u_transform");
	}

	pglUseProgram(shader_prog);

	GLuint vao, vbo;

	pglGenVertexArrays(1, &vao);
	pglBindVertexArray(vao);

	pglGenBuffers(1, &vbo);
	pglBindBuffer(GL_ARRAY_BUFFER, vbo);
	pglBufferData(GL_ARRAY_BUFFER, sizeof(struct Pshtv_Triangle_Vert) * pshtv_triangle_verts_len, pshtv_triangle_verts, GL_STREAM_DRAW);

	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_pos,    2, GL_FLOAT, GL_FALSE, struct Pshtv_Triangle_Vert, pos);
	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_col,    4, GL_FLOAT, GL_FALSE, struct Pshtv_Triangle_Vert, col);
	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_z,      1, GL_FLOAT, GL_FALSE, struct Pshtv_Triangle_Vert, z);

	pglUniformMatrix4fv(u_transform, 1, GL_TRUE, (const float*)pshtv_transform_matrix);
	pglEnableVertexAttribArray(u_transform);

	pglDrawArrays(GL_TRIANGLES, 0, pshtv_triangle_verts_len);

	pglDeleteBuffers(1, &vbo);
	pglDeleteVertexArrays(1, &vao);

	pshtv_triangle_verts_len = 0;
}

void pshtv_flush_all() {
	pshtv_flush_quads();
	pshtv_flush_ellipses();
	pshtv_flush_triangles();
}

void draw_image_buffer(uint8_t *buffer, uint32_t img_w, uint32_t img_h, float x, float y, float w, float h) {
	static GLuint shader_prog;
	static GLint in_pos, in_tex_coord, in_z, u_transform;
	if (!shader_prog) {
		shader_prog = pshtv_make_shader_prog(
			"#version 130\n"

			"in vec2 in_pos;\n"
			"in vec2 in_tex_coord;\n"
			"in float in_z;\n"

			"out vec2 ex_tex_coord;\n"

			"uniform mat4 u_transform;\n"

			"void main() {\n"
			"    gl_Position = u_transform * vec4(in_pos, in_z / 1000000.f, 1.0);\n"
			"    ex_tex_coord = in_tex_coord;\n"
			"}\n",


			"#version 130\n"

			"in vec2 ex_tex_coord;\n"
			"uniform sampler2D sampler\n;"

			"void main() {\n"
			"    gl_FragColor = vec4(texture(sampler, ex_tex_coord).rgb, 1.0);\n"
			// "    gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);\n"
			"}\n"
		);

		in_pos       = pglGetAttribLocation(shader_prog, "in_pos");
		in_tex_coord = pglGetAttribLocation(shader_prog, "in_tex_coord");
		in_z         = pglGetAttribLocation(shader_prog, "in_z");
		u_transform = pglGetUniformLocation(shader_prog, "u_transform");
	}

	pglUseProgram(shader_prog);

	GLuint vao, vbo, texture;

	pglGenVertexArrays(1, &vao);
	pglBindVertexArray(vao);

	pglGenTextures(1, &texture);

	pglBindTexture(GL_TEXTURE_2D, texture);
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	pglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	struct Image_Quad_Vert {
		float pos[2];
		float tex_coord[2];
		float z;
	};
	const struct Image_Quad_Vert img_quad[] = {
		{ .pos = { x,     y     }, .tex_coord = { 0, 0 }, .z = pshtv_z },
		{ .pos = { x + w, y     }, .tex_coord = { 1, 0 }, .z = pshtv_z },
		{ .pos = { x + w, y + h }, .tex_coord = { 1, 1 }, .z = pshtv_z },
		{ .pos = { x,     y + h }, .tex_coord = { 0, 1 }, .z = pshtv_z },
	};
	++pshtv_z;

	pglGenBuffers(1, &vbo);
	pglBindBuffer(GL_ARRAY_BUFFER, vbo);
	pglBufferData(GL_ARRAY_BUFFER, sizeof(img_quad), img_quad, GL_STREAM_DRAW);

	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_pos,       2, GL_FLOAT, GL_FALSE, struct Image_Quad_Vert, pos);
	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_tex_coord, 2, GL_FLOAT, GL_FALSE, struct Image_Quad_Vert, tex_coord);
	PSHTV_PASS_FIELD_AS_ATTRIBUTE(shader_prog, in_z,         1, GL_FLOAT, GL_FALSE, struct Image_Quad_Vert, z);

	pglUniformMatrix4fv(u_transform, 1, GL_TRUE, (const float*)pshtv_transform_matrix);
	pglEnableVertexAttribArray(u_transform);

	pglDrawArrays(GL_QUADS, 0, 4);

	pglDeleteTextures(1, &texture);
	pglDeleteBuffers(1, &vbo);
	pglDeleteVertexArrays(1, &vao);
}

void fill_color(uint32_t c) {
	pshtv_fill_color[0] =     (c >> 16 & 0xff) / 255.f;
	pshtv_fill_color[1] =     (c >>  8 & 0xff) / 255.f;
	pshtv_fill_color[2] =     (c       & 0xff) / 255.f;
	pshtv_fill_color[3] = 1 - (c >> 24 & 0xff) / 255.f;
}

void fill_rect(float x, float y, float w, float h) {
	pshtv_quad_verts[pshtv_quad_verts_len++] = (struct Pshtv_Quad_Vert){ .pos = { x,     y     }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	pshtv_quad_verts[pshtv_quad_verts_len++] = (struct Pshtv_Quad_Vert){ .pos = { x + w, y     }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	pshtv_quad_verts[pshtv_quad_verts_len++] = (struct Pshtv_Quad_Vert){ .pos = { x + w, y + h }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	pshtv_quad_verts[pshtv_quad_verts_len++] = (struct Pshtv_Quad_Vert){ .pos = { x,     y + h }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	++pshtv_z;

	if (pshtv_quad_verts_len == PSHTV_QUAD_VERTS_CAP) pshtv_flush_quads();
}

// TODO Make this into a shader
void fill_line(float x1, float y1, float x2, float y2, float w) {
	const float x0 = x2 - x1;
	const float y0 = y2 - y1;
	const float len = sqrt(x0 * x0 + y0 * y0);
	const float x3 = w * .5 * -y0 / len;
	const float y3 = w * .5 *  x0 / len;

	pshtv_quad_verts[pshtv_quad_verts_len++] = (struct Pshtv_Quad_Vert){ .pos = { x1 - x3, y1 - y3 }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	pshtv_quad_verts[pshtv_quad_verts_len++] = (struct Pshtv_Quad_Vert){ .pos = { x1 + x3, y1 + y3 }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	pshtv_quad_verts[pshtv_quad_verts_len++] = (struct Pshtv_Quad_Vert){ .pos = { x2 + x3, y2 + y3 }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	pshtv_quad_verts[pshtv_quad_verts_len++] = (struct Pshtv_Quad_Vert){ .pos = { x2 - x3, y2 - y3 }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	++pshtv_z;

	if (pshtv_quad_verts_len == PSHTV_QUAD_VERTS_CAP) pshtv_flush_quads();
}

void fill_ellipse(float x, float y, float rx, float ry) {

	pshtv_ellipse_verts[pshtv_ellipse_verts_len++] = (struct Pshtv_Ellipse_Vert){ .pos = { x - rx, y - ry, }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .corner = { -1, -1 }, .z = pshtv_z };
	pshtv_ellipse_verts[pshtv_ellipse_verts_len++] = (struct Pshtv_Ellipse_Vert){ .pos = { x - rx, y + ry, }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .corner = { -1, +1 }, .z = pshtv_z };
	pshtv_ellipse_verts[pshtv_ellipse_verts_len++] = (struct Pshtv_Ellipse_Vert){ .pos = { x + rx, y + ry, }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .corner = { +1, +1 }, .z = pshtv_z };
	pshtv_ellipse_verts[pshtv_ellipse_verts_len++] = (struct Pshtv_Ellipse_Vert){ .pos = { x + rx, y - ry, }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .corner = { +1, -1 }, .z = pshtv_z };
	++pshtv_z;

	if (pshtv_ellipse_verts_len == PSHTV_ELLIPSE_VERTS_CAP) pshtv_flush_ellipses();
}

void fill_triangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	pshtv_triangle_verts[pshtv_triangle_verts_len++] = (struct Pshtv_Triangle_Vert){ .pos = { x1, y1 }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	pshtv_triangle_verts[pshtv_triangle_verts_len++] = (struct Pshtv_Triangle_Vert){ .pos = { x2, y2 }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	pshtv_triangle_verts[pshtv_triangle_verts_len++] = (struct Pshtv_Triangle_Vert){ .pos = { x3, y3 }, .col = { pshtv_fill_color[0], pshtv_fill_color[1], pshtv_fill_color[2], pshtv_fill_color[3] }, .z = pshtv_z };
	++pshtv_z;

	if (pshtv_triangle_verts_len == PSHTV_TRIANGLE_VERTS_CAP) pshtv_flush_triangles();
}


void pshtv_mul_transform_matrix_by(float by[4][4]) {
	pshtv_flush_all();

	float res[4][4] = {};
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int g = 0; g < 4; ++g)
				res[i][j] += pshtv_transform_matrix[i][g] * by[g][j];
	memcpy(pshtv_transform_matrix, res, sizeof(float) * 4 * 4);
}

void translate(float x, float y) {
	float translate_matrix[4][4] = {
		{ 1, 0, 0, x },
		{ 0, 1, 0, y },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 },
	};
	pshtv_mul_transform_matrix_by(translate_matrix);
}

void scale(float x, float y) {
	float scale_matrix[4][4] = {
		{ x, 0, 0, 0 },
		{ 0, y, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 },
	};
	pshtv_mul_transform_matrix_by(scale_matrix);
}

void rotate(float a) {
	float s = sin(a);
	float c = cos(a);
	float rotate_matrix[4][4] = {
		{ c, -s, 0, 0 },
		{ s,  c, 0, 0 },
		{ 0,  0, 1, 0 },
		{ 0,  0, 0, 1 },
	};
	pshtv_mul_transform_matrix_by(rotate_matrix);
}

void pshtv_redraw() {
	pglViewport(0, 0, window_w, window_h);

	pglClearColor(1.0, 1.0, 1.0, 1.0);
	pglClearDepth(0.0);
	pglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			pshtv_transform_matrix[i][j] = i == j ? 1 : 0;

	translate(-1, 1);
	scale(2 / window_w, -2 / window_h);

	pshtv_fill_color[0] = 0; pshtv_fill_color[1] = 0; pshtv_fill_color[2] = 1; pshtv_fill_color[3] = 1;

	pshtv_z = 0;

	draw();
	pshtv_flush_all();
	pglFlush();
	pshtv_swap_buffers();
}

void pshtv_init_opengl() {
	pshtv_load_gls();

	pglEnable(GL_BLEND);
	pglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	pglEnable(GL_DEPTH_TEST);
	pglDepthFunc(GL_GEQUAL);
}

// This part of Pishtov defines the main game loop.
int main() {
	pshtv_open_window("Igra", 800, 600);
	pshtv_init_opengl();

	init();
	while (1) {
		pshtv_handle_events();
		update();
		pshtv_redraw();
	}
}

#ifdef __cplusplus
}
#endif

#endif // PISHTOV_H
