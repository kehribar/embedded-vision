# -----------------------------------------------------------------------------
# 
# 
# -----------------------------------------------------------------------------
import sys
import zmq
import cv2
import time
import json
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
while True:

  # Event handling and command transition
  ev = pygame.event.get()
  for event in ev:
    if event.type == pygame.MOUSEBUTTONDOWN:
      pos = pygame.mouse.get_pos()
      btn = pygame.mouse.get_pressed()
      cmd = {}
      cmd['type'] = 'mouse'
      cmd['isClicked'] = True
      cmd['x_pos'] = pos[0]
      cmd['y_pos'] = pos[1]
      cmd['left_click'] = btn[0]
      cmd['mid_click'] = btn[1]
      cmd['right_click'] = btn[2]
      cmdsock.send_string(json.dumps(cmd))
    elif event.type == pygame.MOUSEBUTTONUP:
      pos = pygame.mouse.get_pos()
      btn = pygame.mouse.get_pressed()
      cmd = {}
      cmd['type'] = 'mouse'
      cmd['isClicked'] = False
      cmd['x_pos'] = pos[0]
      cmd['y_pos'] = pos[1]
      cmd['left_click'] = btn[0]
      cmd['mid_click'] = btn[1]
      cmd['right_click'] = btn[2]
      cmdsock.send_string(json.dumps(cmd))
    elif event.type == pygame.KEYDOWN:
      cmd = {}
      cmd['type'] = 'key'
      cmd['isDown'] = True
      cmd['key'] = event.key
      cmdsock.send_string(json.dumps(cmd))
    elif event.type == pygame.KEYUP:
      cmd = {}
      cmd['type'] = 'key'
      cmd['isDown'] = False
      cmd['key'] = event.key
      cmdsock.send_string(json.dumps(cmd))

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
