#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include <map>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "stb/stb_image.h"

//#include "ft2build.h"
#include <ft2build.h>
#include FT_FREETYPE_H

struct Rect
{
	glm::vec2 pos;
	glm::vec2 extent;

	Rect(float x = 0, float y = 0, float w = 0, float h = 0)
		: pos(x, y)
		, extent(w, h)
	{}

	Rect(const glm::vec2& _pos, const glm::vec2& _extent)
		: pos(_pos)
		, extent(_extent)
	{}

	bool isPointInside(const glm::vec2& point) const
	{
		return !((point.x < pos.x || point.x > pos.x + extent.x)
			|| (point.y < pos.y || point.y > pos.y + extent.y));
	}

	glm::vec4 toVec4() const
	{
		return glm::vec4(pos, extent);
	}

	void addOffset(const glm::vec2& offset)
	{
		pos += offset;
	}

	void append(const Rect& other)
	{
		glm::vec2 bottom01 = pos + extent;
		glm::vec2 bottom02 = other.pos + other.extent;

		pos.x = glm::min(other.pos.x, pos.x);
		pos.y = glm::min(other.pos.y, pos.y);
		extent.x = glm::max(bottom01.x, bottom02.x) - pos.x;
		extent.y = glm::max(bottom01.y, bottom02.y) - pos.y;
	}
};

// GLUtils

namespace glUtils {

// Open and read the given file. Output its coutent in an array of char.
std::vector<char> readFile(const std::string& filePath);
// Load and compile a glsl shader
GLuint loadAndCompileShader(const std::string& shaderFilePath, GLenum shaderType);
// create an opengl shader program, attaching a vertex shader and a fragment shader
GLuint createShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

// transform coodinates from [0..1] [0..1] with top left anchor to [-1..1][-1..1] with center anchor
glm::vec4 viewportTransform(const glm::vec4& rect, const glm::vec2& viewportSize);
void viewportTransformInPlace(glm::vec4& rect, const glm::vec2& viewportSize);
glm::vec2 viewportTransform(const glm::vec2& point, const glm::vec2& viewportSize);
void viewportTransformInPlace(glm::vec2& point, const glm::vec2& viewportSize);

template<typename VertexType>
void initVertexAttributs()
{
	// To specialize
}

////////////////////////////////////////////////////////////////////////////////////
// Base structure representing a vertex
struct Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;

	Vertex(const glm::vec3& _pos, const glm::vec2& _uv)
		: pos(_pos)
		, uv(_uv)
	{}
};

template<>
inline void initVertexAttributs<glUtils::Vertex>()
{
	// pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glUtils::Vertex), (void*)offsetof(glUtils::Vertex, pos));
	// uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glUtils::Vertex), (void*)offsetof(glUtils::Vertex, uv));
}
////////////////////////////////////////////////////////////////////////////////////

// Base class representing a VAO (i.e : a shape made of vertices)
template<typename VertexType>
class VAO
{
private:
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ibo;

	std::vector<VertexType> m_vertices;
	std::vector<GLuint> m_indices;

public:
	VAO()
	{
		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ibo);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		initVertexAttributs<VertexType>();

		// indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

		//unbind
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void setDatas(const std::vector<VertexType>& vertices, const std::vector<GLuint>& indices)
	{
		m_vertices = vertices;
		m_indices = indices;

		pushToGPU();
	}

	void pushToGPU()
	{
		// vbo
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// ibo
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), &m_indices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void draw()
	{
		glBindVertexArray(m_vao);
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};

// Base class representing a shader program
class ShaderProgram
{
private:
	GLuint m_program;

public:
	ShaderProgram()
		: m_program(0)
	{}

	~ShaderProgram()
	{
		if (m_program > 0)
		{
			glDeleteProgram(m_program);
			m_program = 0;
		}
	}

	void load(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
	{
		if (m_program != 0)
		{
			glDeleteProgram(m_program);
			m_program = 0;
		}
		m_program = createShaderProgram(vertexShaderFilePath, fragmentShaderFilePath);
	}

	void use()
	{
		glUseProgram(m_program);
	}

	GLuint getGLId() const
	{
		return m_program;
	}
};

// Base class representing an opengl texture
class Texture
{
private:
	GLuint m_glId;
	int m_texWidth;
	int m_texHeight;
	unsigned char* m_imageDatas;

	GLint m_internalFormat;
	GLenum m_format;
	GLenum m_type;

public:

