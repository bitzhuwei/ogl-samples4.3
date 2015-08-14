//**********************************
// OpenGL Framebuffer Multisample
// 16/06/2010 - 01/04/2011
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	std::string const SAMPLE_NAME = "OpenGL Framebuffer Multisample";
	std::string const SHADER_VERT_SOURCE(glf::DATA_DIRECTORY + "gl-420/texture-2d.vert");
	std::string const SHADER_FRAG_SOURCE(glf::DATA_DIRECTORY + "gl-420/texture-2d.frag");
	std::string const TEXTURE_DIFFUSE(glf::DATA_DIRECTORY + "kueken3-bgr8.dds");
	glm::ivec2 const FRAMEBUFFER_SIZE(320, 240);
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(2);

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-4.0f,-3.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f,-3.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f, 3.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(-4.0f, 3.0f), glm::vec2(0.0f, 1.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2,
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	namespace program
	{
		enum type
		{
			VERTEX,
			FRAGMENT,
			MAX
		};
	}//namespace program

	GLuint VertexArrayName(0);
	GLuint PipelineName(0);
	GLuint ProgramName[program::MAX] = {0, 0};

	GLuint BufferName[buffer::MAX] = {0, 0, 0};
	GLuint TextureName(0);
	GLuint SamplerName(0);

	GLuint MultisampleTextureName(0);
	GLuint ColorTextureName(0);

	GLuint FramebufferRenderName(0);
	GLuint FramebufferResolveName(0);

	GLuint UniformDiffuse(0);	

}//namespace

bool initProgram()
{
	bool Validated = true;

	glGenProgramPipelines(1, &PipelineName);

	if(Validated)
	{
		GLuint VertShader = glf::createShader(GL_VERTEX_SHADER, SHADER_VERT_SOURCE);

		ProgramName[program::VERTEX] = glCreateProgram();
		glProgramParameteri(ProgramName[program::VERTEX], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName[program::VERTEX], VertShader);
		glDeleteShader(VertShader);
		glLinkProgram(ProgramName[program::VERTEX]);
		Validated = Validated && glf::checkProgram(ProgramName[program::VERTEX]);
	}

	if(Validated)
	{
		glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT, ProgramName[program::VERTEX]);
		Validated = Validated && glf::checkError("initProgram - stage");
	}

	if(Validated)
	{
		GLuint FragShader = glf::createShader(GL_FRAGMENT_SHADER, SHADER_FRAG_SOURCE);

		ProgramName[program::FRAGMENT] = glCreateProgram();
		glProgramParameteri(ProgramName[program::FRAGMENT], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName[program::FRAGMENT], FragShader);
		glDeleteShader(FragShader);
		glLinkProgram(ProgramName[program::FRAGMENT]);
		Validated = Validated && glf::checkProgram(ProgramName[program::FRAGMENT]);
	}

	if(Validated)
	{
		glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAGMENT]);
		Validated = Validated && glf::checkError("initProgram - stage");
	}

	return glf::checkError("initProgram");
}

bool initBuffer()
{
	glGenBuffers(buffer::MAX, BufferName);
    glNamedBufferDataEXT(BufferName[buffer::ELEMENT], ElementSize, ElementData, GL_STATIC_DRAW);
    glNamedBufferDataEXT(BufferName[buffer::VERTEX], VertexSize, VertexData, GL_STATIC_DRAW);
	glNamedBufferDataEXT(BufferName[buffer::TRANSFORM], VertexSize, VertexData, GL_DYNAMIC_DRAW);

	return glf::checkError("initBuffer");;
}

