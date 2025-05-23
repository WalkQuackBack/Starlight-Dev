#pragma once

#include <string>
#include <vector>
#include "Bfres.h"
#include "PhiveMaterialData.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "FramebufferMgr.h"
#include "ShaderMgr.h"
#include "Vector3F.h"
#include "GLBfres.h"

namespace UICollisionCreator
{
	struct Material
	{
		bool mGenerate = true;
		PhiveMaterialData::Material mMaterial;
	};

	struct ModMaterialInfo
	{
		int mId;
		uint64_t mFlags;
		uint64_t mCollisionFlags;
	};

	extern bool Open;
	extern bool FirstFrame;
	extern std::string GlobalActorName;
	extern std::string GlobalBfresName;
	extern GLBfres* Model;
	extern std::string SecondActorName;
	extern std::vector<Material> Materials;
	extern Vector3F ModelScale;

	extern GLFWwindow* Window;
	extern Framebuffer* ModelFramebuffer;
	extern Shader* InstancedDiscardShader;
	extern Shader* SelectedShader;
	extern Shader* DisabledShader;
	extern glm::mat4 CameraPerspective;
	extern glm::vec3 CameraPosition;
	extern glm::vec3 CameraOrientation;
	extern glm::vec3 CameraUp;
	extern uint32_t ModelViewSize;
	extern Vector3F ModelViewRotate;
	extern bool IsFirstDown;
	extern float MouseOriginalX;
	extern float MouseOriginalY;
	extern float MousePreRotX;
	extern float MousePreRotY;
	extern std::vector<bool> SubModelHovered;
	extern std::vector<const char*> MasterMaterialNames;

	extern int32_t mFlagSubModelIndex;
	extern bool mMaterialPopUpOpen;
	extern float mMaterialPopUpW;
	extern ImVec2 mMaterialPopUpPos;

	extern bool FocusedModelView;

	void DrawMaterialFlagsPopUp();

	void MouseWheelCallback(GLFWwindow* window, double xOffset, double yOffset);
	void UpdateMouseCursorRotation();
	void CalculateCameraPrespective(uint32_t Size);
	void Initialize(GLFWwindow* pWindow);
	bool RenderCollisionModelView(uint32_t Size);
	void DrawCollisionCreatorWindow();
}