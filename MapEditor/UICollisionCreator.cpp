#include "UICollisionCreator.h"

#include "imgui.h"
#include "PhiveSocketConnector.h"
#include "BinaryVectorWriter.h"
#include "Editor.h"
#include "Util.h"
#include <iostream>
#include "tinyfiledialogs.h"
#include "ZStdFile.h"
#include "SARC.h"
#include "Byml.h"
#include "Logger.h"
#include <imgui_stdlib.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "UIMapView.h"

GLFWwindow* UICollisionCreator::Window = nullptr;
Framebuffer* UICollisionCreator::ModelFramebuffer = nullptr;
Shader* UICollisionCreator::InstancedDiscardShader = nullptr;
Shader* UICollisionCreator::SelectedShader = nullptr;
Shader* UICollisionCreator::DisabledShader = nullptr;
glm::vec3 UICollisionCreator::CameraPosition = glm::vec3(18, 12, 12);
glm::vec3 UICollisionCreator::CameraOrientation = glm::vec3(-0.73f, -0.43f, -0.53f);
glm::vec3 UICollisionCreator::CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::mat4 UICollisionCreator::CameraPerspective;
Vector3F UICollisionCreator::ModelScale = Vector3F(1, 1, 1);
Vector3F UICollisionCreator::ModelViewRotate = Vector3F(0, 0, 0);

bool UICollisionCreator::FirstFrame = true;
bool UICollisionCreator::Open = true;
std::string UICollisionCreator::GlobalActorName = "";
std::string UICollisionCreator::GlobalBfresName = "";
GLBfres* UICollisionCreator::Model = nullptr;
std::string UICollisionCreator::SecondActorName = "";
std::vector<UICollisionCreator::Material> UICollisionCreator::Materials;

uint32_t UICollisionCreator::ModelViewSize = 0;

bool UICollisionCreator::FocusedModelView = false;
bool UICollisionCreator::IsFirstDown = true;
float UICollisionCreator::MouseOriginalX = 0.0f;
float UICollisionCreator::MouseOriginalY = 0.0f;
float UICollisionCreator::MousePreRotX = 0.0f;
float UICollisionCreator::MousePreRotY = 0.0f;
std::vector<bool> UICollisionCreator::SubModelHovered;

std::vector<const char*> UICollisionCreator::MasterMaterialNames;

int32_t UICollisionCreator::mFlagSubModelIndex = -1;
float UICollisionCreator::mMaterialPopUpW = 0.0f;
bool UICollisionCreator::mMaterialPopUpOpen = true;
ImVec2 UICollisionCreator::mMaterialPopUpPos = ImVec2(0, 0);

void UICollisionCreator::MouseWheelCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	UIMapView::GLFWScrollCallback(window, xOffset, yOffset);

	if (!FocusedModelView) return;

	CameraPosition += (float)yOffset * CameraOrientation;
}

void UICollisionCreator::UpdateMouseCursorRotation()
{
	if (glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		double MouseX;
		double MouseY;
		// Fetches the coordinates of the cursor
		glfwGetCursorPos(Window, &MouseX, &MouseY);

		if (IsFirstDown)
		{
			IsFirstDown = false;
			MouseOriginalX = MouseX;
			MouseOriginalY = MouseY;
		}
		else
		{
			MousePreRotX = (MouseY - MouseOriginalY) * 0.75f;
			MousePreRotY = MouseX - MouseOriginalX;
		}
	}
}

void UICollisionCreator::Initialize(GLFWwindow* pWindow)
{
	Window = pWindow;
	ModelFramebuffer = FramebufferMgr::CreateFramebuffer(200, 200, "CollisionModelView");
	InstancedDiscardShader = ShaderMgr::GetShader("InstancedDiscard");
	SelectedShader = ShaderMgr::GetShader("Selected");
	DisabledShader = ShaderMgr::GetShader("Disabled");

	MasterMaterialNames.push_back("Apply to all...");
	MasterMaterialNames.insert(MasterMaterialNames.end(), PhiveMaterialData::mMaterialNames.begin(), PhiveMaterialData::mMaterialNames.end());
}

std::string GetBfresName(SarcFile& ActorPack, Vector3F& Scale)
{
	for (SarcFile::Entry& Entry : ActorPack.GetEntries())
	{
		if (Entry.Name.starts_with("Component/ModelInfo/"))
		{
			BymlFile ModelInfo(Entry.Bytes);
			if (ModelInfo.HasChild("DebugModelScale"))
			{
				BymlFile::Node* DebugModelScaleNode = ModelInfo.GetNode("DebugModelScale");
				Scale = Vector3F(DebugModelScaleNode->GetChild("X")->GetValue<float>(),
					DebugModelScaleNode->GetChild("Y")->GetValue<float>(),
					DebugModelScaleNode->GetChild("Z")->GetValue<float>());
			}
			else
			{
				Scale = Vector3F(1, 1, 1);
			}
			if (ModelInfo.HasChild("FmdbName") && ModelInfo.HasChild("ModelProjectName"))
			{
				return ModelInfo.GetNode("ModelProjectName")->GetValue<std::string>() + "." + ModelInfo.GetNode("FmdbName")->GetValue<std::string>();
			}
			break;
		}
	}

	return "";
}

