#include "Utils.h"

void showVector(const SomArray<float> &vec, int w, int h)
{
		CImg<unsigned char> im(w ,h ,1, 3, 0);
		for (int j = 0; j < w * h; j++)
		{
			int x = j / w;
			int y = j % h;
			unsigned char color[3];
			color[0] = int(vec[j] * 255);
			color[1] = int(vec[j] * 255);
			color[2] = int(vec[j] * 255);
			im.draw_point(x ,y ,color);
		}
		unsigned char color[3] = {255,255,255};//for lol
		//im.draw_point(0,0, color);
		im.normalize(0, 255);
		im.resize(300,300,-100,-100,1);
	//	im.save("Train\\sex.bmp");
		CImgDisplay *main_disp = new CImgDisplay(im,"Click a point");
		system("pause");
}