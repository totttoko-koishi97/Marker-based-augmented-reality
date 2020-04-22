#include <string>
#include <opencv2/opencv.hpp>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>


#define PAT_ROW    (10)          /* パターンの行数 */
#define PAT_COL    (7)         /* パターンの列数 */
#define PAT_SIZE   (PAT_ROW*PAT_COL)
#define ALL_POINTS (IMAGE_NUM*PAT_SIZE)
#define CHESS_SIZE (20.0)       /* パターン1マスの1辺サイズ[mm] */

using namespace std;
using namespace glm;

int main() {

	//（１）事前にキャリブレーションしたカメラの内部パラメータと歪み係数を取得
	cv::Mat intrinsic;
	cv::Mat distCoeffs;
	const std::string fileName = "../Camera_Calibration/camera.xml";

	cv::FileStorage fs(fileName, cv::FileStorage::READ);
	if (!fs.isOpened()) {
		std::cout << "File cannot be opened.\n";
		return -1;
	}
	fs["intrinsic"] >> intrinsic;
	fs["distCoeffs"] >> distCoeffs;
	fs.release();

	cout << "get parameter" << endl;

	// (２)3次元空間座標の設定

	std::vector<cv::Point3f> object;
	for (int j = 0; j < PAT_ROW; j++)
	{
		for (int k = 0; k < PAT_COL; k++)
		{
			cv::Point3f p(
				j * CHESS_SIZE,
				k * CHESS_SIZE,
				0.0);
			object.push_back(p);
		}
	}
	std::vector<cv::Point3f> object3d;
	for (int j = 0; j < PAT_ROW; j++)
	{
		for (int k = 0; k < PAT_COL; k++)
		{
			cv::Point3f p(
				j * CHESS_SIZE,
				k * CHESS_SIZE,
				3 * CHESS_SIZE);
			object3d.push_back(p);
		}
	}

	cout << "get 3d cordinate" << endl;

	//GLの設定
	if (!glfwInit())
	{
		cout << "Failed to initialize GLFW" << endl;
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1280, 720, "Marker-based augmented reality", NULL, NULL);
	if (window == NULL) {
		cout << "Failed to open GLFW window." << endl;;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		cout << "Failed to initialize GLEW" << endl;
		glfwTerminate();
		return -1;
	}

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	
//////////////////完璧ゾーン
	GLuint image_VertexArrayID;
	glGenVertexArrays(1, &image_VertexArrayID);
	glBindVertexArray(image_VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint image_programID = LoadShaders("image.vertexshader", "image.fragmentshader");


	static const GLfloat image_g_vertex_buffer_data[] = {
			
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,

		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f

	};


	GLuint image_vertexbuffer;
	glGenBuffers(1, &image_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, image_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(image_g_vertex_buffer_data), image_g_vertex_buffer_data, GL_STATIC_DRAW);

	static const GLfloat image_g_uv_buffer_data[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f
	};

	GLuint image_uvbuffer;
	glGenBuffers(1, &image_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, image_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(image_g_uv_buffer_data), image_g_uv_buffer_data, GL_STATIC_DRAW);


	GLuint image_texture_id = glGetUniformLocation(image_programID,
		"image_texture_sampler");

	////////////////////////////////////完璧ゾーン終わり



	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	
	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");



	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	/*
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	*/
	
		// Load the texture
	GLuint Texture = loadDDS("renga.DDS");
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("ishi.obj", vertices, uvs, normals);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	

	
	//カメラオープン

	cv::VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cerr << "Unable to connect to camera" << endl;
		return 1;
	}
	else {
		cout << "camera open" << endl;
	}
	cv::Mat src;
	cv::Size pattern_size = cv::Size2i(PAT_COL, PAT_ROW);
	

	


	glm::mat4 ModelMatrix = glm::mat4
		(4,0,0,0,
		0,4,0,0,
		0,0,4,0,
		0,0,0,1);
	glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	cv::Mat_<double> initial_camera_origin(3, 1);
	cv::Mat_<double> camera_origin(3, 1);
	initial_camera_origin(0, 0) = object[1].x;
	initial_camera_origin(1, 0) = -object[1].y;
	initial_camera_origin(2, 0) = object3d[1].z/3;
	do {

		///////////////////////////完璧ゾーン始まり

		cap.read(src);
		glClear(GL_COLOR_BUFFER_BIT);
		cv::cvtColor(src, src, cv::COLOR_BGR2RGB);
		cv::flip(src, src, 0);
		cv::flip(src, src, 1);
		
		// Use our shader
		glUseProgram(image_programID);

		GLuint image_texture = loadMat(src);


		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, image_texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(image_texture_id, 0);


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, image_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, image_uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);


		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 2 * 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);


		/////////////////////////////完璧ゾーン終わり




		//（３）2次元空間座標の設定
		vector<cv::Point2f> corners;
		auto found = cv::findChessboardCorners(src, pattern_size, corners);
		cv::Mat_<double> invR(3,3), invT;
		if (found)
		{
			cout << "get 2d cordinate" << endl;
			//（４）PnP問題を解く
			cv::Mat_<double> rvec, tvec(3,1);
			cv::solvePnP(object, corners, intrinsic, distCoeffs, rvec, tvec);
			cv::Rodrigues(rvec, rvec);
			cout << "get rvec & tvec" << endl;

			//（５）3次元点の投影
			
			vector<cv::Point2f> imagePoints, imagePoints3d;
			cv::projectPoints(object, rvec, tvec, intrinsic, distCoeffs, imagePoints);
			cv::projectPoints(object3d, rvec, tvec, intrinsic, distCoeffs, imagePoints3d);
			cv::line(src, imagePoints[35], imagePoints[38], cv::Scalar(255, 0, 0), 4, 16);
			cv::line(src, imagePoints[35], imagePoints[35 + 3 * PAT_COL], cv::Scalar(0, 255, 0), 4, 16);
			cv::line(src, imagePoints[35], imagePoints3d[35], cv::Scalar(0, 0, 255), 4, 16);
			
		
			invR = rvec;
			invT = -tvec/30;
			


				camera_origin = //invT/20 +
			  rvec*initial_camera_origin
				;
				cout << camera_origin << endl;

				int gain = 3;

				/*glm::mat4 ViewMatrix = glm::lookAt(
					vec3(camera_origin(0, 0) + tvec(0, 0) / gain+30, camera_origin(1, 0) - tvec(1, 0) / gain, camera_origin(2, 0)-tvec(2, 0) / gain),          // Camera is here
					vec3(30+tvec(0,0)/gain + camera_origin(0, 0), 0-tvec(1,0)/gain + camera_origin(1, 0), 0+tvec(2,0)/gain + camera_origin(2, 0)), // and looks here : at the same position, plus "direction"
					vec3(0, 1, 0)                  // Head is up (set to 0,-1,0 to look -camera_origin(0, 0)-down)
				);
				*/
				
				glm::mat4 ViewMatrix = glm::mat4(
					invR(0,0),invR(0,1),invR(0,2),0,
					invR(1,0),invR(1,1),invR(1,2),0,
					invR(2,0),invR(2,1),invR(2,2),0,
					tvec(0, 0) / gain, tvec(1, 0) / gain, -tvec(2, 0) / 10,1);

				glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;




				


				// Use our shader
				glUseProgram(programID);

				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
				/*
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
				glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
				*/


				// Bind our texture in Texture Unit 1
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, Texture);
				// Set our "myTextureSampler" sampler to use Texture Unit 0
				glUniform1i(TextureID, 1);

				// 1rst attribute buffer : vertices
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
				glVertexAttribPointer(
					0,                  // attribute
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				// 2nd attribute buffer : UVs
				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
				glVertexAttribPointer(
					1,                                // attribute
					2,                                // size
					GL_FLOAT,                         // type
					GL_FALSE,                         // normalized?
					0,                                // stride
					(void*)0                          // array buffer offset
				);



				// Draw the triangles !
				glDrawArrays(GL_TRIANGLES, 0, vertices.size());

				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);






			}
		else
		{
			cout << "can't get 2d cordinate" << endl;
		}
		

		


	
	



	

// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		!glfwWindowShouldClose(window));

	return 0;
}


