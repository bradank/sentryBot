#include <unordered_map>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

#ifdef _WIN32
#else
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#endif

enum class Platform
{
    Linux,
    Windows,
    Invalid
};

struct WindowWrapper
{
    void* pWindow;
    Platform type;
};

class Bot
{
public:
    Bot();
    Bot(std::string winName);
    ~Bot();

    void setCaptureWindow(std::string winName, bool updateMap = false);
    const WindowWrapper getCaptureWindow() const { return m_captureWindow; }

    cv::Mat getWindowSnapshot();

private:
    void initialize();
    void deinitialize();

    void updateWindowMap();

    static unsigned short m_sInstanceCount;
    static std::unordered_map<std::string, WindowWrapper> m_windowMap;
    WindowWrapper m_captureWindow;


#ifdef _WIN32   // Windows specific variables

#else           // Linux specific variables
    Display* m_pDisplay;
#endif
};