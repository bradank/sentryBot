#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <unistd.h>

#include "testBot.h"

#define DEBUG true


unsigned short Bot::m_sInstanceCount = 0;
std::unordered_map<std::string, WindowWrapper> Bot::m_windowMap;

Bot::Bot()
{

    this->m_captureWindow.pWindow = nullptr;
    this->m_captureWindow.type = Platform::Invalid;
    this->initialize();
};

Bot::Bot(std::string winName)
{
    this->m_captureWindow.pWindow = nullptr;
    this->m_captureWindow.type = Platform::Invalid;
    this->initialize();
    this->setCaptureWindow(winName, true);
}

Bot::~Bot()
{
    if (m_captureWindow.pWindow != nullptr)
    {
        m_captureWindow.pWindow = nullptr;
        m_captureWindow.type = Platform::Invalid;
    }

    deinitialize();
}

void Bot::initialize()
{
    m_sInstanceCount++;

#ifdef _WIN32
    // TODO: Write implementation for Windows here
#else
    m_pDisplay = XOpenDisplay(nullptr);
#endif
}

void Bot::deinitialize()
{
    m_sInstanceCount--;

#ifdef _WIN32
    // TODO: Write implementation for Windows here
#else
    if (m_sInstanceCount == 0)
    {
        for (auto it : m_windowMap)
        {
            XFree(it.second.pWindow);
        }

        XCloseDisplay(m_pDisplay);
        m_pDisplay = nullptr;
    }
#endif
}

void Bot::updateWindowMap()
{
#ifdef _WIN32
    // TODO: Write implementation for Windows here
#else

    if (m_pDisplay != nullptr)
    {
        Window defaultWindow = XDefaultRootWindow(m_pDisplay);

        Atom listProperty = XInternAtom(m_pDisplay, "_NET_CLIENT_LIST", False), listType;
        int format;
        unsigned long itemCount, itemRemain;
        unsigned char* pList;

        int retVal = XGetWindowProperty(m_pDisplay, defaultWindow, listProperty,
                            0, 1024, False, XA_WINDOW, &listType, &format, &itemCount, &itemRemain, &pList);
        if (retVal == Success)
        {
            if (DEBUG) std::cout << "Found " << itemCount << " top-level windows!" << std::endl;

            Window* pWinList = reinterpret_cast<Window*>(pList);

            for (unsigned long i = 0; i < itemCount; i++)
            {
                Window currentWin = pWinList[i];
                XTextProperty nameProp;
                Status status = XGetWMName(m_pDisplay, currentWin, &nameProp);
                if (status != 0)
                {
                    Window* pWin = new Window;
                    *pWin = pWinList[i];
                    WindowWrapper wrapper =
                    {
                        .pWindow = pWin,
                        .type = Platform::Linux
                    };
                    m_windowMap.emplace(std::make_pair(std::string((char*)nameProp.value), wrapper));
                }
            }

            XFree(pList);
        }
        else
        {
            std::cerr << "XGetWindowProperty for \"_NET_CLIENT_LIST\" failed!" << std::endl;
        }

        if (DEBUG)
        {
            for (auto win : m_windowMap)
            {
                std::cout << std::left << std::setw(80) << win.first <<  " : " << std::right << win.second.pWindow << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "XOpenDisplay failed! m_pDisplay = " << m_pDisplay << std::endl;
    }
#endif
}

void Bot::setCaptureWindow(std::string winName, bool updateMap)
{

    if (updateMap)
    {
        updateWindowMap();
    }

    if (winName.empty())
    {
        m_captureWindow = m_windowMap.begin()->second;
    }
    else
    {
        for (auto it = m_windowMap.begin(); it != m_windowMap.end(); it++)
        {
            if (it->first.find(winName) != std::string::npos)
            {
                m_captureWindow = it->second;
                break;
            }
        }
    }
}

cv::Mat Bot::getWindowSnapshot()
{
    if (m_captureWindow.pWindow == nullptr)
    {
        return cv::Mat(100, 100, CV_8UC4);
    }

#ifdef _WIN32
    // TODO: Write implementation for Windows here
#else
    if (m_captureWindow.type != Platform::Linux)
    {
        std::cerr << "Mismatched type = " << (unsigned int)m_captureWindow.type << std::endl;
        return cv::Mat(100, 100, CV_8UC4);
    }

    Window window = *reinterpret_cast<Window*>(m_captureWindow.pWindow);
    XWindowAttributes attributes = {0};
    Status retVal = XGetWindowAttributes(m_pDisplay, window, &attributes);
    if (retVal != 0)
    {
        XImage* image = XGetImage(m_pDisplay, window, 0, 0, attributes.width, attributes.height, AllPlanes, ZPixmap);
        cv::Mat res = cv::Mat(attributes.height, attributes.width, CV_8UC4, image->data);
        XFree(image);
        return res;
    }
    else
    {
        std::cerr << "XGetWindowAttributes failed!" << std::endl;
        return cv::Mat(100, 100, CV_8UC4);
    }
#endif
}

int main(int argc, char* argv[])
{
    std::string target = "Desktop";
    unsigned short fps = 15;
    if (argc > 1)
    {
        target = argv[1];
    }
    Bot test(target);
    while(true)
    {
        cv::Mat image = test.getWindowSnapshot();
        cv::Mat resizedImage;
        cv::resize(image, resizedImage, {image.cols / 2, image.rows / 2});
        cv::imshow("testBot", resizedImage);
        int key = cv::waitKey(1000 / fps);
        if (key == 27) break;
    }
    cv::destroyAllWindows();
    return 0;
}