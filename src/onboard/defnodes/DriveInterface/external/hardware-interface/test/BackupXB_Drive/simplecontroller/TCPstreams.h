//C++ TCPstreams Version 1.0
#pragma once
#include <string> 


int openserver(std::string IP, int port);


int accept(int server);

void tsenddat(int socketin, char* MSG);

std::string tgetdat(int socketin, int buffer = 1024);

int openclient(std::string IP, int port);
