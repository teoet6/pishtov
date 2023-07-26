#include <stdbool.h>

#include "../pishtov.h"
#include "arr.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define IMAGE_PATH "/home/teoet6/pics/walls/purple-water-purple-sky.jpg"

int img_w;
int img_h;
uint8_t *img_data;

bool fill_rect_otherwise_fill_ellipse;
int sharpens_per_update = 1;

typedef struct Node Node;
struct Node {
	bool stop;

	int avgr, avgg, avgb, avga;
	int minr, ming, minb, mina;
	int maxr, maxg, maxb, maxa;
};

Node *qt;

void fill_color_rgba(uint32_t c) {
	pshtv_fill_color[0] = (c       & 0xff) / 255.f;
	pshtv_fill_color[1] = (c >>  8 & 0xff) / 255.f;
	pshtv_fill_color[2] = (c >> 16 & 0xff) / 255.f;
	pshtv_fill_color[3] = (c >> 24 & 0xff) / 255.f;
}

bool qt_build(Node **qt, int root, int l, int r, int u, int d) {
	if (l == r || u == d) return false;

	if (arr_len(*qt) <= root) arr_resize(qt, root);

	if (l + 1 == r && u + 1 == d) {
		(*qt)[root].avgr = (*qt)[root].minr = (*qt)[root].maxr = img_data[4 * u * img_w + 4 * l + 0];
		(*qt)[root].avgg = (*qt)[root].ming = (*qt)[root].maxg = img_data[4 * u * img_w + 4 * l + 1];
		(*qt)[root].avgb = (*qt)[root].minb = (*qt)[root].maxb = img_data[4 * u * img_w + 4 * l + 2];
		(*qt)[root].avga = (*qt)[root].mina = (*qt)[root].maxa = img_data[4 * u * img_w + 4 * l + 3];

		return true;
	}

	int lr = (l + r) / 2;
	int ud = (u + d) / 2;

	// +1 +2
	// +3 +4
	bool ok[4];
	ok[0] = qt_build(qt, root * 4 + 1, l,  lr, u,  ud);
	ok[1] = qt_build(qt, root * 4 + 2, lr,  r, u,  ud);
	ok[2] = qt_build(qt, root * 4 + 3, l,  lr, ud,  d);
	ok[3] = qt_build(qt, root * 4 + 4, lr,  r, ud,  d);

	uint32_t sumr = 0, sumg = 0, sumb = 0, suma = 0;

	(*qt)[root].minr = 255;
	(*qt)[root].ming = 255;
	(*qt)[root].minb = 255;
	(*qt)[root].mina = 255;

	(*qt)[root].maxr = 0;
	(*qt)[root].maxg = 0;
	(*qt)[root].maxb = 0;
	(*qt)[root].maxa = 0;

	for (int i = 1; i <= 4; i += 1) {
		if (ok[i - 1]) {
			sumr += (*qt)[root * 4 + i].avgr;
			sumg += (*qt)[root * 4 + i].avgg;
			sumb += (*qt)[root * 4 + i].avgb;
			suma += (*qt)[root * 4 + i].avga;

			if ((*qt)[root * 4 + i].minr < (*qt)[root].minr) (*qt)[root].minr = (*qt)[root * 4 + i].minr;
			if ((*qt)[root * 4 + i].ming < (*qt)[root].ming) (*qt)[root].ming = (*qt)[root * 4 + i].ming;
			if ((*qt)[root * 4 + i].minb < (*qt)[root].minb) (*qt)[root].minb = (*qt)[root * 4 + i].minb;
			if ((*qt)[root * 4 + i].mina < (*qt)[root].mina) (*qt)[root].mina = (*qt)[root * 4 + i].mina;

			if ((*qt)[root * 4 + i].maxr > (*qt)[root].maxr) (*qt)[root].maxr = (*qt)[root * 4 + i].maxr;
			if ((*qt)[root * 4 + i].maxg > (*qt)[root].maxg) (*qt)[root].maxg = (*qt)[root * 4 + i].maxg;
			if ((*qt)[root * 4 + i].maxb > (*qt)[root].maxb) (*qt)[root].maxb = (*qt)[root * 4 + i].maxb;
			if ((*qt)[root * 4 + i].maxa > (*qt)[root].maxa) (*qt)[root].maxa = (*qt)[root * 4 + i].maxa;
		}
	}

	uint32_t num_cols = ok[0] + ok[1] + ok[2] + ok[3];

	(*qt)[root].avgr = sumr / num_cols;
	(*qt)[root].avgg = sumg / num_cols;
	(*qt)[root].avgb = sumb / num_cols;
	(*qt)[root].avga = suma / num_cols;

	return true;
}

