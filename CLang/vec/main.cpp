#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <iostream>

union vec {
	struct {
		float x, y, z, w;
	};
	float raw[4];

	vec() {
		x = y = z = w = 0.0f;
	}

	vec(float a, float b, float c, float d = 1.0) {
		x = a;
		y = b;
		z = c;
		w = d;
	}

	vec operator + (vec & a) { return vec(x + a.x, y + a.y, z + a.z, w + a.w); }
	vec operator - (vec & a) { return vec(x - a.x, y - a.y, z - a.z, w - a.w); }
	vec operator * (vec & a) { return vec(x * a.x, y * a.y, z * a.z, w * a.w); }
	vec operator / (vec & a) { return vec(x / a.x, y / a.y, z / a.z, w / a.w); }
	vec operator + (float a) { return vec(x + a, y + a, z + a, w + a); }
	vec operator - (float a) { return vec(x - a, y - a, z - a, w - a); }
	vec operator * (float a) { return vec(x * a, y * a, z * a, w * a); }
	vec operator / (float a) { return vec(x / a, y / a, z / a, w / a); }
	vec operator += (vec & a) { x += a.x; y += a.y; z += a.z; w += a.w; return *this; }
	vec operator -= (vec & a) { x -= a.x; y -= a.y; z -= a.z; w -= a.w; return *this; }
	vec operator *= (vec & a) { x *= a.x; y *= a.y; z *= a.z; w *= a.w; return *this; }
	vec operator /= (vec & a) { x /= a.x; y /= a.y; z /= a.z; w /= a.w; return *this; }
	vec operator += (float a) { x += a; y += a; z += a; w += a; return *this; }
	vec operator -= (float a) { x -= a; y -= a; z -= a; w -= a; return *this; }
	vec operator *= (float a) { x *= a; y *= a; z *= a; w *= a; return *this; }
	vec operator /= (float a) { x /= a; y /= a; z /= a; w /= a; return *this; }
	float dot(vec & a) { return x * a.x + y * a.y + z * a.z; }
	vec cross(vec & a) { 
		return vec(
			y * a.z - z * a.y,
			z * a.x - x * a.z,
			x * a.y - y * a.x,
			1.0);
	}
	float length() { return sqrt(dot(*this)); }
	void normalize() {
		auto l = length();
		x /= l;
		y /= l;
		z /= l;
		w = 1.0;
	}
	

	void print() {
		auto & c = std::cout;
		c << __FUNCTION__ << "(";
		c << x << ", ";
		c << y << ", ";
		c << z << ")" << std::endl;
	}
};

int main() {
	vec v0(1, 2, 3);
	vec v1(4, 5, 6);
	vec va(1, 0, 0);
	vec vb(0, 1, 0);
	
	(v0 + v1).print();
	(v0 - v1).print();
	(v0 * v1).print();
	(v0 / v1).print();
	
	(v0 + 123).print();
	(v0 - 123).print();
	(v0 * 123).print();
	(v0 / 123).print();
	
	(v0 += v1);
	v0.print();
	(v0 -= v1);
	v0.print();
	(v0 *= v1);
	v0.print();
	(v0 /= v1);
	v0.print();
	v0.normalize();
	v0.print();
	float a = v0.dot(v0);
	printf("a = %f\n", a);
	(va.cross(vb)).print();
	return 0;
}

