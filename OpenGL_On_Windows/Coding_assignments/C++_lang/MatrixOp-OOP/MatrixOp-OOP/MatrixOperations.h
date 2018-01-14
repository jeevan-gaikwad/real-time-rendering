#pragma once
#define MAT4_DIMENSIONS 4
#include<iostream>
#include<stdio.h>

class Mat4 {
private:
	float elements[MAT4_DIMENSIONS][MAT4_DIMENSIONS];
public:
	Mat4();
	Mat4(float mat_elements[16]);
	Mat4(float mat_elements[MAT4_DIMENSIONS][MAT4_DIMENSIONS]);
	Mat4 operator+(Mat4);
	Mat4 operator-(Mat4);
	Mat4 operator*(Mat4 matrix);

	void displayMat();
};