	Texture()
		: m_imageDatas(nullptr)
		, m_texWidth(0)
		, m_texHeight(0)
		, m_glId(0)
	{}

	~Texture()
	{
		if (m_imageDatas != nullptr)
			delete m_imageDatas;
		if (m_glId != 0)
			popFromGPU();
	}

	void setFormatAndType(GLint internalFormat, GLenum format, GLenum type)
	{
		m_internalFormat = internalFormat;
		m_format = format;
		m_type = type;
	}

	void load(const std::string& fileName, int desiredChannelCount, const std::vector<std::pair<GLenum, GLint>>& params, GLint internalFormat, GLenum format , GLenum type)
	{

		if (m_imageDatas != nullptr)
			delete m_imageDatas;
		if (m_glId != 0)
			popFromGPU();

		setFormatAndType(internalFormat, format, type);

		int channelCountInFile;
		m_imageDatas = stbi_load(fileName.c_str(), &m_texWidth, &m_texHeight, &channelCountInFile, desiredChannelCount);

		assert(desiredChannelCount <= channelCountInFile);

		setParameters(params);

		pushToGPU();
	}
	// You give the texture the ownership over datas. Texture will delete it when it is destroyed.
	void create(int width, int height, unsigned char* datas, const std::vector<std::pair<GLenum, GLint>>& params, GLint internalFormat, GLenum format, GLenum type)
	{

		if (m_imageDatas != nullptr)
			delete m_imageDatas;
		if (m_glId != 0)
			popFromGPU();

		setFormatAndType(internalFormat, format, type);

		m_imageDatas = datas;
		m_texWidth = width;
		m_texHeight = height;

		setParameters(params);
		pushToGPU();
	}

	GLuint getGLId() const
	{
		return m_glId;
	}
	int GetTexWidth() const
	{
		return m_texWidth;
	}
	int GetTexHeight() const
	{
		return m_texHeight;
	}

