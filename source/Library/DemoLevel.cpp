#include "DemoLevel.h"
#include "Utility.h"
#include "Systems.inl"
#include "ER_MaterialsCallbacks.h"

namespace Library {

	DemoLevel::DemoLevel()
	{
	}
	DemoLevel::~DemoLevel()
	{
		Destroy();
	}
	
	void DemoLevel::Destroy()
	{
        DeleteObject(mRenderStateHelper);
        DeleteObject(mDirectionalLight);
        DeleteObject(mSkybox);
        DeleteObject(mGrid);
        DeleteObject(mPostProcessingStack);
        DeleteObject(mGBuffer);
        DeleteObject(mShadowMapper);
        DeleteObject(mFoliageSystem);
        DeleteObject(mVolumetricClouds);
        DeleteObject(mIllumination);
        DeleteObject(mScene);
        DeleteObject(mIlluminationProbesManager);
	}

    void DemoLevel::Initialize(Game& game, Camera& camera, const std::string& sceneName, const std::string& sceneFolderPath)
    {
        mRenderStateHelper = new RenderStateHelper(game);

		#pragma region INIT_SCENE
		game.CPUProfiler()->BeginCPUTime("Scene init: " + sceneName);
        mScene = new Scene(game, camera, sceneFolderPath + sceneName + ".json");
		//TODO move to scene
        camera.SetPosition(mScene->cameraPosition);
        camera.SetDirection(mScene->cameraDirection);
        camera.SetFarPlaneDistance(100000.0f);
		game.CPUProfiler()->EndCPUTime("Scene init: " + sceneName);
#pragma endregion

		#pragma region INIT_EDITOR
		mEditor = (Editor*)game.Services().GetService(Editor::TypeIdClass());
		assert(mEditor);
		mEditor->LoadScene(mScene);
#pragma endregion

		#pragma region INIT_GBUFFER
        game.CPUProfiler()->BeginCPUTime("Gbuffer init");
        mGBuffer = new GBuffer(game, camera, game.ScreenWidth(), game.ScreenHeight());
        mGBuffer->Initialize();
        game.CPUProfiler()->EndCPUTime("Gbuffer init");
#pragma endregion

		#pragma region INIT_CONTROLS
        mKeyboard = (Keyboard*)game.Services().GetService(Keyboard::TypeIdClass());
        assert(mKeyboard);
#pragma endregion

		#pragma region INIT_SKYBOX
		game.CPUProfiler()->BeginCPUTime("Skybox init");
        mSkybox = new Skybox(game, camera, Utility::GetFilePath(Utility::ToWideString(mScene->skyboxPath)), 10000);
        mSkybox->Initialize();
		game.CPUProfiler()->EndCPUTime("Skybox init");
#pragma endregion

		#pragma region INIT_GRID
        mGrid = new Grid(game, camera, 200, 56, XMFLOAT4(0.961f, 0.871f, 0.702f, 1.0f));
        mGrid->Initialize();
        mGrid->SetColor((XMFLOAT4)ColorHelper::LightGray);
#pragma endregion

		#pragma region INIT_DIRECTIONAL_LIGHT
        mDirectionalLight = new DirectionalLight(game, camera);
        mDirectionalLight->ApplyRotation(XMMatrixRotationAxis(mDirectionalLight->RightVector(), -XMConvertToRadians(70.0f)) * XMMatrixRotationAxis(mDirectionalLight->UpVector(), -XMConvertToRadians(25.0f)));
        mDirectionalLight->SetAmbientColor(mScene->ambientColor);
        mDirectionalLight->SetSunColor(mScene->sunColor);
#pragma endregion

		#pragma region INIT_SHADOWMAPPER
		game.CPUProfiler()->BeginCPUTime("Shadow mapper init");
        mShadowMapper = new ShadowMapper(game, camera, *mDirectionalLight, 4096, 4096);
        mDirectionalLight->RotationUpdateEvent->AddListener("shadow mapper", [&]() { mShadowMapper->ApplyTransform(); });
		game.CPUProfiler()->EndCPUTime("Shadow mapper init");
#pragma endregion

		#pragma region INIT_POST_PROCESSING
		game.CPUProfiler()->BeginCPUTime("Post processing stack init");
        mPostProcessingStack = new Rendering::PostProcessingStack(game, camera);
        mPostProcessingStack->Initialize(false, false, true, true, true, false, false, false);
        mPostProcessingStack->SetDirectionalLight(mDirectionalLight);
        mPostProcessingStack->SetSunOcclusionSRV(mSkybox->GetSunOcclusionOutputTexture());
		game.CPUProfiler()->EndCPUTime("Post processing stack init");
#pragma endregion

		#pragma region INIT_VOLUMETRIC_CLOUDS
		game.CPUProfiler()->BeginCPUTime("Volumetric Clouds init");
        mVolumetricClouds = new VolumetricClouds(game, camera, *mDirectionalLight, *mPostProcessingStack, *mSkybox);
		game.CPUProfiler()->EndCPUTime("Volumetric Clouds init");
#pragma endregion

		#pragma region INIT_ILLUMINATION
		game.CPUProfiler()->BeginCPUTime("Illumination init");
        mIllumination = new Illumination(game, camera, *mDirectionalLight, *mShadowMapper, mScene);
		game.CPUProfiler()->EndCPUTime("Illumination init");
#pragma endregion

		#pragma region INIT_LIGHTPROBES_MANAGER
		game.CPUProfiler()->BeginCPUTime("Light probes manager init");
        mIlluminationProbesManager = new ER_IlluminationProbeManager(game, camera, mScene, *mDirectionalLight, *mShadowMapper);
		mIlluminationProbesManager->SetLevelPath(Utility::ToWideString(sceneFolderPath));
		mIllumination->SetProbesManager(mIlluminationProbesManager);
		game.CPUProfiler()->EndCPUTime("Light probes manager init");
#pragma endregion

		#pragma region INIT_FOLIAGE_MANAGER
		game.CPUProfiler()->BeginCPUTime("Foliage init");
        mFoliageSystem = new FoliageSystem();
        mFoliageSystem->FoliageSystemInitializedEvent->AddListener("foliage initialized for GI",  [&]() { mIllumination->SetFoliageSystemForGI(mFoliageSystem); });
		//TODO add foliage from level
		//mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 3500, Utility::GetFilePath("content\\textures\\foliage\\grass_type1.png"), 2.5f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		//mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 100, Utility::GetFilePath("content\\textures\\foliage\\grass_flower_type1.png"), 3.5f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		//mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 50, Utility::GetFilePath("content\\textures\\foliage\\grass_flower_type3.png"), 2.5f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		//mFoliageSystem->AddFoliage(new Foliage(*mGame, *mCamera, *mDirectionalLight, 100, Utility::GetFilePath("content\\textures\\foliage\\grass_flower_type10.png"), 3.5f, 100.0f, XMFLOAT3(0.0, 0.0, 0.0), FoliageBillboardType::SINGLE));
		mFoliageSystem->Initialize();
		game.CPUProfiler()->EndCPUTime("Foliage init");
#pragma endregion

		#pragma region INIT_MATERIAL_CALLBACKS
		game.CPUProfiler()->BeginCPUTime("Material callbacks init");
		ER_MaterialSystems materialSystems;
		materialSystems.mCamera = &camera;
		materialSystems.mDirectionalLight = mDirectionalLight;
		materialSystems.mShadowMapper = mShadowMapper;
		materialSystems.mProbesManager = mIlluminationProbesManager;

		for (auto& object : mScene->objects) {
			if (!object.second->IsForwardShading() /*TODO|| object.second->IsInGBuffer()*/)
				object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::deferredPrepassMaterialName, [&, matSystems = materialSystems](int meshIndex) { Library::ER_MaterialsCallbacks::UpdateDeferredPrepassMaterialVariables(matSystems, object.second, meshIndex); });
			
			object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::shadowMapMaterialName + " " + std::to_string(0), [&, matSystems = materialSystems](int meshIndex) { Library::ER_MaterialsCallbacks::UpdateShadowMappingMaterialVariables(matSystems, object.second, meshIndex, 0); });
			object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::shadowMapMaterialName + " " + std::to_string(1), [&, matSystems = materialSystems](int meshIndex) { Library::ER_MaterialsCallbacks::UpdateShadowMappingMaterialVariables(matSystems, object.second, meshIndex, 1); });
			object.second->MeshMaterialVariablesUpdateEvent->AddListener(MaterialHelper::shadowMapMaterialName + " " + std::to_string(2), [&, matSystems = materialSystems](int meshIndex) { Library::ER_MaterialsCallbacks::UpdateShadowMappingMaterialVariables(matSystems, object.second, meshIndex, 2); });
		}
		game.CPUProfiler()->EndCPUTime("Material callbacks init");
