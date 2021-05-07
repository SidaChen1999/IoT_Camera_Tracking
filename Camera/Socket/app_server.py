#!/usr/bin/env python3

import sys
import socket
import selectors
import traceback
import cv2 as cv
import numpy as np

import libserver

def accept_wrapper(sock):
    conn, addr = sock.accept()  # Should be ready to read
    print("accepted connection from", addr)
    conn.setblocking(False)
    message = libserver.Message(sel, conn, addr)
    sel.register(conn, selectors.EVENT_READ, data=message)

host = socket.gethostbyname(socket.gethostname())
port = 65432
sel = selectors.DefaultSelector()
lsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Avoid bind() exception: OSError: [Errno 48] Address already in use
lsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
lsock.bind((host, port))
lsock.listen()
print("listening on", (host, port))
lsock.setblocking(False)
sel.register(lsock, selectors.EVENT_READ, data=None)

windowNo = 0
try:
    while True:
        events = sel.select(timeout=None)
        for key, mask in events:
            if key.data is None:
                accept_wrapper(key.fileobj)
            else:
                message = key.data
                try:
                    message.process_events(mask)
                    reshaped = list(message._recv_buffer)
                    message._recv_buffer = message._recv_buffer[len(message._recv_buffer):]
                    face = np.array(reshaped, dtype=np.uint8)
                    if len(face) % 1024 != 0:
                        continue
                    Suspect = face.reshape((-1,32,32))

                    for i in range(len(reshaped) // 1024):
                        face = cv.UMat(Suspect[i])
                        cv.imshow('Suspected face: '+str(i), cv.resize(face, (256,256)))
                    if windowNo != 0:
                        for i in range(len(reshaped)//1024, windowNo):
                            cv.destroyWindow('Suspected face: '+str(i))
                    windowNo = len(reshaped) // 1024
                    

                except Exception:
                    print(
                        "main: error: exception for",
                        f"{message.addr}:\n{traceback.format_exc()}",
                    )
                    cv.destroyAllWindows()
                    message.close()
                except KeyboardInterrupt:
                    break

        if cv.waitKey(10) == 27:
            break

except KeyboardInterrupt:
    print("caught keyboard interrupt, exiting")
finally:
    sel.close()
