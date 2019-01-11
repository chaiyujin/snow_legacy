#include "glsl.h"
#include "window.h"

double gSeconds = 0;
double gLastTime = 0;
bool   gPlaying = false;
double gTimeClock = 0;

std::map<std::string, VisWindow *> Application::gWindowMap;
snow::App *                        Application::gAppPtr = nullptr;
ModelType                          Application::gModelType = ModelType::Obj;

PublicData  VisWindow::gShared;
bool        VisWindow::gAudiable = false;

/* static methods */

void VisWindow::audioCallback(void *voidUserData, uint8_t *stream, int len) {
    // printf("audio cb\n");
    // select signal
    auto &userData = *(AudioGroup*)voidUserData;
    const uint8_t *data = (uint8_t*)userData.mSignals[userData.mSelectId].data();
    // double current index and want index, int16_t -> uint8_t
    int length = (int)userData.mSignals[userData.mSelectId].size() * 2;
    int currIdx = userData.mCurrentIndex * 2;
    int wantIdx = userData.mWantIndex * 2;
    if (len <= 0) return;
    // adjust current_index
    if (abs(currIdx - wantIdx) > userData.mSampleRate / 2)
        currIdx = wantIdx;
    // printf("cur %d len %d length %d ID %d\n",
    //        currIdx, len, length, userData.mSelectId);
    // copy data
    if (currIdx + len > length) {
        // not enough
        int left = length - currIdx;
        if (left > 0) {            
            memcpy(stream, data + currIdx, left);
            currIdx += left;
            len -= left;
        }
        // pad zero
        memset(stream, 0, len);
    }
    else {
        // enough data
        memcpy(stream, data + currIdx, len);
        currIdx += len;
    }
    // update current index uint8_t -> int16_t
    userData.mCurrentIndex = currIdx / 2;
}

void VisWindow::openAudio(int sampleRate) {
    if (gAudiable) return;

    gAudiable = true;
    auto *wantSpec = new SDL_AudioSpec();
    SDL_memset(wantSpec, 0, sizeof(SDL_AudioSpec));
    wantSpec->freq = sampleRate;
    wantSpec->format = AUDIO_S16SYS;
    wantSpec->channels = 1;
    wantSpec->silence = 0;
    wantSpec->samples = 1024;
    wantSpec->callback = VisWindow::audioCallback;
    wantSpec->userdata = (void*)&(gShared.gAudioGroup);
    SDL_AudioSpec obtain;
    SDL_OpenAudio( wantSpec, &obtain );
    std::cout << "[AnimeViewer]: audio device sample rate: " << obtain.freq << std::endl;
}

void VisWindow::addAudio(std::string tag, const S16Signal &signal, int sampleRate) {
    if (gShared.gAudioGroup.mSampleRate == 0)
        gShared.gAudioGroup.mSampleRate = sampleRate;
    else if (gShared.gAudioGroup.mSampleRate != sampleRate)
        snow::fatal("[addAudio]: Different samplerates are given.");
    // push
    gShared.gAudioGroup.mSignals.push_back(signal);
    gShared.gAudioGroup.mTags.push_back(tag);
}

void VisWindow::terminate() {
    if (gAudiable) {
        SDL_CloseAudio();
        gAudiable = false;
    }
    // clear public data
    gShared.clear();
    // clear seconds
    gSeconds = gLastTime = 0;
    gPlaying = false;
}

/* non-static */

void VisWindow::releaseObj() {
    if (mModelPtr) {
        delete mModelPtr;
        delete mShaderPtr;
        delete mCameraPtr;
        mModelPtr = nullptr;
        mShaderPtr  = nullptr;
        mCameraPtr  = nullptr;
    }
}

void VisWindow::setSubtitle(std::string sentence, const std::vector<int> &pos, float scale) {
    mPrivate.mSubtitle = sentence;
    mPrivate.mSubtitlePos = pos;
    mPrivate.mSubtitleScale = scale;
}

void VisWindow::setObj(std::string filepath) {
    if (!std::ifstream(filepath).good()) {
        std::cerr << "No such file: " << filepath << std::endl;
        return;
    }
    if (mModelType != ModelType::Obj)
        snow::fatal("setObj() is not supported for Obj Model");
    this->glMakeCurrent();  // important !!
    this->releaseObj();
    
    // this->mModelPtr  = (snow::Model *) (new ObjMesh(filepath));
    this->mModelPtr = new snow::Model(filepath);
    // this->mCameraPtr = new snow::ArcballCamera(glm::vec3(0.f, 0.f, -3.f), glm::vec3(0.f, -1.f, 0.f));
    this->mCameraPtr = new snow::ArcballCamera(glm::vec3(0.f, 0.f, 10.f), glm::vec3(0.f, 1.f, 0.f));
    this->mShaderPtr = new snow::Shader();
    this->mShaderPtr->buildFromCode(VERT_GLSL, FRAG_GLSL);
    // this->mShaderPtr->buildFromFile("../glsl/vert.glsl", "../glsl/frag.glsl");
    this->mPrivate.mObjPath = filepath;
    std::cout << "[AnimeViewer]: Open mesh: " << filepath << std::endl;
}

