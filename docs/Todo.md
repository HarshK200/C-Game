### TODO:

[ ] Write a Texture sampler
	[x] finish arena allocation & arena reset and what memory allignment is?
	[x] Maybe create a AppMemory struct? which will hold AppMemory.PermanentAllocator and AppMemory.TempAllocator
	[x] File IO with arena allocation
	[x] setup stb_image
	[x] Write CreateTexture() function *yoink code from odin code*
	[x] Load texture files
	[x] Modify default.hlsl shader so it takes uv/texcoords as input'
	[x] Test the texture loading implemention and default.hlsl by sampling a texture.
		[x] Fix sRGB bug. (FIXED BY load texture as sRGB *d3d11 automatically converts sRGB -> linear, internal Texture*
		*format remain non-sRGB i.e. linear, and finally BackBuffer's RenderTargetView set as sRGB, so d3d11 automatically*
		*converts Linear -> sRGB*
		[x] Fix wonky aaa BUG texture sampling. *DON'T FORGET THE D3D11_APPEND_ALIGNED_ELEMENT when editing shaders you dumbass*
		[x] *RenderDoc is the GOAT!!!!*
	[x] Write Matrix tranformation functions in game_math.cpp *Yoink from odin code*
		[x] Look at *handmade hero video* and decide if a *glm like matrix tranformation API* is better
		or raw multiplication is better with *operator overloading
		[x] Write Mat4xMat4()
		[x] For Vec3 x Mat4 multiplications overload *operator
	[x] create camera2d with camera camera_view_matrix() function in game/camera2d.cpp
	[x] create projection matrix in utils/game_math.cpp
	[x] Write build script for git bash cause build.bat is utterly useless and slow *it takes 6 SECONDS!!!*
	[ ] Figure out how to upload uniforms to D3D11 shaders
		- Uniforms are just buffers in D3D11_CONSTANT_BUFFER you can set if they are modifiable
		by the CPU or not by setting the CPUAcessFlags
		- On the Renderer create a shader uniforms array which will have `D3D11Buffer* array`
		the buffer pointer array will be similar to Textures array on the Renderer with an
		enum used for indexing.
		- The `ID3D11Buffer* ShaderUniforms[UNIFORM_COUNT];` will be populated by the function
		`HRESULT CreateShaderUniformBuffers(); wihc calls r->Device->CreateBuffer();`
		- And then when making draw calls you can just upload uniform data to these buffers by just
		calling `r->DeviceContext->Map()`.
		NOTE(harsh): during data upload `DeviceContext->Map()` takes a `ID3D11Resource` and since
		`ID3D11Buffer` inherits `ID3D11Resource` you just put the ID3D11Buffer pointer there.
		The main data is put in `D3D11_MAPPED_SUBRESOURCE.pData` where pData is a void pointer like
		so: `void *pData;` just type cast the pointer to `FrameUniforms` or whatever uniform struct
		derefrence it and fill the values in from there.
		Make sure you call `DeviceContext->Map()` first on the `D3D11_MAPPED_SUBRESOURCE` so the
		`void*` points to allocated memory to fill in the struct.
	[ ] create model matrix per entity draw call
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
[ ] Hot code reloading
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

# Inspiration
	Dungeon environment inspired by anime *Delicious in Dungeon*
	Adventures guild/quests inspired by anime *Overlord*
	Combat inspirted by game *Hyperlight Drifter*
	Skill tree inspired by a combination of games *Avencast + PathOfExile*
	Dungeon corruption spread highly inspired by game *Terraria*

# Gameplay loop:
	*NOT LORE HEAVY, just slight lore about each continent boss in inscriptions like Hyperlight drifter*
	Procedurally generated world, villages and dungeon, *3 Continents Human, Elf, Demon*
	Player can choose to play either of the character.
	First spawn at the starter town, near the edge of a forest with rags + rusty sword.
	Get quests from adventurers guild,
	Kill monsters in dungeon -> monster drops items -> Sell items
	-> Get better gear *More Power* -> Complete difficult quest *Higher rank quest unlocks*
	-> Player level go++ *Each level give stats to spend on skill tree*
	-> Unlock skill get strong -> Harder quests difficult enemies
	-> Enventually defeat the human continents dungeon boss
	-> Unlock travel to Elf then Demon contienet clear the continents
	-> Colonize *Monsters don't respawn every time you clear an area/continent that place is your to keep*
	
	*Game's END GOAL* Kill the final boss of each dungeon free continent from the demon taking over.
	*Dungeons spread like corruption in terraria*
	You clear the dungeon the dungeon corruption goes away, demons stop spawning.

# First Play (5-10 Minutes of FUN):
	Spawn in the forest
	Player goes to the village to explore *main building Adventurers Guild used to get dungeon quests*
	Register as adventurer
	Go to dungeon kill enemies, find treasure and useful items not just useless bullshit
	Items drops based on the area difficulty
	*Higher area difficulty = Better reward/ rare treasure & items drops*
	Each dungeon has a type like a pokemon lighting/grass/blood/fire, etc...
	Clear level and the final level for each dungeon has a boss based on dungeon type.
	Dungeon clear = defeat final boss/ dungeon stop spreading/corrupting world, monster spawn stops.
	
	*Player Goals*
	- Clear all dungeons
	- Get stronger
	- Get better gear
	- Final endgame Travel to Demon Continent and defeat Demon queen *or spare her for a secret*