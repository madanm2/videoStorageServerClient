#ifndef SERVER_H
#define SERVER_H
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <iostream>
#include <fstream>
#include "../../common/common.h"
using namespace std;

class video {

private:
	int		m_width;
	int		m_height;
	int		m_fps;
	int		m_maxBitrate;
	float   m_duration;   //time seconds
	int		m_size;
	string	m_format;
	string  m_fileName;
	FILE*   m_fp;

public:
	video() 
	{
		m_width = 640;
		m_height = 480;
		m_maxBitrate = 600000;
		m_fps = 30;
		m_duration = 0.0;
		m_size = 0;
		m_format = "h.264";
		m_fp = NULL;
		m_fileName = "C:/tmp.264";
	};

	video(int width, int height, int bitrate, int fps, 
		string compressionFormat, string fileName)
	{
		m_width			= width;
		m_height		= height;
		m_maxBitrate	= bitrate;
		m_fps			= fps;
		m_duration		= 0.0;
		m_size			= 0;
		m_format		= compressionFormat;
		m_fp			= NULL;
		m_fileName		= fileName;

		if (!m_fileName.empty()) 
		{
			m_fp = fopen(m_fileName.c_str(), "wb");
		}
		else
		{
			cout << "couldnt open file" << endl;
		}
	};

	~video() {};

	/* set params */
	int setBitrate(int bitrate) 
	{
		if (bitrate > m_maxBitrate) 
		{
			m_maxBitrate = bitrate;
		}
	}
	float setDuration(float duration) 
	{
		m_duration = duration;
	}
	int setSize(int fileSize) 
	{
		m_size = fileSize;
	}


	/* get params */
	int getMaxBitrate() { return m_maxBitrate; }
	int getFps()		{ return m_fps; }
	int getWidth()		{ return m_width; }
	int getHeight()		{ return m_height; }
	int getSize()		{ return m_size; }
	FILE* getFile()     { return m_fp; }
	float getDuration() { return m_duration; }
};

class videoStorageServer: public video 
{
public:
	string	m_videoFileName;
	video* m_video;

private:
	HANDLE	m_handle_pipe;
	int		m_id;

public:
	videoStorageServer(HANDLE hpipe, int instanceID);
	~videoStorageServer();
	void create(int width, int height, int bitrate, int fps, 
				string compressionFormat, string fileName);

};
#endif SERVER_H