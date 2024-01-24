# This code is design to create a client remote evaluate simple arithmetical 
# expressions using only one of the operators +, -, * and / , also for 7374 assignment

# the name Author is Taoran_liu
# NU_ID is  002763713

# this code build on Python3 ,depends on Socket package, 
# to run this code, make sure you have installed correct emvironment
# the code is using TCP protocol

# Process of the code
# 1. set up a client and define IP and port information to connect with server
# 2. client send introductory msgs to server
# 3. when client receive msgs contain expressions from server, calculate ans and send it back
# 4. repeat step 3, untill get msgs contain SUCC header, save it as flag
# 5. close socket, disconnect with server
## for more detailed information, sees comments

# SUCC flag:  2e387502fe49033c06e70baf28304778d80bd7891c7be34bbd1a6cc81e084926




from socket import *
IP = '129.10.132.64'
# IP='kopi.ece.neu.edu'
# IP = 'localhost'
SERVER_PORT=5210
BUFLEN=512
 

# define success code ans and rec string
recved_str=''
ans_str= 'EECE7374 INTR 002763713'
flag = ''

if __name__ == '__main__':

    # creat an socket object
    dataSocket =socket(AF_INET,SOCK_STREAM)
    
    # connecting server socket
    dataSocket.connect((IP,SERVER_PORT))

    # send msgs/ encode it into byte
    dataSocket.send(ans_str.encode())

    # get receive msgs
    recved=dataSocket.recv(BUFLEN)
    recved_str = recved.decode()

    # continuous receiving in dead loop unless get"succ" of error occur
    while True:

        # make sure get new msgs in buffer
        if recved_str !='':

            # split received data
            recved_data = recved_str.split(' ')

            # check whether it is question msgs
            if recved_data[1] == 'EXPR':
                left_value = int(recved_data[2])
                right_value = int(recved_data[4])
                selected_operation = recved_data[3]
                
                # calaulate answer
                if selected_operation == '+':
                    ans = left_value + right_value
                elif selected_operation == '-':
                    ans = left_value - right_value
                elif selected_operation == '*':
                    ans = left_value * right_value
                elif selected_operation == '/':
                    ans = left_value / right_value

                ans_str = 'EECE7374 RSLT '+ str(ans) 

                # send ans to server
                dataSocket.send(ans_str.encode())

            # check whether it is successful
            elif recved_data[1] == 'SUCC':
                flag = recved_data[2]
                print('Successful flag : ',flag)
                break

            # other error occur
            else:
                print('error')
                break

        # send another answer
        recved = dataSocket.recv(BUFLEN)
        recved_str = recved.decode()

            
    # shutdown client
    dataSocket.close()

