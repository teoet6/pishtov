#include <stdbool.h>

#include "../pishtov.h"
#include "stdio.h"
#include "arr.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define IMAGE_PATH "/home/teoet6/Downloads/Telegram Desktop/flowers.jpg"

int img_w;
int img_h;
uint8_t *img_data;

bool fill_rect_otherwise_fill_ellipse;
int sharpens_per_update = 1;

typedef struct Node Node;
struct Node {
	uint32_t avg_col;
	float amp_col;
	uint32_t stop;
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
		(*qt)[root].avg_col = 0
			| img_data[4 * u * img_w + 4 * l + 0]
			| img_data[4 * u * img_w + 4 * l + 1] << 8
			| img_data[4 * u * img_w + 4 * l + 2] << 16
			| img_data[4 * u * img_w + 4 * l + 3] << 24;
		return true;
	}

	int lr = (l + r) / 2;
	int ud = (u + d) / 2;

	// +1 +2
	// +3 +4
	bool ok1 = qt_build(qt, root * 4 + 1, l,  lr, u,  ud);
	bool ok2 = qt_build(qt, root * 4 + 2, lr,  r, u,  ud);
	bool ok3 = qt_build(qt, root * 4 + 3, l,  lr, ud,  d);
	bool ok4 = qt_build(qt, root * 4 + 4, lr,  r, ud,  d);

	{
		uint32_t sumr =   0, sumg =   0, sumb =   0, suma =   0;
		uint32_t minr = 255, ming = 255, minb = 255, mina = 255;
		uint32_t maxr =   0, maxg =   0, maxb =   0, maxa =   0;

		if (ok1) {
			uint32_t r_ = (*qt)[root * 4 + 1].avg_col       & 0xff;
			uint32_t g_ = (*qt)[root * 4 + 1].avg_col >>  8 & 0xff;
			uint32_t b_ = (*qt)[root * 4 + 1].avg_col >> 16 & 0xff;
			uint32_t a_ = (*qt)[root * 4 + 1].avg_col >> 24 & 0xff;

			if (r_ < minr) minr = r_;
			if (g_ < ming) ming = g_;
			if (b_ < minb) minb = b_;
			if (a_ < mina) mina = a_;

			if (r_ > maxr) maxr = r_;
			if (g_ > maxg) maxg = g_;
			if (b_ > maxb) maxb = b_;
			if (a_ > maxa) maxa = a_;

			sumr += r_;
			sumg += g_;
			sumb += b_;
			suma += a_;
		}

		if (ok2) {
			uint32_t r_ = (*qt)[root * 4 + 2].avg_col       & 0xff;
			uint32_t g_ = (*qt)[root * 4 + 2].avg_col >>  8 & 0xff;
			uint32_t b_ = (*qt)[root * 4 + 2].avg_col >> 16 & 0xff;
			uint32_t a_ = (*qt)[root * 4 + 2].avg_col >> 24 & 0xff;

			if (r_ < minr) minr = r_;
			if (g_ < ming) ming = g_;
			if (b_ < minb) minb = b_;
			if (a_ < mina) mina = a_;

			if (r_ > maxr) maxr = r_;
			if (g_ > maxg) maxg = g_;
			if (b_ > maxb) maxb = b_;
			if (a_ > maxa) maxa = a_;

			sumr += r_;
			sumg += g_;
			sumb += b_;
			suma += a_;
		}

		if (ok3) {
			uint32_t r_ = (*qt)[root * 4 + 3].avg_col       & 0xff;
			uint32_t g_ = (*qt)[root * 4 + 3].avg_col >>  8 & 0xff;
			uint32_t b_ = (*qt)[root * 4 + 3].avg_col >> 16 & 0xff;
			uint32_t a_ = (*qt)[root * 4 + 3].avg_col >> 24 & 0xff;

			if (r_ < minr) minr = r_;
			if (g_ < ming) ming = g_;
			if (b_ < minb) minb = b_;
			if (a_ < mina) mina = a_;

			if (r_ > maxr) maxr = r_;
			if (g_ > maxg) maxg = g_;
			if (b_ > maxb) maxb = b_;
			if (a_ > maxa) maxa = a_;

			sumr += r_;
			sumg += g_;
			sumb += b_;
			suma += a_;
		}

		if (ok4) {
			uint32_t r_ = (*qt)[root * 4 + 4].avg_col       & 0xff;
			uint32_t g_ = (*qt)[root * 4 + 4].avg_col >>  8 & 0xff;
			uint32_t b_ = (*qt)[root * 4 + 4].avg_col >> 16 & 0xff;
			uint32_t a_ = (*qt)[root * 4 + 4].avg_col >> 24 & 0xff;

			if (r_ < minr) minr = r_;
			if (g_ < ming) ming = g_;
			if (b_ < minb) minb = b_;
			if (a_ < mina) mina = a_;

			if (r_ > maxr) maxr = r_;
			if (g_ > maxg) maxg = g_;
			if (b_ > maxb) maxb = b_;
			if (a_ > maxa) maxa = a_;

			sumr += r_;
			sumg += g_;
			sumb += b_;
			suma += a_;
		}

		uint32_t num_cols = ok1 + ok2 + ok3 + ok4;
		uint32_t avgr = sumr / num_cols;
		uint32_t avgg = sumg / num_cols;
		uint32_t avgb = sumb / num_cols;
		uint32_t avga = suma / num_cols;

		uint32_t deltar = maxr - minr;
		uint32_t deltag = maxg - ming;
		uint32_t deltab = maxb - minb;
		uint32_t deltaa = maxa - mina;

		(*qt)[root].avg_col = avgr | avgg << 8 | avgb << 16 | avga << 24;
		(*qt)[root].amp_col = sqrt(deltar * deltar + deltag * deltag + deltab * deltab + deltaa * deltaa);
	}

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

	if (qt[root].amp_col * (r - l) * (d - u) > *amp) {
		*amp = qt[root].amp_col * (r - l) * (d - u);
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
		fill_color_rgba(qt[root].avg_col);

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

