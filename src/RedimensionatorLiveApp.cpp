/**
 *
 * Redimensionator Live
 * The Webcam Slit Scan Dimension Trader
 *
 * Copyleft 2011 
 * Forrest Oliphant, Sembiki Interactive, http://sembiki.com/
 * 
 * Made for Computational Photography class at Media Lab Helsinki, http://mlab.taik.fi/
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *
 **/

#include "cinder/app/AppBasic.h"
#include "cinder/Surface.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Text.h"
#include "cinder/Capture.h"

using namespace ci;
using namespace ci::app;
using std::string;

class RedimensionatorLive : public AppBasic {
public:
  void prepareSettings(Settings* settings);
  void setup();
  void update();
  void draw();
  void shutdown();
  
private:
  Capture capture;
  Surface inSurface;
  Surface stillSurface;
  Surface motionSurface;

  int frame;
  Surface* frames;
  Surface* frames2;
  
  int video_width;
  int video_height;
  int video_slice_x1;
  int video_slice_x2;
  int video_slice_x3;
  int output_width;
  int window_width;
  int window_height;
  int draw_position_x;
  bool newFrame;
  bool process_motion;
  bool show_progress;
};

void RedimensionatorLive::prepareSettings( Settings *settings )
{
  video_width = 640;
  video_height = 320;
  video_slice_x1 = 0;
  video_slice_x2 = video_width / 2;
  video_slice_x3 = video_width - 1;
  
  output_width = video_width;
  
  window_width = output_width;
  window_height = video_height*2;
  newFrame = false;
  
  inSurface = Surface(video_width, video_height, ci::SurfaceChannelOrder::RGB);
  stillSurface = Surface(output_width, video_height, ci::SurfaceChannelOrder::RGB);
  motionSurface = Surface(output_width, video_height, ci::SurfaceChannelOrder::RGB);
  
  frame = 0;
  frames = new Surface[output_width];
  frames2 = new Surface[output_width];
  process_motion = false;
  
  show_progress = true;
  
  settings->setWindowSize(window_width, window_height);
  settings->setFullScreen( false );
  settings->setResizable( false );
}

void RedimensionatorLive::setup()
{
  try {
    capture = Capture(video_width, video_height);
    capture.start();
  } catch (...) {
    // if we threw in the start close the program 
    ci::app::console() << "Unable to open webcam." << std::endl;
    exit(1);
  }
  
  if (capture.getWidth() != video_width || capture.getHeight() != video_height) {
    ci::app::console() << "Unable to open webcam at desired size." << std::endl;
    exit(1);
  }  
    
}

void RedimensionatorLive::update()
{  
  newFrame = false;
  
  if (!capture || !capture.checkNewFrame()) 
    return;
  
  newFrame = true;
  
  inSurface = capture.getSurface();
  
  frames[frame] = inSurface.clone();
  frame++;
  if (frame >= output_width) {
    // Copy the buffer to a new buffer to use for the motion side
    for (int i=0; i<output_width; ++i) {
      frames2[i] = frames[i];
    }
    frame = 0;
    process_motion = true;
  }
  
  // Still slit-scan preview
  stillSurface.copyFrom(stillSurface, Area( 1, 0, output_width, video_height ), Vec2i( -1, 0 ) );
  stillSurface.copyFrom(inSurface, Area( video_slice_x1, 0, video_slice_x1+1, video_height ), Vec2i( output_width-1, 0 ) );
  stillSurface.copyFrom(inSurface, Area( video_slice_x2, 0, video_slice_x2+1, video_height ), Vec2i( output_width/3*2 - video_slice_x2, 0 ) );
  stillSurface.copyFrom(inSurface, Area( video_slice_x3, 0, video_slice_x3+1, video_height ), Vec2i( output_width/3 - video_slice_x3, 0 ) );
  
  // Motion slit-scan
  if (process_motion) {
    for (int i=0; i<output_width; ++i) {
      motionSurface.copyFrom(frames2[i], Area(frame, 0, frame+1, video_height), Vec2i(i-frame, 0));
    }
    //console() << frame << std::endl;
  }
  
}

void RedimensionatorLive::draw()
{
  if (!newFrame)
    return;
  
  if( stillSurface )
    gl::draw( gl::Texture( stillSurface ) ); 
  if( motionSurface )
    gl::draw( gl::Texture( motionSurface ), Vec2f( 0, video_height ) ); 
  
  if(show_progress) 
    gl::drawSolidRect(Rectf(0, video_height-1, frame, video_height));

  
}

void RedimensionatorLive::shutdown() {
  delete [] frames;
  delete [] frames2;
}

CINDER_APP_BASIC( RedimensionatorLive, RendererGl )