	void bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_glId);
	}
	void unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void setParameters(const std::vector<std::pair<GLenum, GLint>>& params)
	{
		if (m_glId <= 0)
			return;

		glBindTexture(GL_TEXTURE_2D, m_glId);

		for (const auto& param : params)
		{
			glTexParameteri(GL_TEXTURE_2D, param.first, param.second);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void pushToGPU()
	{
		if (m_glId != 0)
			popFromGPU();

		glGenTextures(1, &m_glId);
		glBindTexture(GL_TEXTURE_2D, m_glId);

		glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_texWidth, m_texHeight, 0, m_format, m_type, m_imageDatas);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void popFromGPU()
	{
		if (m_glId != 0)
		{
			glDeleteTextures(1, &m_glId);
			m_glId = 0;
		}
	}

// Helpers to load a texture
public:

	static std::shared_ptr<Texture> load_RGB_image(const std::string& fileName)
	{
		auto tex = std::make_shared<Texture>();
		tex->load(fileName, 3, { { GL_TEXTURE_WRAP_S, GL_REPEAT },{ GL_TEXTURE_WRAP_T, GL_REPEAT },{ GL_TEXTURE_MIN_FILTER, GL_LINEAR },{ GL_TEXTURE_MAG_FILTER, GL_LINEAR } }, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
		return tex;
	}

	static std::shared_ptr<Texture> create_FontAtlasTex(unsigned int width, unsigned int height, unsigned char* datas)
	{
		auto tex = std::make_shared<Texture>();
		tex->create(width, height, datas, { { GL_TEXTURE_WRAP_S, GL_CLAMP },{ GL_TEXTURE_WRAP_T, GL_CLAMP },{ GL_TEXTURE_MIN_FILTER, GL_LINEAR },{ GL_TEXTURE_MAG_FILTER, GL_LINEAR } }, GL_RED, GL_RED, GL_UNSIGNED_BYTE);
		return tex;
	}
};

} // namespace glUtils


// Font : 


struct Glyph
{
	glm::ivec2 size;				// Size of glyph
	glm::ivec2 bearing;				// Offset from baseline to left/top of glyph
	GLuint     advance;				// Offset to advance to next glyph
	GLuint     advanceInPixel;		// Offset to advance to next glyph

	Glyph()
		: size(0, 0)
		, bearing(0, 0)
		, advance(0)
		, advanceInPixel(0)
	{}

	Glyph(glm::ivec2 _size, glm::ivec2 _bearing, GLuint _advance)
		: size(_size)
		, bearing(_bearing)
		, advance(_advance)
	{
		advanceInPixel = (advance >> 6);
	}

	Rect getGlyphRect() const
	{
		return Rect(glm::vec2(bearing.x, -bearing.y), size);
	}

	Rect getGlyphRectIncludingAdvance() const
	{
		return Rect(glm::vec2(0, -bearing.y), glm::vec2(advanceInPixel, bearing.y));
	}
};

struct FontAtlas
{
	std::shared_ptr<glUtils::Texture> m_fontTexture;
	float m_glyphWidth;
	float m_glyphHeight;
	unsigned int m_glyphCount;
	unsigned int m_colCount;
	unsigned int m_rowCount;	
};

class Font
{
private:
	FontAtlas m_atlas;

	float m_fontSize;
	std::string m_fontName;

	std::map<unsigned long, unsigned int> m_charToIndex; // charcode -> glyph index 
	std::vector<Glyph> m_glyphInfos; // glyphinfos

public:
	void load(FT_Face face, const std::string& fontName, unsigned int fontSize)
	{
		m_fontSize = fontSize;
		m_fontName = fontName;

		FT_Set_Pixel_Sizes(face, 0, m_fontSize);

		create(face);
	}
	const FontAtlas& getAtlas() const
	{
		return m_atlas;
	}
	Rect computeTextBounds(const std::string& text) const
	{
		const Glyph* glyph = nullptr;
		bool success = true;
		Rect bound(0, 0, 0, 0);
		glm::vec2 cursor(0, 0);
		for (const auto& character : text)
		{
			success = getGlyphInfoFromChar(character, &glyph);

			if (!success)
				break;

			Rect glyphRect = glyph->getGlyphRectIncludingAdvance();
			glyphRect.addOffset(cursor);
			bound.append(glyphRect);
			cursor.x += glyph->advanceInPixel;

		}

		return bound;
	}

	glm::vec2 getCursorPos(const std::string& text, unsigned int cursorIdx)
	{
		const Glyph* glyph = nullptr;
		bool success = true;
		glm::vec2 cursor(0, 0);
		for (int i = 0; i < cursorIdx; i++)
		{
			const auto& character = text[i];
			success = getGlyphInfoFromChar(character, &glyph);

			if (!success)
				break;

			cursor.x += glyph->advanceInPixel;
		}

		return cursor;
	}
	unsigned int getCursorIdx(const std::string& text, const glm::vec2& cursorPos)
	{
		const Glyph* glyph = nullptr;
		bool success = true;
		float currentPos = 0;
		for (int i = 0; i < text.size(); i++)
		{
			const auto& character = text[i];
			success = getGlyphInfoFromChar(character, &glyph);

			if (!success)
				break;

			currentPos += glyph->advanceInPixel;

			if (currentPos - glyph->advanceInPixel * 0.5f > cursorPos.x)
				return i;
		}

		return text.size();
	}
	glm::vec2 getMaxGlyphSize() const
	{
		return glm::vec2(m_atlas.m_glyphWidth, m_atlas.m_glyphHeight);
	}

	void create(FT_Face& face)
	{
		unsigned int glyphCountPerRow = std::sqrt(face->num_glyphs);
		unsigned int rowCount = glyphCountPerRow + (std::ceil(glyphCountPerRow) - glyphCountPerRow);

		unsigned int maxGlyphWidth = 0;
		unsigned int maxGlyphHeight = 0;
		for (int i = 0; i < face->num_glyphs; i++)
		{
			if (FT_Load_Char(face, i, FT_LOAD_RENDER))
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;

			unsigned int glyphWidth = face->glyph->bitmap.width;
			unsigned int glyphHeight = face->glyph->bitmap.rows;

			if (glyphWidth > maxGlyphWidth)
				maxGlyphWidth = glyphWidth;
			if (glyphHeight > maxGlyphHeight)
				maxGlyphHeight = glyphHeight;
		}

		m_atlas.m_glyphCount = face->num_glyphs;
		m_atlas.m_glyphWidth = maxGlyphWidth;
		m_atlas.m_glyphHeight = maxGlyphHeight;
		m_atlas.m_colCount = glyphCountPerRow;
		m_atlas.m_rowCount = rowCount;
		m_glyphInfos.resize(m_atlas.m_glyphCount);

		unsigned int texWidth = m_atlas.m_glyphWidth * glyphCountPerRow;
		unsigned int texHeight = m_atlas.m_glyphHeight * rowCount;
		unsigned int texPixelCount = texWidth * texHeight;
		unsigned char* textureDatas = new unsigned char[texPixelCount];

		// populate m_charToIndex
		FT_UInt index;
		FT_ULong character = FT_Get_First_Char(face, &index);
		while (true) {

			m_charToIndex[character] = index;

			character = FT_Get_Next_Char(face, character, &index);
			if (!index) break;
		}
		//////////////////////

		unsigned int pixelCursor = 0;
		for (unsigned int glyphIdx = 0, rowIdx = 0; rowIdx < rowCount; rowIdx++)
		{
			for (unsigned int colIdx = 0; colIdx < glyphCountPerRow; colIdx++, glyphIdx++)
			{
				if (FT_Load_Glyph(face, glyphIdx, FT_LOAD_RENDER))
					std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;

				unsigned int glyphWidth = face->glyph->bitmap.width;
				unsigned int glyphHeight = face->glyph->bitmap.rows;

				m_glyphInfos[glyphIdx] = Glyph(
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					face->glyph->advance.x
				);

				for (unsigned int j = 0; j < maxGlyphHeight; j++)
				{
					for (unsigned int i = 0; i < maxGlyphWidth; i++)
					{
						int posX = colIdx * maxGlyphWidth + i;
						int posY = rowIdx * maxGlyphHeight + j;

						if (i < glyphWidth && j < glyphHeight)
						{
							//int invJ = (glyphHeight-1) - j;
							textureDatas[posX + posY * texWidth] = face->glyph->bitmap.buffer[i + j * glyphWidth];
						}
						else
						{
							textureDatas[posX + posY * texWidth] = 0;
						}
					}
				}
			}
		}

		m_atlas.m_fontTexture = glUtils::Texture::create_FontAtlasTex(texWidth, texHeight, textureDatas);

		// debug
		debugPrintChatToIndex();
	}

	glm::vec4 getGlyphSourceRect(unsigned int glyphIndex) const
	{
		unsigned int colIdx = glyphIndex % m_atlas.m_colCount;
		unsigned int rowIdx = glyphIndex / m_atlas.m_colCount;

		const Glyph& glyphInfo = m_glyphInfos[glyphIndex];

		glm::vec4 outRect(colIdx * m_atlas.m_glyphWidth, rowIdx * m_atlas.m_glyphHeight, glyphInfo.size.x, glyphInfo.size.y);

		const float texWidth = m_atlas.m_fontTexture->GetTexWidth();
		const float texHeight = m_atlas.m_fontTexture->GetTexHeight();
		return (outRect / glm::vec4(texWidth, texHeight, texWidth, texHeight));
	}
	bool getGlyphDestRect(unsigned int glyphIndex, const glm::vec2& destLocation, glm::vec2& outNextDestLocation, glm::vec4& outGlyphDestRect) const
	{
		if (glyphIndex < 0 || glyphIndex >= m_glyphInfos.size())
			return false;

		const Glyph& glyphInfo = m_glyphInfos[glyphIndex];

		outGlyphDestRect = glm::vec4(destLocation + glm::vec2(glyphInfo.bearing.x, -glyphInfo.bearing.y), glm::vec2(glyphInfo.size.x, glyphInfo.size.y));
		outNextDestLocation = destLocation + glm::vec2(glyphInfo.advanceInPixel, 0);

		return true;
	}
	bool getGlyphDisplayInfos(unsigned long charcode, const glm::vec2& destLocation, glm::vec2& outNextDestLocation, glm::vec4& outGlyphDestRect, glm::vec4& outGlyphSourceRect) const
	{
		unsigned int glyphIndex;
		bool lookupSuccess = getGlyphIndex(charcode, glyphIndex);
		if (!lookupSuccess)
			return false;

		outGlyphSourceRect = getGlyphSourceRect(glyphIndex);
		return  getGlyphDestRect(glyphIndex, destLocation, outNextDestLocation, outGlyphDestRect);
	}

	bool getGlyphInfoFromChar(unsigned long charcode, Glyph const ** outGlyph) const
	{
		unsigned int glyphIndex;
		bool lookupSuccess = getGlyphIndex(charcode, glyphIndex);
		if (!lookupSuccess)
			return false;

		*outGlyph = &m_glyphInfos[glyphIndex];
		return true;
	}
	bool getGlyphIndex(unsigned char charcode, unsigned int& outGlyphIndex) const
	{
		auto& found = m_charToIndex.find(charcode);
		if (found != m_charToIndex.end())
		{
			outGlyphIndex = found->second;
			return true;
		}
		else
			return false;
	}

	void bindTexture() const
	{
		m_atlas.m_fontTexture->bind();
	}
	void unbindTexture() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void debugPrintChatToIndex() const
	{
		std::cout << "chat to glyph index : " << std::endl;
		for (const auto& pair : m_charToIndex)
		{
			std::cout << (char)pair.first << " : " << pair.second << std::endl;
		}
	}
	
};

struct FontKey
{
	std::string fontName;
	unsigned int fontSize;

	FontKey(const std::string& _fontName, unsigned int _fontSize)
		: fontName(_fontName)
		, fontSize(_fontSize)
	{}
};

class FontFactory
{
private:
	FT_Library m_ft;
	std::map<std::string, std::map<unsigned int, std::shared_ptr<Font>>> m_fonts;
	std::string m_defaultFontName;
	unsigned int m_defaultFontSize;

public:
	FontFactory()
		: m_defaultFontName("")
		, m_defaultFontSize(0)
	{
		if (FT_Init_FreeType(&m_ft))
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	}
	~FontFactory()
	{
		FT_Done_FreeType(m_ft);
	}

	bool loadFont(const std::string& fileName, const std::string& fontName, unsigned int fontSize)
	{
		if (hasFont(fontName, fontSize))
			return false;

		FT_Face face;
		if (FT_New_Face(m_ft, fileName.c_str(), 0, &face))
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

		std::shared_ptr<Font> newFont = std::make_shared<Font>();
		newFont->load(face, fontName, fontSize);
		// insert font in container
		m_fonts[fontName][fontSize] = newFont;

		FT_Done_Face(face);

		return true;
	}

	std::shared_ptr<Font> getFont(const std::string& fontName)
	{
		auto& foundFontName = m_fonts.find(fontName);
		if (foundFontName != m_fonts.end())
		{
			auto& foundFontSize = foundFontName->second.begin();
			if (foundFontSize != foundFontName->second.end())
				return foundFontSize->second;
			else
				return nullptr;
		}
		else
			return nullptr;
	}

	std::shared_ptr<Font> getFont(const std::string& fontName, unsigned int fontSize)
	{
		auto& foundFontName = m_fonts.find(fontName);
		if (foundFontName != m_fonts.end())
		{
			auto& foundFontSize = foundFontName->second.find(fontSize);
			if (foundFontSize != foundFontName->second.end())
				return foundFontSize->second;
			else
				return nullptr;
		}
		else
			return nullptr;
	}

	bool hasFont(const std::string& fontName, unsigned int fontSize)
	{
		auto& foundFontName = m_fonts.find(fontName);
		if (foundFontName != m_fonts.end())
		{
			return foundFontName->second.find(fontSize) != foundFontName->second.end();
		}
		else
			return false;
	}

	std::shared_ptr<Font> getDefaultFont() const
	{
		if (m_defaultFontName != "" && m_defaultFontSize > 0)
		{
			auto& foundByName = m_fonts.find(m_defaultFontName);
			if (foundByName != m_fonts.end())
			{
				auto& foundBySize = foundByName->second.find(m_defaultFontSize);
				if (foundBySize != foundByName->second.end())
					return foundBySize->second;
			}
		}

		// fallback if the font isn't found
		auto& fontBySize = m_fonts.begin()->second;
		if (fontBySize.size() > 0)
		{
			auto& fontIt = fontBySize.begin();
			return fontIt->second;
		}
		else
			return nullptr;
	}

	void setFontAsDefault(const std::string& fontName, unsigned int fontSize)
	{
		m_defaultFontName = fontName;
		m_defaultFontSize = fontSize;
	}
};

static std::vector<glUtils::Vertex> vertices = {
	glUtils::Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0, 0)),
	glUtils::Vertex(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec2(1, 0)),
	glUtils::Vertex(glm::vec3(0.5f,  0.5f, 0.0f), glm::vec2(1, 1)),
	glUtils::Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec2(0, 1))
};

static std::vector<GLuint> indices = {
	0, 1, 2,
	2, 3, 0
};

// End Font
