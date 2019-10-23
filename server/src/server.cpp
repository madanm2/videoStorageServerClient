#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <string> 
#include "server.h"

using namespace std;

//global variables
static int totalInstances	= 0;
string filepath				= "D:/data/";
videoStorageServer* server[MAX_SERVER_INSTANCE];

//function prototype 
DWORD WINAPI InstanceThread(LPVOID);

videoStorageServer::videoStorageServer(HANDLE hpipe, int instanceID )
{
	totalInstances++;
	m_handle_pipe	= hpipe;
	m_id			= instanceID;
	m_videoFileName =  filepath + "file"+std::to_string(m_id) + ".264";
	m_video = new video(1920, 1080, 1200000, 30, "h.264", m_videoFileName);
}

void videoStorageServer::create(int width, int height, int bitrate, int fps, 
								string compressionFormat, string fileName)
{
	m_video = new video(width, height, bitrate, fps, compressionFormat, fileName);
}

videoStorageServer::~videoStorageServer()
{
}

int main(void)
{
	BOOL   fConnected = FALSE;
	DWORD  dwThreadId = 0;
	HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
	
	// Multithreaded pipe server, supporting multiple clients with Max instance limit
	for (int inst= 0;inst < MAX_SERVER_INSTANCE;inst++)
	{
		cout << "\nVideoStorageServer: awaiting client connection" << endl;
		hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\mynamedpipe"),
								PIPE_ACCESS_DUPLEX,
								PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 
								PIPE_UNLIMITED_INSTANCES, // max. instances  
								BUFSIZE,                  // output buffer size 
								MESSAGE_SIZE,             // input buffer size 
								0,                        // client time-out 
								NULL);                    // default security attribute 

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			cout << "CreateNamedPipe failed, GLE=" << GetLastError() << endl;
			return -1;
		}

		// Wait for the client to connect
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE:(GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected)
		{
			cout<<"Client connected, creating a processing thread.\n"<<endl;
			// Create a thread for this client. 
			hThread = CreateThread(
				NULL,              // no security attribute 
				0,                 // default stack size 
				InstanceThread,    // thread proc
				(LPVOID)hPipe,     // thread parameter 
				0,                 // not suspended 
				&dwThreadId);      // returns thread ID 

			if (hThread == NULL)
			{
				cout<<"CreateThread failed, GLE="<< GetLastError()<<endl;
				return -1;
			}
			else CloseHandle(hThread);
		}
		else 
		{
			CloseHandle(hPipe);
		}
	}
	return 0;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
{
	HANDLE hHeap		= GetProcessHeap();
	TCHAR* pchmessage	= (TCHAR*)HeapAlloc(hHeap, 0, MESSAGE_SIZE * sizeof(TCHAR));
	TCHAR command[COMMAND_SIZE];
	DWORD cbBytesRead	= 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess		= FALSE;
	HANDLE hPipe		= NULL;
	DWORD instanceID    = totalInstances;
	FILE* fp_out		= NULL;
	if (instanceID > MAX_SERVER_INSTANCE)
	{
		cout << "ERROR - Exceeded max videoStorage server instance supported " << endl;
		if (pchmessage != NULL) HeapFree(hHeap, 0, pchmessage);
		return (DWORD)-1;
	}
	if (lpvParam == NULL)
	{
		cout << "ERROR - Pipe Server Failure" << endl;
		if (pchmessage != NULL) HeapFree(hHeap, 0, pchmessage);
		return (DWORD)-1;
	}
	if (pchmessage == NULL)
	{
		cout << "ERROR - Pipe Server Failure" << endl;
		return (DWORD)-1;
	}
	hPipe = (HANDLE)lpvParam;

	/* wait for video object creation from client */
	while (1)
	{
		fSuccess = ReadFile(hPipe,         
							command,
							COMMAND_SIZE * sizeof(TCHAR),
							&cbBytesRead,
							NULL);
		if ((command[0] == VIDEO_CREATE)&& fSuccess)
		{
			char createcommand[CREATE_COMMAND_SIZE];
			fSuccess = ReadFile(hPipe,
								createcommand,
								CREATE_COMMAND_SIZE * sizeof(TCHAR),
								&cbBytesRead,
								NULL);
			cout << "creating new videoStorage server with properties " << createcommand << endl;
			
			/*TBD: parse createcommand 
				& create() with video param*/
			server[instanceID] = new videoStorageServer(hPipe, instanceID);
			fp_out = server[instanceID]->m_video->getFile();

			cout << "videoStorageServer instance created "<< instanceID << endl;
			// send ack to client
			command[0] = REQUEST_ACK;
			fSuccess = WriteFile(hPipe,        
								 command,     
								 COMMAND_SIZE,
								 &cbWritten,   
								 NULL);
			break;
		}
	}

	/* read and store compressed data from client */
	while (1)
	{
		//read compressed frame from client
		fSuccess = ReadFile(hPipe,
							pchmessage,
							MESSAGE_SIZE-1,
							&cbBytesRead,
							NULL);
	
		if (!fSuccess || cbBytesRead == 0)
		{
			cout << "server: compressed data read failed " << endl;
			break; 
		}
		else
		{			
			if ((*pchmessage == VIDEO_FRAME_IDR) || (*pchmessage == VIDEO_FRAME_NON_IDR))
			{
				cout << "server: compressed data received " << cbBytesRead << endl;
				fwrite(pchmessage+HEADER_SIZE, sizeof(char), cbBytesRead, fp_out);
			}
			else if (*pchmessage == VIDEO_STOP)
			{
				cout << "server: video stop request received " << cbBytesRead << endl;
				break;
			}

			// send ack to client
			command[0] = REQUEST_ACK;
			fSuccess = WriteFile(hPipe,
				command,
				COMMAND_SIZE,
				&cbWritten,
				NULL);

			if (!fSuccess || COMMAND_SIZE != cbWritten)
			{
				cout << "server: sending ack to client for frame read failed " << endl;
				break;
			}
		}
	}
	if(fp_out)
	{
		fclose(fp_out);
	}
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	HeapFree(hHeap, 0, pchmessage);
	cout << "InstanceThread exitting" << endl;
	return 1;
}

