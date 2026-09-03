### TODO:
[ ] Figure out window internal render resolution upscaling i.e. 640x360 (16:9) -> 1280x720(16:9)
    - NOTE: snap the scale factor to the nearest integer (×2, ×3, ×4...) when computing draw_w/draw_h in that blit pass
[ ] On window Resize release the old RenderTargetView in the Renderer struct and call reisze buffer and
    recreate the RenderTargetView
[ ] Handle sRGB and Linear color format in the sampler and shader i.e. when loading sRGB files make sure to specify
    the file uses sRGB color encoding by declaring texture with `DXGI_FORMAT_R8G8B8A8_UNORM_SRGB` and the GPU will do
    the sRGB -> Linear conversion itself
[ ] Create a Arena Allocation system
    - watch handmade hero and also check randy's resources
[ ] Create Input Manager and Action map
    - yoink the odin game code i made

### Done:
[x] Setup a Basic clean Cross-platform Api Layer
[x] Open a win32 window
[x] Figure out how to get D3D11_CREATE_DEVICE_DEBUG feature working
    - Just had to install Direct3D 12 debug layers (system component) from here:
    `https://learn.microsoft.com/en-us/windows/ai/directml/dml-debug-layer#installing-the-directml-and-direct3d-12-debug-layers-system-component`
[x] Draw a triangle
