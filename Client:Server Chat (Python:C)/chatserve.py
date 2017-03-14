from __future__ import print_function
from socket import *
import sys



#===================
#======= MAIN ======
#===================
def main():

	serverPort = checkSetup() #check valid command line input, assign server port#
	messenger(serverPort)  #start main messenger function




#================
#Function: checkSetup()
#Description: Validates command line arguments by checking for the
#			correct number of arguments as well as a port# between 1024-49151.
#			Returns port number if valid.  Exits if error detected. 
#================
def checkSetup():

	numOfComArgs = len(sys.argv) 

	if numOfComArgs != 2:  #Verify number of args =2, if not, give usage and quit
		print("Usage: chatserve.py port#")
		quit()

	port = int(sys.argv[1])  #assign int value to second argument for port
	if port < 1024 or port > 49151:
		print("Invalid port entry. Rage 1024-49151")
		print("Usage: chatserve.py port#")
		quit()

	return port

#================
#Function: setupConnection()
#Description: messenger is the main driver function, and makes calls to secondary functions.
#			   The precondition is to have a valid port number passed in.  The post coniditon 
#			   is an ongoing open socket connection for a single connection at a time.  The
#			   messenger works by commuincating one message back and forth between a client and 
#			   this server. 
#================
def messenger(serverPort):

	serverSocket = socket(AF_INET,SOCK_STREAM)
	serverSocket.bind(('',serverPort))
	serverSocket.listen(1)
	endOfMessage = "-END-" #End of message tag sent from clinet
	clientHandle = ''
	
	initalConnFlag =0

	serverHandle = getServerHandle()  # Call function to get sever handle 

	
	
	
	
	while 1:  #This is continues until client or server enters \quit
		
		if initalConnFlag==0:  #inital flag is set to 0 if we are starting socket connection with new client
			initalConnFlag=1
			print("The server is ready to recieve")
			connectionSocket, addr = serverSocket.accept()
			print("Connection from", addr)
			connectionSocket.send(serverHandle)
			clientHandle = connectionSocket.recv(1024)  #Recieve up to 1024 characters
			
			if endOfMessage in clientHandle:
				clientHandle = clientHandle.split("-END-")[0] #parse out the end of message signifier attached to each client message
			
			print("Connection established to: ",clientHandle)
			

		# Using this site as reference on recv() stratgies http://code.activestate.com/recipes/408859/
		else:
			clientMessage = connectionSocket.recv(1024)
			sentence = clientMessage.split("-END-")[0] #parse to remove end message signifier from client
			

			if sentence == "\quit":  #Check for \quit command from client
				print(clientHandle, "left the chat.")
				connectionSocket.close()
				initalConnFlag=0

			else: # client message was not quit
				print(clientHandle,": ",sentence)
				serverMessage = getServerMessage()
				
				if serverMessage == "\quit": #if server message is quit, send to client and close current soccet
					connectionSocket.send(serverMessage)
					print("Server is cloing connection with ", clientHandle)
					connectionSocket.close()
					initalConnFlag=0
				else:
					connectionSocket.send(serverMessage)

			
			
		
	
	
		

#================
#Function: getServerHandle()
#Description: Get input from user for a handle to send to client.  Will only store a one word
#			   handle between the length of 1-10.  Returns the handle to the calling function.
#================
def getServerHandle():
	
	while 1:
		serverHandle = raw_input("Input Handle:")
		lengthOfHandle = len(serverHandle)
		serverHandle = serverHandle.split(" ")[0]
		if lengthOfHandle > 10 or lengthOfHandle < 1:
			print("Invalid Entry.  Plese enter a handle of length 1-10 characters one word.")
		else:
			break

	return serverHandle

#================
#Function: getServerMessage()
#Description: Get input from user for the message to send to client.  Will store string input
#			   between the length of 1-1023 characters.  Returns the handle to the calling function.
#			   Returns the message
#================
def getServerMessage():
	while 1:
		serverMessage = raw_input("> ")
		lengthOfHandle = len(serverMessage)
		
		if lengthOfHandle > 1023 or lengthOfHandle < 1:
			print("Invalid Entry.  Plese enter a handle of length 1-1023 characters.")
		else:
			break

	return serverMessage








#================
if __name__ == "__main__":
    main()
#================