void qt_ampest_stop(Node *qt, int root, int l, int r, int u, int d, int *ampest, float *amp) {
	if (root >= arr_len(qt)) return;

	if (!qt[root].stop) {
		int lr = (l + r) / 2;
		int ud = (u + d) / 2;

		qt_ampest_stop(qt, root * 4 + 1, l,  lr, u,  ud, ampest, amp);
		qt_ampest_stop(qt, root * 4 + 2, lr,  r, u,  ud, ampest, amp);
		qt_ampest_stop(qt, root * 4 + 3, l,  lr, ud,  d, ampest, amp);
		qt_ampest_stop(qt, root * 4 + 4, lr,  r, ud,  d, ampest, amp);
		return;
	}

	float dr = qt[root].maxr - qt[root].minr;
	float dg = qt[root].maxg - qt[root].ming;
	float db = qt[root].maxb - qt[root].minb;
	float da = qt[root].maxa - qt[root].mina;

	float cur_amp = sqrt(dr * dr + dg * dg + db * db + da * da);

	if (cur_amp > *amp) {
		*amp = cur_amp;
		*ampest = root;
	}
}

void qt_sharpen() {
	int ampest = -1;
	float amp = 0;
	qt_ampest_stop(qt, 0, 0, img_w, 0, img_h, &ampest, &amp);
	if (ampest == -1) {
		printf("Ampest was not found\n");
		return;
	}

	qt[ampest].stop = false;
	if (ampest * 4 + 1 < arr_len(qt)) qt[ampest * 4 + 1].stop = true;
	if (ampest * 4 + 2 < arr_len(qt)) qt[ampest * 4 + 2].stop = true;
	if (ampest * 4 + 3 < arr_len(qt)) qt[ampest * 4 + 3].stop = true;
	if (ampest * 4 + 4 < arr_len(qt)) qt[ampest * 4 + 4].stop = true;
}

void init() {
	int n;
	img_data = stbi_load(IMAGE_PATH, &img_w, &img_h, &n, 4);

	qt = arr_create(Node);
	qt_build(&qt, 0, 0, img_w, 0, img_h);
	qt[0].stop = true;
}

void update() {
	for (int i = 0; i < sharpens_per_update; i += 1) {
		qt_sharpen();
	}
}

void qt_draw(Node *qt, int root, int l, int r, int u, int d) {
	if (root >= arr_len(qt)) return;

	if (qt[root].stop) {
		uint32_t col = qt[root].avgr | qt[root].avgg << 8 | qt[root].avgb << 16 | qt[root].avga << 24;
		fill_color_rgba(col);

		if (fill_rect_otherwise_fill_ellipse) {
			fill_rect(l, u, r - l, d - u);
		} else {
			fill_ellipse((l + r) / 2.f, (u + d) / 2.f, (r - l) / 2.f, (d - u) / 2.f);
		}
		return;
	}

	int lr = (l + r) / 2;
	int ud = (u + d) / 2;

	// +1 +2
	// +3 +4
	qt_draw(qt, root * 4 + 1, l,  lr, u,  ud);
	qt_draw(qt, root * 4 + 2, lr,  r, u,  ud);
	qt_draw(qt, root * 4 + 3, l,  lr, ud,  d);
	qt_draw(qt, root * 4 + 4, lr,  r, ud,  d);
}

void draw() {
	{
		float scale_w = (float) window_w / img_w;
		float scale_h = (float) window_h / img_h;
		if (scale_w < scale_h) scale(scale_w, scale_w);
		else                   scale(scale_h, scale_h);
	}

	qt_draw(qt, 0, 0, img_w, 0, img_h);
}

void keydown(int key) {
	if (key == ' ') fill_rect_otherwise_fill_ellipse = !fill_rect_otherwise_fill_ellipse;

	if (key >= '0' && key <= '9') {
		sharpens_per_update = (1 << (key - '0')) >> 1;
		printf("%d spu\n", sharpens_per_update);
	}
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) { }

