#pragma once
#include <snow.h>
#include <string>
#include <vector>
#include "../obj/biwi_obj.h"
#include "../facedb/visualize.h"

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
    std::map<std::string, ScrollImage>  mScrollImageMap;
    // for modeltype: obj
    std::vector<Vertices>               mAnime;     
    // for modeltype: bilinear
    std::vector<double>                 mIden;      
    std::vector<std::vector<double>>    mExprList;
    int                                 mFrames;
    PrivateData()
        : mText(""), mObjPath(""), mScrollImageMap()
        , mAnime(0), mIden(0), mExprList(0), mFrames(0) {}
};

enum ModelType {
    Obj      = 0,
    Bilinear = 1
};

class Application;
class VisWindow : public snow::AbstractWindow {
private:
    friend class Application;
    // ui and play related
    static PublicData    gShared;
    static bool          gAudiable;

    ModelType            mModelType;
    PrivateData          mPrivate;
    // window specific
    snow::Model         *mModelPtr;
    snow::Shader        *mShaderPtr;
    snow::ArcballCamera *mCameraPtr;

    std::map<std::string, uint32_t> mTextureMap;

    void releaseObj();
    void processEvent(SDL_Event &event);
    void setTexture(std::string title, uint8_t *data, int rows, int cols);

public:

    VisWindow(ModelType type, const char *title="")
        : AbstractWindow(title)
        , mModelType(type)
        , mPrivate()
        , mModelPtr(nullptr)
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
    void setIden(const std::vector<double> &iden);
    void setExprList(const std::vector<std::vector<double>> &exprList);

    void draw();
    
    int frames() const { return mPrivate.mFrames; }
    static void SeekBegin() { gShared.gCurrentFrame = 0; }
    static void NextFrame() { gShared.gCurrentFrame ++;  }
};

class Application {
private:
    static ModelType                          gModelType;
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
            ret = new VisWindow(gModelType, name.c_str());
            // gWindowMap.insert(std::pair<std::string, VisWindow *>(name, ret));
            gWindowMap.insert({name, ret});
        }
        return ret;
    }
public:
    static void newAPP(ModelType type) {
        terminate();
        gModelType = type;
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
    // off-screen
    static void offscreen(double fps) {
        if (std::abs(std::round(fps) - fps) > 1e-5)
            throw std::runtime_error("offscreen only support integer fps");
        int frames = 0;
        int w, h;
        VisWindow::SeekBegin();
        for (auto it = gWindowMap.begin(); it != gWindowMap.end(); ++it) {
            VisWindow *win = it->second;
            frames = std::max(frames, win->frames());
            w = win->width();
            h = win->height();
        }
        snow::MediaWriter writer("../../../assets/test_write.mp4");
        writer.addAudioStream(VisWindow::gShared.gAudioGroup.mSampleRate);
        writer.addVideoStream(w, h, 4, (int)fps);
        writer.setAudioData(VisWindow::gShared.gAudioGroup.mSignals[0]);
        writer.start();
        for (int iFrame = 0; iFrame < frames; ++iFrame) {
            // printf("%d\n", iFrame);
            for (auto it = gWindowMap.begin(); it != gWindowMap.end(); ++it) {
                VisWindow *win = it->second;
                snow::Image image;
                win->_draw(&image);
                // snow::Image::Write(std::string("../../../assets/images/frame") + std::to_string(iFrame) + ".png", image);
                writer.appendImage(image);
            }
            VisWindow::NextFrame();
        }
        writer.finish();
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

    static void setIden(std::string window, const std::vector<double> &iden) {
        if (gAppPtr == nullptr)
            throw std::runtime_error("You forget newAPP() first!");
        VisWindow *win = getWindow(window, true);
        win->setIden(iden);
    }

    static void setExprList(std::string window, const std::vector<std::vector<double>> &exprList) {
        if (gAppPtr == nullptr)
            throw std::runtime_error("You forget newAPP() first!");
        VisWindow *win = getWindow(window, true);
        win->setExprList(exprList);
    }
};