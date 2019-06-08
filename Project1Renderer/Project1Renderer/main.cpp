#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
const TGAColor white = TGAColor(255,255,255,255);
const TGAColor red = TGAColor(255,0,0,255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 200;
const int height = 200;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy)*2;
	float error2 = 0;
	int y = y0;

	for (int x=x0; x<=x1; x++)
	{
		if (steep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
		error2 += derror2;
		if (error2 > dx)
		{
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx*2;
		}
		
	}
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
	if (t0.y == t1.y && t0.y == t2.y)
		return;
	if (t0.y > t1.y)
		std::swap(t0, t1);
	if (t0.y > t2.y)
		std::swap(t0, t2);
	if (t1.y > t2.y)
		std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	for (int i = 0; i <total_height; i++)
	{
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i/ total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
		Vec2i A = t0 + (t2 - t0)*alpha;
		Vec2i B = second_half ? t1 + (t2 - t1)*beta : t0 + (t1 - t0)*beta;
		if (A.x > B.x)
			std::swap(A, B);
		for (int j = A.x; j <= B.x; j++)
			image.set(j, t0.y+1, color);
	}

	for (int y = t1.y; y <= t2.y; y++)
	{
		int segment_height = t2.y - t1.y + 1;
		float alpha = (float)(y - t0.y) / total_height;
		float beta = (float)(y - t1.y) / segment_height;
		Vec2i A = t0 + (t2 - t0)*alpha;
		Vec2i B = t1 + (t2 - t1)*beta;
		if (A.x > B.x) 
			std::swap(A, B);
		for (int j = A.x; j <= B.x; j++)
			image.set(j, y, color); 
	}
}


int main(int argc, char** argv)
{
	TGAImage image(100,100,TGAImage::RGB);
	image.set(52,42,red);
	image.flip_vertically();
	image.write_tga_file("output.tga");
//	for (int i = 0; i < model->nfaces(); i++)
//	{
//		std::vector<int> face = model->face(i);
//		for (int j = 0; j < 3; j++)
//		{
//			Vec3f v0 = model->vert(face[j]);
//			Vec3f v1 = model->vert(face[(j + 1) % 3]);
//			int x0 = (v0.x + 1.)*width / 2.;
//			int y0 = (v0.y + 1.)*height / 2.;
//			int x1 = (v1.x + 1.)*width / 2.;
//			int y1 = (v1.y + 1.)*height / 2.;
//			line(x0, y0, x1, y1, image, white);
//		}
//	}
	return 0;
}