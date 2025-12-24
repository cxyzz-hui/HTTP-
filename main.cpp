#include"Server.hpp"
#include<stdio.h>
#include<thread>
#include<iostream>
#include"Log/Logger.hpp"
#include"Http/HttpRequest.hpp"
#include"Http/HttpContext.hpp"
#include"WebSocket/WebSocketServer.hpp"



void onRequest(const Buffer* input, Buffer* output)
{
	//½øÐÐecho»Ø¸´
	output->append(input->peek(),input->readableBytes());
}


int main(int argc, char* argv[])
{
  int numThreads = 0;
  if (argc > 1) 
  {
    Logger::SetLogLevel(Logger::LogLevel::WARN);
    numThreads = atoi(argv[1]);
  }

  EventLoop loop;
  WebSocketServer server(&loop, InetAddr(9000));
  server.SetWebScoketCallback(onRequest);
  server.start(numThreads);
  loop.loop();

  return 0;
}