void VisWindow::setAnime(const std::vector<Vertices> &anime) {
    if (mModelType != ModelType::Obj)
        snow::fatal("setAnime() is not supported for Obj Model");
    mPrivate.mAnime = anime;
    mPrivate.mFrames = (int)anime.size();
    if (anime.size() > gShared.gMaxFrames) {
        gShared.gMaxFrames = (int)anime.size();
    }
}

void VisWindow::addScrollImage(std::string title, const ScrollImage &scrollImage) {
    mPrivate.mScrollImageMap.insert({title, scrollImage});
}

void VisWindow::setTexture(std::string title, uint8_t *data, int rows, int cols) {
    uint32_t id = 0;
    auto it = mTextureMap.find(title);
    if (it != mTextureMap.end()) id = it->second;
    if (id == 0) {
        // create texture
        uint32_t texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cols, rows, 0, GL_RGB, GL_UNSIGNED_BYTE, (void *)data);
        // glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0 );
        mTextureMap.insert({title, texture_id});
    }
    else {
        // sub data
        // printf("update texture %d %d\n", rows, cols);
        glBindTexture(  GL_TEXTURE_2D, id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGB, GL_UNSIGNED_BYTE, (void*)data);
        // glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(  GL_TEXTURE_2D, 0 );
    }
}

/* for bilinear model */
void VisWindow::setIden(const std::vector<double> &iden) {
    if (iden.size() != FaceDB::LengthIdentity)
        snow::fatal("<iden> size() is not correct!");
    if (mModelType != ModelType::Bilinear)
        snow::fatal("setIden() is not supported for Bilinear Model");
    this->glMakeCurrent();

    if (mModelPtr == nullptr) {
        this->mModelPtr  = (snow::Model *) (new ShowModel());
        this->mCameraPtr = new snow::ArcballCamera(glm::vec3(0.f, 0.f, 10.f), glm::vec3(0.f, 1.f, 0.f));
        this->mShaderPtr = new snow::Shader();
        this->mShaderPtr->buildFromCode(VERT_GLSL, FRAG_GLSL);
        // this->mShaderPtr->buildFromFile("../glsl/vert.glsl", "../glsl/frag.glsl");
    }
    
    mPrivate.mIden = iden;
    ((ShowModel *)mModelPtr)->updateIden(mPrivate.mIden);
}

void VisWindow::setExprList(const std::vector<std::vector<double>> &exprList) {
    if (exprList.size() == 0) return;
    if (exprList[0].size() != FaceDB::LengthExpression)
        snow::fatal("<expr> size() is not correct!");
    if (mModelType != ModelType::Bilinear)
        snow::fatal("setExprList() is not supported for Bilinear Model");
    mPrivate.mExprList = exprList;
    mPrivate.mFrames = (int)exprList.size();
    if (exprList.size() > gShared.gMaxFrames) {
        gShared.gMaxFrames = (int)exprList.size();
    }
}

/* overwrite events */

void VisWindow::processEvent(SDL_Event &event) {
    if (mCameraPtr)
        mCameraPtr->processMouseEvent(event);
}

