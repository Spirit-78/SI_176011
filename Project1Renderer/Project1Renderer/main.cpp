#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <algorithm>
#include "our_gl.h"

const TGAColor white = TGAColor(255,255,255,255);
const TGAColor red = TGAColor(255,0,0,255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const TGAColor yellow = TGAColor(255, 255, 0, 255);
const int width = 800;
const int height = 800;
Model *model = NULL;
int *zbuffer = NULL;
Vec3f light_dir(1,1,1);
Vec3f camera(0, 0, 3);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);
const int depth = 255;

Vec3f m2v(Matrix m) {
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

//Matrix viewport(int x, int y, int w, int h) {
//	Matrix m = Matrix::identity(4);
//	m[0][3] = x + w / 2.f;
//	m[1][3] = y + h / 2.f;
//	m[2][3] = depth / 2.f;

//	m[0][0] = w / 2.f;
//	m[1][1] = h / 2.f;
//	m[2][2] = depth / 2.f;
//	return m;
//}


//Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
//	Vec3f z = (eye - center).normalize();
	//Vec3f x = (up^z).normalize();
	//Vec3f y = (z^x).normalize();
	//Matrix res = Matrix::identity(4);
	//for (int i = 0; i < 3; i++) {
	//	res[0][i] = x[i];
	//	res[1][i] = y[i];
	//	res[2][i] = z[i];
	//	res[i][3] = -center[i];
	//}
	//return res;
//}

struct GouraudShader : public IShader {
	Vec3f varying_intensity;

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir);
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); 
		return Viewport * Projection*ModelView*gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor &color) {
		float intensity = varying_intensity * bar;   
		color = TGAColor(255, 255, 255)*intensity;
		return false;                      
	}
};


int main(int argc, char** argv) {
	if (2 == argc) 
		model = new Model(argv[1]);
	else 
		model = new Model("obj/african_head.obj");

	zbuffer = new int[width*height];
	for (int i = 0; i < width*height; i++)
		zbuffer[i] = std::numeric_limits<int>::min();

	lookat(eye, center, up);
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projection(-1.f / (eye - center).norm());
	light_dir.normalize();

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

	GouraudShader shader;
	for (int i = 0; i < model->nfaces(); i++) {
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++) {
			screen_coords[j] = shader.vertex(i, j);
		}
		triangle(screen_coords, shader, image, zbuffer);
	}

	image.flip_vertically(); // to place the origin in the bottom left corner of the image
	zbuffer.flip_vertically();
	image.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");
	delete model;
	//delete[] zbuffer;
	return 0;
}