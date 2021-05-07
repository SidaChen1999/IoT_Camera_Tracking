from __future__ import print_function
import cv2 as cv
import argparse
import numpy as np
import os
import sys
import traceback
from Socket import app_client as client
import func
import Serial

dirname = os.path.dirname(__file__)
filename1 = os.path.join(dirname, 'haarcascades/haarcascade_frontalface_alt2.xml')
filename2 = os.path.join(dirname, 'haarcascades/haarcascade_eye_tree_eyeglasses.xml')
facesCenter = []
faceGray = []

parser = argparse.ArgumentParser(description='Code for Cascade Classifier tutorial.')
parser.add_argument('--face_cascade',
        help='Path to face cascade.',
        default=filename1)
parser.add_argument('--eyes_cascade',
        help='Path to eyes cascade.',
        default=filename2)
parser.add_argument('--camera', help='Camera divide number.', type=int, default=1)
args = parser.parse_args()
face_cascade_name = args.face_cascade
eyes_cascade_name = args.eyes_cascade
face_cascade = cv.CascadeClassifier()
eyes_cascade = cv.CascadeClassifier()
#-- 1. Load the cascades
if not face_cascade.load(cv.samples.findFile(face_cascade_name)):
    print('--(!)Error loading face cascade')
    exit(0)
if not eyes_cascade.load(cv.samples.findFile(eyes_cascade_name)):
    print('--(!)Error loading eyes cascade')
    exit(0)
# camera_device = args.camera
cap = cv.VideoCapture(1)
# cv.CAP_DSHOW
# cap = cv.VideoCapture(camera_device)
cap.set(cv.CAP_PROP_FRAME_WIDTH, 1280)
cap.set(cv.CAP_PROP_FRAME_HEIGHT, 720)
if not cap.isOpened:
    print('--(!)Error opening video capture')
    exit(0)

image_id = 0
try:
    while True:  
        ret, frame = cap.read()
        if frame is None:
            print('--(!) No captured frame -- Break!')
            break
        func.detectAndDisplay(frame, facesCenter, faceGray, face_cascade, eyes_cascade)
        ang = func.pix2ang(facesCenter)
        if ang[0] != -1:
            operation = func.ang2op(ang)
            print('Op: ', ang)
            Serial.write_read('x'+str(operation[0]))
            Serial.write_read('y'+str(operation[1]))
            
            reshaped = []
            if len(faceGray) > 4:
                faceGray = faceGray[:4]
            for i in range(len(faceGray)):
                reshaped.append(faceGray[i].reshape(32*32).astype(np.ubyte).tobytes())
                cv.imwrite('images/'+str(image_id)+'.jpg', faceGray[i])
                image_id += 1

            events = client.sel.select(timeout=1)
            for key, mask in events:
                message = key.data
                try:
                    for i in range(len(reshaped)):
                        # message._send_buffer = message._send_buffer[1024:]
                        message._send_buffer += reshaped[i]
                        # print('Len: ', len(message._send_buffer))
                    message._write()
                except Exception:
                    print(
                        "main: error: exception for",
                        f"{message.addr}:\n{traceback.format_exc()}",
                    )
                    message.close()
            # Check for a socket being monitored to continue.
            if not client.sel.get_map():
                break

        if cv.waitKey(10) == 27:
            break

except KeyboardInterrupt:
    print("caught keyboard interrupt, exiting")
finally:
    client.sel.close()