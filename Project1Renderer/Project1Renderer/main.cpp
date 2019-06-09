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
float *shadowbuffer = NULL;
Model *model = NULL;
int *zbuffer = NULL;
Vec3f light_dir(1,1,1);
Vec3f camera(0, 0, 3);
Vec3f eye(1, 1, 4);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);
const int depth = 255;

Vec3f m2v(Matrix m) {
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

struct GouraudShader : public IShader {
	Vec3f varying_intensity;

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir);
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); 
		return Viewport * Projection*ModelView*gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor &color) {
		float intensity = varying_intensity * bar; 
		if (intensity > .85) intensity = 1;
		else if (intensity > .60) intensity = .80;
		else if (intensity > .45) intensity = .60;
		else if (intensity > .30) intensity = .45;
		else if (intensity > .15) intensity = .30;
		else intensity = 0;
		color = TGAColor(255, 255, 255)*intensity;
		return false;                      
	}
};

struct Shader : public IShader {
	mat<2, 3, float> varying_uv;
	mat<3, 3, float> varying_nrm;

	mat<4, 4, float> uniform_M;
	mat<4, 4, float> uniform_MIT;
	mat<4, 4, float> uniform_Mshadow;

};

float max_elevation_angle(float *zbuffer, Vec2f p, Vec2f dir) {
	float maxangle = 0;
	for (float t = 0.; t < 1000.; t += 1.) {
		Vec2f cur = p + dir * t;
		if (cur.x >= width || cur.y >= height || cur.x < 0 || cur.y < 0) return maxangle;

		float distance = (p - cur).norm();
		if (distance < 1.f) continue;
		float elevation = zbuffer[int(cur.x) + int(cur.y)*width] - zbuffer[int(p.x) + int(p.y)*width];
		maxangle = std::max(maxangle, atanf(elevation / distance));
	}
	return maxangle;

}

	struct ZShader : public IShader {
		mat<4, 3, float> varying_tri;

		virtual Vec4f vertex(int iface, int nthvert) {
			Vec4f gl_Vertex = Projection * ModelView*embed<4>(model->vert(iface, nthvert));
			varying_tri.set_col(nthvert, gl_Vertex);
			return gl_Vertex;
		}

		virtual bool fragment(Vec3f gl_FragCoord, Vec3f bar, TGAColor &color) {
			color = TGAColor(0, 0, 0);
			return false;
		}
	};

	struct DepthShader : public IShader {
		mat<3, 3, float> varying_tri;

		DepthShader() : varying_tri() {}

		virtual Vec4f vertex(int iface, int nthvert) {
			Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
			gl_Vertex = Viewport * Projection*ModelView*gl_Vertex;          // transform it to screen coordinates
			varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
			return gl_Vertex;
		}

		virtual bool fragment(Vec3f bar, TGAColor &color) {
			Vec3f p = varying_tri * bar;
			color = TGAColor(255, 255, 255)*(p.z / depth);
			return false;
		}
	};


	int main(int argc, char** argv) {
		if (2 > argc) {
			std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
			return 1;
		}

		float *zbuffer = new float[width*height];
		for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());
		model = new Model(argv[1]);

		TGAImage frame(width, height, TGAImage::RGB);
		lookat(eye, center, up);
		viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		projection(-1.f / (eye - center).norm());

		ZShader zshader;
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				zshader.vertex(i, j);
			}
			triangle(zshader.varying_tri, zshader, frame, zbuffer);
		}

		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				if (zbuffer[x + y * width] < -1e5) continue;
				float total = 0;
				for (float a = 0; a < M_PI * 2 - 1e-4; a += M_PI / 4) {
					total += M_PI / 2 - max_elevation_angle(zbuffer, Vec2f(x, y), Vec2f(cos(a), sin(a)));
				}
				total /= (M_PI / 2) * 8;
				total = pow(total, 100.f);
				frame.set(x, y, TGAColor(total * 255, total * 255, total * 255));
			}
		}

		frame.flip_vertically();
		frame.write_tga_file("framebuffer.tga");
		delete[] zbuffer;
		delete model;
		return 0;
	}