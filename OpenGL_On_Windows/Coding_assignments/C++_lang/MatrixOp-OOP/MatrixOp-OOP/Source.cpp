#include<windows.h>
#include"MatrixOperations.h"

void display_menu() {
	std::cout << "---------------------" << std::endl;
	std::cout << "1.Show matrix 1 and 2" << std::endl;
	std::cout << "2.Add" << std::endl;
	std::cout << "3.Subtract" << std::endl;
	std::cout << "4.Multiply" << std::endl;
	std::cout << "5.Exit" << std::endl;
	std::cout << "Enter your choice:";
}

int main(void) {

	float data1[4][4] = {1.0f, 5.0f, 9.0f, 13.0f,
				   2.0f, 6.0f, 10.f, 14.0f,
		           3.0f, 7.0f, 11.0f,15.0f,
		           4.0f, 8.0f, 12.0f,16.0f };
	float data2[16] = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f,
					   100.0f,110.0f, 120.0f, 130.0f, 140.0f, 150.0f, 160.0f};

	Mat4 mat1(data1);
	Mat4 mat2(data2);
	Mat4 result_sum;
	Mat4 result_subtraction;

	int choice = 0;
	while (choice != 5) {
		display_menu();
		std::cin >> choice;
		switch (choice)
		{
		case 1:
			std::cout << "Matrix1:" << std::endl;
			mat1.displayMat();
			std::cout << "Matrix2:" << std::endl;
			mat2.displayMat();
			break;
		case 2:
			result_sum = mat1 + mat2;
			std::cout << "Addition of mat1 and mat2:" << std::endl;
			result_sum.displayMat();
			break;
		case 3:
			result_subtraction = mat1 - mat2;
			std::cout << "Subtraction of mat1 and mat2:" << std::endl;
			result_subtraction.displayMat();
			break;
		default: std::cout << "Wrong choice entered.Try again." << std::endl;
			break;
		}
	}
	getchar();
	return 0;
}