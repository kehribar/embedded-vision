# -----------------------------------------------------------------------------
# 
# 
# -----------------------------------------------------------------------------
import sys
import zmq
import cv2
import time
import pygame
import numpy as np
from packetUtils import *

# ...
WIDTH = int(sys.argv[2])
HEIGHT = int(sys.argv[3])
if(sys.argv[4] == 'gray'):
  DEPTH = 8
else:
  DEPTH = 24
imgff = np.zeros((WIDTH, HEIGHT, 3), dtype=np.uint8)

# ...
context = zmq.Context()

#  Socket to talk to server
imsock = context.socket(zmq.SUB)
imsock.connect("tcp://%s:8002" % sys.argv[1])
imsock.setsockopt(zmq.SUBSCRIBE, b"")

# Socket to send command
cmdsock = context.socket(zmq.PAIR)
cmdsock.connect("tcp://%s:8003" % sys.argv[1])

# ...
pygame.init()
pygame.display.set_caption("imgrab client")
screen = pygame.display.set_mode((WIDTH,HEIGHT), pygame.DOUBLEBUF)
camera_surface = pygame.surface.Surface((WIDTH,HEIGHT), 0, DEPTH).convert()

# ...
prev = time.time()
cmdbuf = np.zeros(256, np.uint8)
while True:

  # Event handling and command transition
  ev = pygame.event.get()
  for event in ev:
    if event.type == pygame.MOUSEBUTTONDOWN:
      pos = pygame.mouse.get_pos()
      btn = pygame.mouse.get_pressed()
      cmdbuf[0] = 0
      cmdbuf[1] = 1
      cmdbuf[2] = btn[0]
      cmdbuf[3] = btn[1]
      cmdbuf[4] = btn[2]
      cmdbuf[5:7] = put16b(pos[0])
      cmdbuf[7:9] = put16b(pos[1])
      cmdsock.send(cmdbuf[0:9])
    elif event.type == pygame.MOUSEBUTTONUP:
      pos = pygame.mouse.get_pos()
      btn = pygame.mouse.get_pressed()
      cmdbuf[0] = 0
      cmdbuf[1] = 0
      cmdbuf[2] = btn[0]
      cmdbuf[3] = btn[1]
      cmdbuf[4] = btn[2]
      cmdbuf[5:7] = put16b(pos[0])
      cmdbuf[7:9] = put16b(pos[1])
      cmdsock.send(cmdbuf[0:9])
    elif event.type == pygame.KEYDOWN:
      cmdbuf[0] = 1
      cmdbuf[1] = 1
      cmdbuf[2] = event.key
      cmdsock.send(cmdbuf[0:3])
    elif event.type == pygame.KEYUP:
      cmdbuf[0] = 1
      cmdbuf[1] = 0
      cmdbuf[2] = event.key
      cmdsock.send(cmdbuf[0:3])

  # Image visualisation
  dat = imsock.recv()
  dat_np = np.frombuffer(dat, np.uint8)
  img = cv2.imdecode(dat_np, cv2.IMREAD_COLOR)
  if(sys.argv[4] == 'gray'):
    dat_np = np.frombuffer(dat, np.uint8).reshape(HEIGHT, WIDTH).T
    cv2.cvtColor(dat_np, cv2.COLOR_GRAY2BGR, dst=imgff)
    pygame.surfarray.blit_array(camera_surface, imgff)
  else:
    dat_np = np.frombuffer(img, np.uint8).reshape(HEIGHT, WIDTH, 3).transpose(1,0,2)
    cv2.cvtColor(dat_np, cv2.COLOR_RGB2BGR, dst=imgff)
    pygame.surfarray.blit_array(camera_surface, imgff)
  screen.blit(camera_surface, (0, 0))
  pygame.display.flip()
  now = time.time()
  dt = now - prev
  prev = now 

  # Analytics 
  if False:
    print(str(round((dt * 1000.0), 2)) + " ms")
