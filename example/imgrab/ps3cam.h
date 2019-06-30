// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include <vector>
#include <chrono>
#include <cstdint>
#include <iostream>
#include "ps3eye.h"
#include <stdio.h>
#include <stdlib.h>

// ----------------------------------------------------------------------------
using namespace std;
using namespace ps3eye;
 
// ----------------------------------------------------------------------------
class ps3cam{

  public:

    // ...
    uint8_t* frame;
    int32_t frameSize;
    PS3EYECam::PS3EYERef eye;

    // ...
    ps3cam()
    {

    }

    // ------------------------------------------------------------------------
    // Valid FPS values for 640x480 resolution:
    //   2, 3, 5, 8, 10, 15, 20, 25, 30, 40, 50, 60, 75
    // ------------------------------------------------------------------------
    // Valid FPS values for 320x240 resolution:
    //   2, 3, 5, 7, 10, 12, 15, 17, 30, 37, 40, 50, 60,
    //   75, 90, 100, 125, 137, 150, 187
    // ------------------------------------------------------------------------
    int32_t open(
      int32_t idx, int32_t width, int32_t height, int32_t fps, bool isColor
    )
    {
      // ...
      vector<PS3EYECam::PS3EYERef> devices(PS3EYECam::getDevices());
      if(devices.empty())
      {
        cout << "No PS3 eye camera found! Try sudo?" << endl;
        return -1;
      }

      // ...
      bool rv;
      eye = devices.at(idx);
      if(isColor)
      {
        rv = eye->init(width, height, fps, PS3EYECam::EOutputFormat::BGR);
      }
      else
      {
        rv = eye->init(width, height, fps, PS3EYECam::EOutputFormat::Gray);
      }

      // ...
      if(rv == false)
      {
        cout << "Invalid resolution or FPS!" << endl;
        return -1;
      }

      // Start with auto adjustment mode
      eye->start();
      eye->setAutogain(true);
      eye->setAutoWhiteBalance(true);

      // Create video frame memory
      if(isColor)
      {
        this->frameSize = eye->getWidth() * eye->getHeight() * 3;
        this->frame = (uint8_t*)malloc(eye->getWidth() * eye->getHeight() * 3);        
      }
      else
      {
        this->frameSize = eye->getWidth() * eye->getHeight() * 1;
        this->frame = (uint8_t*)malloc(eye->getWidth() * eye->getHeight() * 1);        
      }
      return 0;
    }
 
    // ...
    void updateFrame()
    {
      eye->getFrame(this->frame);
    }
};
