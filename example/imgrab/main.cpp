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
#include "json.hpp"

// ----------------------------------------------------------------------------
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// ----------------------------------------------------------------------------
using namespace cv;
using json = nlohmann::json;

// ----------------------------------------------------------------------------
static ps3cam cam;
static int32_t rect_x1 = 0;
static int32_t rect_x2 = 0;
static int32_t rect_y1 = 0;
static int32_t rect_y2 = 0;
static int s_interrupted = 0;
static bool rect_valid = false;

// ----------------------------------------------------------------------------
static float fps_calc();
static void s_catch_signals(void);
static void cmd_handler(json& cmd);
static void s_signal_handler(int signal_value);

// ----------------------------------------------------------------------------
int main(int argc, char const *argv[])
{
  if(argc < 5)
  {
    printf("\n");
    printf("Missing Arguments!\r\n");
    printf("\n");
    printf("Usage Example\n");
    printf("  %s WIDTH HEIGHT FPS JPEG_COMPRESSION\r\n",argv[0]);
    return -1;
  }

  // ...
  int32_t width = atoi(argv[1]);
  int32_t height = atoi(argv[2]);
  int32_t fps = atoi(argv[3]);
  int32_t jpeg_compression = atoi(argv[4]);
  printf("\n");
  printf("Parameters\n");
  printf("  Width: %d Height: %d FPS: %d JPG Compression: %d\n", 
    width, height, fps, jpeg_compression
  );

  if(cam.open(0, width, height, fps, true) == -1)
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
  printf("\n");
  printf("Streaming from port %d ...\n", 8002);

  // ...
  uint8_t cmd_buff[1024];
  zsock_t* cmd_sock = zsock_new_pair("@tcp://*:8003");
  void* cmd_sock_raw = zsock_resolve(cmd_sock);
  printf("Listening commands from port %d ...\n\n", 8003);

  // Create memory linked opencv frame object
  Mat cvFrame(Size(width, height), CV_8UC3, (void*)cam.frame);
  std::vector<uint8_t> compimg;

  // ...
  s_catch_signals();
  while(true)
  {
    // ...
    int32_t cmdlen = zmq_recv(cmd_sock_raw, &cmd_buff, 256, ZMQ_DONTWAIT);
    if(cmdlen > 0)
    {
      cmd_buff[cmdlen] = 0;
      try
      {
        json cmd_json = json::parse(cmd_buff);
        cmd_handler(cmd_json);
      }
      catch(int e)
      {
        printf("JSON parse problem! Err %d\n", e); 
      }
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

    // Overlay some text
    Point pt_text(5,20);
    char status_text[32];
    Scalar color_text(255,255,255);
    snprintf(status_text, sizeof(status_text), "FPS: %4.1f", fps_calc());
    putText(cvFrame, status_text, pt_text, FONT_HERSHEY_PLAIN, 1.0, color_text);

    // ...
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(jpeg_compression);
    imencode(".jpg", cvFrame, compimg, compression_params);

    // Transmit the compressed camera frame over network
    zmq_send(
      inpFrame_sock_raw, compimg.data(), 
      compimg.size(), ZMQ_DONTWAIT
    );

    // Exit handling
    if(s_interrupted)
    {
      printf("\n");
      printf("Interrupt received. Exiting.\n");
      break;
    }
  }

  // ...
  zsock_destroy(&inpFrame_sock);
  return 0;
}

// ----------------------------------------------------------------------------
static float fps_calc()
{
  static int32_t frame_counter = 0;  
  static auto start = chrono::system_clock::now();
  frame_counter += 1;
  auto end = chrono::system_clock::now();
  auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
  float fps_calculated = (float)frame_counter /(elapsed.count() / 1000.0);
  return fps_calculated;
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
static void cmd_handler(json& cmd)
{
  try
  {
    // ----------------------------------------------------------------------
    // Mouse click event
    if(cmd["type"] == "mouse")
    {
      uint8_t isMouseDownEvent = cmd["isClicked"];
      uint8_t isLeftButton = cmd["left_click"];
      uint8_t isMiddleButton = cmd["mid_click"];
      uint8_t isRightButton = cmd["right_click"];
      int16_t mouseX = cmd["x_pos"];
      int16_t mouseY = cmd["y_pos"];

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
    }
    // ----------------------------------------------------------------------
    // Keyboard event
    else if(cmd["type"] == "key")
    {
      uint8_t isKeyPressedEvent = cmd["isDown"];
      uint8_t key = cmd["key"];

      // ...
      printf("keyDown: %d key: %d\n", isKeyPressedEvent, key);
    }
  }
  catch(int e)
  {
    printf("Command parse issue\n");
  }
}