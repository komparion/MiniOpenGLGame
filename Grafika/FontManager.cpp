#include "FontManager.h"
#include "TextureManager.h"
#include FT_BITMAP_H
FT_Library FontManager::mLibrary;



bool FontManager::initFreeType() {
    //Init FreeType for single threaded applications
    #ifndef __FREEGLUT_H__
    FT_Error error = FT_Init_FreeType(&mLibrary);
    if (error)
    {
        printf("FreeType error:%X", error);
        return false;
    }
    #endif

    return true;
}

bool FontManager::loadFreeType(std::string path, GLuint pixelSize)
{
    //Error flag
    FT_Error error = NULL;

    //Init FreeType for multithread applications
#ifdef __FREEGLUT_H__
    error = FT_Init_FreeType(&mLibrary);
    if (error)
    {
        printf("FreeType error:%X", error);
        return false;
    }
#endif

    //Get cell dimensions
    GLuint cellW = 0;
    GLuint cellH = 0;
    int maxBearing = 0;
    int minHang = 0;

    //Character data
    TextureManager bitmaps[256];
    FT_Glyph_Metrics metrics[256];


    /*
    To start loading our font, we first have to load a FT_Face.In a nutshell a font face is a set of glyphs you can render with.
    A font can have multiple faces such as regular, italic, bold, bold italic, etc.
    This call to FT_New_Face() opens at the face at index 0 since the font should have at least one face.
    If you want your font renderer to support multiple faces you can check if a certain face index exists with FT_Open_Face().You can learn more about it in the FreeType documentation.
    After loading the font face at index 0, we check if there were any errors.If not, we continue loading the font.*/

    //Load face
    FT_Face face = NULL;
    error = FT_New_Face(mLibrary, path.c_str(), 0, &face);
    if (!error)
    {

        /*  With the font face loaded, we want to control the size of the our character sprites.
            Our loadFreeType() takes in "pixelSize" which we're passing to FT_Set_Pixel_Sizes() to set the maximum height for our gylphs.
            The second argument FT_Set_Pixel_Sizes() is the maximum width for the glyphs.
            Because it's set to zero, FT_Set_Pixel_Sizes() will only take into account our maximum height.*/

        //Set face size
        error = FT_Set_Pixel_Sizes(face, 0, pixelSize);
        if (!error)
        {
            /*
            Now it's time to get the individual 256 glyphs.
            The function FT_Load_Char() loads a glyph from the given face with the given ASCII value.
            The third argument "FT_LOAD_RENDER" will make the FT_Load_Char() function not only get the glyph data but also render the pixels we need from the glyph.*/

            //Go through extended ASCII to get glyph data
            for (int i = 0; i < 256; ++i)
            {
                //Load and render glyph
                error = FT_Load_Char(face, i, FT_LOAD_RENDER);
                if (!error)
                {
                    
                    /*
                    If the glyph successfully rendered, we want to get the glyph's data.
                    The face object has a member "glyph" which contains the information for the glyph that was loaded with FT_Load_Char().
                    Here we get the glyphs metrics from the "metrics" member of the glyph.
                    Then we need to store the pixels from the glyph image inside of our LTexture array using the copyPixels8() function.
                    The glyph has a "bitmap" member that has the image data we need.
                    "bitmap.buffer" contains the 8bit pixel data pointer, "bitmap.width" has the width of the glyph image, and "bitmap.rows" 
                    has how many rows of pixels there are which is the glyph image height.*/

                    //Get metrics
                    metrics[i] = face->glyph->metrics;

                    //Copy glyph bitmap
                    bitmaps[i].copyPixels8(face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows);

                    /*
                    Now we need to find the maximum bearing using the metrics.
                    The member "horiBearingY" is the distance from the baseline to top pixel in the glyph image when text is rendered horizontally.
                    FreeType also has the ability to render text vertically, but that's beyond the scope of this project.
                    If you're wondering why "horiBearingY" is divided by 64 it's because the metrics aren't measured in pixels.
                    They're measured in points which are 1/64 of a pixel.
                    The "maxBearing" variable is in pixels so we need to convert the metrics from points to pixels.
                    Next we need to find the maximum width in order to know how big to make the cells.
                    After that we find the hang which is how much the glyph goes below the baseline.
                    The hang is the bearing minus the overall height of the glyph:*/

                    //Calculate max bearing
                    if (metrics[i].horiBearingY / 64 > maxBearing)
                    {
                        maxBearing = metrics[i].horiBearingY / 64;
                    }

                    //Calculate max width
                    if (metrics[i].width / 64 > cellW)
                    {
                        cellW = metrics[i].width / 64;
                    }

                    //Calculate gylph hang
                    int glyphHang = (metrics[i].horiBearingY - metrics[i].height) / 64;
                    if (glyphHang < minHang)
                    {
                        minHang = glyphHang;
                    }
                }else
                {
                    printf("Unable to load glyph. FreeType error:%X\n", error);
                    error = NULL;
                }
            }

            /*Now that we're done getting all of the glyph images and all of the glyph metrics, it's time to create our texture.
              First, we calculate how big we need to make the cell by taking the highest pixel point and subtracting the lowest point.
              Now that we know the cell width/height, we use createPixels8() to create a texture large enough to hold all the glyph images.
              Now, it's time to go through all the glyph images we have a blit them to the FontManager texture.*/

            //Create bitmap font
            cellH = maxBearing - minHang;
            //createPixels8(cellW * 16, cellH * 16);

            //Begin creating bitmap font
            GLuint currentChar = 0;
           // LFRect nextClip = { 0.f, 0.f, cellW, cellH };

            //Blitting coordinates
            int bX = 0;
            int bY = 0;

            /*Now we're going through all of the cell rows/columns to put all the glyph images in their place.
              Since we already know how big the glyph images are, we can set the width of the character sprite as the width of the glyph (in pixels).
              We don't adjust the height of the character sprite because as with the bitmap font we want all the character sprites to be of uniform height.*/

            //Go through cell rows
            for (unsigned int rows = 0; rows < 16; rows++)
            {
                //Go through each cell column in the row
                for (unsigned int cols = 0; cols < 16; cols++)
                {
                    //Set base offsets
                    bX = cellW * cols;
                    bY = cellH * rows;

                    //Initialize clip
                    nextClip.x = bX;
                    nextClip.y = bY;
                    nextClip.w = metrics[currentChar].width / 64;
                    nextClip.h = cellH;

                    /*When blitting the glyph image, the x offset is just the left side of the cell.
                    The y offset is the highest top point "maxBearing" minus the glyph's bearing in pixels.
                    When we blit the glyph, we blit it to the current LFont object we're loading.
                    Since an LFont is a LSpriteSheet which is a LTexture, the texture blitting operations work the same.
                    Then we add the clipping rectangle to sprite sheet and keep going through until all of the glyph images are blitted to the sprite sheet.*/

                    //Blit character
                    //bitmaps[currentChar].blitPixels8(bX, bY + maxBearing - metrics[currentChar].horiBearingY / 64, *this);

                    //Go to the next character
                    //mClips.push_back(nextClip);
                    currentChar++;
                }
            }

            //After all of the glyph images are blitted, we pad the pixels so the sprite sheet is a power of two. Then we load the texture and generate the VBO data

             //Make texture power of two
            //padPixels8();

            //Create texture
            if (loadTextureFromPixels8())
            {
                //Build vertex buffer from sprite sheet data
                if (!generateDataBuffer(LSPRITE_ORIGIN_TOP_LEFT))
                {
                    printf("Unable to create vertex buffer for bitmap font!");
                    error = 0xA2;
                }
            }
            else
            {
                printf("Unable to create texture from generated bitmap font!\n");
                error = 0xA2;
            }

            /*Here set the font texture wrap and calculate the spacing variables.
            Our spacing variables are calculated a little differently, but they pretty much work the same.
            After we're done with the font face, we free it using FT_Done_Face().*/

            //Set texture wrap
            //glBindTexture(GL_TEXTURE_2D, getTextureID());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            //Set spacing variables
            mSpace = cellW / 2;
            mNewLine = maxBearing;
            mLineHeight = cellH;
        }
        else
        {
        printf("Unable to set font size. FreeType error:%X\n", error);
        }

        //Free face
        FT_Done_Face(face);
    
        
    //Finally, when we're done with our FT_Library in our multithreaded application we call FT_Done_FreeType() to free it.
    }else
    {
    printf("Unable to load load font face. FreeType error:%X\n", error);
    return false;
    }

    //Close FreeType for multithreaded applications
#ifdef __FREEGLUT_H__
    FT_Done_FreeType(mLibrary);
#endif

    return error == NULL;
}


