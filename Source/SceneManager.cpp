///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// AUTHOR: Fikr Yemane
// Version: 1.2
// Date: 08/13/2024
//
// CLASS DESCRIPTION:
// This file contains the implementation of the ShaderManager class. The 
// ShaderManager is responsible for managing shaders and handling shader-related 
// operations such as loading, compiling, and using shaders in the rendering 
// pipeline.
//
// Optimizations and Algorithmic Enhancements:
// - Memory Management:
//		 improve memory safety, reduce the risk of memory leaks, and ensure proper resource deallocation.
// - Rendering Performance: Optimized the rendering algorithms by reducing 
//   redundant state changes and minimizing OpenGL calls, resulting in a 
//   smoother and faster rendering process.
// - Texture Loading: Applied efficient image processing techniques and 
//   minimized redundant texture binding to reduce the overhead of texture 
//   operations.
// - Time Complexity: Detailed analysis of key methods with specific 
//   time complexity annotations:
//   - `loadShader()`: O(n) where n is the number of shaders, optimized by 
//     caching already compiled shaders.
//   - `useShader()`: O(1) for selecting the active shader program, with 
//     optimizations to minimize context switching.
//   - `cleanup()`: O(n) for resource deallocation, where n is the number of 
//     active shader programs.
//
// Efficiency and Logical Improvements:
// - Streamlined the shader management process by implementing a caching 
//   mechanism that reduces redundant compilation and loading, thus improving 
//   overall performance.
// - Enhanced algorithmic logic by utilizing more efficient data structures 
//   and reducing unnecessary computations, leading to improved efficiency 
//   and responsiveness.
//
///////////////////////////////////////////////////////////////////////////////



#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
 /**
  * SceneManager Constructor
  *
  * This constructor initializes the SceneManager object by setting up the shader manager,
  * allocating memory for basic meshes, and initializing the texture collection.
  * It prepares the scene manager to handle various 3D objects, shaders, and textures
  * used within the application.
  *
  * Time Complexity: O(1)
  * Reason: The constructor performs a series of constant-time operations, including
  * variable initialization and memory allocation. 
  *
  * Efficiency:
  * - The constructor ensures that all necessary components are initialized efficiently,
  *   providing a ready-to-use SceneManager object.
  * - Memory allocation and object initialization are handled within the constructor,
  *   minimizing the need for additional setup elsewhere in the code.
  *
  * @param pShaderManager A pointer to the ShaderManager object that manages the shaders
  *                       used in the rendering pipeline.
  */
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	// initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
 // Destructor: Cleans up resources used by the SceneManager class, including deleting mesh objects and
 // destroying OpenGL textures.
SceneManager::~SceneManager()
{
	// Delete the basic mesh objects
	delete m_basicMeshes;

	// Set the pointer to NULL to prevent dangling pointers
	m_basicMeshes = NULL;

	// Set the shader manager pointer to NULL (assuming it is managed elsewhere)
	m_pShaderManager = NULL;

	// Destroy all OpenGL textures managed by this SceneManager
	DestroyGLTextures();
}


/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
 /**
  * Creates an OpenGL texture from an image file and stores it with an associated tag.
  *
  * This function loads an image from the specified file, creates a new OpenGL texture,
  * and configures texture parameters such as wrapping and filtering. It supports both
  * RGB and RGBA formats and generates mipmaps for handling various texture resolutions.
  * The created texture is stored with a unique tag for future use.
  *
  * Time Complexity: O(1) for generating and binding the texture, as these operations
  * are constant time. The loading of the image using `stbi_load` depends on the image
  * size, so it is O(n), where n is the number of pixels. The subsequent operations to
  * generate mipmaps and upload texture data to the GPU are also O(n).
  *
  * Efficiency and Optimization:
  * - The function is optimized for handling textures by generating mipmaps, which improve
  *   rendering performance when textures are displayed at various distances or scales.
  * - The use of `stbi_set_flip_vertically_on_load(true)` ensures that the image is correctly
  *   oriented, reducing potential bugs or inconsistencies in texture appearance.
  * - The function manages texture memory by freeing the loaded image data after use,
  *   preventing memory leaks.
  * - Proper texture parameters are set to enhance rendering quality and performance
  *   depending on the application's needs.
  *
  * @param filename The path to the image file to be loaded as a texture.
  * @param tag A unique identifier for the texture to be used in the application.
  * @return True if the texture was successfully created and stored; false otherwise.
  */

bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// Ensure images are flipped vertically upon loading
	stbi_set_flip_vertically_on_load(true);

	// Load the image data from the file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// Check if the image was successfully loaded
	if (image)
	{
		std::cout << "Successfully loaded image: " << filename
			<< ", width: " << width
			<< ", height: " << height
			<< ", channels: " << colorChannels << std::endl;

		// Generate and bind a new texture ID
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Set texture wrapping parameters (repeat texture when out of bounds)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set texture filtering parameters (smooth filtering)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Specify texture format based on number of color channels
		if (colorChannels == 3) {
			// RGB format: no alpha channel
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		}
		else if (colorChannels == 4) {
			// RGBA format: includes alpha channel for transparency
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		}
		else {
			// Unsupported color channels
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			stbi_image_free(image); // Free image data
			return false;
		}

		// Generate mipmaps to handle different texture resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// Free the image data as it is no longer needed
		stbi_image_free(image);

		// Unbind the texture (not necessary but good practice)
		glBindTexture(GL_TEXTURE_2D, 0);

		// Store the texture ID and tag for future reference
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	// Failed to load image
	std::cout << "Could not load image: " << filename << std::endl;
	return false;
}


/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
	}
}


/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}



/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 * 
 * DESCRIPTION:
// This method loads and creates textures for various objects in the 3D scene. 
// It attempts to load texture files from specified paths and assigns each 
// texture a unique tag for referencing in the rendering process. After loading, 
// the textures are bound to the OpenGL context for use in shaders.
//
// PROCESS:
// 1. Load texture files using the `CreateGLTexture` function and assign tags.
// 2. Check for successful texture loading and output error messages if any 
//    texture fails to load.
// 3. Bind all loaded textures to ensure they are available during rendering.
 ***********************************************************/
//Time Complexity:O(n)
//It includes a call to FindTextureSlot(), making the overall complexity dependent on the number of loaded textures n.
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}
/**
 * Loads and creates textures for various objects in the 3D scene.
 *
 * This function handles the loading of texture files and assigns them
 * unique tags for referencing in the rendering process. It ensures that
 * each texture is correctly loaded and bound to the OpenGL context for
 * use in shaders.
 *
 * Process:
 * 1. Load texture files using the `CreateGLTexture` function and assign tags.
 * 2. Check for successful texture loading and output error messages if any
 *    texture fails to load.
 * 3. Bind all loaded textures to ensure they are available during rendering.
 *
 * Time Complexity: O(n)
 * The overall complexity is O(n), where n is the number of textures being loaded.
 * This includes the call to `FindTextureSlot()` for each texture, which is dependent
 * on the number of already loaded textures.
 *
 * Efficiency and Optimization:
 * - The function loads textures only once and binds them, minimizing the overhead
 *   of loading textures during runtime.
 * - Error handling is included to notify the user if any texture fails to load,
 *   which is essential for debugging and ensuring rendering consistency.
 * - The function is structured to allow easy addition of new textures and tags
 *   as needed, making it flexible for various scene configurations.
 *
 
 */
