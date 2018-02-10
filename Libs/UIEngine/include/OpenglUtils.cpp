#include "OpenglUtils.h"

std::vector<char> readFile(const std::string& filePath)
{
	std::vector<char> output;

	std::string line;
	std::ifstream fileIn(filePath);
	if (fileIn.is_open())
	{
		fileIn.seekg(0, fileIn.end);
		int fileSize = fileIn.tellg();
		fileIn.seekg(0, fileIn.beg);

		output.resize(fileSize);

		fileIn.read(&output[0], fileSize);

		fileIn.close();
	}
	else
	{
		std::cout << "error : can't open file " << filePath.c_str() << std::endl;
	}

	return output;
}

GLuint loadAndCompileShader(const std::string& shaderFilePath, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	std::vector<char> shaderFileContent = readFile(shaderFilePath);
	const char* shaderFileContentPtr = shaderFileContent.data();
	glShaderSource(shader, 1, &shaderFileContentPtr, nullptr);
	glCompileShader(shader);

	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	return shader;
}

GLuint createShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
{
	GLuint vertexShader = loadAndCompileShader(vertexShaderFilePath, GL_VERTEX_SHADER);
	GLuint fragmentShader = loadAndCompileShader(fragmentShaderFilePath, GL_FRAGMENT_SHADER);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	int  success;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER_PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

glm::vec4 viewportTransformInPlace(const glm::vec4& rect, const glm::vec2& viewportSize)
{
	glm::vec4 box = rect;
	box /= glm::vec4((viewportSize), (viewportSize));
	box *= glm::vec4(2, -2, 2, -2);
	box += glm::vec4(-1, 1, 0, 0);
	return box;
}
void viewportTransform(glm::vec4& rect, const glm::vec2& viewportSize)
{
	rect /= glm::vec4((viewportSize), (viewportSize));
	rect *= glm::vec4(2, -2, 2, -2);
	rect += glm::vec4(-1, 1, 0, 0);
}
glm::vec2 viewportTransformInPlace(const glm::vec2& point, const glm::vec2& viewportSize)
{
	glm::vec2 p = point;
	p /= viewportSize;
	p *= glm::vec2(2, -2);
	p += glm::vec2(-1, 1);
	return p;
}
void viewportTransform(glm::vec2& point, const glm::vec2& viewportSize)
{
	point /= viewportSize;
	point *= glm::vec2(2, -2);
	point += glm::vec2(-1, 1);
}