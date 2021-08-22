// Copyright (c) 2021 Theodor Totev - teoet6
/*
Pishtov is a C++ header that mimics Yashu's JS Pishtov. It tries not only
to be faster by being written in C++ but to also be more powerful.

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

This Pishtov tries to retain the ease-of-use for new programmers, but also to
allow skilled programmers to create faster and bigger games and simulations.
Another goal of this Pishtov is to expand the material learned from the
original JS Pishtov with materials like files, threading, network programming,
and more.

An extra goal is being able to be compiled in a default Code::Block Windows
install i.e. without changing any linker or compiler settings. This is
important in order to retain the no-boilerplate policy of the JS Pishtov.
Originally all you had to do is edit game.js and open start.html in a browser.
Similarly all you should do with the C++ Pishtov is edit game.cpp, compile and
run. Making the end-user (who is possibly a child) edit linker settings before
compiling his game breaks this workflow.

An extra extra goal is compiling to WASM and Android.

TODO In no particular order
    [X] Rectangles
    [X] Circles
    [ ] Arcs
    [ ] Lines
    [ ] Images
    [ ] Text
    [ ] Translation and rotation of the canvas
    [ ] OS-independent network-programming
    [ ] 3D graphics
    [ ] Shaders
    [ ] An optional define stating whether or not you want a main game loop or just the library functions
    [ ] Default Code::Blocks Windows compile
    [ ] WASM compile
    [ ] Android compile
*/

#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>

void keydown(int key);
void keyup(int key);
void mousedown(int button);
void mouseup(int button);
void update();
void draw();
float mouse_x, mouse_y;
float window_w, window_h;

namespace pshtv {

    // This part of Pishtov deals with OS-specific stuff. This includes opening
    // a window, making an OpenGL context for said window, processing keyboard
    // and mouse input, swapping the buffers, providing functions that aren't
    // in the C++ standard library and more. You need to have the following
    // functions defined for each OS:
    void open_window(const char *name, int w, int h); // Opens a window.
    void handle_events(); // Asks the OS for new events and reacts accordingly.
    void swap_buffers(); // Swaps the OpenGL buffers. Can be a noop.

#if defined(_WIN32)
#error Windows not supported
#elif defined(__APPLE__)
#error Apple not supported
#elif defined(__ANDROID__)
#error Andoird not supported
#else // Assume X11
    // https://github.com/gamedevtech/X11OpenGLWindow

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <dlfcn.h>

    Display *display;
    Window window;
    int screen_id;
    GLXContext context;
    Atom atom_wm_delete_window;

    void open_window(const char *name, int w, int h) {

        display = XOpenDisplay(NULL);
        if (display == NULL) {
            std::cerr << "Error opening X11 display" << std::endl;
            exit(-1);
        }

        screen_id = DefaultScreen(display);

        GLint majorGLX = 0, minorGLX = 0;
        glXQueryVersion(display, &majorGLX, &minorGLX);
        if (majorGLX < 1 && minorGLX < 2) {
            std::cerr << "glx version is " << majorGLX << "." << minorGLX << std::endl;
            std::cerr << "Minimum supported glx version is 1.2" << std::endl;
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
        XVisualInfo* visual_info = glXChooseVisual(display, screen_id, glx_attribs);
        if (visual_info == NULL) {
            std::cerr << "Could not create correct visual window\n" << std::endl;
            exit(-1);
        }

        XSetWindowAttributes attributes;
        attributes.border_pixel = BlackPixel(display, screen_id);
        attributes.background_pixel = WhitePixel(display, screen_id);
        attributes.override_redirect = True;
        attributes.colormap = XCreateColormap(display, RootWindow(display, screen_id), visual_info->visual, AllocNone);
        attributes.event_mask = ExposureMask;

        window = XCreateWindow(display, RootWindow(display, screen_id), 0, 0, w, h, 0, visual_info->depth, InputOutput, visual_info->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &attributes);

        atom_wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display, window, &atom_wm_delete_window, 1);

        context = glXCreateContext(display, visual_info, NULL, GL_TRUE);
        glXMakeCurrent(display, window, context);

        XClearWindow(display, window);

        XSelectInput(display, window,
                     PointerMotionMask |
                     ButtonPressMask |
                     ButtonReleaseMask |
                     KeyPressMask |
                     KeyReleaseMask |
                     KeymapStateMask |
                     StructureNotifyMask);

        XStoreName(display, window, name);

        XMapWindow(display, window);
    }

