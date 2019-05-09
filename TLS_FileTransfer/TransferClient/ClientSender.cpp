#define BUF_SIZE 4096
#if 1
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <WinSock2.h>
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "Ws2_32.lib")
typedef unsigned long long u64;
u64 GetMicroCounter();

//파라메터 :  <server IP> <port> <filename>
int main(int argc, char **argv) {
	u64 start, end;
	WSADATA wsaData;
	struct sockaddr_in server_addr;
	SOCKET s;

	if (argc != 4) {
		printf("Command parameter does not right. \n<server IP> <port> <filename>\n");
		exit(1);
	}

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Socket Creat Error.\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	if (connect(s, (SOCKADDR *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		printf("Socket Connection Error.\n");
		exit(1);
	}

	printf("File Send Start");

	int totalBufferNum;
	int BufferNum;
	int sendBytes;
	long totalSendBytes;
	long file_size;
	char buf[BUF_SIZE];

	FILE *fp;
	fp = fopen(argv[3], "rb");
	if (fp == NULL) {
		printf("File not Exist");
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	totalBufferNum = file_size / sizeof(buf) + 1;
	fseek(fp, 0, SEEK_SET);
	BufferNum = 0;
	totalSendBytes = 0;

	_snprintf(buf, sizeof(buf), "%d", file_size);
	sendBytes = send(s, buf, sizeof(buf), 0);

	start = GetMicroCounter();
	while ((sendBytes = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
		send(s, buf, sendBytes, 0);
		BufferNum++;
		totalSendBytes += sendBytes;
		//printf("In progress: %d/%dByte(s) [%d%%]\n", totalSendBytes, file_size, ((BufferNum * 100) / totalBufferNum));
	}
	end = GetMicroCounter();
	//printf("time: %d second(s)", end - start);
	std::cout << "time : " << (end - start) / 1000 << "ms" << std::endl;

	closesocket(s);
	WSACleanup();

	return 0;
}

u64 GetMicroCounter()
{
	u64 Counter;

#if defined(_WIN32)
	u64 Frequency;
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER *)&Counter);
	Counter = 1000000 * Counter / Frequency;
#elif defined(__linux__)
	struct timeval t;
	gettimeofday(&t, 0);
	Counter = 1000000 * t.tv_sec + t.tv_usec;
#endif

	return Counter;
}

#else

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <boost/asio.hpp>

typedef unsigned long long u64;
u64 GetMicroCounter();

using namespace boost::asio::ip;

int main(int argc, char **argv) {
	u64 start, end;
	boost::asio::io_context ioc;

	if (argc != 4) {
		printf("Command parameter does not right. \n<server IP> <port> <filename>\n");
		exit(1);
	}

	tcp::endpoint endpoint(address::from_string(argv[1]), atoi(argv[2]));

	tcp::socket socket(ioc);

	socket.connect(endpoint);

	printf("File Send Start");

	int totalBufferNum;
	int BufferNum;
	int sendBytes;
	long totalSendBytes;
	long file_size;
	char buf[BUF_SIZE];

	FILE *fp;
	fp = fopen(argv[3], "rb");
	if (fp == NULL) {
		printf("File not Exist");
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	totalBufferNum = file_size / sizeof(buf) + 1;
	fseek(fp, 0, SEEK_SET);
	BufferNum = 0;
	totalSendBytes = 0;

	_snprintf(buf, sizeof(buf), "%d", file_size);
	//sendBytes = send(s, buf, sizeof(buf), 0);
	sendBytes = socket.send(boost::asio::buffer(buf, BUF_SIZE));

	start = GetMicroCounter();
	while ((sendBytes = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
		socket.send(boost::asio::buffer(buf, BUF_SIZE));
		BufferNum++;
		totalSendBytes += sendBytes;
		printf("In progress: %d/%dByte(s) [%d%%]\n", totalSendBytes, 
			file_size, ((BufferNum * 100) / totalBufferNum));
	}
	end = GetMicroCounter();
	//printf("time: %f second(s)", (end - start)/10000);
	std::cout << "time : " << (end - start) / 1000 << "ms" << std::endl;

	return 0;
}


u64 GetMicroCounter()
{
	u64 Counter;

	u64 Frequency;
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER *)&Counter);
	Counter = 1000000 * Counter / Frequency;

	return Counter;
}
#endif