BymlFile GenerateShapeParamFile(std::string PhshName, Vector3F Min, Vector3F Max)
{
	BymlFile Byml;
	Byml.GetType() = BymlFile::Type::Dictionary;
	
	BymlFile::Node PhshMeshNode(BymlFile::Type::Array, "PhshMesh");
	
	BymlFile::Node PhshMeshNodeDict(BymlFile::Type::Dictionary);

	BymlFile::Node PhshMeshNodeString(BymlFile::Type::StringIndex, "PhshMeshPath");
	PhshMeshNodeString.SetValue<std::string>("Work/Phive/Shape/Dcc/" + PhshName + ".phsh");

	PhshMeshNodeDict.AddChild(PhshMeshNodeString);

	PhshMeshNode.AddChild(PhshMeshNodeDict);


	BymlFile::Node AutoCalcNode(BymlFile::Type::Dictionary, "AutoCalc");

	BymlFile::Node AxisNode(BymlFile::Type::Dictionary, "Axis");
	BymlFile::Node AxisXNode(BymlFile::Type::Float, "X");
	AxisXNode.SetValue<float>(0.0f);
	BymlFile::Node AxisYNode(BymlFile::Type::Float, "Y");
	AxisYNode.SetValue<float>(0.0f);
	BymlFile::Node AxisZNode(BymlFile::Type::Float, "Z");
	AxisZNode.SetValue<float>(0.0f);
	AxisNode.AddChild(AxisXNode);
	AxisNode.AddChild(AxisYNode);
	AxisNode.AddChild(AxisZNode);

	BymlFile::Node CenterNode(BymlFile::Type::Dictionary, "Center");
	BymlFile::Node CenterXNode(BymlFile::Type::Float, "X");
	CenterXNode.SetValue<float>(0.0f);
	BymlFile::Node CenterYNode(BymlFile::Type::Float, "Y");
	CenterYNode.SetValue<float>(0.0f);
	BymlFile::Node CenterZNode(BymlFile::Type::Float, "Z");
	CenterZNode.SetValue<float>(0.0f);
	CenterNode.AddChild(CenterXNode);
	CenterNode.AddChild(CenterYNode);
	CenterNode.AddChild(CenterZNode);

	BymlFile::Node MaxNode(BymlFile::Type::Dictionary, "Max");
	BymlFile::Node MaxXNode(BymlFile::Type::Float, "X");
	MaxXNode.SetValue<float>(Max.GetX());
	BymlFile::Node MaxYNode(BymlFile::Type::Float, "Y");
	MaxYNode.SetValue<float>(Max.GetY());
	BymlFile::Node MaxZNode(BymlFile::Type::Float, "Z");
	MaxZNode.SetValue<float>(Max.GetZ());
	MaxNode.AddChild(MaxXNode);
	MaxNode.AddChild(MaxYNode);
	MaxNode.AddChild(MaxZNode);

	BymlFile::Node MinNode(BymlFile::Type::Dictionary, "Min");
	BymlFile::Node MinXNode(BymlFile::Type::Float, "X");
	MinXNode.SetValue<float>(Min.GetX());
	BymlFile::Node MinYNode(BymlFile::Type::Float, "Y");
	MinYNode.SetValue<float>(Min.GetY());
	BymlFile::Node MinZNode(BymlFile::Type::Float, "Z");
	MinZNode.SetValue<float>(Min.GetZ());
	MinNode.AddChild(MinXNode);
	MinNode.AddChild(MinYNode);
	MinNode.AddChild(MinZNode);

	BymlFile::Node TensorNode(BymlFile::Type::Dictionary, "Tensor");
	BymlFile::Node TensorXNode(BymlFile::Type::Float, "X");
	TensorXNode.SetValue<float>(0.0f);
	BymlFile::Node TensorYNode(BymlFile::Type::Float, "Y");
	TensorYNode.SetValue<float>(0.0f);
	BymlFile::Node TensorZNode(BymlFile::Type::Float, "Z");
	TensorZNode.SetValue<float>(0.0f);
	TensorNode.AddChild(TensorXNode);
	TensorNode.AddChild(TensorYNode);
	TensorNode.AddChild(TensorZNode);

	AutoCalcNode.AddChild(AxisNode);
	AutoCalcNode.AddChild(CenterNode);
	AutoCalcNode.AddChild(MaxNode);
	AutoCalcNode.AddChild(MinNode);
	AutoCalcNode.AddChild(TensorNode);

	Byml.GetNodes().push_back(AutoCalcNode);
	Byml.GetNodes().push_back(PhshMeshNode);

	return Byml;
}

BymlFile GenerateRigidBodyEntityParamByml(std::string ShapeName)
{
	BymlFile Byml;
	Byml.GetType() = BymlFile::Type::Dictionary;
	
	BymlFile::Node BuoyancyScaleNode(BymlFile::Type::Float, "BuoyancyScale");
	BuoyancyScaleNode.SetValue<float>(1.0f);

	BymlFile::Node MassNode(BymlFile::Type::Float, "Mass");
	MassNode.SetValue<float>(1.0f);

	BymlFile::Node WaterFlowScaleNode(BymlFile::Type::Float, "WaterFlowScale");
	WaterFlowScaleNode.SetValue<float>(1.0f);

	BymlFile::Node MotionPropertyNode(BymlFile::Type::StringIndex, "MotionProperty");
	MotionPropertyNode.SetValue<std::string>("Default");

	BymlFile::Node LayerEntityNode(BymlFile::Type::StringIndex, "LayerEntity");
	LayerEntityNode.SetValue<std::string>("GroundHighRes");

	BymlFile::Node MotionTypeNode(BymlFile::Type::StringIndex, "MotionType");
	MotionTypeNode.SetValue<std::string>("Static");

	BymlFile::Node SubLayerEntityNode(BymlFile::Type::StringIndex, "SubLayerEntity");
	SubLayerEntityNode.SetValue<std::string>("Unspecified");

	BymlFile::Node ShapeNameNode(BymlFile::Type::StringIndex, "ShapeName");
	ShapeNameNode.SetValue<std::string>(ShapeName);

	Byml.GetNodes().push_back(BuoyancyScaleNode);
	Byml.GetNodes().push_back(LayerEntityNode);
	Byml.GetNodes().push_back(MassNode);
	Byml.GetNodes().push_back(MotionPropertyNode);
	Byml.GetNodes().push_back(MotionTypeNode);
	Byml.GetNodes().push_back(ShapeNameNode);
	Byml.GetNodes().push_back(SubLayerEntityNode);
	Byml.GetNodes().push_back(WaterFlowScaleNode);

	return Byml;
}

