#ifndef COMMON_H
#define COMMON_H

#define MAX_SERVER_INSTANCE 10 //Max videostorage server
#define COMMAND_SIZE 3         //command size for communication between server and client
#define CREATE_COMMAND_SIZE 30 //max string length of create command
#define MESSAGE_SIZE 2073600   //1920x1080bytes
#define BUFSIZE 512
#define HEADER_SIZE 1          //header size in each frame

enum ecommand {
	REQUEST_ACK = 0,
	REQUEST_FAILED = 1,
	VIDEO_PARAM = 2,
	VIDEO_CREATE = 3,
	VIDEO_FRAME_IDR = 4,
	VIDEO_FRAME_NON_IDR = 5,
	VIDEO_STOP = 6
};

#endif COMMON_H