    // TODO use KF86 keys as well
    int translate_key(int native) {
        switch (native) {
                                             // VK_LBUTTON 0x01 Left mouse button
                                             // VK_RBUTTON 0x02 Right mouse button
                                             // VK_CANCEL 0x03 Control-break processing
                                             // VK_MBUTTON 0x04 Middle mouse button (three-button mouse)
                                             // VK_XBUTTON1 0x05 X1 mouse button
                                             // VK_XBUTTON2 0x06 X2 mouse button
                                             // --- 0x07 Undefined
        case XK_BackSpace:      return 0x08; // VK_BACK 0x08 BACKSPACE key
        case XK_Tab:            return 0x09; // VK_TAB 0x09 TAB key
                                             // --- 0x0A-0B Reserved
        case XK_Clear:          return 0x0c; // VK_CLEAR 0x0C CLEAR key
        case XK_KP_Enter:
        case XK_Return:         return 0x0d; // VK_RETURN 0x0D ENTER key
                                             // --- 0x0E-0F Undefined
                                             // VK_SHIFT 0x10 SHIFT key
                                             // VK_CONTROL 0x11 CTRL key
        case XK_Alt_L:
        case XK_Alt_R:          return 0x12; // VK_MENU 0x12 ALT key
        case XK_Pause:          return 0x13; // VK_PAUSE 0x13 PAUSE key
        case XK_Caps_Lock:      return 0x14; // VK_CAPITAL 0x14 CAPS LOCK key
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
        case XK_Escape:         return 0x1b; // VK_ESCAPE 0x1B ESC key
                                             // VK_CONVERT 0x1C IME convert VK_NONCONVERT 0x1D IME nonconvert
                                             // VK_ACCEPT 0x1E IME accept
                                             // VK_MODECHANGE 0x1F IME mode change request
        case XK_space:          return 0x20; // VK_SPACE 0x20 SPACEBAR
        case XK_Page_Up:        return 0x21; // VK_PRIOR 0x21 PAGE UP key
        case XK_Page_Down:      return 0x22; // VK_NEXT 0x22 PAGE DOWN key
        case XK_End:            return 0x23; // VK_END 0x23 END key
        case XK_Begin:
        case XK_Home:           return 0x24; // VK_HOME 0x24 HOME key
        case XK_Left:           return 0x25; // VK_LEFT 0x25 LEFT ARROW key
        case XK_Up:             return 0x26; // VK_UP 0x26 UP ARROW key
        case XK_Right:          return 0x27; // VK_RIGHT 0x27 RIGHT ARROW key
        case XK_Down:           return 0x28; // VK_DOWN 0x28 DOWN ARROW key
        case XK_Select:         return 0x29; // VK_SELECT 0x29 SELECT key
                                             // VK_PRINT 0x2A PRINT key
        case XK_Execute:        return 0x2b; // VK_EXECUTE 0x2B EXECUTE key
        case XK_Print:          return 0x2c; // VK_SNAPSHOT 0x2C PRINT SCREEN key
        case XK_Insert:         return 0x2d; // VK_INSERT 0x2D INS key
        case XK_Delete:         return 0x2e; // VK_DELETE 0x2E DEL key
        case XK_Help:           return 0x2f; // VK_HELP 0x2F HELP key
        case XK_0:              return 0x30; // 0x30 0 key
        case XK_1:              return 0x31; // 0x31 1 key
        case XK_2:              return 0x32; // 0x32 2 key
        case XK_3:              return 0x33; // 0x33 3 key
        case XK_4:              return 0x34; // 0x34 4 key
        case XK_5:              return 0x35; // 0x35 5 key
        case XK_6:              return 0x36; // 0x36 6 key
        case XK_7:              return 0x37; // 0x37 7 key
        case XK_8:              return 0x38; // 0x38 8 key
        case XK_9:              return 0x39; // 0x39 9 key
                                             // --- 0x3A-40 Undefined
        case XK_a:
        case XK_A:              return 0x41; // 0x41 A key
        case XK_b:
        case XK_B:              return 0x42; // 0x42 B key
        case XK_c:
        case XK_C:              return 0x43; // 0x43 C key
        case XK_d:
        case XK_D:              return 0x44; // 0x44 D key
        case XK_e:
        case XK_E:              return 0x45; // 0x45 E key
        case XK_f:
        case XK_F:              return 0x46; // 0x46 F key
        case XK_g:
        case XK_G:              return 0x47; // 0x47 G key
        case XK_h:
        case XK_H:              return 0x48; // 0x48 H key
        case XK_i:
        case XK_I:              return 0x49; // 0x49 I key
        case XK_j:
        case XK_J:              return 0x4a; // 0x4A J key
        case XK_k:
        case XK_K:              return 0x4b; // 0x4B K key
        case XK_l:
        case XK_L:              return 0x4c; // 0x4C L key
        case XK_m:
        case XK_M:              return 0x4d; // 0x4D M key
        case XK_n:
        case XK_N:              return 0x4e; // 0x4E N key
        case XK_o:
        case XK_O:              return 0x4f; // 0x4F O key
        case XK_p:
        case XK_P:              return 0x50; // 0x50 P key
        case XK_q:
        case XK_Q:              return 0x51; // 0x51 Q key
        case XK_r:
        case XK_R:              return 0x52; // 0x52 R key
        case XK_s:
        case XK_S:              return 0x53; // 0x53 S key
        case XK_t:
        case XK_T:              return 0x54; // 0x54 T key
        case XK_u:
        case XK_U:              return 0x55; // 0x55 U key
        case XK_v:
        case XK_V:              return 0x56; // 0x56 V key
        case XK_w:
        case XK_W:              return 0x57; // 0x57 W key
        case XK_x:
        case XK_X:              return 0x58; // 0x58 X key
        case XK_y:
        case XK_Y:              return 0x59; // 0x59 Y key
        case XK_z:
        case XK_Z:              return 0x5a; // 0x5A Z key
        case XK_Super_L:        return 0x5b; // VK_LWIN 0x5B Left Windows key (Natural keyboard)
        case XK_Super_R:        return 0x5c; // VK_RWIN 0x5C Right Windows key (Natural keyboard)
                                             // VK_APPS 0x5D Applications key (Natural keyboard)
                                             // --- 0x5E Reserved
                                             // VK_SLEEP 0x5F Computer Sleep key
        case XK_KP_0:
        case XK_KP_Insert:      return 0x60; // VK_NUMPAD0 0x60 Numeric keypad 0 key
        case XK_KP_1:
        case XK_KP_End:         return 0x61; // VK_NUMPAD1 0x61 Numeric keypad 1 key
        case XK_KP_2:
        case XK_KP_Down:        return 0x62; // VK_NUMPAD2 0x62 Numeric keypad 2 key
        case XK_KP_3:
        case XK_KP_Page_Down:   return 0x63; // VK_NUMPAD3 0x63 Numeric keypad 3 key
        case XK_KP_4:
        case XK_KP_Left:        return 0x64; // VK_NUMPAD4 0x64 Numeric keypad 4 key
        case XK_KP_5:
        case XK_KP_Begin:       return 0x65; // VK_NUMPAD5 0x65 Numeric keypad 5 key
        case XK_KP_6:
        case XK_KP_Right:       return 0x66; // VK_NUMPAD6 0x66 Numeric keypad 6 key
        case XK_KP_7:
        case XK_KP_Home:        return 0x67; // VK_NUMPAD7 0x67 Numeric keypad 7 key
        case XK_KP_8:
        case XK_KP_Up:          return 0x68; // VK_NUMPAD8 0x68 Numeric keypad 8 key
        case XK_KP_9:
        case XK_KP_Page_Up:     return 0x69; // VK_NUMPAD9 0x69 Numeric keypad 9 key
        case XK_KP_Multiply:    return 0x6a; // VK_MULTIPLY 0x6A Multiply key
        case XK_KP_Add:         return 0x6b; // VK_ADD 0x6B Add key
        case XK_KP_Separator:   return 0x6c;
        case XK_KP_Subtract:    return 0x6d; // VK_SUBTRACT 0x6D Subtract key
        case XK_KP_Decimal:     
        case XK_KP_Delete:      return 0x6e; // VK_DECIMAL 0x6E Decimal key
        case XK_KP_Divide:      return 0x6f; // VK_DIVIDE 0x6F Divide key
        case XK_F1:             return 0x70; // VK_F1 0x70 F1 key
        case XK_F2:             return 0x71; // VK_F2 0x71 F2 key
        case XK_F3:             return 0x72; // VK_F3 0x72 F3 key
        case XK_F4:             return 0x73; // VK_F4 0x73 F4 key
        case XK_F5:             return 0x74; // VK_F5 0x74 F5 key
        case XK_F6:             return 0x75; // VK_F6 0x75 F6 key
        case XK_F7:             return 0x76; // VK_F7 0x76 F7 key
        case XK_F8:             return 0x77; // VK_F8 0x77 F8 key
        case XK_F9:             return 0x78; // VK_F9 0x78 F9 key
        case XK_F10:            return 0x79; // VK_F10 0x79 F10 key
        case XK_F11:            return 0x7a; // VK_F11 0x7A F11 key
        case XK_F12:            return 0x7b; // VK_F12 0x7B F12 key
        case XK_F13:            return 0x7c; // VK_F13 0x7C F13 key
        case XK_F14:            return 0x7d; // VK_F14 0x7D F14 key
        case XK_F15:            return 0x7e; // VK_F15 0x7E F15 key
        case XK_F16:            return 0x7f; // VK_F16 0x7F F16 key
        case XK_F17:            return 0x80; // VK_F17 0x80 F17 key
        case XK_F18:            return 0x81; // VK_F18 0x81 F18 key
        case XK_F19:            return 0x82; // VK_F19 0x82 F19 key
        case XK_F20:            return 0x83; // VK_F20 0x83 F20 key
        case XK_F21:            return 0x84; // VK_F21 0x84 F21 key
        case XK_F22:            return 0x85; // VK_F22 0x85 F22 key
        case XK_F23:            return 0x86; // VK_F23 0x86 F23 key
        case XK_F24:            return 0x87; // VK_F24 0x87 F24 key
                                             // --- 0x88-8F Unassigned
        case XK_Num_Lock:       return 0x90; // VK_NUMLOCK 0x90 NUM LOCK key
        case XK_Scroll_Lock:    return 0x91; // VK_SCROLL 0x91 SCROLL LOCK key
                                             // --- 0x92-96 OEM specific
                                             // --- 0x97-9F Unassigned
        case XK_Shift_L:        return 0xa0; // VK_LSHIFT 0xA0 Left SHIFT key
        case XK_Shift_R:        return 0xa1; // VK_RSHIFT 0xA1 Right SHIFT key
        case XK_Control_L:      return 0xa2; // VK_LCONTROL 0xA2 Left CONTROL key
        case XK_Control_R:      return 0xa3; // VK_RCONTROL 0xA3 Right CONTROL key
                                             // VK_LMENU 0xA4 Left MENU key
        case XK_Menu:           return 0xa5; // VK_RMENU 0xA5 Right MENU key
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
        case XK_equal:          return 0xbb; // VK_OEM_PLUS 0xBB For any country/region, the '+' key
        case XK_comma:          return 0xbc; // VK_OEM_COMMA 0xBC For any country/region, the ',' key
        case XK_minus:          return 0xbd; // VK_OEM_MINUS 0xBD For any country/region, the '-' key
        case XK_period:         return 0xbe; // VK_OEM_PERIOD 0xBE For any country/region, the '.' key
        case XK_slash:          return 0xbf; // VK_OEM_2 0xBF Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the '/?' key
        case XK_grave:          return 0xc0; // VK_OEM_3 0xC0 Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the '`~' key
                                             // --- 0xC1-D7 Reserved
                                             // --- 0xD8-DA Unassigned
        case XK_bracketleft:    return 0xdb; // VK_OEM_4 0xDB Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the '[{' key
        case XK_backslash:      return 0xdc; // VK_OEM_5 0xDC Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the '\|' key
        case XK_bracketright:   return 0xdd; // VK_OEM_6 0xDD Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ']}' key
        case XK_apostrophe:     return 0xde; // VK_OEM_7 0xDE Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the 'single-quote/double-quote' key
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
        std::cout << native << " " << XKeysymToString(native) << std::endl;
        return 0;
    }

    void handle_events() {
        while (XPending(display)) {
            XEvent ev;
            XNextEvent(display, &ev);
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
                keydown(translate_key(XLookupKeysym(&ev.xkey, 0)));
                break;
            case KeyRelease: 
                keyup(translate_key(XLookupKeysym(&ev.xkey, 0)));
                break;
            case KeymapNotify:
                XRefreshKeyboardMapping(&ev.xmapping);
                break;
            case ConfigureNotify:
                window_w = ev.xconfigure.width;
                window_h = ev.xconfigure.height;
                glViewport(0, 0, window_w, window_h);
                break;
            case ClientMessage:
                if (ev.xclient.data.l[0] == atom_wm_delete_window)
                    exit(0);
                break;
            case DestroyNotify:
                exit(0);
                break;
            }
        }
    }

