#pragma once
#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "GLEW/glew.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "TextureManager.h"
#include <stdio.h>
#include <string>

class FontManager
{
public:
    static bool initFreeType();
    /*
    Pre Condition:
     -None
    Post Condition:
     -Instantiates FreeType library used for every LFont object
    Side Effects:
     -None
    */

    FontManager();
    /*
    Pre Condition:
     -None
    Post Condition:
     -Initializes spacing variables
    Side Effects:
     -None
    */

    FontManager();
    /*
    Pre Condition:
     -None
    Post Condition:
     -Frees font
    Side Effects:
     -None
    */

    bool loadBitmap(std::string path);
    /*
    Pre Condition:
     -None
    Post Condition:
     -Loads bitmap font
     -Returns true on success
     -Reports errors to console
    Side Effects:
     -None
    */

    bool loadFreeType(std::string path, GLuint pixelSize);
    /*
    Pre Condition:
     -None
    Post Condition:
     -Creates internal 8bit texture with given font file
     -Reports freetype error code if freetype error occurs
    Side Effects:
     -None
    */

    void freeFont();
    /*
    Pre Condition:
     -None
    Post Condition:
     -Deallocates texture and sprite sheet data
    Side Effects:
     -None
    */

    void renderText(GLfloat x, GLfloat y, std::string text);
    /*
    Pre Condition:
     -A loaded font
    Post Condition:
     -Renders text
    Side Effects:
     -Binds member texture and data buffers
    */

private:
    //Font TTF library
    static FT_Library mLibrary;

    //Spacing variables
    GLfloat mSpace;
    GLfloat mLineHeight;
    GLfloat mNewLine;
};
#endif