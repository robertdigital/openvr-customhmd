#ifndef TrackedHMD_H
#define TrackedHMD_H

#include "TrackedDevice.h" 
//#include <ScreenGrab.h>
//#include <DirectXTex.h>
#include <DXGI1_2.h>
#include <turbojpeg.h>


//#include <D3D11_1.h>
#include <map>

using namespace vr;

struct TextureData
{
	uint32_t Index;
	ID3D11Texture2D *pTexture;
	IDXGIResource1 *pResource1;
	ID3D11Resource *pResource;
	HANDLE hSharedHandle;
};

struct TextureSet
{
	uint32_t Pid;
	//D3D11_TEXTURE2D_DESC Desc;	
	TextureData Data[3];
	bool HasHandle(HANDLE handle)
	{
		for (auto i = 0; i < 3; i++)
		{
			if (Data[i].hSharedHandle == handle)
				return true;
		}
		return false;
	}
};

struct TextureLink
{
	TextureData *pData;
	TextureSet *pSet;
};

class CTrackedHMD : 
	public IVRDisplayComponent,
	public IVRCameraComponent,
	public IVRDriverDirectModeComponent,
	public CTrackedDevice
{
private:	
	HMDData m_HMDData;		
	CameraData m_Camera;

public:
	CTrackedHMD(std::string displayName, CServerDriver *pServer);
	~CTrackedHMD();
	bool IsConnected();
	DriverPose_t GetPose() override;

public: //ITrackedDeviceServerDriver
	EVRInitError Activate(uint32_t unObjectId) override;
	void Deactivate() override;
	void EnterStandby() override;
	void *GetComponent(const char *pchComponentNameAndVersion) override;
	void DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) override;

public: //IVRDisplayComponent
	void GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	bool IsDisplayOnDesktop() override;
	bool IsDisplayRealDisplay() override;
	void GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight) override;
	void GetEyeOutputViewport(EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	void GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) override;
	DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV) override;

public: //IVRCameraComponent
	bool GetCameraFrameDimensions(ECameraVideoStreamFormat nVideoStreamFormat, uint32_t *pWidth, uint32_t *pHeight) override;
	bool GetCameraFrameBufferingRequirements(int *pDefaultFrameQueueSize, uint32_t *pFrameBufferDataSize) override;
	bool SetCameraFrameBuffering(int nFrameBufferCount, void **ppFrameBuffers, uint32_t nFrameBufferDataSize) override;
	bool SetCameraVideoStreamFormat(ECameraVideoStreamFormat nVideoStreamFormat) override;
	ECameraVideoStreamFormat GetCameraVideoStreamFormat() override;
	bool StartVideoStream() override;
	void StopVideoStream() override;
	const CameraVideoStreamFrame_t *GetVideoStreamFrame() override;
	void ReleaseVideoStreamFrame(const CameraVideoStreamFrame_t *pFrameImage) override;
	bool SetAutoExposure(bool bEnable) override;
	bool PauseVideoStream() override;
	bool ResumeVideoStream() override;
	bool IsVideoStreamActive(bool *pbPaused, float *pflElapsedTime) override;
	bool GetCameraDistortion(float flInputU, float flInputV, float *pflOutputU, float *pflOutputV) override;
	bool GetCameraProjection(vr::EVRTrackedCameraFrameType eFrameType, float flZNear, float flZFar, vr::HmdMatrix44_t *pProjection) override;
	bool SetFrameRate(int nISPFrameRate, int nSensorFrameRate) override;
	bool SetCameraVideoSinkCallback(ICameraVideoSinkCallback *pCameraVideoSinkCallback) override;
	bool GetCameraCompatibilityMode(ECameraCompatibilityMode *pCameraCompatibilityMode) override;
	bool SetCameraCompatibilityMode(ECameraCompatibilityMode nCameraCompatibilityMode) override;
	bool GetCameraFrameBounds(EVRTrackedCameraFrameType eFrameType, uint32_t *pLeft, uint32_t *pTop, uint32_t *pWidth, uint32_t *pHeight) override;
	bool GetCameraIntrinsics(EVRTrackedCameraFrameType eFrameType, HmdVector2_t *pFocalLength, HmdVector2_t *pCenter) override;


