//#include "../Bind/Common.h"
//
//#include "glew/GL/glew.h"
//
//#include "glm/vec3.hpp" // glm::vec3
//#include "glm/vec4.hpp" // glm::vec4
//#include "glm/mat4x4.hpp" // glm::mat4
//#include "glm/ext/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale
//#include "glm/ext/matrix_clip_space.hpp" // glm::perspective
//#include "glm/ext/scalar_constants.hpp" // glm::pi
//#include <cmath>
//#include "DrawCommon.h"
//
//
//export module Viewer3d;
//
//import Obejct3DModule;
//
//enum Key {
//	Key_Escape = 0x01000000,                // misc keys
//	Key_Tab = 0x01000001,
//	Key_Backtab = 0x01000002,
//	Key_Backspace = 0x01000003,
//	Key_Return = 0x01000004,
//	Key_Enter = 0x01000005,
//	Key_Insert = 0x01000006,
//	Key_Delete = 0x01000007,
//	Key_Pause = 0x01000008,
//	Key_Print = 0x01000009,               // print screen
//	Key_SysReq = 0x0100000a,
//	Key_Clear = 0x0100000b,
//	Key_Home = 0x01000010,                // cursor movement
//	Key_End = 0x01000011,
//	Key_Left = 0x01000012,
//	Key_Up = 0x01000013,
//	Key_Right = 0x01000014,
//	Key_Down = 0x01000015,
//	Key_PageUp = 0x01000016,
//	Key_PageDown = 0x01000017,
//	Key_Shift = 0x01000020,                // modifiers
//	Key_Control = 0x01000021,
//	Key_Meta = 0x01000022,
//	Key_Alt = 0x01000023,
//	Key_CapsLock = 0x01000024,
//	Key_NumLock = 0x01000025,
//	Key_ScrollLock = 0x01000026,
//	Key_F1 = 0x01000030,                // function keys
//	Key_F2 = 0x01000031,
//	Key_F3 = 0x01000032,
//	Key_F4 = 0x01000033,
//	Key_F5 = 0x01000034,
//	Key_F6 = 0x01000035,
//	Key_F7 = 0x01000036,
//	Key_F8 = 0x01000037,
//	Key_F9 = 0x01000038,
//	Key_F10 = 0x01000039,
//	Key_F11 = 0x0100003a,
//	Key_F12 = 0x0100003b,
//	Key_F13 = 0x0100003c,
//	Key_F14 = 0x0100003d,
//	Key_F15 = 0x0100003e,
//	Key_F16 = 0x0100003f,
//	Key_F17 = 0x01000040,
//	Key_F18 = 0x01000041,
//	Key_F19 = 0x01000042,
//	Key_F20 = 0x01000043,
//	Key_F21 = 0x01000044,
//	Key_F22 = 0x01000045,
//	Key_F23 = 0x01000046,
//	Key_F24 = 0x01000047,
//	Key_F25 = 0x01000048,                // F25 .. F35 only on X11
//	Key_F26 = 0x01000049,
//	Key_F27 = 0x0100004a,
//	Key_F28 = 0x0100004b,
//	Key_F29 = 0x0100004c,
//	Key_F30 = 0x0100004d,
//	Key_F31 = 0x0100004e,
//	Key_F32 = 0x0100004f,
//	Key_F33 = 0x01000050,
//	Key_F34 = 0x01000051,
//	Key_F35 = 0x01000052,
//	Key_Super_L = 0x01000053,                 // extra keys
//	Key_Super_R = 0x01000054,
//	Key_Menu = 0x01000055,
//	Key_Hyper_L = 0x01000056,
//	Key_Hyper_R = 0x01000057,
//	Key_Help = 0x01000058,
//	Key_Direction_L = 0x01000059,
//	Key_Direction_R = 0x01000060,
//	Key_Space = 0x20,                // 7 bit printable ASCII
//	Key_Any = Key_Space,
//	Key_Exclam = 0x21,
//	Key_QuoteDbl = 0x22,
//	Key_NumberSign = 0x23,
//	Key_Dollar = 0x24,
//	Key_Percent = 0x25,
//	Key_Ampersand = 0x26,
//	Key_Apostrophe = 0x27,
//	Key_ParenLeft = 0x28,
//	Key_ParenRight = 0x29,
//	Key_Asterisk = 0x2a,
//	Key_Plus = 0x2b,
//	Key_Comma = 0x2c,
//	Key_Minus = 0x2d,
//	Key_Period = 0x2e,
//	Key_Slash = 0x2f,
//	Key_0 = 0x30,
//	Key_1 = 0x31,
//	Key_2 = 0x32,
//	Key_3 = 0x33,
//	Key_4 = 0x34,
//	Key_5 = 0x35,
//	Key_6 = 0x36,
//	Key_7 = 0x37,
//	Key_8 = 0x38,
//	Key_9 = 0x39,
//	Key_Colon = 0x3a,
//	Key_Semicolon = 0x3b,
//	Key_Less = 0x3c,
//	Key_Equal = 0x3d,
//	Key_Greater = 0x3e,
//	Key_Question = 0x3f,
//	Key_At = 0x40,
//	Key_A = 0x41,
//	Key_B = 0x42,
//	Key_C = 0x43,
//	Key_D = 0x44,
//	Key_E = 0x45,
//	Key_F = 0x46,
//	Key_G = 0x47,
//	Key_H = 0x48,
//	Key_I = 0x49,
//	Key_J = 0x4a,
//	Key_K = 0x4b,
//	Key_L = 0x4c,
//	Key_M = 0x4d,
//	Key_N = 0x4e,
//	Key_O = 0x4f,
//	Key_P = 0x50,
//	Key_Q = 0x51,
//	Key_R = 0x52,
//	Key_S = 0x53,
//	Key_T = 0x54,
//	Key_U = 0x55,
//	Key_V = 0x56,
//	Key_W = 0x57,
//	Key_X = 0x58,
//	Key_Y = 0x59,
//	Key_Z = 0x5a,
//};
//
//// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
//enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };
//
//// Default camera values
//const GLfloat YAW = -90.0f;
//const GLfloat PITCH = 0.0f;
//const GLfloat SPEED = 200.0f;
//const GLfloat SENSITIVTY = 0.25f;
//const GLfloat ZOOM = 45.0f;
//
//
//// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
//class CameraGui
//{
//	bool enableTraking = true;
//public:
//
//	// Camera Attributes
//	glm::vec3 Position;
//	glm::vec3 Front, baseFront;
//	glm::vec3 Up;
//	glm::vec3 Right, baseRight;
//	glm::vec3 WorldUp;
//	// Eular Angles
//	GLfloat Yaw;
//	GLfloat Pitch;
//	// Camera options
//	GLfloat MovementSpeed;
//	GLfloat MouseSensitivity;
//	GLfloat Zoom;
//
//	// Constructor with vectors
//	CameraGui(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
//	{
//		this->Position = position;
//		this->WorldUp = up;
//		this->Yaw = yaw;
//		this->Pitch = pitch;
//		this->updateCameraVectors();
//		firstMouse = true;
//		baseFront = Front;
//		baseRight = Right;
//	}
//	// Constructor with scalar values
//	CameraGui(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
//	{
//		this->Position = glm::vec3(posX, posY, posZ);
//		this->WorldUp = glm::vec3(upX, upY, upZ);
//		this->Yaw = yaw;
//		this->Pitch = pitch;
//		this->updateCameraVectors();
//		firstMouse = true;
//		baseFront = Front;
//		baseRight = Right;
//	}
//
//	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
//	glm::mat4x4 GetViewMatrix()
//	{
//		//Up->y*=-1;
//		glm::mat4x4 view = glm::lookAt(this->Position, this->Position + this->Front, this->Up);
//		return view;
//	}
//
//	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
//
//	float setZ(float word)
//	{
//		return word < 0 ? -1 : 1;
//	}
//	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime, float factor = 1.f)
//	{
//		GLfloat velocity = this->MovementSpeed * deltaTime;
//		velocity *= factor;
//
//		glm::vec3 front;
//		front.x = (cos(glm::radians(this->Yaw)));
//		front.y = (0);
//		front.z = (sin(glm::radians(this->Yaw)));
//		front = glm::normalize(front);
//
//		if (direction == FORWARD)
//		{
//			//			glm::vec3 poscor(z = nac(Front.x), 0, z = nac(Front.z));
//			//			glm::vec3 res = this->Position + poscor * velocity;
//			glm::vec3 res = this->Position + front * velocity;
//
//			this->Position.x = (res.x);
//			this->Position.z = (res.z);
//		}
//		if (direction == BACKWARD)
//		{
//			glm::vec3 res = this->Position - front * velocity;
//			this->Position.x = (res.x);
//			this->Position.z = (res.z);
//		}
//		if (direction == LEFT)
//		{
//			glm::vec3 res = this->Position - Right * velocity;
//			this->Position.x = (res.x);
//			this->Position.z = (res.z);
//		}
//		if (direction == RIGHT)
//		{
//			glm::vec3 res = this->Position + this->Right * velocity;
//			this->Position.x = (res.x);
//			this->Position.z = (res.z);
//		}
//
//	}
//
//	GLfloat lastX = 400, lastY = 300;
//	bool firstMouse = true;
//
//	//	GLfloat deltaTime = 0.0f;
//	//	GLfloat lastFrame = 0.0f;
//
//	bool invertX = false, invertY = false;
//	void setEnableTraking(bool val)
//	{
//		enableTraking = val;
//		if (val == false)
//		{
//			firstMouse = true;
//		}
//	}
//
//	bool isEnableTraking()
//	{
//		return enableTraking;
//	}
//	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
//	void ProcessMouseMovement(GLfloat xpos, GLfloat ypos, float delta, GLboolean constrainPitch = true)
//	{
//		if (!enableTraking)
//			return;
//		if (firstMouse)
//		{
//			lastX = xpos;
//			lastY = ypos;
//			firstMouse = false;
//		}
//
//
//		GLfloat xoffset = xpos - lastX;
//		GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left
//
//		lastX = xpos;
//		lastY = ypos;
//		//		std::cout << "mpoused";
//
//		xoffset *= this->MouseSensitivity * delta * 40;
//		yoffset *= this->MouseSensitivity * delta * 40;
//
//
//		this->Yaw += invertX ? -xoffset : xoffset;
//		this->Pitch += invertY ? -yoffset : yoffset;
//
//		//		std::cout << Yaw << Pitch << Position.x << Position.y << Position.z;
//				// Make sure that when pitch is out of bounds, screen doesn't get flipped
//		if (constrainPitch)
//		{
//			if (this->Pitch > 89.0f)
//				this->Pitch = 89.0f;
//			if (this->Pitch < -89.0f)
//				this->Pitch = -89.0f;
//		}
//
//		// Update Front, Right and Up Vectors using the updated Eular angles
//		this->updateCameraVectors();
//	}
//
//	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
//	void ProcessMouseScroll(GLfloat yoffset, float deltaTime)
//	{
//		GLfloat velocity = 25 * yoffset * deltaTime;
//		this->Position += this->Front * velocity;
//		//		if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
//		//			this->Zoom -= yoffset;
//		//		if (this->Zoom <= 1.0f)
//		//			this->Zoom = 1.0f;
//		//		if (this->Zoom >= 45.0f)
//		//			this->Zoom = 45.0f;
//	}
//
//	// Calculates the front vector from the Camera's (updated) Eular Angles
//	void updateCameraVectors()
//	{
//		// Calculate the new Front vector
//		glm::vec3 front;
//		front.x = (cos(glm::radians(this->Yaw)) * cos(glm::radians(Pitch)));
//		front.y = (sin(glm::radians(this->Pitch)));
//		front.z = (sin(glm::radians(this->Yaw)) * cos(glm::radians(Pitch)));
//		this->Front = glm::normalize(front);
//		// Also re-calculate the Right and Up vector
//		this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
//		this->Up = glm::normalize(glm::cross(this->Right, this->Front));
//		//Up.y
////		std::cout << Front << Right << Up;
//	}
//private:
//};
//
//
//
////#include "cubegl.h"
//
////struct MaterialInfo
////{
////	BackString Name;
////	glm::vec3 Ambient;
////	glm::vec3 Diffuse;
////	glm::vec3 Specular;
////	float Shininess;
////};
//
////struct LightInfo
////{
////	QVector4D Position;
////	glm::vec3 Intensity;
////};
//
////struct Mesh
////{
////	BackString name;
////	unsigned int indexCount;
////	unsigned int indexOffset;
////	QSharedPointer<MaterialInfo> material;
////};
//
////struct Node
////{
////	BackString name;
////	glm::mat4 transformation;
////	QVector<QSharedPointer<Mesh>> meshes;
////	QVector<Node> nodes;
////};
//
//
//enum class DisplayMode {
//	Heimap,
//	object,
//	texture
//
//};
////struct ObjBuffers
////{
////	QOpenGLBuffer arrayBuf;
////	QOpenGLBuffer indexBuf;
////};
//
//class QOpenGLBuffer
//{
//public:
//	void create()
//	{
//		glGenBuffers(1, &m_buffer);
//	}
//	void bind()
//	{
//		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
//	}
//	void allocate(const void* data, int count)
//	{
//		glBufferData(GL_ARRAY_BUFFER, count, data, GL_STATIC_DRAW);
//	}
//
//	void setUsagePattern(bool useDymanic)
//	{
//		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
//		glBufferData(GL_ARRAY_BUFFER, m_size, nullptr, dymanic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
//	}
//
//private:
//	GLuint m_buffer;
//	GLint m_size = 0;
//	bool dymanic = true;
//};
//
//class QOpenGLShaderProgram {
//public:
//	void create() {
//		m_program = glCreateProgram();
//	}
//	bool addShaderFromSourceCode(GLenum type, const char* source) {
//		GLuint shader = glCreateShader(type);
//		glShaderSource(shader, 1, &source, nullptr);
//		glCompileShader(shader);
//		glAttachShader(m_program, shader);
//		return true;
//	}
//	void link() {
//		glLinkProgram(m_program);
//	}
//	void bind() {
//		glUseProgram(m_program);
//	}
//	GLint attributeLocation(const char* name) {
//		return glGetAttribLocation(m_program, name);
//	}
//	GLint uniformLocation(const char* name) {
//		return glGetUniformLocation(m_program, name);
//	}
//private:
//	GLuint m_program;
//};
//
//class QOpenGLVertexArrayObject
//{
//public:
//	void create() {
//		glGenVertexArrays(1, &m_vao);
//	}
//	void bind() {
//		glBindVertexArray(m_vao);
//	}
//	void release() {
//		glBindVertexArray(0);
//	}
//private:
//	GLuint m_vao;
//};
//
//class Terrain
//{
//	// OpenGL data
//	size_t faceSize, vertSize;
//	QOpenGLBuffer arrayBuf;
//	QOpenGLBuffer indexBuf;
//	//	QVector<ObjBuffers*> buffers;
//	QOpenGLShaderProgram heimapShader, objectShader, textureShader;
//
//
//	// Display data
//	DisplayMode displayMode;
//	int textNum;
//	float minH;
//	float maxH;
//	std::vector<TextureId*> textures;
//
//public:
//	Terrain();
//	~Terrain();
//	void initGL();
//	void displayHeimap(float minH, float maxH)
//	{
//		displayMode = DisplayMode::Heimap;
//		this->minH = minH;
//		this->maxH = maxH;
//	}
//
//	Object3d obj;
//	void displayTexture(int textNum)
//	{
//		this->textNum = textNum;
//		displayMode = DisplayMode::texture;
//	}
//
//	void displayObject() { displayMode = DisplayMode::object; }
//
//	void addTexture(BackString path);
//
//	QOpenGLVertexArrayObject vao;
//
//
//	void clearTextures()
//	{
//		vao.bind();
//		for (int var = 0; var < textures.size(); ++var)
//		{
//			if (textures[var] != nullptr)
//			{
//				delete textures[var];
//				textures[var] = nullptr;
//			}
//		}
//		vao.release();
//	}
//
//private:
//	//QOpenGLExtraFunctions* f;
//
//	static void initShader(QOpenGLShaderProgram& prog, BackString vert, BackString frag)
//	{
//		// Compile vertex shader
//		if (!prog.addShaderFromSourceCode(0, vert.c_str())) // QOpenGLShader::Vertex
//			return;
//
//		// Compile fragment shader
//		if (!prog.addShaderFromSourceCode(1, frag.c_str())) // QOpenGLShader::Fragment
//			return;
//
//		// Link shader pipeline
//		//if (!prog.link())
//		//	return;
//
//		// Bind shader pipeline for use
////		if (!prog.bind())
////			return;
//	}
//
//	void initShaders();
//
//public:
//	void initArrays();
//	void drawFull(glm::mat4& view, glm::mat4& projection);
//	void setTexture(int num, BackString path);
//
//	vertex getValue(size_t offset);
//	float getValue(int x, int z);
//};
//
//Terrain::Terrain() : indexBuf(QOpenGLBuffer::IndexBuffer), displayMode(DisplayMode::Heimap), textNum(0)
//{
//	textures.push_back(nullptr);
//	textures.push_back(nullptr);
//}
//
//Terrain::~Terrain()
//{
//	clearTextures();
//}
//
//void Terrain::initGL()
//{
//	initShaders();
//}
//
//void Terrain::initShaders()
//{
//	initShader(heimapShader, ":/shaders/HeightFactor.vert", ":/shaders/HeightFactor.frag");
//	initShader(objectShader, ":/shaders/simpleColor.vert", ":/shaders/terraColor.frag");
//	initShader(textureShader, ":/shaders/simpleColor.vert", ":/shaders/simpleColor.frag");
//	//	initShader(textureShader, ":/vshader.glsl", ":/fshader.glsl");
//}
//
//
//float Terrain::getValue(int x, int z)
//{
//	if (x < 0 || z < 0 || x >= proj->modelWid || z >= proj->modelHei)
//		return -9999;
//
//	return getValue(x * proj->modelHei + z).y;
//}
//
//vertex Terrain::getValue(size_t offset)
//{
//	vertex vert[1];
//	arrayBuf.bind();
//	arrayBuf.read(offset * sizeof(vertex), vert, sizeof(vertex));
//	arrayBuf.release();
//	return vert[0];
//}
//
//void Terrain::setTexture(int num, BackString path)
//{
//	// Load cube.png image
//	//":/shaders/cube.png"
//	std::cout << path;
//	if (path.length() == 0)
//		return;
//	if (path.starts_with("file:///"))
//		path = path.remove(0, 8);
//
//	QOpenGLTexture* texture = new QOpenGLTexture(QImage(path).mirrored());
//
//	texture->setMinificationFilter(QOpenGLTexture::Nearest);
//	texture->setMagnificationFilter(QOpenGLTexture::Linear);
//
//	// Wrap texture coordinates by repeating
//	// f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
//	texture->setWrapMode(QOpenGLTexture::WrapMode::Repeat);
//
//	if (textures[num] != nullptr)
//		delete textures[num];
//	textures[num] = texture;
//}
//
//void Terrain::addTexture(BackString path)
//{
//	// Load cube.png image
//	//":/shaders/cube.png"
//	std::cout << path;
//
//	QOpenGLTexture* texture = new QOpenGLTexture(QImage(path).mirrored());
//
//	texture->setMinificationFilter(QOpenGLTexture::Nearest);
//	texture->setMagnificationFilter(QOpenGLTexture::Linear);
//
//	// Wrap texture coordinates by repeating
//	// f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
//	texture->setWrapMode(QOpenGLTexture::WrapMode::Repeat);
//
//	textures.push_back(texture);
//}
//
//void Terrain::initArrays()
//{
//	arrayBuf.create();
//	indexBuf.create();
//	vao.create();
//	vao.bind();
//
//	textureShader.bind();
//	// Transfer vertex data to VBO 0
//	std::cout << "Vert tatol size:" << obj.vetexes.size() * sizeof(vertex);
//	arrayBuf.bind();
//	arrayBuf.allocate((const void*)obj.vetexes.data(), obj.vetexes.size() * sizeof(vertex));
//	arrayBuf.setUsagePattern(false);
//
//	// Transfer index data to VBO 1
//	indexBuf.bind();
//	indexBuf.allocate((const void*)obj.faces.data(), obj.faces.size() * sizeof(face));
//	indexBuf.setUsagePattern(false);
//
//	int offset = 0;
//	int vertexLocation = textureShader.attributeLocation("a_position");
//	textureShader.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(vertex));
//	textureShader.enableAttributeArray(vertexLocation);
//
//	// Offset for texture coordinate
//	offset += sizeof(glm::vec3);
//
//	int texcoordLocation = textureShader.attributeLocation("a_texcoord");
//	textureShader.setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(vertex));
//	textureShader.enableAttributeArray(texcoordLocation);
//
//	vao.release();
//	textureShader.release();
//
//	arrayBuf.release();
//	indexBuf.release();
//	faceSize = obj.faces.size() * 3;
//	vertSize = obj.vetexes.size();
//	obj.faces.clear();
//	obj.vetexes.clear();
//}
//
//void Terrain::drawFull(glm::mat4& view, glm::mat4& projection)
//{
//	// Tell OpenGL which VBOs to use
//
//	// Offset for position
//	QOpenGLShaderProgram* curshader;
//	switch (this->displayMode)
//	{
//	case DisplayMode::Heimap:
//		curshader = &this->heimapShader;
//		curshader->bind();
//		//curshader->setUniformValue("minHei", proj->getImgMinVal() / proj->u_displayFactor);
//		//curshader->setUniformValue("maxHei", proj->getImgMaxVal() / proj->u_displayFactor);
//		break;
//	case DisplayMode::object:
//		curshader = &this->objectShader;
//		curshader->bind();
//		break;
//	case DisplayMode::texture:
//		curshader = &this->textureShader;
//		if (textNum < textures.size() && textures[textNum] != nullptr)
//			textures[textNum]->bind();
//
//		curshader->bind();
//		//			int idt = textures[textNum]->textureId() - 1;
//		curshader->setUniformValue("texture0", 0);
//		break;
//	default:
//		return;
//	}
//
//	glm::mat4 model;
//	model.setToIdentity;
//	model.scale(1, proj->u_heiFactor, 1);
//	model.translate(0, 0, 0);
//
//	curshader->setUniformValue("projection", projection);
//	curshader->setUniformValue("view", view);
//	curshader->setUniformValue("model", model);
//
//	// Draw cube geometry using indices from VBO 1
//	vao.bind();
//	glDrawElements(GL_TRIANGLES, faceSize, GL_UNSIGNED_INT, nullptr);
//	vao.release();
//
//	curshader->release();
//
//	switch (this->displayMode)
//	{
//	case DisplayMode::texture:
//		if (textNum < textures.size() && textures[textNum] != nullptr)
//			textures[textNum]->release();
//		break;
//	default:
//		break;
//	}
//}
//
//
//
//class CubeGL;
//typedef std::chrono::time_point<std::chrono::steady_clock> timeType;
//
//class MainWidget : public QOpenGLWidget, protected QOpenGLFunctions
//{
//	Q_OBJECT
//
//		const float zNear = 0.1, zFar = 10000.0, fov = 60.0;
//
//
//public:
//	bool useTimer = false;
//
//	MainWidget(QWidget* parent = nullptr);
//	~MainWidget();
//
//	QLabel* fpsLabel;
//	void Do_Movement();
//
//	Terrain* terra = nullptr;
//	SpotZones* zones;
//	SpotZones* badZones;
//	StaticMarkers* markers;
//	UserMarkers* userMarkers;
//	std::unique_ptr<StaticMarkers> importedMakrers;
//	Line line;
//
//	bool drawTerra = false;
//	bool drawZones = false;
//
//
//	void printErrors();
//
//
//	void readGeoshape();
//protected:
//	glm::vec3 getMouseCast(const QVector2D& mousePos);
//
//	void mouseMoveEvent(QMouseEvent* event) override;
//	void wheelEvent(QWheelEvent* event) override;
//	void keyPressEvent(QKeyEvent* event) override;
//	void keyReleaseEvent(QKeyEvent* event) override;
//
//	void mousePressEvent(QMouseEvent* e) override;
//	void mouseReleaseEvent(QMouseEvent* e) override;
//
//	void initializeGL() override;
//	void resizeGL(int w, int h) override;
//	void paintGL() override;
//
//	void initShaders();
//	void initTextures();
//
//private:
//	bool shitfp;
//	bool ctrl;
//	bool keys[1024];
//	bool pressed[1024];
//	bool polyLine = false;
//	float aspect;
//	glm::vec3 cameraStartPos;
//
//
//	double deltaTime = 0;
//	timeType lastFrame;
//	timeType timeStart;
//
//	CameraGui* camera;
//	SkyBoxGUI* sky;
//	Text2d* text;
//
//	CubeGui* cubeRot = nullptr;
//
//
//	QOpenGLDebugLogger* logger;
//
//	glm::mat4 projection;
//
//	QVector2D mousePressPosition;
//	QVector2D mouseCurrentPosition;
//	glm::vec3 rotationAxis;
//	float angularSpeed = 0;
//	QQuaternion rotation;
//
//	QOpenGLExtraFunctions* f;
//	// QPaintDevice interface
//	// QWidget interface
//	QVector4D getVal(int x, int z);
//
//};
//
//
//MainWidget::MainWidget(QWidget*/*parent*/)
//{
//	cameraStartPos = glm::vec3(-25, 551, -159);
//
////	sky = new SkyBoxGUI();//58.25 -34.25 -24.4309 573.158 -205.05
////	camera = new CameraGui(cameraStartPos, glm::vec3(0, 1, 0), 56, -32.25);
////	terra = new Terrain();
////	zones = new SpotZones();
////	badZones = new SpotZones();
////	text = new Text2d();
////
////	markers = new StaticMarkers();
////#ifdef ENABLE_MARKERS
////	userMarkers = new UserMarkers();
////#endif
////	importedMakrers.reset(new StaticMarkers());
////
////	useTimer = false;
//}
//
//MainWidget::~MainWidget()
//{
//	// Make sure the context is current when deleting the texture
//	// and the buffers.
//
//	delete sky;
//	delete terra;
//	delete camera;
//	delete zones;
//	delete badZones;
//	delete text;
//	delete markers;
//#ifdef ENABLE_MARKERS
//	delete userMarkers;
//#endif
//	delete cubeRot;
//}
//
////! [1]
//
//
//void MainWidget::initializeGL()
//{
//	memset(keys, 0, 1024);
//
//	timeStart = std::chrono::steady_clock::now();
//	lastFrame = std::chrono::steady_clock::now();
//
//	initializeOpenGLFunctions();
//	f = QOpenGLContext::currentContext()->extraFunctions();
//	//	QOpenGLContext *ctx = QOpenGLContext::currentContext();
//
//
//	glEnable(GL_DEPTH_TEST);
//	// Enable back face culling
////	glEnable(GL_CULL_FACE);
////	glCullFace(GL_FRONT_AND_BACK);
//
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	glClearColor(0.3, 0.3, 0, 1);
//
//	sky->initializeGL();
//
//	camera->setEnableTraking(false);
//	camera->invertX = true;
//	camera->invertY = true;
//	//	camera->invertX = false;
//	//	camera->invertY = false;
//
//	terra->initGL();
//
//	zones->initGL();
//	badZones->initGL();
//	text->initGL();
//
//	markers->initGL();
//#ifdef ENABLE_MARKERS
//	userMarkers->initGL();
//#endif
//
//	importedMakrers->initGL();
//}
//
//
//glm::vec4 MainWidget::getVal(int x, int z) { return glm::vec4(x, terra->getValue(x, z), z, 1.0); }
//
//
//void MainWidget::mousePressEvent(QMouseEvent* e)
//{
//	camera->setEnableTraking(true);
//
//	// Save mouse press position
//	mousePressPosition = QVector2D(e->localPos());
//	//	QVector2D vec(mousePressPosition.x * width(), mousePressPosition.y * height());
//
//	//	if (e->button() != Qt::MouseButton::RightButton)
//	//		return;
//
//}
//
//void MainWidget::mouseReleaseEvent(QMouseEvent* e)
//{
//	camera->setEnableTraking(false);
//	// #######################################################
//	// #######################################################
//	// #######################################################
//
//	glm::vec2 diff = glm::vec2(e->localPos()) - mousePressPosition;
//	glm::vec3 n = glm::normalize(glm::vec3(diff.y, diff.x, 0.0));
//	float acc = diff.length() / 100.0;
//	rotationAxis = glm::normalize(rotationAxis * angularSpeed + n * acc);
//	angularSpeed += acc;
//}
//
//void MainWidget::mouseMoveEvent(QMouseEvent* event)
//{
//	//	sky->mouseMoveEvent(event);
//	mouseCurrentPosition = glm::vec2(event->localPos());
//	camera->ProcessMouseMovement(mouseCurrentPosition.x, mouseCurrentPosition.y, deltaTime);
//	//	std::cout << vec;
//#ifdef ENABLE_MARKERS
//
//	if (userMarkers->show)
//	{
//		//		vec.x = (event->localPos().x * width());
//		//		vec.y = (event->localPos().y * height());
////		QVector2D vecs(event->localPos().x * width(), event->localPos().y * height());
//
//		userMarkers->move(0, getMouseCast(mouseCurrentPosition));
//	}
//#endif
//}
//
//void MainWidget::wheelEvent(QWheelEvent* event)
//{
//	//	sky->wheelEvent(event);
//	camera->ProcessMouseScroll(event->delta(), deltaTime);
//	//	if (!useTimer)
//	//		update();
//}
//
//
//void MainWidget::keyPressEvent(QKeyEvent* event)
//{
//	shitfp = event->modifiers() & Qt::ShiftModifier;
//	ctrl = event->modifiers() & Qt::ControlModifier;
//	auto key = event->nativeVirtualKey;
//	if (key < 1024)
//	{
//		//		bool jg = keys[key];
//		keys[key] = true;
//
//		//		if (!jg)
//		//		{
//		//			++pressed;
//		//		}
//
//		if (key == Key::Key_R)
//		{
//			camera->Position = cameraStartPos;
//			camera->Up = glm::vec3(0, 1, 0);
//			camera->Yaw = 56;
//			camera->Pitch = -32.25;
//			memset(keys, 0, 1024);
//
//			//			line.setLine(glm::vec3(0, 0, 0), glm::vec3(100, 100, 100));
//
//
//
//						// camera->Position = glm::vec3(981.52, 29.6308, 271.393);
//						// camera->Up = glm::vec3(0.468725, 0.881798, 0.0522446);
//						// camera->Yaw = 6.36;
//						// camera->Pitch = -28.14;
//			//			camera->Position = glm::vec3(1024.25, 29.6308, 233.794);
//			//			camera->Up = glm::vec3(0.468725, 0.881798, 0.0522446);
//			//			camera->Yaw = 6.36;
//			//			camera->Pitch = -28.14;
//			camera->updateCameraVectors();
//		}
//
//
//		// if (ctrl)
//		// {
//		// 	std::cout << camera->Position;
//		// 	std::cout << camera->Up;
//		// 	std::cout << camera->Yaw;
//		// 	std::cout << camera->Pitch;
//		// 	memset(keys, 0, 1024);
//		// }
//	}
//	//	if (!useTimer)
//	//		update();
//}
//
//void MainWidget::keyReleaseEvent(QKeyEvent* event)
//{
//	//shitfp = event->modifiers() & Qt::ShiftModifier;
//	//ctrl = event->modifiers() & Qt::ControlModifier;
//
//	auto key = event->nativeVirtualKey;
//	if (key < 1024)
//	{
//		//		bool jg = keys[key];
//
//		keys[key] = false;
//
//		//		if (jg)
//		//		{
//		//			--pressed;
//		//		}
//
//	}
//
//	//	std::cout << pressed;
//
//	//	if (!useTimer)
//	//		update();
//}
//
////! [5]
//void MainWidget::resizeGL(int w, int h)
//{
//	//	sky->resizeGL(w, h);
//
//		// Calculate aspect ratio
//	aspect = float(w) / float(h ? h : 1);
//
//	//	if (!useTimer)
//	//		update();
//
//		// Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
//	//	const float zNear = 3.0, zFar = 7.0, fov = 45.0;
//
//		// Reset projection
//	//	projection.setToIdentity;
//
//		// Set perspective projection
//	//	projection.perspective(fov, aspect, zNear, zFar);x
//
//}
////! [5]
//
//void MainWidget::printErrors()
//{
//	//	const QList<QOpenGLDebugMessage> messages = logger->loggedMessages();
//	//	for (const QOpenGLDebugMessage &message : messages)
//	//		std::cout << message;
//
//	GLenum err;
//	while ((err = glGetError()) != GL_NO_ERROR)
//	{
//		switch (err)
//		{
//		case GL_INVALID_ENUM:
//			std::cout << "GL_INVALID_ENUM";
//			break;
//		case GL_INVALID_VALUE:
//			std::cout << "GL_INVALID_VALUE";
//			break;
//		case GL_INVALID_OPERATION:
//			std::cout << "GL_INVALID_OPERATION";
//			break;
//		case GL_INVALID_FRAMEBUFFER_OPERATION:
//			std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION";
//			break;
//		case GL_OUT_OF_MEMORY:
//			std::cout << "GL_OUT_OF_MEMORY";
//			break;
//		case GL_STACK_UNDERFLOW:
//			std::cout << "GL_STACK_UNDERFLOW";
//			break;
//		case GL_STACK_OVERFLOW:
//			std::cout << "GL_STACK_OVERFLOW";
//			break;
//		default:
//			break;
//		}
//		std::cout << err;
//		// Process/log the error.
//	}
//}
//double timediff(timeType& t1, timeType& t2)
//{
//	return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t2).count() * 0.001;
//}
//void MainWidget::paintGL()
//{
//	angularSpeed *= 0.99;
//
//	if (angularSpeed < 0.01)
//	{
//		angularSpeed = 0.0;
//	}
//	else
//	{
//		// Update rotation
//		rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
//	}
//
//
//	timeType currentFrame = std::chrono::steady_clock::now();
//	deltaTime = timediff(currentFrame, lastFrame);
//	lastFrame = currentFrame;
//	if (fpsLabel != nullptr)
//	{
//		fpsLabel->setText("fps: " + intToStr(round(1.0 / deltaTime)));
//	}
//
//	Do_Movement();
//	glm::mat4 view = camera->GetViewMatrix();
//	glm::mat4 skyboxview(view.normalMatrix);
//	glm::mat4 projection;
//	projection.setToIdentity();
//	projection.perspective(fov, aspect, zNear, zFar);
//
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//
//	//	geometries->model.setToIdentity;
//	//	geometries->model.translate(0.0, 0.0, 0.0);
//	//	geometries->model.rotate(timediff(currentFrame, timeStart) * 20.0f, glm::vec3(0.0f, 1.0f, 0.0f));
//	//	geometries->drawCubeGeometry(view, projection);
//
//	if (drawTerra)
//	{
//		//		if (polyLine)
//		//			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//
//		terra->drawFull(view, projection);
//		markers->renderGL(view, projection);
//
//#ifdef ENABLE_MARKERS
//		if (userMarkers->show)
//			userMarkers->renderGL(view, projection);
//#endif
//
//		if (importedMakrers->show)
//			importedMakrers->renderGL(view, projection);
//
//		//		if (polyLine)
////			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//
//		//		line.renderGL(view, projection);
//	}
//
//
//	///skybox
//	glDepthFunc(GL_LESS);
//
//	printErrors();
//}
//
//
//void MainWidget::Do_Movement()
//{
//	// Camera controls
//	float factor = 1.f;
//	if (shitfp)
//		factor = 2.f;
//	if (ctrl)
//		factor = 0.3;
//
//	if (keys[Key::Key_W])
//		camera->ProcessKeyboard(FORWARD, deltaTime, factor);
//	if (keys[Key::Key_S])
//		camera->ProcessKeyboard(BACKWARD, deltaTime, factor);
//	if (keys[Key::Key_A])
//		camera->ProcessKeyboard(LEFT, deltaTime, factor);
//	if (keys[Key::Key_D])
//		camera->ProcessKeyboard(RIGHT, deltaTime, factor);
//
//	if (keys[Key::Key_E])
//	{
//		camera->Yaw += 50 * deltaTime;
//		camera->updateCameraVectors();
//	}
//	if (keys[Key::Key_Q])
//	{
//		camera->Yaw -= 50 * deltaTime;
//		camera->updateCameraVectors();
//	}
//
//	if (keys[Key::Key_P])
//	{
//		polyLine = !polyLine;
//	}
//
//	if (keys[38])
//		camera->ProcessKeyboard(FORWARD, deltaTime, factor);
//	if (keys[40])
//		camera->ProcessKeyboard(BACKWARD, deltaTime, factor);
//	if (keys[37])
//		camera->ProcessKeyboard(LEFT, deltaTime, factor);
//	if (keys[39])
//		camera->ProcessKeyboard(RIGHT, deltaTime, factor);
//}