    void swap_buffers() {
        glXSwapBuffers(display, window);
    }

    void *load_gl_ext(const char *name) {
        static void *gl_handle;
        if (!gl_handle)
            gl_handle = dlopen("libGL.so", RTLD_LAZY);
        return dlsym(gl_handle, name);
    }

#endif

#include <GL/gl.h>
#include <GL/glext.h>

    PFNGLCREATESHADERPROC      glCreateShader;
    PFNGLSHADERSOURCEPROC      glShaderSource;
    PFNGLCOMPILESHADERPROC     glCompileShader;
    PFNGLGETSHADERIVPROC       glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC  glGetShaderInfoLog;
    PFNGLCREATEPROGRAMPROC     glCreateProgram;
    PFNGLATTACHSHADERPROC      glAttachShader;
    PFNGLLINKPROGRAMPROC       glLinkProgram;
    PFNGLGETPROGRAMIVPROC      glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    PFNGLDELETESHADERPROC      glDeleteShader;
    PFNGLUSEPROGRAMPROC        glUseProgram;

#define LOAD_GL_EXT(T, X) X = (T)load_gl_ext(#X)
    void load_gl_exts() {
        LOAD_GL_EXT(PFNGLCREATESHADERPROC,      glCreateShader);
        LOAD_GL_EXT(PFNGLSHADERSOURCEPROC,      glShaderSource);
        LOAD_GL_EXT(PFNGLCOMPILESHADERPROC,     glCompileShader);
        LOAD_GL_EXT(PFNGLGETSHADERIVPROC,       glGetShaderiv);
        LOAD_GL_EXT(PFNGLGETSHADERINFOLOGPROC,  glGetShaderInfoLog);
        LOAD_GL_EXT(PFNGLCREATEPROGRAMPROC,     glCreateProgram);
        LOAD_GL_EXT(PFNGLATTACHSHADERPROC,      glAttachShader);
        LOAD_GL_EXT(PFNGLLINKPROGRAMPROC,       glLinkProgram);
        LOAD_GL_EXT(PFNGLGETPROGRAMIVPROC,      glGetProgramiv);
        LOAD_GL_EXT(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
        LOAD_GL_EXT(PFNGLDELETESHADERPROC,      glDeleteShader);
        LOAD_GL_EXT(PFNGLUSEPROGRAMPROC,        glUseProgram);
    }
#undef LOAD_GL_EXT

    // This part of Pishtov deals with OpenGL stuff. It deals with different
    // drawing functions that the end user might use. It currently provides the
    // following functions:
    void fill_rect(float x, float y, float w, float h); // Draws a rectangle at x, y with dimensions w, h
    void fill_style(float r, float g, float b); // Changes the color. Values for each channel are in the range [0; 255]
    void fill_cricle(float x, float y, float r); // Draws a circle at x, y with radius r
    void begin_draw();
    void end_draw();

    unsigned int prog_solid;
    unsigned int prog_circle;

    unsigned int compile_shader(const char *src, GLenum type) {
        unsigned int shader = glCreateShader(type);

        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info[512];
            glGetShaderInfoLog(shader, 512, NULL, info);
            std::cerr << info << std::endl;
        }

        return shader;
    }