private:
	SharedTextureHandle_t m_SyncTexture;
	ID3D11Texture2D *m_pSyncTexture;
	ID3D11DeviceContext *m_pContext;
	ID3D11Device *m_pDevice;
	D3D_FEATURE_LEVEL m_FeatureLevel;
	ID3D11Texture2D *m_pLeftTexture;
	ID3D11Resource *m_pLeftResource;
	ID3D11Texture2D *m_pRightTexture;
	ID3D11Resource *m_pRightResource;
	std::vector<TextureSet*> m_TextureSets;
	std::map<SharedTextureHandle_t, TextureLink> m_TextureMap;	
	void ProcessFrame();
	HANDLE m_hBufferLock;
	HANDLE m_hTextureMapLock;
	bool m_HasDirectFrame;
	tjhandle m_hTJ;


public: //IVRDriverDirectModeComponent
	void CreateSwapTextureSet(uint32_t unPid, uint32_t unFormat, uint32_t unWidth, uint32_t unHeight, vr::SharedTextureHandle_t(*pSharedTextureHandles)[3]) override;
	void DestroySwapTextureSet(vr::SharedTextureHandle_t sharedTextureHandle) override;
	void DestroyAllSwapTextureSets(uint32_t unPid) override;
	void GetNextSwapTextureSetIndex(vr::SharedTextureHandle_t sharedTextureHandles[2], uint32_t(*pIndices)[2]) override;
	void SubmitLayer(vr::SharedTextureHandle_t sharedTextureHandles[2], const vr::VRTextureBounds_t(&bounds)[2], const vr::HmdMatrix34_t *pPose) override;
	void Present(vr::SharedTextureHandle_t syncTexture) override;


protected: //CTrackedDevice
	void PacketReceived(USBPacket *pPacket, HmdVector3d_t *pCenterEuler, HmdVector3d_t *pRelativePos) override;
	void RunFrame(DWORD currTick) override;

private:		
	void SetupCamera();
	static void CameraFrameUpdateCallback(CCaptureDevice *pCaptureDevice, void *pUserData);
	void OnCameraFrameUpdate();	
	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);



protected:
	void SetDefaultProperties() override;
	bool ReportsTimeSinceVSync;
	float SecondsFromVsyncToPhotons;
	float DisplayFrequency;
	float UserIpdMeters;
	uint64_t CurrentUniverseId;
	uint64_t PreviousUniverseId;
	uint64_t DisplayFirmwareVersion;
	bool IsOnDesktop;
	int32_t DisplayMCType;
	float DisplayMCOffset;
	float DisplayMCScale;
	int32_t EdidVendorID;
	std::string DisplayMCImageLeft;
	std::string DisplayMCImageRight;
	float DisplayGCBlackClamp;
	int32_t EdidProductID;
	HmdMatrix34_t CameraToHeadTransform;
	int32_t DisplayGCType;
	float DisplayGCOffset;
	float DisplayGCScale;
	float DisplayGCPrescale;
	std::string DisplayGCImage;
	float LensCenterLeftU;
	float LensCenterLeftV;
	float LensCenterRightU;
	float LensCenterRightV;
	float UserHeadToEyeDepthMeters;
	uint64_t CameraFirmwareVersion;
	std::string CameraFirmwareDescription;
	uint64_t DisplayFPGAVersion;
	uint64_t DisplayBootloaderVersion;
	uint64_t DisplayHardwareVersion;
	uint64_t AudioFirmwareVersion;
	int32_t CameraCompatibilityMode;
	float ScreenshotHorizontalFieldOfViewDegrees;
	float ScreenshotVerticalFieldOfViewDegrees;
	bool DisplaySuppressed;
	bool DisplayAllowNightMode;
	int32_t DisplayMCImageWidth;
	int32_t DisplayMCImageHeight;
	int32_t DisplayMCImageNumChannels;
	void *DisplayMCImageData;
	bool UsesDriverDirectMode;
};

#endif // TrackedHMD_H