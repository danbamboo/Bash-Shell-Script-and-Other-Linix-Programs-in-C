from __future__ import print_function
from socket import *
import sys
import os.path


MAXBUF = 50

#===================
#======= MAIN ======
#===================
def main():

	serverHost, serverPort, command, fileName, dataPort  = checkSetup() #check valid command line input, assign variables
	serverSocket = setUpSocketConn(serverPort,serverHost) 
	validateCommandsWithServer(command,serverSocket, dataPort)
	executeBasedOnCommandType(command, serverSocket, dataPort, fileName, serverHost, serverPort)

#================
#Function: checkSetup()
#Description: Validates command line arguments: serverHost, serverPort, command, fileName, dataPort
#				Returns all validated arguemnts for future use.
#================
def checkSetup():

	numOfComArgs = len(sys.argv) 
	fileName=""

	#Check for correct number of arguments
	if numOfComArgs != 5 and numOfComArgs != 6:  #Verify number of args =2, if not, give usage and quit
		print("Usage: ftclient.py serverHost serverPort Command FileName DataPort")
		quit()

	serverHost = sys.argv[1]

	#Check host name input 
	if serverHost != "flip1" and serverHost != "flip2" and serverHost != "flip3":
		print("Please enter flip1, flip2, or flip3 for serverHost")
		print("Usage: ftclient.py serverHost serverPort Command FileName DataPort")
		quit()

	portServer = int(sys.argv[2])  #assign int value to second argument for port
	
	if(numOfComArgs == 5):

		portData = int(sys.argv[4])
	
	else:
		fileName = sys.argv[4]
		portData = int(sys.argv[5])

	#Check Command inputs
	command = sys.argv[3]
	if command != "-l" and command != "-g":
		print("Invalid command.  Please enter -l (dir list) or -g (file transfer)")
		print("Usage: ftclient.py serverHost serverPort Command FileName DataPort")
		quit()

	#Check Port Number input
	if (portData < 1024) or (portServer < 1024) or (portData > 49151) or (portServer > 49151):
		print("Invalid port entry. Rage 1024-49151")
		print("Usage: ftclient.py serverHost serverPort Command FileName DataPort")
		quit()

	return serverHost, portServer, command, fileName, portData


#================
#Function: setUpSocketConn()
#Description: Takes a port and host name.  Prepends hostname to '.engr.oregonstate.edu'
#				Outputs serverSocket after connection made.
#================
def setUpSocketConn(serverPort, serverHost):

	serverSocket = socket(AF_INET,SOCK_STREAM)
	serverAddress = serverHost + ".engr.oregonstate.edu"
	serverSocket.connect((serverAddress,serverPort))
	
	return serverSocket


#================
#Function: validateCommandsWithServer()
#Description: Checks command (-l or -g).  Server requires a check.  Also, send dataPort number through server socket
#				Nothing is returned.
#================
def validateCommandsWithServer(command,serverSocket, dataPort):
	serverSocket.send(command)
	commandFeedback = serverSocket.recv(MAXBUF)
	
	if commandFeedback == "Error1":
		print("Server Flagged Invalid Command.")
		quit()

	serverSocket.send(str(dataPort))  #send dataPort num so Client can ready a new connection

	return


#================
#Function: executeBasedOnCommandType(command, serverSocket, dataPort, fileName, serverHost,serverPort)
#Description: Works to direct execution path for seperate commands (-l or -g)
#             Pass variables to either getDirectory (for -l) or getFileTransfer (for -g)
#================
def executeBasedOnCommandType(command, serverSocket, dataPort, fileName, serverHost,serverPort):
	
	if command == "-l":
		getDirectory(serverSocket, dataPort,serverHost)
	else:   #validated twice, if not -l will be -g
		getFileTransfer(serverSocket,dataPort,fileName,serverHost,serverPort)


#================
#Function: getDirectory(serverSocket, dataPort, serverHost)
#Description: Driver function for getting directory structure form server
#            Setup new connection on dataPort.  Recieve data from Server and output to stdout
#================
def getDirectory(serverSocket, dataPort, serverHost):
	
	dataSocket = setUpSocketConn(dataPort,serverHost)
	endOfMessage = "-END-" #End of message tag sent from clinet

	print("Receiving directory structure from", serverHost, ":",dataPort)
	print("----------------")

	while 1:
		commandFeedback = dataSocket.recv(MAXBUF) 

		if endOfMessage in commandFeedback:   #when the -END- command is sent, we know completes
			commandFeedback = commandFeedback.split("-END-")[0] #parse out the end of message signifier attached to each client message
			print(commandFeedback, end="")
			print("----------------")
			dataSocket.close()
			serverSocket.close()
			quit()

		else:
			print(commandFeedback, end="")



#================
#Function: getFileTransfer(serverSocket, dataPort, fileName, serverHost,serverPort)
#Description:  Main driver for send/recv file from server.
#   		If invalid file, will validate through the serverPort conn before establishing new data Connection
#			Inputs fill contents of file into new file.
#================
def getFileTransfer(serverSocket, dataPort, fileName, serverHost,serverPort):
	
	serverSocket.send(fileName)
	validFileCheck = "ValidFile"
	endOfMessage = "-END-" #End of message tag sent from server

	fileValid = serverSocket.recv(MAXBUF)

	#Check for valid file name on serverSocket before making data socket
	if validFileCheck in fileValid:
		print("Receiving \"", fileName, "\" from ", serverHost, ":", dataPort)

		dataSocket = setUpSocketConn(dataPort,serverHost)
		fileExists = os.path.isfile(fileName) #check if file exists
		if fileExists == True:
			#File exists and will be overwritten
			print("The file \"", fileName, "\" exists, overwriting...")
		fileTransfer = open(fileName, "wb+")    #python file reference: https://learnpythonthehardway.org/book/ex16.html

		while 1:
			commandFeedback = dataSocket.recv(MAXBUF)

			if endOfMessage in commandFeedback: #when the -END- command is sent, we know completes
				commandFeedback = commandFeedback.split("-END-")[0] #parse out the end of message signifier attached to each client message
				fileTransfer.write(commandFeedback)
				print("File transfer complete.")
				dataSocket.close()
				serverSocket.close()
				quit()

			else:
				fileTransfer.write(commandFeedback)

	#File not found, message and exit
	else:
		print(serverHost, ":", serverPort, "says\n FILE NOT FOUND")
		quit()


	return



#================
if __name__ == "__main__":
    main()
#================