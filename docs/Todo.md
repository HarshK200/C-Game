### TODO:

[ ] Write a Texture sampler
	[x] finish arena allocation & arena reset and what memory allignment is?
	[x] Maybe create a AppMemory struct? which will hold AppMemory.PermanentAllocator and AppMemory.TempAllocator
	[x] File IO with arena allocation
	[x] setup stb_image
	[ ] Write CreateTexture() function *yoink code from odin code*
	[ ] Load texture files
	[ ] Modify default.hlsl shader so it takes uv/texcoords as input
	[ ] In draw.h and draw.cpp write the draw game function which is the master draw function
[ ] Write the InputManager and figure out how to split it for cross platform
	- NOTE: maybe write win32/input.h and win32/input.cpp and implement them for each platform
	and in the main.h define what services are expected to be implemented by them like so:
	```main.h
	// PLATFORM AGNOSTIC
	enum INPUT_ACTIONS {
		MOVE_RIGHT,
		MOVE_LEFT,
		MOVE_UP,
		MOVE_DOWN,
		DODGE,
	};
	enum ACTION_STATE {
		PRESSED,
		HELD,
		RELEASED,
	};
	enum MOUSE_COORDINATES;
	
	// PLATFORM SPECIFIC
	ACTION_STATE ActionMap[INPUT_ACTION];
	struct InputManager;
	void InputPollMessage(PlatformApp* app);
	```
[ ] Delta time
[ ] Setup Audio Layer (use XAudio2 built into windows 11 SDK, low to mid level API)
	- NOTE: Audio API used by Handmade hero i.e. DirectSound is long depricated Deprecated since
	the era of DirectX 8/Vista.
[ ] Disable WindowResize and Make window resizing done through settings.
[ ] Handle sRGB and Linear color format in the sampler and shader i.e. when loading sRGB files make sure to specify
    the file uses sRGB color encoding by declaring texture with `DXGI_FORMAT_R8G8B8A8_UNORM_SRGB` and the GPU will do
    the sRGB -> Linear conversion itself

### Done:
[x] Setup a Basic clean Cross-platform Api Layer
[x] Open a win32 window
[x] Figure out how to get D3D11_CREATE_DEVICE_DEBUG feature working
    - Just had to install Direct3D 12 debug layers (system component) from here:
    `https://learn.microsoft.com/en-us/windows/ai/directml/dml-debug-layer#installing-the-directml-and-direct3d-12-debug-layers-system-component`
[x] Draw a triangle
[x] Draw a Quad
[x] Figure out Internal Render Resolution Upscaling
	- First Pass (Drawing to internal render texture 640x360):
		[x] Create a InternalRenderTexture size 640x360 i.e. 16:9 ratio
		by calling r->Device->CreateTexture2D()
		[x] Create (render target view) + (shader resource view) for
		the InternalRenderTexture
		[x] We draw the game to this InternalRTV

	- Second Pass (Draw the InternalRenderTexture to a RenderTargetTexture as a ShaderResourceView):
		[x] Create the point sampler in the RendererInit()
		[x] Bind the InternalTextureSRV and the PointSampler i.e. Nearest Neighbour sampler
		[x] Write the pixel shader for upscaling
[x] Correct UV coordinates for UpscaleQuadMesh
[x] Create a Shader.cpp and Shader.h file and move CreateShader() and LoadAllShaders() function to that
[x] write main.cpp file as main entry point
[x] Create the logger with assert
[x] Create a Arena Allocation system
    - NOTE: watch handmade hero and also check randy's resources