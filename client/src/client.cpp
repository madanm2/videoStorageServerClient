#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <iostream>
#include "../../common/common.h"

//globals
TCHAR command[COMMAND_SIZE];
TCHAR message[MESSAGE_SIZE];
using namespace std;

int waitForAck(HANDLE hPipe)
{
	int status = -1;
	while (1)
	{
		BOOL fSuccess = FALSE;
		DWORD cbBytesRead = 0;
		//wait for acknowledgment from server
		fSuccess = ReadFile(hPipe,
							command,
							COMMAND_SIZE * sizeof(TCHAR),
							&cbBytesRead,
							NULL);
		if (fSuccess == TRUE)
		{
			if (command[0] == REQUEST_ACK)
			{
				status = 0;
			}
			break;
		}
	}
	return status;
}

int main(void)
{
	HANDLE hPipe;
	DWORD dwWritten;

	BOOL fSuccess = FALSE;
	DWORD cbBytesRead = 0;
	DWORD messageSize = 0;
	int frameCount = 0;

	hPipe = CreateFile(TEXT("\\\\.\\pipe\\mynamedpipe"),
						GENERIC_READ | GENERIC_WRITE,
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPipe);
		cout << "pipe creation filed" << endl;
		return (0);
	}

	/* video storage object creation request in server from client */
	command[0] = VIDEO_CREATE;
	WriteFile(hPipe,
		command,
		COMMAND_SIZE * sizeof(TCHAR),
		&dwWritten,
		NULL);
	
	// TBD, parse from input argument and update createcommand.
	char createcommand[] = "1920x1080, 1200000, 30, h.264 ";

	WriteFile(hPipe,
		createcommand,
		(DWORD)strlen(createcommand),
		&dwWritten,
		NULL);

	/* wait for video object creation in server */
	int status = waitForAck(hPipe);
	if (status == 0)
	{
		cout << "serverACK: video storage object created in server" << endl;
	}
	

	/* Send compressed frames at fixed time interval */
	if (status == 0)
	{
		/* synchronized encoded data transfer to server */
		while (1)
		{
			/**************** 1. Read RAW data ****************/
			/**************** 2. Compress frame ***************/
			frameCount++;
			if (frameCount < 10)
			{
				DWORD encodedFrameSize = 100;
				message[0] = VIDEO_FRAME_IDR;//fill 1st byte with picture type.
				memset((message+ HEADER_SIZE), 21, encodedFrameSize);
				messageSize = encodedFrameSize + 1;
			}
			else
			{
				messageSize = COMMAND_SIZE;
				message[0] = VIDEO_STOP;//fill 1st byte with picture type.				
			}

			/********** 3. Transmit compressed frame ***********/
			DWORD cbWritten = 0;

			// Write the reply to the pipe. 
			fSuccess = WriteFile(
				hPipe,
				message,
				messageSize,
				&cbWritten,
				NULL);

			int status = waitForAck(hPipe);
			if (status == 0)
			{
				cout << "serverACK: video frame received" << endl;
			}

			if (message[0] == VIDEO_STOP)
			{
				break;
			}
		}
	}
	return (0);
}