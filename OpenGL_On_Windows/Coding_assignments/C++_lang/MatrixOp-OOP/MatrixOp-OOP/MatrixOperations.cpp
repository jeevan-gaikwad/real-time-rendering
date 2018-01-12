#include"MatrixOperations.h"

Mat4::Mat4() { //default constructor. Create identity matrix
	for (int i = 0; i < MAT4_DIMENSIONS; i++) {
		for (int j = 0; j < MAT4_DIMENSIONS; j++) {
			if (i == j)
				elements[j][i] = 1;
			else
				elements[j][i] = 0;
		}
	}
}

Mat4::Mat4(float mat_elements[16]) {
	int k = 0;
	for (int i = 0;i < MAT4_DIMENSIONS;i++) {
		for (int j = 0;j < MAT4_DIMENSIONS;j++) {
			elements[j][i] = mat_elements[k++];
		}
	}
}
Mat4::Mat4(float mat_elements[MAT4_DIMENSIONS][MAT4_DIMENSIONS]) {
	for (int i = 0;i < MAT4_DIMENSIONS;i++) {
		for (int j = 0;j < MAT4_DIMENSIONS;j++) {
			elements[j][i] = mat_elements[j][i];
		}
	}
}
Mat4 Mat4::operator+(Mat4 matrix) {
	Mat4 result;
	for (int i = 0;i < MAT4_DIMENSIONS;i++) {
		for (int j = 0;j < MAT4_DIMENSIONS;j++) {
			result.elements[i][j] = (this)->elements[i][j] + matrix.elements[i][j];
		}
	}
	return result;
}

Mat4 Mat4::operator-(Mat4 matrix) {
	Mat4 result;
	for (int i = 0;i < MAT4_DIMENSIONS;i++) {
		for (int j = 0;j < MAT4_DIMENSIONS;j++) {
			result.elements[i][j] = (this)->elements[i][j] - matrix.elements[i][j];
		}
	}
	return result;
}

Mat4 Mat4::operator*(Mat4 matrix) {
	Mat4 result;
	//TODO: implement column by row mat mul
	return result;
}


void Mat4::displayMat() {

	for (int i = 0;i < MAT4_DIMENSIONS;i++) {
		std::cout << "| ";
		for (int j = 0;j < MAT4_DIMENSIONS;j++) {
			std::cout << elements[i][j]<<"  ";
		}
		std::cout << "|" << std::endl;
	}
	std::cout << std::endl;
}