#ifndef CAPTURE_SOMAGIC_H_
#define CAPTURE_SOMAGIC_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "base/base.h"
#include "gui/unique_sdl_surface.h"
#include "capture/source.h"

class Screen;

// TODO(mayah): Make this singleton?
class SomagicSource : public Source {
public:
    // fps must be 60 or 30.
    explicit SomagicSource(const char* name);
    virtual ~SomagicSource();

    virtual UniqueSDLSurface getNextFrame() override;

    virtual bool start() override;

    // Sets the current image. This is YUV422 format.
    // each line has 1440 bytes. Maybe 480 lines. (interlace)
    void setBuffer(const unsigned char* buf, int size);

private:
    std::thread th_;

    std::mutex mu_;
    std::condition_variable cond_;

    std::unique_ptr<int[]> currentPixelData_;
};

#endif
