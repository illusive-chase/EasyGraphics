#ifndef LINUX_RENDER_HH_
#define LINUX_RENDER_HH_

struct RenderImpl {
    static int Width();
	static int Height();
	static uint8_t* Data();
	static void Render();
    static int* MouseClick();
    static int* MouseMove();
};

template<typename T>
void Register() {
    T::RegisterRender(RenderImpl::Render);
    T::RegisterData(RenderImpl::Data);
    int width = RenderImpl::Width();
    int height = RenderImpl::Height();
    T::RegisterSize(width, height);
    T::RegisterMouseClick(RenderImpl::MouseClick);
    T::RegisterMouseMove(RenderImpl::MouseMove);
}


#endif