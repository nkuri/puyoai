#include <gflags/gflags.h>
#include <glog/logging.h>

#include "capture/ac_analyzer.h"
#include "capture/capture.h"
#include "capture/screen_shot_saver.h"
#include "capture/movie_source.h"
#include "capture/movie_source_key_listener.h"
#include "gui/bounding_box_drawer.h"
#include "gui/box.h"
#include "gui/main_window.h"

#include "gui/screen.h"

#include <iostream>

DEFINE_bool(save_screenshot, false, "save screenshot");
DEFINE_bool(draw_result, true, "draw analyzer result");
DEFINE_int32(fps, 30, "FPS. When 0, hitting space will go next step.");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <in-movie>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    MovieSource::init();

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    MovieSource source(argv[1]);
    if (!source.ok()) {
        fprintf(stderr, "Failed to load %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    source.setFPS(FLAGS_fps);

    ACAnalyzer analyzer;
    Capture capture(&source, &analyzer);

    unique_ptr<AnalyzerResultDrawer> analyzerResultDrawer;
    if (FLAGS_draw_result)
        analyzerResultDrawer.reset(new AnalyzerResultDrawer(&capture));

    unique_ptr<ScreenShotSaver> saver;
    if (FLAGS_save_screenshot)
        saver.reset(new ScreenShotSaver);

    unique_ptr<MovieSourceKeyListener> movieSourceKeyListener;
    if (FLAGS_fps == 0) {
        movieSourceKeyListener.reset(new MovieSourceKeyListener(&source));
    }

    MainWindow mainWindow(720, 480, Box(0, 0, 720, 480));
    mainWindow.addDrawer(&capture);
    if (saver.get())
        mainWindow.addDrawer(saver.get());
    if (analyzerResultDrawer.get())
        mainWindow.addDrawer(analyzerResultDrawer.get());
    if (movieSourceKeyListener.get())
        mainWindow.addEventListener(movieSourceKeyListener.get());

    capture.start();

    mainWindow.runMainLoop();

    return 0;
}