void VisWindow::draw() {
    glEnable(GL_DEPTH_TEST);
    if (mModelPtr == nullptr) {
        ImGui::Begin(""); ImGui::Text("No mesh!"); ImGui::End();
        return;
    }

    /* draw text at bottom */ if (mPrivate.mText.length() > 0) {
        int height = ImGui::GetItemsLineHeightWithSpacing() * 2 + ImGui::GetStyle().WindowPadding.y * 2;
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - height),0);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, height),0);
        ImGui::Begin("text", nullptr, ImGuiWindowFlags_NoResize);
        ImGui::TextWrapped("%s", mPrivate.mText.c_str());
        ImGui::End();
    }

    /* draw player controller on top */ if (mPrivate.mFrames > 0) {
        // position and size, on top of window
        if (mController) {
            int height = ImGui::GetItemsLineHeightWithSpacing() * 3 + ImGui::GetStyle().WindowPadding.y * 2;
            ImGui::SetNextWindowPos(ImVec2(0, 0),0);
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, height),0);
            ImGui::Begin("", nullptr, ImGuiWindowFlags_NoResize);
        }
        else if (mPrivate.mMessage.length() > 0) {    
            int height = ImGui::GetItemsLineHeightWithSpacing() * 2 + ImGui::GetStyle().WindowPadding.y * 2;
            ImGui::SetNextWindowPos(ImVec2(0, 0), 0);
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, height),0);
            ImGui::Begin("", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::Text("%s", mPrivate.mMessage.c_str());
            ImGui::End();
        }

        double currentTime = ImGui::GetTime();

        /* lambda */
        auto _frame2AudioIndex = [&](int frame) -> int {
            return (double)frame * (double)gShared.gAudioGroup.mSampleRate / gShared.gFPS;
        };
        auto _restart = [&]() -> void {
            gSeconds = 0.0; gLastTime = currentTime;
            gShared.gAudioGroup.mCurrentIndex = gShared.gAudioGroup.mWantIndex = 
                _frame2AudioIndex(gShared.gCurrentFrame);
        };
        auto _nextFrame = [&]() -> void {
            gShared.gCurrentFrame ++;
            if (gAudiable)
                gShared.gAudioGroup.mWantIndex += _frame2AudioIndex(1);
        };
        auto _clip = [](int x, int l, int r) -> int { if (x < l) return l; else if (x > r) return r; else return x; };

        if (mController) {
            // slider controller
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);
            if (ImGui::SliderInt("Frame", &gShared.gCurrentFrame, 0, gShared.gMaxFrames - 1)) {
                _restart();
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
            // go to controller
            if (ImGui::InputInt(" ", &gShared.gCurrentFrame, 1)) {
                _restart();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset")) { gShared.gCurrentFrame = 0; }
            ImGui::SameLine();

            // playing, next frame;        
            if (gPlaying) {
                double delta = currentTime - gLastTime;
                gSeconds += delta;
                if (gSeconds > 1.0 / gShared.gFPS) {
                    // next frame
                    _nextFrame();
                    gSeconds -= 1.0 / gShared.gFPS;
                }
            }

            // pause or play
            if ((gPlaying && ImGui::Button("Pause")) || gShared.gCurrentFrame >= mPrivate.mFrames) {
                if (gAudiable) SDL_PauseAudio(1);
                gPlaying = false;
            }
            else if (!gPlaying && ImGui::Button("Play ")) {
                if (gAudiable) SDL_PauseAudio(0);
                gPlaying = true;
                _restart();
            }

            // clip current frame
            gShared.gCurrentFrame = _clip(gShared.gCurrentFrame, 0, gShared.gMaxFrames - 1);

            // update clock
            ImGui::SameLine();
            gTimeClock = (double)(gShared.gCurrentFrame) / gShared.gFPS + gSeconds;
            ImGui::Text("%.3f s", gTimeClock);

            /* audio select */ if (gAudiable) {
                for (size_t i = 0; i < gShared.gAudioGroup.mSignals.size(); ++i) {
                    if (i > 0) ImGui::SameLine();
                    ImGui::RadioButton((std::string("audio ") + gShared.gAudioGroup.mTags[i]).c_str(),
                                    &(gShared.gAudioGroup.mSelectId),
                                    i);
                }
            }
        }

        /* update frame */ {
            int idx = gShared.gCurrentFrame;
            switch (mModelType) {
            case ModelType::Obj:
                if (idx < mPrivate.mAnime.size()) mModelPtr->modify(&mPrivate.mAnime.at(idx));
                break;
            case ModelType::Bilinear:
                if (idx < mPrivate.mExprList.size()) mModelPtr->modify(&mPrivate.mExprList.at(idx));
                break;
            default:
                snow::fatal("Unknown model type.");
            }
            // update scroll image
            for (auto it = mPrivate.mScrollImageMap.begin(); it != mPrivate.mScrollImageMap.end(); ++it) {
                int r = idx * it->second.mHopLength;
                if (r < it->second.mRows) {
                    setTexture(it->first,
                        (uint8_t*)(it->second.mImg.data()) + it->second.mCols * 3 * r,
                        it->second.mWinLength,
                        it->second.mCols
                    );
                }
            }

            // update subtitle
            if (mPrivate.mSubtitle.length() > 0) {
                const size_t half = 22;
                int pos = mPrivate.mSubtitle.length();
                if (idx < mPrivate.mSubtitlePos.size())
                    pos = mPrivate.mSubtitlePos[idx] + 1;
                int left = 0, len = 0;
                if (pos > half) left = pos - half;
                len = half * 2 - (pos - left);
                if (pos + len > mPrivate.mSubtitle.length()) len = mPrivate.mSubtitle.length() - pos;
                if (len < half) left -= (half - len);
                if (left < 0) left = 0;
                std::string past = mPrivate.mSubtitle.substr(left, pos-left);
                std::string future = mPrivate.mSubtitle.substr(pos, len);

                this->textLine("center", 25.f, mPrivate.mSubtitleScale, {past, future},
                    {snow::Text::HighColor, snow::Text::BaseColor});
            }
        }
        
        gLastTime = currentTime;

        if (mController) ImGui::End();
    }

    /* show scroll images */ 
    for (auto it = mTextureMap.begin(); it != mTextureMap.end(); ++it) {
        auto itImage = mPrivate.mScrollImageMap.find(it->first);
        ImGui::Begin(it->first.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Image((void*)(intptr_t)(it->second), ImVec2(itImage->second.mCols, itImage->second.mWinLength * 5));
        ImGui::End();
    }

    /* draw model */ {

        // vec3 position;
        
        // float constant;
        // float linear;
        // float quadratic;
        
        // vec3 ambient;
        // vec3 diffuse;
        // vec3 specular;

        mShaderPtr->use();
        mShaderPtr->setVec3("viewPos", mCameraPtr->eye());
        mShaderPtr->setVec3("lightPos", mCameraPtr->eye());
        
        mShaderPtr->setVec3("dirLight.direction", {0, 0, -10});
        mShaderPtr->setVec3("dirLight.ambient", {0.05f, 0.05f, 0.05f});
        mShaderPtr->setVec3("dirLight.diffuse", {1.0f, 1.0f, 1.0f});
        mShaderPtr->setVec3("dirLight.specular", {0.0f, 0.0f, 0.0f});
        mShaderPtr->setFloat("material.shininess", 4.0f);

        mShaderPtr->setVec3("pointLights[0].position", {5, 3, 3});
        mShaderPtr->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        mShaderPtr->setVec3("pointLights[0].diffuse", 0.5f, 0.5f, 0.5f);
        mShaderPtr->setVec3("pointLights[0].specular", {0.1f, 0.1f, 0.1f});
        mShaderPtr->setFloat("pointLights[0].constant", 1.0f);
        mShaderPtr->setFloat("pointLights[0].linear", 0.09);
        mShaderPtr->setFloat("pointLights[0].quadratic", 0.032);

        mShaderPtr->setVec3("pointLights[1].position", {-5, 3, 3});
        mShaderPtr->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        mShaderPtr->setVec3("pointLights[1].diffuse", 0.5f, 0.5f, 0.5f);
        mShaderPtr->setVec3("pointLights[1].specular", {0.1f, 0.1f, 0.1f});
        mShaderPtr->setFloat("pointLights[1].constant", 1.0f);
        mShaderPtr->setFloat("pointLights[1].linear", 0.09);
        mShaderPtr->setFloat("pointLights[1].quadratic", 0.032);

        mShaderPtr->setVec3("pointLights[2].position", {5, -3, 3});
        mShaderPtr->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        mShaderPtr->setVec3("pointLights[2].diffuse", 0.5f, 0.5f, 0.5f);
        mShaderPtr->setVec3("pointLights[2].specular", {0.1f, 0.1f, 0.1f});
        mShaderPtr->setFloat("pointLights[2].constant", 1.0f);
        mShaderPtr->setFloat("pointLights[2].linear", 0.09);
        mShaderPtr->setFloat("pointLights[2].quadratic", 0.032);
    
        mShaderPtr->setVec3("pointLights[3].position", {-5, -3, 3});
        mShaderPtr->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        mShaderPtr->setVec3("pointLights[3].diffuse", 0.5f, 0.5f, 0.5f);
        mShaderPtr->setVec3("pointLights[3].specular", {0.1f, 0.1f, 0.1f});
        mShaderPtr->setFloat("pointLights[3].constant", 1.0f);
        mShaderPtr->setFloat("pointLights[3].linear", 0.09);
        mShaderPtr->setFloat("pointLights[3].quadratic", 0.032);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        // view. projection
        glm::mat4 projection = this->perspective(mCameraPtr);
        glm::mat4 view = mCameraPtr->viewMatrix();
        mShaderPtr->setMat4("projection", projection);
        mShaderPtr->setMat4("view", view);
        // model, normal
        glm::mat4 model = mModelPtr->autoModelTransform(projection * view, 0.25);
        glm::mat4 normal = glm::transpose(glm::inverse(model));
        mShaderPtr->setMat4("model", model);
        mShaderPtr->setMat4("normal", normal);
        // draw model
        mModelPtr->draw(*mShaderPtr);
    }

}