#pragma endregion

    }

	void DemoLevel::UpdateLevel(Game& game, const GameTime& gameTime)
	{
		//TODO refactor to updates for elements of ER_CoreComponent type

		//TODO refactor skybox updates
		mSkybox->SetUseCustomSkyColor(mEditor->IsSkyboxUsingCustomColor());
		mSkybox->SetSkyColors(mEditor->GetBottomSkyColor(), mEditor->GetTopSkyColor());
		mSkybox->SetSunData(mDirectionalLight->IsSunRendered(),
			mDirectionalLight->DirectionVector(),
			XMVECTOR{ mDirectionalLight->GetDirectionalLightColor().x, mDirectionalLight->GetDirectionalLightColor().y, mDirectionalLight->GetDirectionalLightColor().z, 1.0 },
			mDirectionalLight->GetSunBrightness(), mDirectionalLight->GetSunExponent());
		mSkybox->Update(gameTime);
		mSkybox->UpdateSun(gameTime);

		mGrid->Update(gameTime);
		mPostProcessingStack->Update();
		mVolumetricClouds->Update(gameTime);
		mIllumination->Update(gameTime, mScene);
		mIlluminationProbesManager->UpdateProbes(game); //TODO move to Illumination
		mShadowMapper->Update(gameTime);
		//mFoliageSystem->Update(gameTime, mWindGustDistance, mWindStrength, mWindFrequency); //TODO
		//mDirectionalLight->UpdateProxyModel(gameTime, mCamera->ViewMatrix4X4(), mCamera->ProjectionMatrix4X4()); TODO refactor to DebugRenderer

		mScene->GetCamera().Cull(mScene->objects);
		for (auto& object : mScene->objects)
			object.second->Update(gameTime);

        UpdateImGui();
	}

    void DemoLevel::UpdateImGui()
    {
        ImGui::Begin("Systems Config");

        if (ImGui::Button("Post Processing Stack")) 
            mPostProcessingStack->Config();

		if (ImGui::Button("Volumetric Clouds"))
			mVolumetricClouds->Config();

        if (ImGui::Button("Global Illumination"))
			mIllumination->Config();

		//TODO shadow mapper config

		//TODO skybox config

        ImGui::End();
    }

	void DemoLevel::DrawLevel(Game& game, const GameTime& gameTime)
	{
		if (mRenderStateHelper)
			mRenderStateHelper->SaveRasterizerState();

		ID3D11DeviceContext* direct3DDeviceContext = game.Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		#pragma region DRAW_GBUFFER

		mGBuffer->Start();
		for (auto it = mScene->objects.begin(); it != mScene->objects.end(); it++)
			it->second->Draw(MaterialHelper::deferredPrepassMaterialName, true);
		mFoliageSystem->Draw(gameTime, nullptr, FoliageRenderingPass::TO_GBUFFER);
		mGBuffer->End();

#pragma endregion

		#pragma region DRAW_SHADOWS
		mShadowMapper->Draw(mScene);
		mRenderStateHelper->RestoreRasterizerState();
#pragma endregion

		#pragma region DRAW_GLOBAL_ILLUMINATION
		if (!mIlluminationProbesManager->AreProbesReady())
		{
			game.CPUProfiler()->BeginCPUTime("Compute or load light probes");
			mIlluminationProbesManager->ComputeOrLoadProbes(game, gameTime, mScene->objects, mSkybox);
			game.CPUProfiler()->EndCPUTime("Compute or load light probes");
		}

		mRenderStateHelper->SaveAll();
		mIllumination->DrawGlobalIllumination(mGBuffer, gameTime);
		mRenderStateHelper->RestoreAll();
#pragma endregion

		mPostProcessingStack->Begin(true, mGBuffer->GetDepth());

		#pragma region DRAW_LOCAL_ILLUMINATION
		mSkybox->Draw();
		mIllumination->DrawLocalIllumination(mGBuffer, mPostProcessingStack->GetMainRenderTarget(), Utility::IsEditorMode);

		if (Utility::IsEditorMode)
		{
			mDirectionalLight->DrawProxyModel(gameTime); //TODO move to Illumination() or better to separate debug renderer system
		}
#pragma endregion

		mPostProcessingStack->End();

		#pragma region DRAW_SUN
		mSkybox->DrawSun(nullptr, mPostProcessingStack);
#pragma endregion

		#pragma region DRAW_VOLUMETRIC_CLOUDS
		mVolumetricClouds->Draw(gameTime);
#pragma endregion

		#pragma region DRAW_POSTPROCESSING
		mPostProcessingStack->UpdateCompositeLightingMaterial(mPostProcessingStack->GetMainRenderTarget()->GetSRV(), mIllumination->GetGlobaIlluminationSRV(), mIllumination->GetDebugVoxels(), mIllumination->GetDebugAO());
		mPostProcessingStack->UpdateSSRMaterial(mGBuffer->GetNormals()->GetSRV(), mGBuffer->GetDepth()->getSRV(), mGBuffer->GetExtraBuffer()->GetSRV(), (float)gameTime.TotalGameTime());
		mPostProcessingStack->DrawEffects(gameTime);
#pragma endregion

		mRenderStateHelper->SaveAll();

		#pragma region DRAW_IMGUI
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

#pragma endregion
	}
}