void FontManager::renderText(GLfloat x, GLfloat y, std::string text)
{
    //If there is a texture to render from
    if (getTextureID() != 0)
    {
        //Draw positions
        GLfloat dX = x;
        GLfloat dY = y;

        //Move to draw position
        glTranslatef(x, y, 0.f);

        //Set texture
        glBindTexture(GL_TEXTURE_2D, getTextureID());

        //Enable vertex and texture coordinate arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        //Bind vertex data
        glBindBuffer(GL_ARRAY_BUFFER, mVertexDataBuffer);

        //Set texture coordinate data
        glTexCoordPointer(2, GL_FLOAT, sizeof(LVertexData2D), (GLvoid*)offsetof(LVertexData2D, texCoord));

        //Set vertex data
        glVertexPointer(2, GL_FLOAT, sizeof(LVertexData2D), (GLvoid*)offsetof(LVertexData2D, position));

        //Go through string
        for (int i = 0; i < text.length(); ++i)
        {
            //Space
            if (text[i] == ' ')
            {
                glTranslatef(mSpace, 0.f, 0.f);
                dX += mSpace;
            }
            //Newline
            else if (text[i] == '\n')
            {
                glTranslatef(x - dX, mNewLine, 0.f);
                dY += mNewLine;
                dX += x - dX;
            }
            //Character
            else
            {
                //Get ASCII
                GLuint ascii = (unsigned char)text[i];

                //Draw quad using vertex data and index data
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[ascii]);
                glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, NULL);

                //Move over
                glTranslatef(mClips[ascii].w, 0.f, 0.f);
                dX += mClips[ascii].w;
            }
        }

        //Disable vertex and texture coordinate arrays
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}