BymlFile GenerateRigidBodyControllerEntityParamByml()
{
	BymlFile Byml;
	Byml.GetType() = BymlFile::Type::Dictionary;

	BymlFile::Node RigidBodyControllerUnitAryNode(BymlFile::Type::Array, "RigidBodyControllerUnitAry");

	auto CreateBoolNode = [](std::string Key, bool Value)
		{
			BymlFile::Node Node(BymlFile::Type::Bool, Key);
			Node.SetValue<bool>(Value);
			return Node;
		};

	auto CreateStringNode = [](std::string Key, std::string Value)
		{
			BymlFile::Node Node(BymlFile::Type::StringIndex, Key);
			Node.SetValue<std::string>(Value);
			return Node;
		};

	BymlFile::Node ChildDict(BymlFile::Type::Dictionary);

	ChildDict.AddChild(CreateStringNode("BoneBindModePosition", "All"));
	ChildDict.AddChild(CreateStringNode("BoneBindModeRotation", "All"));
	ChildDict.AddChild(CreateStringNode("ContactCollectionName", ""));
	ChildDict.AddChild(CreateBoolNode("IgnoreChangeMassOnScaling", false));
	ChildDict.AddChild(CreateBoolNode("IsAddToWorldOnReset", true));
	ChildDict.AddChild(CreateBoolNode("IsSkipTrackingWhenNoHit", true));
	ChildDict.AddChild(CreateBoolNode("IsTrackingActor", false));
	ChildDict.AddChild(CreateBoolNode("IsTrackingEntity", false));
	ChildDict.AddChild(CreateStringNode("Name", "Physical_Col"));
	ChildDict.AddChild(CreateStringNode("TrackingBoneName", ""));
	ChildDict.AddChild(CreateStringNode("TrackingEntityAlias", ""));
	ChildDict.AddChild(CreateBoolNode("UseNextMainRigidBodyMatrix", false));
	ChildDict.AddChild(CreateStringNode("WarpMode", "AfterUpdateWorldMtx"));

	RigidBodyControllerUnitAryNode.AddChild(ChildDict);

	Byml.GetNodes().push_back(RigidBodyControllerUnitAryNode);

	return Byml;
}

BymlFile GenerateControllerSetParamByml(std::string ActorName)
{
	BymlFile Byml;
	Byml.GetType() = BymlFile::Type::Dictionary;

	auto CreateStringNode = [](std::string Key, std::string Value)
		{
			BymlFile::Node Node(BymlFile::Type::StringIndex, Key);
			Node.SetValue<std::string>(Value);
			return Node;
		};

	Byml.GetNodes().push_back(CreateStringNode("ConstraintControllerPath", ""));

	BymlFile::Node ControllerEntityNamePathAryNode(BymlFile::Type::Array, "ControllerEntityNamePathAry");

	BymlFile::Node ControllerEntityNamePathAryDictNode(BymlFile::Type::Dictionary);
	ControllerEntityNamePathAryDictNode.AddChild(CreateStringNode("FilePath", "Work/Phive/RigidBodyControllerEntityParam/" + ActorName + "_Body.phive__RigidBodyControllerEntityParam.gyml"));
	ControllerEntityNamePathAryDictNode.AddChild(CreateStringNode("Name", "Body"));
	ControllerEntityNamePathAryNode.AddChild(ControllerEntityNamePathAryDictNode);

	BymlFile::Node RigidBodyEntityNamePathAryNode(BymlFile::Type::Array, "RigidBodyEntityNamePathAry");

	BymlFile::Node RigidBodyEntityNamePathAryDictNode(BymlFile::Type::Dictionary);
	RigidBodyEntityNamePathAryDictNode.AddChild(CreateStringNode("FilePath", "Work/Phive/RigidBodyEntityParam/" + ActorName + "_Body.phive__RigidBodyEntityParam.gyml"));
	RigidBodyEntityNamePathAryDictNode.AddChild(CreateStringNode("Name", "Physical_Col"));
	RigidBodyEntityNamePathAryNode.AddChild(RigidBodyEntityNamePathAryDictNode);

	BymlFile::Node ShapeNamePathAryNode(BymlFile::Type::Array, "ShapeNamePathAry");
	BymlFile::Node ShapeNamePathAryDictNode(BymlFile::Type::Dictionary);
	ShapeNamePathAryDictNode.AddChild(CreateStringNode("FilePath", "Work/Phive/ShapeParam/" + ActorName + "__Physical_Col.phive__ShapeParam.gyml"));
	ShapeNamePathAryDictNode.AddChild(CreateStringNode("Name", ActorName + "__Physical_Col"));
	ShapeNamePathAryNode.AddChild(ShapeNamePathAryDictNode);

	Byml.GetNodes().push_back(ControllerEntityNamePathAryNode);
	Byml.GetNodes().push_back(RigidBodyEntityNamePathAryNode);
	Byml.GetNodes().push_back(ShapeNamePathAryNode);

	return Byml;
}

