### TODO:
[ ] Create Input Manager and Action map
    - yoink the odin game code i made
[ ] Setup Audio Layer (use XAudio2 built into windows 11 SDK, low to mid level API)
	- NOTE: Audio API used by Handmade hero i.e. DirectSound is long depricated Deprecated since
	the era of DirectX 8/Vista.
[ ] snap the scale factor to the nearest integer (×2, ×3, ×4...) when computing draw_w/draw_h in that blit pass
[ ] On window Resize release the old RenderTargetView in the Renderer struct and call reisze buffer and
    recreate the RenderTargetView
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
[x] Figure out window internal render resolution upscaling i.e. 640x360 (16:9) -> 1280x720(16:9)
[x] write main.cpp file as main entry point
[x] Create the logger with assert
[x] Create a Arena Allocation system
    - watch handmade hero and also check randy's resources
