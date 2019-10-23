# videoStorageServerClient
Multithreaded Pipe Server application to store video received from client application. 

1. Multiple client application could connect to Server and send compressed data for storage.
2. Example: client could be multiple camera in a home network, which could connect to server and store compressed video data.
3. client app sends "videoCreate" command to server to create a videoStorage instance in server.
4. server app on successful instance creation, sends ack to client to send compressed frame data for storage.
5. client app could send "stop" command to close video storage.

Server is based on Multithreaded pipe server.
https://docs.microsoft.com/en-us/windows/win32/ipc/multithreaded-pipe-server

Application is visual studio based.
1. client/build/clientAPP.sln
2. server/build/serverAPP.sln 