bool UICollisionCreator::RenderCollisionModelView(uint32_t Size)
{
	bool Result = false;

	if (Model == nullptr)
		return false;

	if (ModelViewSize != Size)
	{
		ModelFramebuffer->RescaleFramebuffer(Size, Size);
		ModelViewSize = Size;
		Result = true;
	}

	ModelFramebuffer->Bind();
	glViewport(0, 0, Size, Size);
	//DefaultShader->Activate();
	glClearColor(ImGui::GetStyle().Colors[ImGuiCol_WindowBg].x, ImGui::GetStyle().Colors[ImGuiCol_WindowBg].y, ImGui::GetStyle().Colors[ImGuiCol_WindowBg].z, ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w);
	//glClearColor(1.0f, ImGui::GetStyle().Colors[ImGuiCol_WindowBg].y, ImGui::GetStyle().Colors[ImGuiCol_WindowBg].z, ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	std::vector<glm::mat4> InstanceMatrix(1);

	glm::mat4& GLMModel = InstanceMatrix[0];
	GLMModel = glm::mat4(1.0f); // Identity matrix

	CalculateCameraPrespective(Size);

	GLMModel = glm::rotate(GLMModel, glm::radians(ModelViewRotate.GetZ()), glm::vec3(0.0, 0.0f, 1.0));
	GLMModel = glm::rotate(GLMModel, glm::radians(ModelViewRotate.GetY() + MousePreRotY), glm::vec3(0.0f, 1.0, 0.0));
	GLMModel = glm::rotate(GLMModel, glm::radians(ModelViewRotate.GetX() + MousePreRotX), glm::vec3(1.0, 0.0f, 0.0));

	InstancedDiscardShader->Activate();
	glUniformMatrix4fv(glGetUniformLocation(InstancedDiscardShader->ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(CameraPerspective));
	glUniform3fv(glGetUniformLocation(InstancedDiscardShader->ID, "lightColor"), 1, &UIMapView::mLightColor[0]);
	glUniform3fv(glGetUniformLocation(InstancedDiscardShader->ID, "lightPos"), 1, &CameraPosition[0]);
	SelectedShader->Activate();
	glUniformMatrix4fv(glGetUniformLocation(SelectedShader->ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(CameraPerspective));
	glUniform3fv(glGetUniformLocation(SelectedShader->ID, "lightColor"), 1, &UIMapView::mLightColor[0]);
	glUniform3fv(glGetUniformLocation(SelectedShader->ID, "lightPos"), 1, &CameraPosition[0]);
	DisabledShader->Activate();
	glUniformMatrix4fv(glGetUniformLocation(DisabledShader->ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(CameraPerspective));
	glUniform3fv(glGetUniformLocation(DisabledShader->ID, "lightColor"), 1, &UIMapView::mLightColor[0]);
	glUniform3fv(glGetUniformLocation(DisabledShader->ID, "lightPos"), 1, &CameraPosition[0]);
	
	Model->mInstanceMatrix.SetData<glm::mat4>(InstanceMatrix);

	for (uint16_t i : Model->mOpaqueObjects)
	{
		Shader* UsedShader = SubModelHovered[i] ? SelectedShader : InstancedDiscardShader;
		if (!SubModelHovered[i] && !Materials[i].mGenerate) UsedShader = DisabledShader;

		UsedShader->Activate();

		Model->mShapeVAOs[i].Enable(UsedShader);
		Model->mShapeVAOs[i].Use();

		Model->mMaterials[i].mAlbedoTexture->Bind();

		if (Model->mMaterials[i].mRenderStateDisplayFace != GL_NONE)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(Model->mMaterials[i].mRenderStateDisplayFace);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glDrawElementsInstanced(GL_TRIANGLES, Model->mIndexBuffers[i].second, Model->mMaterials[i].mIndexFormat, 0, 1);
	}

	for (uint16_t i : Model->mTransparentObjects)
	{
		Shader* UsedShader = SubModelHovered[i] ? SelectedShader : InstancedDiscardShader;
		if (!SubModelHovered[i] && !Materials[i].mGenerate) UsedShader = DisabledShader;

		UsedShader->Activate();

		Model->mShapeVAOs[i].Enable(UsedShader);
		Model->mShapeVAOs[i].Use();

		Model->mMaterials[i].mAlbedoTexture->Bind();

		if (Model->mMaterials[i].mRenderStateDisplayFace != GL_NONE)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(Model->mMaterials[i].mRenderStateDisplayFace);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glDrawElementsInstanced(GL_TRIANGLES, Model->mIndexBuffers[i].second, Model->mMaterials[i].mIndexFormat, 0, 1);
	}

	/*
	for (uint32_t SubModelIndexOpaque : LODModel->OpaqueObjects)
	{
		Shader* UsedShader = SubModelHovered[SubModelIndexOpaque] ? SelectedShader : DefaultShader;
		if (!SubModelHovered[SubModelIndexOpaque] && !Materials[SubModelIndexOpaque].Generate) UsedShader = DisabledShader;
		UsedShader->Activate();
		LODModel->GL_Meshes[SubModelIndexOpaque].UpdateInstances(1);
		glUniformMatrix4fv(glGetUniformLocation(UsedShader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(GLMModel));
		LODModel->GL_Meshes[SubModelIndexOpaque].Draw();
	}

	for (uint32_t SubModelIndexTransparent : LODModel->TransparentObjects)
	{
		Shader* UsedShader = SubModelHovered[SubModelIndexTransparent] ? SelectedShader : DefaultShader;
		if (!SubModelHovered[SubModelIndexTransparent] && !Materials[SubModelIndexTransparent].Generate) UsedShader = DisabledShader;
		UsedShader->Activate();
		LODModel->GL_Meshes[SubModelIndexTransparent].UpdateInstances(1);
		glUniformMatrix4fv(glGetUniformLocation(UsedShader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(GLMModel));
		LODModel->GL_Meshes[SubModelIndexTransparent].Draw();
	}
	*/

	ImGui::Image((void*)ModelFramebuffer->GetFrameTexture(), ImVec2(Size, Size), ImVec2(0, 1),
		ImVec2(1, 0));
	
	FocusedModelView = FocusedModelView && ImGui::IsItemHovered();

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		if (FocusedModelView || !IsFirstDown)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) UpdateMouseCursorRotation();
		}
	}
	else
	{
		if (!IsFirstDown)
		{
			IsFirstDown = true;

			ModelViewRotate = Vector3F(ModelViewRotate.GetX() + MousePreRotX, ModelViewRotate.GetY() + MousePreRotY, ModelViewRotate.GetZ());

			MouseOriginalX = 0;
			MouseOriginalY = 0;
			MousePreRotX = 0;
			MousePreRotY = 0;
		}
	}

	ModelFramebuffer->Unbind();

	return Result;
}

