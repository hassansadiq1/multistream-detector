#
#   Hello World client in Python
#   Connects REQ socket to tcp://localhost:5555
#   Sends "Hello" to server, expects "World" back
#

import zmq
import time
context = zmq.Context()

#  Socket to talk to server
print("Connecting to c++ server…")
socket = context.socket(zmq.PAIR)
socket.connect("tcp://127.0.0.1:5555")

#  Do 10 requests, waiting each time for a response
while(1):
    message = socket.recv()
    print("Received message %s" % message)