    unsigned int simple_shader_prog(const char *vertex_src, const char *fragment_src) {
        unsigned int prog = glCreateProgram();

        unsigned int vertex_shader = compile_shader(vertex_src, GL_VERTEX_SHADER);
        unsigned int fragment_shader = compile_shader(fragment_src, GL_FRAGMENT_SHADER);

        glAttachShader(prog, vertex_shader);
        glAttachShader(prog, fragment_shader);
        glLinkProgram(prog);

        int success;
        glGetProgramiv(prog, GL_LINK_STATUS, &success);
        if (!success) {
            char info[512];
            glGetProgramInfoLog(prog, 512, NULL, info);
            std::cerr << info << std::endl;
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        return prog;
    }

    void fill_rect(float x, float y, float w, float h) {
        glUseProgram(prog_solid);

        glBegin(GL_QUADS);
        glVertex2f(x + 0, y + 0);
        glVertex2f(x + w, y + 0);
        glVertex2f(x + w, y + h);
        glVertex2f(x + 0, y + h);
        glEnd();
    }

    void fill_circle(float x, float y, float r) {
        glUseProgram(prog_circle);

        glBegin(GL_QUADS);
        glTexCoord2f(-1, -1); glVertex2f(x - r, y - r);
        glTexCoord2f( 1, -1); glVertex2f(x + r, y - r);
        glTexCoord2f( 1,  1); glVertex2f(x + r, y + r);
        glTexCoord2f(-1,  1); glVertex2f(x - r, y + r);
        glEnd();
    }

    void fill_style(float r, float g, float b) {
        glColor3f(r, g, b);
    }

    void begin_draw() {
        glClearColor(1.0, 1.0, 1.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);

        float w = 2 / (float)window_w;
        float h = 2 / (float)window_h;
        float m[] = { // OpenGL wants the matrix transposed
             w,  0,  0,  0,
             0, -h,  0,  0,
             0,  0,  1,  0,
            -1,  1,  0,  1,
        };
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&m[0]);
    }

