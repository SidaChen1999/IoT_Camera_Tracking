from __future__ import print_function
import cv2 as cv
import argparse
import numpy as np

def detectAndDisplay(frame, facesCenter, faceGray, face_cascade, eyes_cascade):
    frame_gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)
    frame_gray = cv.equalizeHist(frame_gray)
    facesCenter[:] = []
    faceGray[:] = []
    #-- Detect faces
    faces = face_cascade.detectMultiScale(frame_gray)
    for (x,y,w,h) in faces:
        center = (x + w//2, y + h//2)
        frame = cv.ellipse(frame, center, (w//2, h//2), 0, 0, 360, (255, 0, 255), 4)
        faceROI = frame_gray[y:y+h,x:x+w]
        if w > h:
            singleFace = frame_gray[y+h//2-w//2:y+h//2+w//2, x:x+w]
        else:
            singleFace = frame_gray[y:y+h, x+w//2-h//2:x+w//2+h//2]
        #-- In each face, detect eyes
        eyes = eyes_cascade.detectMultiScale(faceROI)
        if len(eyes) != 0:
            facesCenter.append(center)
            resizedFace = cv.resize(singleFace, (32,32))
            faceGray.append(resizedFace)
        for (x2,y2,w2,h2) in eyes:
            eye_center = (x + x2 + w2//2, y + y2 + h2//2)
            radius = int(round((w2 + h2)*0.25))
            frame = cv.circle(frame, eye_center, radius, (255, 0, 0 ), 4)
    # print('x: ', int(pix2ang(facesCenter)[0]))
    if(len(facesCenter) != 0):
        target = np.mean(facesCenter, 0)
        frame = cv.circle(frame,
            (int(target[0]), int(target[1])),
            10, (255, 255, 0 ), 4)
    cv.imshow('Capture - Face detection', frame)
    # for i in range(0, len(faceGray)):
    #     face = cv.UMat(np.array(faceGray[i], dtype=np.uint8))
    #     cv.imshow('Faces detected', cv.resize(face, (128,128)))

pix2ang_x = 46.8476*2/1280
pix2ang_y = 30.9638*2/720
imageCenter = (1280/2, 720/2)
def pix2ang(facesCenter):
    if(len(facesCenter) == 0):
        return (-1, -1)
    target = np.mean(facesCenter, 0)
    ang_x = pix2ang_x * (target[0] - imageCenter[0])
    ang_y = pix2ang_y * (target[1] - imageCenter[1])
    return (ang_x, ang_y)

def ang2op(ang):
    actualAngel_x = round((ang[0] / 47 + 0.5) * 256).astype(np.int)
    actualAngel_y = round((ang[1] / 31 + 0.5)* 256).astype(np.int)
    return [actualAngel_x, actualAngel_y]