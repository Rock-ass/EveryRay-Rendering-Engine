#pragma once
#include "Common.h"
#include "GameComponent.h"
#include "Camera.h"
#include "ModelMaterial.h"
#include "ER_Material.h"

#include "..\JsonCpp\include\json\json.h"

namespace Library
{
	class ER_RenderingObject;
	class DirectionalLight;
	class ER_Foliage;

	class ER_Scene : public GameComponent
	{
	public:
		ER_Scene(Game& pGame, Camera& pCamera, const std::string& path);
		~ER_Scene();

		void SaveRenderingObjectsTransforms();
		void SaveFoliageZonesTransforms(const std::vector<ER_Foliage*>& foliageZones);

		ER_Material* GetMaterialByName(const std::string& matName, const MaterialShaderEntries& entries, bool instanced);
		Camera& GetCamera() { return mCamera; }

		bool HasLightProbesSupport() { return mHasLightProbes; }
		const XMFLOAT3& GetLightProbesVolumeMinBounds() const { return mLightProbesVolumeMinBounds; }
		const XMFLOAT3& GetLightProbesVolumeMaxBounds() const { return mLightProbesVolumeMaxBounds; }
		float GetLightProbesDiffuseDistance() { return mLightProbesDiffuseDistance; }
		float GetLightProbesSpecularDistance() { return mLightProbesSpecularDistance; }
		
		bool HasFoliage() { return mHasFoliage; }
		void LoadFoliageZones(std::vector<ER_Foliage*>& foliageZones, DirectionalLight& light);

		bool HasTerrain() { return mHasTerrain; }
		int GetTerrainTilesCount() { return mTerrainTilesCount; }
		float GetTerrainTileScale() { return mTerrainTileScale; }
		const std::wstring& GetTerrainSplatLayerTextureName(int index) { return mTerrainSplatLayersTextureNames[index]; }

		bool HasVolumetricFog() { return mHasVolumetricFog; }

		std::map<std::string, ER_RenderingObject*> objects;

		//TODO remove to private and make public methods
		std::string skyboxPath;
		XMFLOAT3 cameraPosition;
		XMFLOAT3 cameraDirection;
		XMFLOAT3 sunDirection;
		XMFLOAT3 sunColor;
		XMFLOAT3 ambientColor;

	private:
		void LoadRenderingObjectData(ER_RenderingObject* aObject);
		void LoadRenderingObjectInstancedData(ER_RenderingObject* aObject);

		Json::Value root;
		Camera& mCamera;
		std::string mScenePath;
		
		bool mHasVolumetricFog = false;

		bool mHasFoliage = false;

		bool mHasTerrain = false;
		int mTerrainTilesCount = 0;
		float mTerrainTileScale = 1.0f;
		std::wstring mTerrainSplatLayersTextureNames[4];

		bool mHasLightProbes = true;
		XMFLOAT3 mLightProbesVolumeMinBounds = { 0,0,0 };
		XMFLOAT3 mLightProbesVolumeMaxBounds = { 0,0,0 };
		float mLightProbesDiffuseDistance = -1.0f;
		float mLightProbesSpecularDistance = -1.0f;
	};
}