    void end_draw() {
        swap_buffers();
    }

    void init_opengl() {
        load_gl_exts();

        const char *vertex_src_solid = R"XXX(
            #version 110
            void main() {
                gl_FrontColor = gl_Color;
                gl_BackColor  = gl_Color;
                gl_Position   = gl_ProjectionMatrix * gl_Vertex;
            }
        )XXX";
        const char *fragment_src_solid = R"XXX(
            #version 110
            void main() {
                gl_FragColor = gl_Color;
            }
        )XXX";
        prog_solid = simple_shader_prog(vertex_src_solid, fragment_src_solid);

        // TODO anti-aliasing
        const char *vertex_src_circle = R"XXX(
            #version 110
            void main() {
                gl_FrontColor = gl_Color;
                gl_BackColor  = gl_Color;
                gl_TexCoord[0] = gl_MultiTexCoord0;
                gl_Position   = gl_ProjectionMatrix * gl_Vertex;
            }
        )XXX";
        const char *fragment_src_circle = R"XXX(
            #version 110
            void main() {
                if (dot(gl_TexCoord[0].st, gl_TexCoord[0].st) >= 1.0)
                    discard;
                gl_FragColor = gl_Color;
            }
        )XXX";
        prog_circle = simple_shader_prog(vertex_src_circle, fragment_src_circle);
    }
}

// This part of Pishtov defines the main game loop.

int main() {
    pshtv::open_window("Igra", 800, 600);
    pshtv::init_opengl();
    while (true) {
        pshtv::handle_events();
        update();
        pshtv::begin_draw();
        draw();
        pshtv::end_draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
