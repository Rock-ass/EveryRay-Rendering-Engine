#pragma once
#include "Common.h"
#include "ConstantBuffer.h"
#include "PostProcessingStack.h"
#include "CustomRenderTarget.h"

namespace Library
{
	class FullScreenRenderTarget;
	class DirectionalLight;
	class GameComponent;
	class GameTime;
	class Camera;
	class Skybox;

	namespace VolumetricCloudsCBufferData {
		struct FrameCB
		{
			XMMATRIX	InvProj;
			XMMATRIX	InvView;
			XMVECTOR	LightDir;
			XMVECTOR	LightCol;
			XMVECTOR	CameraPos;
			XMFLOAT2	Resolution;
		};

		struct CloudsCB
		{
			XMVECTOR AmbientColor;
			XMVECTOR WindDir;
			float WindSpeed;
			float Time;
			float Crispiness;
			float Curliness;
			float Coverage;
			float Absorption;
			//float CloudsLayerSphereInnerRadius;
			//float CloudsLayerSphereOuterRadius;
		};
	}

	class VolumetricClouds : public GameComponent
	{
	public:
		VolumetricClouds(Game& game, Camera& camera, DirectionalLight& light, Rendering::PostProcessingStack& stack, Skybox& skybox);
		~VolumetricClouds();

		void Initialize();

		void Draw(const GameTime& gametime);
		void Update(const GameTime& gameTime);

	private:
		void UpdateImGui();

		Camera& mCamera;
		DirectionalLight& mDirectionalLight;
		Skybox& mSkybox;
		Rendering::PostProcessingStack& mPostProcessingStack;
		
		ConstantBuffer<VolumetricCloudsCBufferData::FrameCB> mFrameConstantBuffer;
		ConstantBuffer<VolumetricCloudsCBufferData::CloudsCB> mCloudsConstantBuffer;

		CustomRenderTarget* mMainRenderTargetCS_Custom;
		FullScreenRenderTarget* mMainRenderTargetPS = nullptr;
		FullScreenRenderTarget* mCompositeRenderTarget = nullptr;
		FullScreenRenderTarget* mBlurRenderTarget = nullptr;

		ID3D11ComputeShader* mMainCS = nullptr;
		ID3D11PixelShader* mMainPS = nullptr;
		ID3D11PixelShader* mCompositePS = nullptr;
		ID3D11PixelShader* mBlurPS = nullptr;

		ID3D11ShaderResourceView* mCloudTextureSRV = nullptr;
		ID3D11ShaderResourceView* mWeatherTextureSRV = nullptr;
		ID3D11ShaderResourceView* mWorleyTextureSRV = nullptr;

		ID3D11SamplerState* mCloudSS = nullptr;
		ID3D11SamplerState* mWeatherSS = nullptr;

		float mCrispiness = 40.0f;
		float mCurliness = 0.2f;
		float mCoverage = 0.5f;
		float mAmbientColor[3] = { 121.0f / 255.0f, 133.0f / 255.0f, 138.0f / 255.0f };
		float mWindSpeedMultiplier = 1000.0f;
		float mLightAbsorption = 0.002f;
		//float mCloudsLayerInnerHeight = 5000.0f;
		//float mCloudsLayerOuterHeight = 17000.0f;

		bool mUseComputeShaderVersion = true;
		bool mEnabled = true;
	};
}