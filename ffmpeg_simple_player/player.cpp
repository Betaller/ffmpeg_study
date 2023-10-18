#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <format>
#include <string>
#include <print>
#include"decoder.h"

using namespace std;
#pragma warning(disable : 4996)

int main(int argc, char* argv[]) {
	string input_file;
	print("请输入文件名：");
	cin >> input_file;

	av_log(NULL, AV_LOG_INFO, "your input is %s!\n", input_file.c_str());
	decoder dec(input_file);

	dec.decode_display();

	return 0;
}