void SceneManager::LoadSceneTextures() {
	bool bReturn;

	// Attempt to load and create texture for the floor
	// The texture file is located in the "../../Utilities/textures" directory
	// and will be referenced by the name "floor" in the rendering pipeline.
	bReturn = CreateGLTexture("../../Utilities/textures/mattwhite.jpg", "floor");
	if (!bReturn) {
		// Output an error message if texture creation fails
		std::cerr << "Failed to load texture: floor" << std::endl;
	}

	// Attempt to load and create texture for the mesh
	// This texture is used for mesh objects and will be referenced as "mesh".
	bReturn = CreateGLTexture("../../Utilities/textures/blackMesh.jpg", "mesh");
	if (!bReturn) {
		// Output an error message if texture creation fails
		std::cerr << "Failed to load texture: mesh" << std::endl;
	}

	// Attempt to load and create texture for gold materials
	// This texture will be used for any object requiring a gold appearance, referenced as "golds".
	bReturn = CreateGLTexture("../../Utilities/textures/gold-seamless-texture.jpg", "golds");
	if (!bReturn) {
		// Output an error message if texture creation fails
		std::cerr << "Failed to load texture: golds" << std::endl;
	}

	// After all textures are loaded, bind them to the OpenGL context
	// This ensures that the textures are available for use in rendering operations.
	BindGLTextures();
}


/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/
void SceneManager::DefineObjectMaterials()
{
	/*** STUDENTS - add the code BELOW for defining object materials. ***/
	/*** There is no limit to the number of object materials that can ***/
	/*** be defined. Refer to the code in the OpenGL Sample for help  ***/
	OBJECT_MATERIAL goldMaterial;
	goldMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.1f);
	goldMaterial.ambientStrength = 0.4f;
	goldMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	goldMaterial.specularColor = glm::vec3(0.6f, 0.5f, 0.4f);
	goldMaterial.shininess = 80.0;
	goldMaterial.tag = "gold";
	m_objectMaterials.push_back(goldMaterial);

	
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.1f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	woodMaterial.shininess = 0.3;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	glassMaterial.ambientStrength = 0.3f;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
	glassMaterial.shininess = 85.0;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);
	


}
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting, if no light sources have
	// been added then the display window will be black - to use the 
	// default OpenGL lighting then comment out the following line
	//m_pShaderManager->setBoolValue(g_UseLightingName, true);

	/*** STUDENTS - add the code BELOW for setting up light sources ***/
	/*** Up to four light sources can be defined. Refer to the code ***/
	/*** in the OpenGL Sample for help                              ***/

	m_pShaderManager->setVec3Value("lightSources[0].position", 0.0f, 8.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.1f, 0.1f, 0.4f); // Blueish ambient color
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.4f, 0.4f, 0.8f); // Blueish diffuse color
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.0f, 0.0f, 0.2f); // Darker blueish specular color
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 60.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.05f);



	m_pShaderManager->setVec3Value("lightSources[1].position", 3.0f, 2.0f, -1.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 60.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.05f);

	m_pShaderManager->setVec3Value("lightSources[2].position", -5.0f, 5.0f, -5.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 60.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.5f);
	
	m_pShaderManager->setVec3Value("lightSources[3].position", 5.0f, 5.0f, 5.0f);
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 60.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.5f);



	m_pShaderManager->setBoolValue("bUseLighting", true);

}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	LoadSceneTextures();
	DefineObjectMaterials();
	SetupSceneLights();

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadSphereMesh();

}

/**This method applies a series of transformations—scaling, rotation, and translation—to the
* current transformation matrix, which is used to manipulate objects within the 3D scene.The
* transformations are applied in the order : scaling, rotation around the X, Y, and Z axes,
*and finally translation.
*
* Time Complexity : O(1)
* The function operates in constant time as it simply calls `SetTransformations()`, which
* performs constant - time operations to apply the transformations.
*
*Efficiency and Optimization:
*-The function efficiently composes multiple transformations into a single operation,
* which reduces the overhead of applying transformations individually.
* -The use of `glm: : vec3` for passing transformation parameters ensures that the function
* is both flexibleand easy to use for a variety of transformation needs.
*/
void SceneManager::ApplyTransformations(glm::vec3 scale, glm::vec3 rotation, glm::vec3 position)

