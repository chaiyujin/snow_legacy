#include "show.h"

void ShowWindow::setColor(const Image *imgPtr) {
    mColorPtr = imgPtr;
    mImageShader.uploadImage(mColorPtr->data(), mColorPtr->width(), mColorPtr->height(), (mColorPtr->bpp() == 4) ? GL_RGBA : GL_RGB);
}

void ShowWindow::setDepth(const Image *depthPtr) {
    mDepthPtr = depthPtr;
    Image::ColorizeDepth(*mDepthPtr, *mColorizedDepth);
    mImageShader.uploadImage(mColorizedDepth->data(), mColorizedDepth->width(), mColorizedDepth->height(), (mColorizedDepth->bpp() == 4) ? GL_RGBA : GL_RGB);
}