void UICollisionCreator::DrawMaterialFlagsPopUp()
{
	if (mFlagSubModelIndex == -1)
		return;

	ImGui::SetNextWindowPos(ImVec2(mMaterialPopUpPos.x, mMaterialPopUpPos.y));
	if (ImGui::BeginPopup("CollisionCreatorMaterialFlagPopUp"))
	{
		ImGui::Text("Flags");
		float PosX = ImGui::GetCursorPosX();
		ImGui::SameLine();
		ImGui::SetCursorPosX(PosX + ImGui::CalcTextSize("ForbidAutoPlacementAndEnemyEntry").x + ImGui::GetStyle().ItemSpacing.x * 4.0f + ImGui::GetFrameHeight());
		ImVec2 FilterBasePos = ImGui::GetCursorPos();
		ImGui::Text("Filters");
		FilterBasePos.y = ImGui::GetCursorPosY();

		ImGui::BeginChild("##CollisionCreatorMaterialFlagPopUpFlagChild", ImVec2(ImGui::CalcTextSize("ForbidAutoPlacementAndEnemyEntry").x + ImGui::GetStyle().ItemSpacing.x * 3.0f + ImGui::GetFrameHeight(), ImGui::GetFrameHeight() * 8.0f * Editor::UIScale));
		for (size_t i = 0; i < PhiveMaterialData::mMaterialFlagNames.size(); i++)
		{
			ImGui::Checkbox(PhiveMaterialData::mMaterialFlagNames[i], &Materials[mFlagSubModelIndex].mMaterial.mFlags[i]);
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(FilterBasePos);

		ImGui::BeginChild("##CollisionCreatorMaterialFlagPopUpFiltersChild", ImVec2(ImGui::CalcTextSize("HitOnlyEnemyVehicleAndAirWall").x + ImGui::GetStyle().ItemSpacing.x * 3.0f + ImGui::GetFrameHeight(), ImGui::GetFrameHeight() * 8.0f * Editor::UIScale));
		for (size_t i = 0; i < PhiveMaterialData::mMaterialFilterNames.size(); i++)
		{
			ImGui::Checkbox(PhiveMaterialData::mMaterialFilterNames[i], &Materials[mFlagSubModelIndex].mMaterial.mCollisionFlags[i]);
		}
		ImGui::EndChild();

		ImGui::EndPopup();
	}
}

void UICollisionCreator::CalculateCameraPrespective(uint32_t Size)
{
	// Initializes matrices since otherwise they will be the null matrix
	glm::mat4 View = glm::mat4(1.0f);
	glm::mat4 Projection = glm::mat4(1.0f);

	// Makes camera look in the right direction from the right position
	View = glm::lookAt(CameraPosition, CameraPosition + CameraOrientation, CameraUp);
	// Adds perspective to the scene
	Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 30000.0f);
	CameraPerspective = Projection * View;
}