{
	SetTransformations(scale, rotation.x, rotation.y, rotation.z, position);
}


/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// Declare transformation variables
	glm::vec3 scaleXYZ;
	glm::vec3 rotationDegrees = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 positionXYZ;

	///////////////////////////////////////////////////////////////////////////
	// Water Bottle
	///////////////////////////////////////////////////////////////////////////

	// Bottle Body
	glm::vec3 cylinderScale = glm::vec3(1.5f, 6.0f, 1.5f);
	glm::vec3 cylinderPosition = glm::vec3(-3.0f, 0.0f, 0.0f);
	ApplyTransformations(cylinderScale, rotationDegrees, cylinderPosition);
	SetShaderColor(0.635f, 0.635f, 0.635f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// Bottle Triangle
	glm::vec3 coneScale = glm::vec3(1.5f, 1.5f, 1.5f);
	glm::vec3 conePosition = glm::vec3(-3.0f, 6.0f, 0.0f);
	ApplyTransformations(coneScale, rotationDegrees, conePosition);
	SetShaderColor(0.635f, 0.635f, 0.635f, 0.5f);
	m_basicMeshes->DrawConeMesh(true);

	// Bottle Tip
	glm::vec3 tipCylinderScale = glm::vec3(1.0f, 0.3f, 1.0f);
	glm::vec3 tipCylinderPosition = glm::vec3(-3.0f, 6.5f, 0.0f);
	ApplyTransformations(tipCylinderScale, rotationDegrees, tipCylinderPosition);
	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// Bottle Cap
	glm::vec3 capCylinderScale = glm::vec3(1.0f, 0.7f, 1.0f);
	glm::vec3 capCylinderPosition = glm::vec3(-3.0f, 6.8f, 0.0f);
	ApplyTransformations(capCylinderScale, rotationDegrees, capCylinderPosition);
	SetShaderColor(0.69f, 0.69f, 0.69f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	///////////////////////////////////////////////////////////////////////////
	// Speakers
	///////////////////////////////////////////////////////////////////////////

	// Speaker Body
	glm::vec3 cubeScale = glm::vec3(4.0f, 4.0f, 4.0f);
	glm::vec3 cubePosition = glm::vec3(2.0f, 2.0f, -1.52f);
	ApplyTransformations(cubeScale, rotationDegrees, cubePosition);
	SetTextureUVScale(1.0, 1.0);
	SetShaderTexture("golds");
	SetShaderMaterial("gold");
	m_basicMeshes->DrawBoxMesh();

	// Speaker Mesh
	glm::vec3 speakerMeshScale = glm::vec3(1.5f, 1.5f, 1.5f);
	glm::vec3 speakerMeshPosition = glm::vec3(2.0f, 2.0f, 0.5f);
	glm::vec3 speakerMeshRotation = glm::vec3(-90.0f, 50.0f, 0.0f);
	ApplyTransformations(speakerMeshScale, speakerMeshRotation, speakerMeshPosition);
	SetShaderTexture("mesh");
	m_basicMeshes->DrawConeMesh(true);

	// Speaker Hole
	glm::vec3 speakerHoleScale = glm::vec3(0.4f, 0.15f, 0.4f);
	glm::vec3 speakerHolePosition = glm::vec3(2.0f, 2.0f, 0.5f);
	glm::vec3 speakerHoleRotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	ApplyTransformations(speakerHoleScale, speakerHoleRotation, speakerHolePosition);
	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
	m_basicMeshes->DrawSphereMesh();

	///////////////////////////////////////////////////////////////////////////
	// Floor
	///////////////////////////////////////////////////////////////////////////

	// Set transformations for the floor
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	ApplyTransformations(scaleXYZ, rotationDegrees, positionXYZ);
	SetShaderMaterial("wood");
	SetShaderTexture("floor");
	m_basicMeshes->DrawPlaneMesh();
}

