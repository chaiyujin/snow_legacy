#pragma once
#include <snow.h>
#include <string>
#include <vector>
#include "biwi_obj.h"

typedef std::vector<int16_t> S16Signal;

/* Image will scroll at row */
struct ScrollImage {
    std::vector<uint8_t> mImg;
    int mRows, mCols;
    int mWinLength;     // (row) window length of scroll image
    int mHopLength;     // (row) hop length of scroll image

    ScrollImage() : mImg(0), mRows(0), mCols(0), mWinLength(0), mHopLength(0) {}
};

struct AudioGroup {
    std::vector<S16Signal>   mSignals;
    std::vector<std::string> mTags;
    int mSampleRate;    // the sample rate of signal
    int mSelectId;      // the selected signal id
    int mCurrentIndex;  // the index of audio data
    int mWantIndex;     // the wanted index

    AudioGroup() { clear(); }
    
    void clear() {
        mSignals.clear();
        mTags.clear();
        mSampleRate = 0;
        mSelectId = 0;
        mCurrentIndex = 0;
        mWantIndex = 0;
    }
};

/* app global
 * - text: the given sentence text.
 * - vector<audio>: the audio list.
 * */
struct PublicData {
    double                  gFPS;
    AudioGroup              gAudioGroup;
    int                     gCurrentFrame;
    int                     gMaxFrames;

    PublicData() { clear(); }

    void clear() {
        gFPS = 0;
        gCurrentFrame = 0;
        gMaxFrames = 0;
        gAudioGroup.clear();
    }
};

/* window's member
 * - obj: 3d model
 * - anime: sequence of vertices
 * - scroll_image: mel image 
 * Each window also show the app global content (keep same)
 * */
struct PrivateData {
    std::string                         mText;
    std::string                         mObjPath;
    std::vector<Vertices>               mAnime;
    std::map<std::string, ScrollImage>  mScrollImageMap;

    PrivateData(): mText(""), mObjPath(""), mAnime(0), mScrollImageMap() {}
};

class VisWindow : public snow::AbstractWindow {
private:
    // ui and play related
    static PublicData    gShared;
    static bool          gAudiable;
    PrivateData          mPrivate;
    // window specific
    ObjMesh             *mObjMeshPtr;
    snow::Shader        *mShaderPtr;
    snow::ArcballCamera *mCameraPtr;

    std::map<std::string, uint32_t> mTextureMap;

    void releaseObj();
    void processEvent(SDL_Event &event);
    void draw();
    void setTexture(std::string title, uint8_t *data, int rows, int cols);

public:

    VisWindow(const char *title="")
        : AbstractWindow(title)
        , mPrivate()
        , mObjMeshPtr(nullptr)
        , mShaderPtr(nullptr)
        , mCameraPtr(nullptr)    
    {}

    static void audioCallback(void *userdata, uint8_t *stream, int len);
    static void openAudio(int sampleRate);
    static void setFPS(double fps) { gShared.gFPS = fps; }
    static void addAudio(std::string tag, const S16Signal &signal, int sampleRate);

    static void terminate();

    void setObj(std::string filename);
    void setAnime(const std::vector<Vertices> &data);
    void addScrollImage(std::string title, const ScrollImage &scrollImage);
    void setText(std::string sentence) { mPrivate.mText = sentence; }
};

class Application {
private:
    static snow::App                         *gAppPtr;
    static std::map<std::string, VisWindow *> gWindowMap;
    static VisWindow *getWindow(std::string name, bool create=false) {
        VisWindow *ret = nullptr;
        auto it = gWindowMap.find(name);
        if (it != gWindowMap.end()) {
            ret = it->second;
        }
        else if (create) {
            // create on missing
            ret = new VisWindow(name.c_str());
            // gWindowMap.insert(std::pair<std::string, VisWindow *>(name, ret));
            gWindowMap.insert({name, ret});
        }
        return ret;
    }
public:
    static void newAPP() {
        terminate();
        gAppPtr = new snow::App();
        std::cout << "[AnimeViewer]: > Begin----\n";
    }
    static void terminate() {
        VisWindow::terminate();
        // no need to release window, it will be released by app.
        gWindowMap.clear();
        if (gAppPtr) {
            delete gAppPtr;
            gAppPtr = nullptr;
            std::cout << "[AnimeViewer]: > End----\n";
        }
    }
    // global
    static void run(double fps) {
        VisWindow::setFPS(fps);
        for (auto it = gWindowMap.begin(); it != gWindowMap.end(); ++it) {
            VisWindow *win = it->second;
            gAppPtr->addWindow(win);
        }
        gAppPtr->run();
        terminate();
    }

    static void addAudio(std::string tag, const S16Signal &signal, int sampleRate) {
        VisWindow::openAudio(sampleRate);  // it will return, if opened before.
        VisWindow::addAudio(tag, signal, sampleRate);
    }
    
    /* window private */
    
    static void setText(std::string window, std::string text) {
        if (gAppPtr == nullptr)
            throw std::runtime_error("You forget newAPP() first!");
        VisWindow *win = getWindow(window, true);
        win->setText(text);
    }

    static void setObj(std::string window, std::string filename) {
        if (gAppPtr == nullptr)
            throw std::runtime_error("You forget newAPP() first!");
        VisWindow *win = getWindow(window, true);
        win->setObj(filename);
    }

    static void setAnime(std::string window, const std::vector<Vertices> &anime) {
        if (gAppPtr == nullptr)
            throw std::runtime_error("You forget newAPP() first!");
        VisWindow *win = getWindow(window, true);
        win->setAnime(anime);
    }

    static void addScrollImage(std::string window, std::string title, ScrollImage &scrollImage) {
        if (gAppPtr == nullptr)
            throw std::runtime_error("You forget newAPP() first!");
        VisWindow *win = getWindow(window, true);
        win->addScrollImage(title, scrollImage);
    }
};