void UICollisionCreator::DrawCollisionCreatorWindow()
{
	if (!Open) return;

	FocusedModelView = ImGui::Begin("Collision creator", &Open);

	if (FocusedModelView)
	{
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		ImGui::InputText("Actor name (e.g. MrLapinou_House_C_Red_01)", &GlobalActorName);
		ImGui::PopItemWidth();

		if (!SecondActorName.empty())
		{
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
			if (ImGui::InputText("Bfres name", &GlobalBfresName))
			{
				if (Util::FileExists(Editor::GetBfresFile(GlobalBfresName)))
				{
					Model = GLBfresLibrary::GetModel(BfresLibrary::GetModel(GlobalBfresName));
					Materials.clear();
					Materials.resize(Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size());
					for (int SubModelIndex = 0; SubModelIndex < Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size(); SubModelIndex++)
					{
						Materials[SubModelIndex].mMaterial.mMaterialId = 7; //Default material index -> Stone
						Materials[SubModelIndex].mMaterial.mCollisionFlags[0] = true;
					}
					SubModelHovered.resize(Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size());
				}
			}
			ImGui::PopItemWidth();
		}

		if (ImGui::Button("Load"))
		{
			SarcFile ActorPack(ZStdFile::Decompress(Editor::GetRomFSFile("Pack/Actor/" + GlobalActorName + ".pack.zs"), ZStdFile::Dictionary::Pack).Data);
			std::string BfresName = GetBfresName(ActorPack, ModelScale);
			if (BfresName.empty())
			{
				Logger::Error("CollisionCreator", "Could not find model file");
				goto End;
			}
			std::cout << "Model: " << BfresName << std::endl;
			Model = GLBfresLibrary::GetModel(BfresLibrary::GetModel(BfresName));
			if (Model == nullptr)
			{
				goto End;
			}
			SecondActorName = GlobalActorName;
			Materials.clear();
			Materials.resize(Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size());
			for (int SubModelIndex = 0; SubModelIndex < Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size(); SubModelIndex++)
			{
				Materials[SubModelIndex].mMaterial.mMaterialId = 7; //Default material index -> Stone
				Materials[SubModelIndex].mMaterial.mCollisionFlags[0] = true;
			}
			SubModelHovered.resize(Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size());
			GlobalBfresName = BfresName;
		}

		if (SecondActorName.empty())
		{
			goto End;
		}

		ImGui::SameLine();

		if (ImGui::Button("Generate collision"))
		{
			if (!PhiveSocketConnector::IsConnected)
			{
				PhiveSocketConnector::Connect();
			}

			if (!PhiveSocketConnector::IsConnected)
			{
				goto End;
			}

			Util::CreateDir(Editor::GetWorkingDirFile("Save/Phive/Shape/Dcc"));
			std::vector<glm::vec3> Vertices;
			std::vector<std::pair<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t>> Indices;
			uint32_t MaterialOffset = 0;
			uint32_t IndexOffset = 0;
			bool HasGeometry = false;

			std::vector<ModMaterialInfo> MaterialInfo;
			std::unordered_map<size_t, size_t> SubModelIndexToMaterialIndex;

			for (size_t SubModelIndex = 0; SubModelIndex < Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size(); SubModelIndex++)
			{
				if (!Materials[SubModelIndex].mGenerate)
				{
					MaterialOffset++;
					continue;
				}

				bool Found = false;

				uint64_t Flags = 0;

				for (size_t i = 0; i < PhiveMaterialData::mMaterialFlagNames.size(); i++)
				{
					if (!Materials[SubModelIndex].mMaterial.mFlags[i])
						continue;

					Flags |= PhiveMaterialData::mMaterialFlagMasks[i];
				}

				uint64_t Filters = 0;
				//Generating base
				for (size_t i = 0; i < PhiveMaterialData::mMaterialFilterNames.size(); i++)
				{
					if (!Materials[SubModelIndex].mMaterial.mCollisionFlags[i])
						continue;

					if(PhiveMaterialData::mMaterialFilterIsBase[i])
						Filters |= PhiveMaterialData::mMaterialFilterMasks[i];
				}
				//Applying filters
				for (size_t i = 0; i < PhiveMaterialData::mMaterialFilterNames.size(); i++)
				{
					if (!Materials[SubModelIndex].mMaterial.mCollisionFlags[i])
						continue;

					if (!PhiveMaterialData::mMaterialFilterIsBase[i])
						Filters &= PhiveMaterialData::mMaterialFilterMasks[i];
				}

				for (size_t i = 0; i < MaterialInfo.size(); i++)
				{
					if (MaterialInfo[i].mId == Materials[SubModelIndex].mMaterial.mMaterialId &&
						MaterialInfo[i].mFlags == Flags &&
						MaterialInfo[i].mCollisionFlags == Filters)
					{
						Found = true;
						SubModelIndexToMaterialIndex.insert({ SubModelIndex, i });
						break;
					}
				}

				if (!Found)
				{
					ModMaterialInfo Info;
					Info.mId = (int)Materials[SubModelIndex].mMaterial.mMaterialId;
					Info.mFlags = Flags;
					Info.mCollisionFlags = Filters;

					MaterialInfo.push_back(Info);
					SubModelIndexToMaterialIndex.insert({ SubModelIndex, MaterialInfo.size() - 1 });
				}
			}

			for (size_t SubModelIndex = 0; SubModelIndex < Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size(); SubModelIndex++)
			{
				if (!Materials[SubModelIndex].mGenerate)
				{
					MaterialOffset++;
					continue;
				}

				std::vector<glm::vec4> Positions = Model->mBfres->Models.GetByIndex(0).Value.Shapes.GetByIndex(SubModelIndex).Value.Vertices;
				uint32_t Offset = Vertices.size();
				Vertices.resize(Vertices.size() + Positions.size());

				for (size_t i = 0; i < Positions.size(); i++)
				{
					Vertices[Offset + i] = glm::vec3(Positions[i].x * ModelScale.GetX(), Positions[i].y * ModelScale.GetY(), Positions[i].z * ModelScale.GetZ());
				}

				std::vector<uint32_t> Triangles = Model->mBfres->Models.GetByIndex(0).Value.Shapes.GetByIndex(SubModelIndex).Value.Meshes[0].GetIndices();
				Offset = Indices.size();
				Indices.resize(Indices.size() + (Triangles.size() / 3));
				for (size_t i = 0; i < Triangles.size() / 3; i++)
				{
					Indices[Offset + i] = std::make_pair(std::make_tuple(Triangles[i * 3] + IndexOffset, Triangles[i * 3 + 1] + IndexOffset, Triangles[i * 3 + 2] + IndexOffset), SubModelIndexToMaterialIndex[SubModelIndex]);
				}

				IndexOffset += Positions.size();
				HasGeometry = true;
			}
			
			if (HasGeometry)
			{
				BinaryVectorWriter Writer;
				Writer.WriteInteger(MaterialInfo.size(), sizeof(uint32_t));
				Writer.WriteInteger(Vertices.size() * 12, sizeof(uint32_t));
				Writer.WriteInteger(Indices.size() * 16, sizeof(uint32_t));
				for (size_t i = 0; i < MaterialInfo.size(); i++)
				{
					Writer.WriteInteger(MaterialInfo[i].mId, sizeof(uint32_t));
					Writer.WriteInteger(MaterialInfo[i].mFlags, sizeof(uint64_t));
					Writer.WriteInteger(0xFFFFFFFF, sizeof(uint32_t));
					Writer.WriteInteger(MaterialInfo[i].mCollisionFlags, sizeof(uint32_t));
				}
				for (glm::vec3& Vertex : Vertices)
				{
					Writer.WriteRawUnsafeFixed(reinterpret_cast<const char*>(&Vertex.x), sizeof(float));
					Writer.WriteRawUnsafeFixed(reinterpret_cast<const char*>(&Vertex.y), sizeof(float));
					Writer.WriteRawUnsafeFixed(reinterpret_cast<const char*>(&Vertex.z), sizeof(float));
				}
				for (auto& Index : Indices)
				{
					Writer.WriteInteger(std::get<0>(Index.first), sizeof(uint32_t));
					Writer.WriteInteger(std::get<1>(Index.first), sizeof(uint32_t));
					Writer.WriteInteger(std::get<2>(Index.first), sizeof(uint32_t));
					Writer.WriteInteger(Index.second, sizeof(uint32_t));
				}

				PhiveSocketConnector::SendData(PhiveSocketConnector::OperationType::FX_OP_RAWTOPHIVE, Writer.GetData());
				std::vector<unsigned char> Ret = PhiveSocketConnector::GetData();
				Util::WriteFile(Editor::GetWorkingDirFile("Pot2CollisionGeneratedByTotK_Materials.bphsh"), Ret);
				ZStdFile::Compress(Ret, ZStdFile::Dictionary::Base).WriteToFile(Editor::GetWorkingDirFile("Save/Phive/Shape/Dcc/" + SecondActorName + "__Physical_Col.Nin_NX_NVN.bphsh.zs"));

				SarcFile ActorPack(ZStdFile::Decompress(Editor::GetRomFSFile("Pack/Actor/" + SecondActorName + ".pack.zs"), ZStdFile::Dictionary::Pack).Data);

				//Clearing actor pack
				for (auto Iter = ActorPack.GetEntries().begin(); Iter != ActorPack.GetEntries().end(); ) {
					if (Iter->Name.starts_with("Phive/"))
						Iter = ActorPack.GetEntries().erase(Iter);
					else
						Iter++;
				}

				std::string ShapeParamBaseName = SecondActorName + "__Physical_Col";

				{
					SarcFile::Entry Entry;
					Entry.Bytes = GenerateShapeParamFile(ShapeParamBaseName, Vector3F(-10, -10, -10), Vector3F(10, 10, 10)).ToBinary(BymlFile::TableGeneration::Auto);
					Entry.Name = "Phive/ShapeParam/" + ShapeParamBaseName + ".phive__ShapeParam.bgyml";
					ActorPack.GetEntries().push_back(Entry);
				}

				{
					SarcFile::Entry Entry;
					Entry.Bytes = GenerateRigidBodyEntityParamByml(ShapeParamBaseName).ToBinary(BymlFile::TableGeneration::Auto);
					Entry.Name = "Phive/RigidBodyEntityParam/" + SecondActorName + "_Body.phive__RigidBodyEntityParam.bgyml";
					ActorPack.GetEntries().push_back(Entry);
				}

				//RigidBodyControllerEntityParam
				SarcFile::Entry RigidBodyControllerEntityParamEntry;
				RigidBodyControllerEntityParamEntry.Bytes = GenerateRigidBodyControllerEntityParamByml().ToBinary(BymlFile::TableGeneration::Auto);
				RigidBodyControllerEntityParamEntry.Name = "Phive/RigidBodyControllerEntityParam/" + SecondActorName + "_Body.phive__RigidBodyControllerEntityParam.bgyml";
				ActorPack.GetEntries().push_back(RigidBodyControllerEntityParamEntry);

				//ControllerSetParam
				SarcFile::Entry ControllerSetParamEntry;
				ControllerSetParamEntry.Bytes = GenerateControllerSetParamByml(SecondActorName).ToBinary(BymlFile::TableGeneration::Auto);
				ControllerSetParamEntry.Name = "Phive/ControllerSetParam/" + SecondActorName + ".phive__ControllerSetParam.bgyml";
				ActorPack.GetEntries().push_back(ControllerSetParamEntry);

				//PhysicsRef
				SarcFile::Entry* PhysicsEntry = nullptr;
				for (SarcFile::Entry& Entry : ActorPack.GetEntries())
				{
					if (Entry.Name.starts_with("Component/Physics/"))
					{
						PhysicsEntry = &Entry;
						break;
					}
				}
				if (PhysicsEntry != nullptr)
				{
					BymlFile PhysicsRefByml(PhysicsEntry->Bytes);
					if (PhysicsRefByml.HasChild("ControllerSetPath"))
					{
						BymlFile::Node* ControllerRefNode = PhysicsRefByml.GetNode("ControllerSetPath");
						ControllerRefNode->SetValue<std::string>("Work/Phive/ControllerSetParam/" + SecondActorName + ".phive__ControllerSetParam.gyml");
						PhysicsEntry->Bytes = PhysicsRefByml.ToBinary(BymlFile::TableGeneration::Auto);
					}
				}

				Util::CreateDir(Editor::GetWorkingDirFile("Save/Pack/Actor"));

				ZStdFile::Compress(ActorPack.ToBinary(), ZStdFile::Dictionary::Pack).WriteToFile(Editor::GetWorkingDirFile("Save/Pack/Actor/" + SecondActorName + ".pack.zs"));
			}
		}

		{
			uint32_t Size = ImGui::GetWindowHeight() * 0.8f;

			bool Resize = RenderCollisionModelView(Size);

			ImGui::SameLine();

			uint32_t TotalHeight = ImGui::GetFrameHeight() * 2 - ImGui::GetStyle().ItemSpacing.y * 3;
			uint32_t ShapeNameMaxLength = 0;
			for (int SubModelIndex = 0; SubModelIndex < Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size(); SubModelIndex++)
			{
				ImVec2 TextSize = ImGui::CalcTextSize(Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes[Model->mBfres->Models.GetByIndex(0).Value.Shapes.GetKey(SubModelIndex)].Value.Name.c_str());
				ShapeNameMaxLength = max(ShapeNameMaxLength, TextSize.x);
			}

			ImVec2 FirstComboPos = ImVec2(0.0f, 0.0f);
			ImVec2 FirstComboSize = ImVec2(0.0f, 0.0f);

			ImGui::BeginChild("##UICollisionCreatorMeshOptions", ImVec2(0, Size));
			for (int SubModelIndex = 0; SubModelIndex < Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size(); SubModelIndex++)
			{
				if (SubModelIndex == 0)
					ImGui::Separator();

				ImVec2 TextSize = ImGui::CalcTextSize(Model->mBfres->Models.GetByIndex(0).Value.Shapes.GetByIndex(SubModelIndex).Value.Name.c_str());
				uint32_t PaddingTopBottom = (TotalHeight - TextSize.y) / 2;

				ImGui::Columns(3, ("##UICollisionCreatorMeshOptionsShapeColumns" + std::to_string(SubModelIndex)).c_str());

				if (Resize)
				{
					ImGui::SetColumnWidth(0, ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x * 2);
					ImGui::SetColumnWidth(1, min(TextSize.x + (ShapeNameMaxLength - TextSize.x) + ImGui::GetStyle().ItemSpacing.x * 2, ImGui::GetWindowWidth() * 0.6f));
				}

				bool Hovered = false;

				ImGui::Dummy(ImVec2(0, PaddingTopBottom));
				ImGui::Checkbox(("##GenCollisionCreatorShape" + std::to_string(SubModelIndex)).c_str(), &Materials[SubModelIndex].mGenerate);
				ImGui::Dummy(ImVec2(0, PaddingTopBottom));
				ImGui::NextColumn();

				if (!Materials[SubModelIndex].mGenerate)
					ImGui::BeginDisabled();

				ImGui::Dummy(ImVec2(0, PaddingTopBottom + (ImGui::GetFrameHeight() - TextSize.y) / 2));
				ImGui::Text(Model->mBfres->Models.GetByIndex(0).Value.Shapes.GetByIndex(SubModelIndex).Value.Name.c_str());
				Hovered = Hovered || ImGui::IsItemHovered();
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(0, PaddingTopBottom));

				SubModelHovered[SubModelIndex] = Hovered;

				ImGui::NextColumn();
				if (FirstComboPos.x == 0.0f && FirstComboPos.y == 0.0f)
				{
					FirstComboPos = ImGui::GetCursorScreenPos();
				}
				ImGui::Combo(("##UICollisionCreatorMaterialType" + std::to_string(SubModelIndex)).c_str(), reinterpret_cast<int*>(&Materials[SubModelIndex].mMaterial.mMaterialId), PhiveMaterialData::mMaterialNames.data(), PhiveMaterialData::mMaterialNames.size());
				if (FirstComboSize.x == 0.0f && FirstComboSize.y == 0.0f)
				{
					ImGui::SameLine();
					FirstComboSize = ImVec2(ImGui::GetCursorScreenPos().x - FirstComboPos.x, ImGui::GetCursorScreenPos().y - FirstComboPos.y);
					ImGui::NewLine();
				}
				//ImGui::Checkbox(("Climbable##CollisionCreatorShape" + std::to_string(SubModelIndex)).c_str(), &Materials[SubModelIndex].Settings.Climbable);
				float w = ImGui::GetCursorScreenPos().x;
				if (ImGui::Button(("Flags##CollisionCreatorShape" + std::to_string(SubModelIndex)).c_str()))
				{
					mFlagSubModelIndex = SubModelIndex;
					mMaterialPopUpW = w;
					mMaterialPopUpPos = ImGui::GetCursorScreenPos();
					mMaterialPopUpOpen = true;
				}

				ImGui::Columns();

				if (!Materials[SubModelIndex].mGenerate)
					ImGui::EndDisabled();

				ImGui::Separator();
			}
			ImGui::EndChild();

			if (FirstComboPos.x != 0.0f || FirstComboPos.y != 0.0f)
			{
				ImGui::SetCursorScreenPos(ImVec2(FirstComboPos.x, FirstComboPos.y - ImGui::GetStyle().ItemSpacing.y - ImGui::GetFrameHeight() * 2));
				int SelectedMasterMaterial = 0;
				ImGui::PushItemWidth(FirstComboSize.x - ImGui::GetStyle().ItemSpacing.x);
				if (ImGui::Combo("##UICollisionCreatorMasterMaterial", &SelectedMasterMaterial, MasterMaterialNames.data(), MasterMaterialNames.size()))
				{
					if (SelectedMasterMaterial > 0)
					{
						for (int SubModelIndex = 0; SubModelIndex < Model->mBfres->Models.GetByIndex(0).Value.Shapes.Nodes.size(); SubModelIndex++)
						{
							Materials[SubModelIndex].mMaterial.mMaterialId = SelectedMasterMaterial - 1;
						}
					}
				}
				ImGui::PopItemWidth();
			}

			if (FirstFrame)
			{
				FirstFrame = false;
			}
		}
	}
End:
	if (mMaterialPopUpOpen)
	{
		mMaterialPopUpOpen = false;
		ImGui::OpenPopup("CollisionCreatorMaterialFlagPopUp");
	}
	DrawMaterialFlagsPopUp();

	ImGui::End();
}