bool initSampler()
{
	glGenSamplers(1, &SamplerName);
	glSamplerParameteri(SamplerName, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(SamplerName, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glSamplerParameterfv(SamplerName, GL_TEXTURE_BORDER_COLOR, &glm::vec4(0.0f)[0]);
	glSamplerParameterf(SamplerName, GL_TEXTURE_MIN_LOD, -1000.f);
	glSamplerParameterf(SamplerName, GL_TEXTURE_MAX_LOD, 1000.f);
	glSamplerParameterf(SamplerName, GL_TEXTURE_LOD_BIAS, 0.0f);
	glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glBindSampler(0, SamplerName);

	return glf::checkError("initSampler");
}

bool initTexture2D()
{
	glGenTextures(1, &TextureName);
	glTextureParameteriEXT(TextureName, GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTextureParameteriEXT(TextureName, GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTextureParameteriEXT(TextureName, GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTextureParameteriEXT(TextureName, GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
	glTextureParameteriEXT(TextureName, GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteriEXT(TextureName, GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);

	gli::texture2D Image = gli::load(TEXTURE_DIFFUSE);
	for(std::size_t Level = 0; Level < Image.levels(); ++Level)
	{
		glTextureImage2DEXT(
			TextureName,
			GL_TEXTURE_2D,
			GLint(Level),
			GL_RGB8,
			GLsizei(Image[Level].dimensions().x), 
			GLsizei(Image[Level].dimensions().y), 
			0, 
			GL_BGR,
			GL_UNSIGNED_BYTE, 
			Image[Level].data());
	}

	if(Image.levels() == 1)
		glGenerateTextureMipmapEXT(TextureName, GL_TEXTURE_2D);

	return glf::checkError("initTexture2D");
}

bool initFramebuffer()
{
	glGenTextures(1, &MultisampleTextureName);
	glTextureImage2DMultisampleNV(MultisampleTextureName, GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, GL_TRUE);

	glGenFramebuffers(1, &FramebufferRenderName);
	glNamedFramebufferTextureEXT(FramebufferRenderName, GL_COLOR_ATTACHMENT0, MultisampleTextureName, 0);
	if(glCheckNamedFramebufferStatusEXT(FramebufferRenderName, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

    glGenTextures(1, &ColorTextureName);
	glTextureImage2DEXT(ColorTextureName, GL_TEXTURE_2D, 0, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glGenFramebuffers(1, &FramebufferResolveName);
	glNamedFramebufferTextureEXT(FramebufferResolveName, GL_COLOR_ATTACHMENT0, ColorTextureName, 0);
	if(glCheckNamedFramebufferStatusEXT(FramebufferResolveName, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	return glf::checkError("initFramebuffer");
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);
	glVertexArrayVertexAttribOffsetEXT(VertexArrayName, BufferName[buffer::VERTEX], glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), 0);
	glVertexArrayVertexAttribOffsetEXT(VertexArrayName, BufferName[buffer::VERTEX], glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), sizeof(glm::vec2));
	glEnableVertexArrayAttribEXT(VertexArrayName, glf::semantic::attr::POSITION);
	glEnableVertexArrayAttribEXT(VertexArrayName, glf::semantic::attr::TEXCOORD);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	bool Validated(true);
	Validated = Validated && glf::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);
	Validated = Validated && glf::checkExtension("GL_NV_texture_multisample");
	
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initSampler();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initVertexArray();
	if(Validated)
		Validated = initTexture2D();
	if(Validated)
		Validated = initFramebuffer();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteBuffers(buffer::MAX, BufferName);
	glDeleteProgram(ProgramName[program::VERTEX]);
	glDeleteProgram(ProgramName[program::FRAGMENT]);
	glDeleteTextures(1, &TextureName);
	glDeleteTextures(1, &ColorTextureName);
	glDeleteTextures(1, &MultisampleTextureName);
	glDeleteFramebuffers(1, &FramebufferRenderName);
	glDeleteFramebuffers(1, &FramebufferResolveName);
	glDeleteVertexArrays(1, &VertexArrayName);
	glDeleteSamplers(1, &SamplerName);
	glDeleteProgramPipelines(1, &PipelineName);

	return glf::checkError("end");
}

void renderFBO(GLuint Framebuffer)
{
	{
		glm::mat4 Perspective = glm::perspective(45.0f, float(FRAMEBUFFER_SIZE.x) / FRAMEBUFFER_SIZE.y, 0.1f, 100.0f);
		glm::mat4 ViewFlip = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f,-1.0f, 1.0f));
		glm::mat4 ViewTranslate = glm::translate(ViewFlip, glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y * 2.0));
		glm::mat4 View = glm::rotate(ViewTranslate,-15.f, glm::vec3(0.f, 0.f, 1.f));
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Perspective * View * Model;

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(MVP), GL_MAP_WRITE_BIT);
		*Pointer = MVP;
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	glViewportIndexedf(0, 0, 0, float(FRAMEBUFFER_SIZE.x), float(FRAMEBUFFER_SIZE.y));
	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

	glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, TextureName);
	glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
	glBindVertexArray(VertexArrayName);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);

	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);

	glf::checkError("renderFBO");
}

void renderFB(GLuint TextureName)
{
	{
		glm::mat4 Perspective = glm::perspective(45.0f, float(Window.Size.x) / Window.Size.y, 0.1f, 100.0f);
		glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y * 2.0));
		glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Perspective * View * Model;

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(MVP), GL_MAP_WRITE_BIT);
		*Pointer = MVP;
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	//glProgramUniformMatrix4fv(ProgramName[program::VERTEX], UniformMVP, 1, GL_FALSE, &MVP[0][0]);

	glViewportIndexedfv(0, &glm::vec4(0, 0, Window.Size.x, Window.Size.y)[0]);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

	glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, TextureName);
	glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
	glBindSampler(0, SamplerName);
	glBindVertexArray(VertexArrayName);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);

	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);

	glf::checkError("renderFB");
}

void display()
{
	// Clear the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

	glBindProgramPipeline(PipelineName);

	// Step 1: Render the scene in a multisampled framebuffer
	glEnable(GL_MULTISAMPLE);
	renderFBO(FramebufferRenderName);
	glDisable(GL_MULTISAMPLE);

	// Step 2: Resolved multisampling
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FramebufferRenderName);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferResolveName);
	glBlitFramebuffer(
		0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 
		0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 
		GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// Step 3: Generated mipmaps
	glGenerateTextureMipmapEXT(ColorTextureName, GL_TEXTURE_2D);

	// Step 4: Render the colorbuffer from the multisampled framebuffer
	renderFB(ColorTextureName);

	glf::checkError("display");
	glf::swapBuffers();
}

int main(int argc, char* argv[])
{
	return glf::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, ::SAMPLE_MAJOR_VERSION, 
		::SAMPLE_MINOR_VERSION);
}
