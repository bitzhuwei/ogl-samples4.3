//**********************************
// OpenGL Transform Feedback Separated
// 06/04/2010 - 22/06/2011
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
	std::string const SAMPLE_NAME = "OpenGL Transform Feedback Separated";
	std::string const VERT_SHADER_SOURCE_TRANSFORM(glf::DATA_DIRECTORY + "gl-330/transform-separated.vert");
	std::string const VERT_SHADER_SOURCE_FEEDBACK(glf::DATA_DIRECTORY + "gl-330/feedback-separated.vert");
	std::string const FRAG_SHADER_SOURCE_FEEDBACK(glf::DATA_DIRECTORY + "gl-330/feedback-separated.frag");
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(3);
	int const SAMPLE_MINOR_VERSION(3);

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount(6);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec4);
	glm::vec4 const PositionData[VertexCount] =
	{
		glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f)
	};

	GLuint TransformProgramName(0);
	GLuint TransformArrayBufferName(0);
	GLuint TransformVertexArrayName(0);
	GLint TransformUniformMVP(0);

	GLuint FeedbackProgramName(0);
	GLuint FeedbackArrayBufferPositionName(0);
	GLuint FeedbackArrayBufferColorName(0);
	GLuint FeedbackVertexArrayName(0);
	GLint FeedbackUniformDiffuse(0);

	GLuint Query(0);
}//namespace

bool initDebugOutput()
{
	bool Validated(true);

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&glf::debugOutput, NULL);

	return Validated;
}

bool initProgram()
{
	bool Validated = true;
	
	// Create program
	if(Validated)
	{
		GLuint VertexShaderName = glf::createShader(GL_VERTEX_SHADER, VERT_SHADER_SOURCE_TRANSFORM);

		TransformProgramName = glCreateProgram();
		glAttachShader(TransformProgramName, VertexShaderName);
		glDeleteShader(VertexShaderName);

// These two approaches behave the same way
#if 1
		GLchar const * Strings[] = {"gl_Position", "block.Color"}; 
		glTransformFeedbackVaryings(TransformProgramName, 2, Strings, GL_SEPARATE_ATTRIBS); 
#else // OpenGL 4.0
		GLchar const * Strings[] = {"gl_Position", "gl_NextBuffer", "VertColor"}; 
		glTransformFeedbackVaryings(TransformProgramName, 3, Strings, GL_INTERLEAVED_ATTRIBS); 
#endif
		glLinkProgram(TransformProgramName);

		Validated = Validated && glf::checkProgram(TransformProgramName);

		// BUG AMD 10.12
		char Name[64];
		memset(Name, 0, 64);
		GLsizei Length(0);
		GLsizei Size(0);
		GLenum Type(0);

		glGetTransformFeedbackVarying(
			TransformProgramName,
			0,
			64,
			&Length,
			&Size,
			&Type,
			Name);

		Validated = Validated && (Size == 1) && (Type == GL_FLOAT_VEC4);
	}

	// Get variables locations
	if(Validated)
	{
		TransformUniformMVP = glGetUniformLocation(TransformProgramName, "MVP");
		Validated = Validated && (TransformUniformMVP >= 0);
	}

	// Create program
	if(Validated)
	{
		GLuint VertexShaderName = glf::createShader(GL_VERTEX_SHADER, VERT_SHADER_SOURCE_FEEDBACK);
		GLuint FragmentShaderName = glf::createShader(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE_FEEDBACK);

		FeedbackProgramName = glCreateProgram();
		glAttachShader(FeedbackProgramName, VertexShaderName);
		glAttachShader(FeedbackProgramName, FragmentShaderName);
		glDeleteShader(VertexShaderName);
		glDeleteShader(FragmentShaderName);

		glLinkProgram(FeedbackProgramName);
		Validated = Validated && glf::checkProgram(FeedbackProgramName);
	}

	return Validated && glf::checkError("initProgram");
}

bool initVertexArray()
{
	glf::checkError("initVertexArray 0");

	// Build a vertex array object
	glGenVertexArrays(1, &TransformVertexArrayName);
    glBindVertexArray(TransformVertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, TransformArrayBufferName);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	glf::checkError("initVertexArray 1");

	// Build a vertex array object
	glGenVertexArrays(1, &FeedbackVertexArrayName);
    glBindVertexArray(FeedbackVertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, FeedbackArrayBufferPositionName);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, FeedbackArrayBufferColorName);
		glVertexAttribPointer(glf::semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
		glEnableVertexAttribArray(glf::semantic::attr::COLOR);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool initArrayBuffer()
{
	// Generate a buffer object
	glGenBuffers(1, &TransformArrayBufferName);
    glBindBuffer(GL_ARRAY_BUFFER, TransformArrayBufferName);
    glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &FeedbackArrayBufferPositionName);
    glBindBuffer(GL_ARRAY_BUFFER, FeedbackArrayBufferPositionName);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * VertexCount, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &FeedbackArrayBufferColorName);
    glBindBuffer(GL_ARRAY_BUFFER, FeedbackArrayBufferColorName);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * VertexCount, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return glf::checkError("initArrayBuffer");
}

bool begin()
{
	bool Validated = true;
	Validated = Validated && glf::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);
	Validated = Validated && glf::checkExtension("GL_ARB_viewport_array");
	Validated = Validated && glf::checkExtension("GL_ARB_separate_shader_objects");

	glGenQueries(1, &Query);

	if(Validated && glf::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initArrayBuffer();
	if(Validated)
		Validated = initVertexArray();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteVertexArrays(1, &TransformVertexArrayName);
	glDeleteBuffers(1, &TransformArrayBufferName);
	glDeleteProgram(TransformProgramName);
	
	glDeleteVertexArrays(1, &FeedbackVertexArrayName);
	glDeleteBuffers(1, &FeedbackArrayBufferPositionName);
	glDeleteBuffers(1, &FeedbackArrayBufferColorName);
	glDeleteProgram(FeedbackProgramName);

	glDeleteQueries(1, &Query);

	return glf::checkError("end");
}

void display()
{
	// Compute the MVP (Model View Projection matrix)
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	// Set the display viewport
	glViewport(0, 0, Window.Size.x, Window.Size.y);

	// Clear color buffer with black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// First draw, capture the attributes
	{
		// Disable rasterisation, vertices processing only!
		glEnable(GL_RASTERIZER_DISCARD);

		glUseProgram(TransformProgramName);
		glUniformMatrix4fv(TransformUniformMVP, 1, GL_FALSE, &MVP[0][0]);

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, FeedbackArrayBufferPositionName); 
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, FeedbackArrayBufferColorName); 

		glBindVertexArray(TransformVertexArrayName);

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, Query); 
		glBeginTransformFeedback(GL_TRIANGLES);
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		glEndTransformFeedback();
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN); 

		glDisable(GL_RASTERIZER_DISCARD);
	}

	// Second draw, reuse the captured attributes
	{
		glUseProgram(FeedbackProgramName);

		GLuint PrimitivesWritten = 0;
		glGetQueryObjectuiv(Query, GL_QUERY_RESULT, &PrimitivesWritten);

		glBindVertexArray(FeedbackVertexArrayName);
		glDrawArraysInstanced(GL_TRIANGLES, 0, PrimitivesWritten * 3, 1);
	}

	glf::swapBuffers();
	glf::checkError("display");
}

int main(int argc, char* argv[])
{
	return glf::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, ::SAMPLE_MAJOR_VERSION, 
		::SAMPLE_MINOR_VERSION);
}
