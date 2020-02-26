#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "SDL2/SDL.h"
#include "GLEW\glew.h"
#include <string>
#include <vector>

// Singleton Class of Texture Manager
class TextureManager
{
protected:
	struct TextureContainer
	{
		GLuint textureID;
		std::string filename;
		bool hasMipmaps;
		//Current pixels
		GLuint* mPixels32;
		GLubyte* mPixels8;

		//Pixel format
		GLuint mPixelFormat;

		//Texture dimensions
		GLuint mTextureWidth;
		GLuint mTextureHeight;

		//Unpadded image dimensions
		GLuint mImageWidth;
		GLuint mImageHeight;
	};
	std::vector<TextureContainer> textures;

	// find the texture with the fiven filename and mipmaps
	int findTexture(const char* filename, bool hasMipmaps);

public:
	// get the static instance of Texture Manager
	static TextureManager& GetInstance()
	{
		static TextureManager manager;
		return manager;
	}
	~TextureManager();

	// delete all textures
	void Clear();

	// Request a texture handle
	GLuint RequestTexture(const char* filename, bool hasMipmaps = false);

	/*
	Pre Condition:
	 -A valid OpenGL context
	Post Condition:
	 -Copies given pixel data into member pixels
	Side Effects:
	 -None
	*/
	void copyPixels8(GLubyte* pixels, GLuint imgWidth, GLuint imgHeight);
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
	TextureManager();	
	void operator=(TextureManager const&);
	SDL_Texture *mTexture;
};

#endif
