import socket
import time
import threading
import sys


def recieve():
    while True:
        data = clit.recv(500).decode()
        print('\x1b[2K',end="")
        print('\x1b[5D'+str(data),end="")
        print('\nMe > ',end="")


def send(uname):
    while True:
        msg = input('Me > ')
        data = uname + ' > ' + msg
        clit.send(data.encode())

if __name__ == "__main__":   


    host = input("please enter an IP Adress : \n")
    port =  input("\nplease enter a port number\n")
    uname = input("\nplease enter your name/computer's name\n")

    clit = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 

    clit.connect((host, int(port)))

    t1 = threading.Thread(target=send,args=[uname])
    t2 = threading.Thread(target=recieve)

    t1.start()
    t2.start()