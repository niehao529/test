#include <iostream>
#include "chat_server.h"

using namespace std;

typedef struct ClientInfo
{
	int cfd;
	struct sockaddr_in client_addr;
} CINFO;


// 事件处理函数
void* client_handler(void* arg)
{
	CINFO* info = (CINFO*)arg;
	char ip[16] = "";
	
	printf("new client ip=%s, port=%d\n",
		inet_ntop(AF_INET, &info->client_addr.sin_addr.s_addr, ip, 16),
		ntohs(info->client_addr.sin_port));


	while (1)
	{
		char buf[1024];
		memset(buf, 0, sizeof buf);
		int n = read(info->cfd, buf, sizeof buf);
		if (n == 0)
		{
			cout << "client close..." << endl;
			break;
		}
		else
		{
			cout << buf << endl;
			write(info->cfd, buf, n);
		}
	}
	close(info->cfd);
	delete info;
	return NULL;
}

int main(void)
{
	// 创建socket套接字
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		perror("socket");
		return 1;
	}

	// 绑定
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(8000);
	bind(lfd, (struct sockaddr*)&server_addr, sizeof server_addr);

	// 监听
	listen(lfd, 128);

	// 提取
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof client_addr;
	memset(&client_addr, 0, sizeof client_addr);
	//int client_fd = accept(lfd, (struct sockaddr*)&client_addr, &client_addr_len);

	// 设计线程分离
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


	while (true)
	{
		memset(&client_addr, 0, sizeof client_addr);
		int cfd = accept(lfd, (struct sockaddr*)&client_addr, &client_addr_len);

		CINFO* info = new CINFO;
		info->cfd = cfd;
		info->client_addr = client_addr;
		pthread_t tid;
		pthread_create(&tid, &attr, client_handler, info);
	}

	close(lfd);

	return 0;
}
