###################################################################################################
# This program is a sample server for the socket programming assignment. It generates a set of    #
# simple mathematical expressions as strings, which are sent via TCP to the client. The client    #
# needs to send back the correct answer for each of the operations. When enough correct results   #
# have been received, a secret flag is generated and sent back to the client. Finally, the        #
# connection is closed, and the server proceeds to shut down.                                     #
#                                                                                                 #
# This program was initially authored by Carlos Bocanegra <bocanegrac@coe.neu.edu>.               #
# It has since been edited by Kevin Hines <hines.ke@husky.neu.edu> and Abhimanyu Venkatraman      #
# Sheshashayee <abhi@ece.neu.edu> and Moinak Ghoshal <ghoshal.m@northeastern.edu>.                                                                #
###################################################################################################


### Imports --------------------------------------------------
import hashlib
import random
import socket
import sys


### Constants --------------------------------------------------
server_hostname = 'localhost'
default_server_port = 5206
buffer_size = 4096
number_of_expressions = 100
secret_key = "This sample key is secret".encode()


### Functions --------------------------------------------------
def generate_secret_flag( NU_ID ):
    '''Generate a secret given a NU ID number and a secret key.'''

    return hashlib.sha256( secret_key + NU_ID.encode() ).hexdigest()


def generate_maths():
    '''Generate expressions to evaluate. These are generated randomly with format "n1 op n2", where n1 and n2 are random( 1 , 1000 ) and op is an arithmetic operator.'''
    
    # Generate operations.
    list_of_operations = [ '+' , '-' , '*' , '/' ]
    selected_operation = random.choice( list_of_operations )
    left_value = random.randint( 1 , 1000 )
    right_value = random.randint( 1 , 1000 )

    # Generate expression string.
    expression_string = str( left_value ) + " " + selected_operation + " " + str( right_value )

    # Generate expected solution.
    if selected_operation == '+':
        expected_solution = left_value + right_value
    elif selected_operation == '-':
        expected_solution = left_value - right_value
    elif selected_operation == '*':
        expected_solution = left_value * right_value
    elif selected_operation == '/':
        expected_solution = left_value / right_value

    # Output contains the expression and expected solution.
    output = expression_string , expected_solution
    return output


def run_server( server_port ):
     '''Set up server sockets and handle incoming communications.'''

     # Create TCP socket.
     server_socket = socket.socket( socket.AF_INET , socket.SOCK_STREAM )

     # Bind socket to IP address.
     server_socket.bind( ( server_hostname , server_port ) )
     server_socket.listen( 25 )

     # Start accepting client's requests.
     print( "Server ready to accept connections. Waiting..." )
     connection_socket , address = server_socket.accept()
     print( "Connection has been established | " + "Hostname: " + address[ 0 ] + " | Port: " + str( address[ 1 ] ) )

     # Try to receive and decode message from client.
     message_from_client = ""
     try:
          message_from_client = connection_socket.recv( buffer_size ).decode( 'utf-8' )
     except:
          print( "Error: unable to receive message from client." )

     # If message from client is not empty, then proceed. Otherwise, send FAIL message.
     if message_from_client:

          # Tokenise message from client.
          list_of_tokens = message_from_client.split()

          # If message from client is of INTR type, then proceed. Otherwise, send FAIL message.
          if list_of_tokens[ 0 ] == "EECE7374" and list_of_tokens[ 1 ] == "INTR":

               # Extract NU ID from message.
               NU_ID = list_of_tokens[ 2 ]

               # Generate secret flag. This will be sent to the client if the rest of the communication is successful.
               secret_flag = generate_secret_flag( NU_ID )

               # Generate the series of expressions, one at a time.
               # Send each as a separate message to the client.
               # Wait for a response before sending the next message.
               # If the client sends back an incorrect response, break the loop.
               for iteration in range( number_of_expressions ):
                    
                    # Generate expression.
                    expression , solution = generate_maths()

                    # Put expression into an EXPR message, and send.
                    message_to_client = "EECE7374 EXPR " + expression
                    try:
                         connection_socket.send( message_to_client.encode( 'utf-8' ) )
                    except:
                         print( "Error: unable to send message to client. | " + message_to_client )
                         break

                    # Wait to receive and decode message from client.
                    message_from_client = ""
                    try:
                         message_from_client = connection_socket.recv( buffer_size ).decode( 'utf-8' )
                    except:
                         print( "Error: unable to receive message from client." )
                    
                    # If message from client is not empty, then proceed. Otherwise, send FAIL message and break loop.
                    if message_from_client:

                         # Tokenise message from client.
                         list_of_tokens = message_from_client.split()

                         # Check if message from server has the correct header.
                         if list_of_tokens[ 0 ] == "EECE7374":

                              # If message from client is of RSLT type, check the solution. Otherwise, send FAIL message and break loop.
                              if list_of_tokens[ 1 ] == "RSLT":

                                   # Extract result from message.
                                   received_result = list_of_tokens[ 2 ]

                                   # If the extracted solution is incorrect, then send FAIL message and break loop.
                                   if str( solution ) != received_result:

                                        print( "Error: received result is incorrect. | Expected: " + str( solution ) + " | Received: " + received_result )
                                        message_to_client = "EECE7374 FAIL"

                                        try:
                                             connection_socket.send( message_to_client.encode( 'utf-8' ) )
                                        except:
                                             print( "Error: unable to send message to client. | " + message_to_client )

                                        break

                              else:

                                   print( "Error: message is not of RSLT type. | " + message_from_client )
                                   message_to_client = "EECE7374 FAIL"

                                   try:
                                        connection_socket.send( message_to_client.encode( 'utf-8' ) )
                                   except:
                                        print( "Error: unable to send message to client. | " + message_to_client )

                                   break

                         else:

                              print( "Error: message does not have EECE7374 header. | " + message_from_client )
                              message_to_client = "EECE7374 FAIL"

                              try:
                                   connection_socket.send( message_to_client.encode( 'utf-8' ) )
                              except:
                                   print( "Error: unable to send message to client. | " + message_to_client )

                              break

                    else:

                         message_to_client = "EECE7374 FAIL"

                         try:
                              connection_socket.send( message_to_client.encode( 'utf-8' ) )
                         except:
                              print( "Error: unable to send message to client. | " + message_to_client )

                         break

               # If client has successfully sent all correct results, then send SUCC message and secret flag.
               if iteration == ( number_of_expressions - 1 ):
                    print( "Success! | " + NU_ID + " -> " + secret_flag )
                    message_to_client = "EECE7374 SUCC " + secret_flag
                    connection_socket.send( message_to_client.encode( 'utf-8' ) )

     else:

          # If message from client does not match INTR format, then send FAIL message.
          print( "Error: initial message is not of INTR type." | message_from_client )
          message_to_client = "EECE7374 FAIL"

          try:
               connection_socket.send( message_to_client.encode( 'utf-8' ) )
          except:
               print( "Error: unable to send message to client. | " + message_to_client )

     # Close sockets.
     connection_socket.close()
     print( "Connection is closed. | Hostname: " + address[ 0 ] + " | Port: " + str( address[ 1 ] ) )
     server_socket.close()
     print( "Server ready to terminate. Goodbye..." )


### Program entry point.
if __name__ == '__main__':
    if len( sys.argv ) > 1:
        run_server( int( sys.argv[ 1 ] ) )
    else:
        run_server( default_server_port )