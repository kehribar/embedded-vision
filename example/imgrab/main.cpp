// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include <stdint.h>
#include "ps3cam.h"
#include <chrono>
#include <signal.h>
#include <stdio.h>
#include <czmq.h>
#include <zsock.h>
#include "packet_utils.h"

// ----------------------------------------------------------------------------
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// ----------------------------------------------------------------------------
using namespace cv;

// ----------------------------------------------------------------------------
static ps3cam cam;
static int32_t rect_x1 = 0;
static int32_t rect_x2 = 0;
static int32_t rect_y1 = 0;
static int32_t rect_y2 = 0;
static int s_interrupted = 0;
static bool rect_valid = false;

// ----------------------------------------------------------------------------
static void fps_calc();
static void s_catch_signals(void);
static void s_signal_handler(int signal_value);
static void cmd_handler(uint8_t* buff, int32_t bufflen);

// ----------------------------------------------------------------------------
int main(int argc, char const *argv[])
{
  if(cam.open(0, 640, 480, 60, true) == -1)
  {
    return -1;
  }

  int major;
  int minor;
  int patch;
  zmq_version(&major, &minor, &patch);
  printf("\n");
  printf("Current 0MQ version is %d.%d.%d\n", major, minor, patch);

  // ...
  zsock_t* inpFrame_sock = zsock_new_pub("@tcp://*:8002");
  void* inpFrame_sock_raw = zsock_resolve(inpFrame_sock);
  printf("Streaming from port %d ...\n", 8002);

  // ...
  uint8_t cmd_buff[256];
  zsock_t* cmd_sock = zsock_new_sub("tcp://127.0.0.1:8003", "");
  void* cmd_sock_raw = zsock_resolve(cmd_sock);
  printf("Listening commands from port %d ...\n", 8003);

  // Create memory linked opencv frame object
  Mat cvFrame(Size(640, 480), CV_8UC3, (void*)cam.frame);

  // ...
  s_catch_signals();
  while(true)
  {
    // ...
    int32_t cmdlen = zmq_recv(cmd_sock_raw, &cmd_buff, 256, ZMQ_DONTWAIT);
    if(cmdlen > 0)
    {
      cmd_handler(cmd_buff, cmdlen);
    }

    // Grab a frame from camera
    cam.updateFrame();

    // Overlay a rectangle shape onto the camera
    if(rect_valid)
    {
      Scalar color(0,0,255);
      Point pt1(rect_x1, rect_y1);
      Point pt2(rect_x2, rect_y2);
      rectangle(cvFrame, pt1, pt2, color);
    }

    // Transmit the raw camera frame over network
    zmq_send(
      inpFrame_sock_raw, cam.frame, 
      cam.frameSize, ZMQ_DONTWAIT
    );

    // Exit handling
    if(s_interrupted)
    {
      printf("\n");
      printf("Interrupt received. Exiting.\n");
      break;
    }

    // ...
    fps_calc();
  }

  // ...
  zsock_destroy(&inpFrame_sock);
  return 0;
}

// ----------------------------------------------------------------------------
static void fps_calc()
{
  static int32_t frame_counter = 0;  
  static auto start = chrono::system_clock::now();
  frame_counter += 1;
  auto end = chrono::system_clock::now();
  auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
  float fps_calculated =(float)frame_counter /(elapsed.count() / 1000.0);
  static int32_t m_cnt = 0;
  if(m_cnt++ == 240)
  {
    m_cnt = 0;
    printf("FPS: %.2f\n", fps_calculated);
  }
}

// ----------------------------------------------------------------------------
static void s_signal_handler(int signal_value)
{
  s_interrupted = 1;
}

// ----------------------------------------------------------------------------
static void s_catch_signals(void)
{
  struct sigaction action;
  action.sa_handler = s_signal_handler;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
}

// ----------------------------------------------------------------------------
static void cmd_handler(uint8_t* buff, int32_t bufflen)
{
  switch(buff[0])
  {
    // ------------------------------------------------------------------------
    // Mouse click event
    case 0:
    {
      uint8_t isMouseDownEvent = buff[1];
      uint8_t isLeftButton = buff[2];
      uint8_t isMiddleButton = buff[3];
      uint8_t isRightButton = buff[4];
      int16_t mouseX = (int16_t)make16b(buff, 5);
      int16_t mouseY = (int16_t)make16b(buff, 7);

      // ...
      static uint8_t draw_ongoing = false;
      if(isMouseDownEvent)
      {
        if(isLeftButton)
        {
          rect_x1 = mouseX;
          rect_y1 = mouseY;
          rect_valid = false;
          draw_ongoing = true;
        }
        else if(isMiddleButton)
        {
          rect_valid = false;
        }
      }
      else if(draw_ongoing)
      {
        rect_x2 = mouseX;
        rect_y2 = mouseY;
        rect_valid = true;
        draw_ongoing = false;
      }
      
      // ...
      printf("mouse_x: %3d mouse_y: %3d\n", mouseX, mouseY);
      printf("mouseDown: %d: left: %d middle: %d right: %d\n", 
        isMouseDownEvent, isLeftButton, isMiddleButton, isRightButton
      );
      break;
    }
    // ------------------------------------------------------------------------
    // Keyboard event
    case 1:
    {
      uint8_t isKeyPressedEvent = buff[1];
      uint8_t key = buff[2];

      // ...
      printf("keyDown: %d key: %d\n", isKeyPressedEvent, key);
      break;
    }
    // ------------------------------------------------------------------------
    default:
    {
      printf("[cmd_handler]: Unknown command?\n");
      break;
    }
  }
}