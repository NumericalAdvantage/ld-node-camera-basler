/*
 * This file is part of project link.developers/ld-node-camera-basler.
 * It is copyrighted by the contributors recorded in the version control history of the file,
 * available from its original location https://gitlab.com/link.developers.beta/ld-node-camera-basler.
 *
 * SPDX-License-Identifier: MPL-2.0
 */
#ifndef BASLERCAMDRIVER_HPP
#define BASLERCAMDRIVER_HPP

#include <ostream>
#include <sstream>
#include <string>

#include <pylon/PylonIncludes.h>
#include <pylon/DeviceInfo.h>
#include <pylon/TlFactory.h>
#include <pylon/DeviceFactory.h> 
//#include <pylon/InstantCamera.h>
#include <pylon/ConfigurationEventHandler.h>

#include <DRAIVE/Link2/NodeDiscovery.hpp>
#include <DRAIVE/Link2/NodeResources.hpp>
#include <DRAIVE/Link2/SignalHandler.hpp>
#include <DRAIVE/Link2/ConfigurationNode.hpp>
#include <DRAIVE/Link2/OutputPin.hpp>

#define DEFAULT_FRAME_WIDTH 640
#define DEFAULT_FRAME_HEIGHT 480
#define DEFAULT_FRAME_RATE 24
#define DEFAULT_LUMINANCE_CONTROL 100
#define NUMBER_OF_BUFFERS_FOR_GRAB_ENGINE 50

class BaslerCamDriver : public Pylon::CConfigurationEventHandler
{

    DRAIVE::Link2::SignalHandler m_signalHandler;
    DRAIVE::Link2::NodeResources m_nodeResources;
    DRAIVE::Link2::NodeDiscovery m_nodeDiscovery;
    DRAIVE::Link2::OutputPin m_outputPin;

public:
    std::string m_cameraID = "";
    uint64_t m_frameWidth = DEFAULT_FRAME_WIDTH;
    uint64_t m_frameHeight = DEFAULT_FRAME_HEIGHT;
    uint64_t m_frameRate = DEFAULT_FRAME_RATE;
    uint64_t m_luminanceControl = DEFAULT_LUMINANCE_CONTROL;
    bool m_autoGainOnce = true;
    bool m_autoExposure = true;
    bool m_autoGain = false;

    BaslerCamDriver(DRAIVE::Link2::SignalHandler signalHandler,
                    DRAIVE::Link2::NodeResources nodeResources,
                    DRAIVE::Link2::NodeDiscovery nodeDiscovery,
                    DRAIVE::Link2::OutputPin outputPin,
                    std::string cameraID,
                    uint64_t frameWidth, 
                    uint64_t frameHeight,
                    uint64_t frameRate,
                    uint64_t luminanceControl,
                    bool autoGainOnce,
                    bool autoExposure,
                    bool autoGain) :
                    m_signalHandler(signalHandler),
                    m_nodeResources(nodeResources),
                    m_nodeDiscovery(nodeDiscovery),
                    m_outputPin(outputPin),
                    m_cameraID(cameraID),
                    m_frameWidth(frameWidth),
                    m_frameHeight(frameHeight),
                    m_frameRate(frameRate),
                    m_luminanceControl(luminanceControl),
                    m_autoGainOnce(autoGainOnce),
                    m_autoExposure(autoExposure),
                    m_autoGain(autoGain)
    {
    }

    int run();

};


class BaslerCamConfigEvents : public Pylon::CConfigurationEventHandler
{
public:
    uint64_t m_frameWidth, m_frameHeight, m_frameRate, m_percentageGain;
    bool m_autoGain = false;
    void OnOpened(Pylon::CInstantCamera& camera);
    BaslerCamConfigEvents(uint64_t frameWidth, 
                          uint64_t frameHeight,
                          uint64_t frameRate,
                          bool autoGain) :
                          m_frameWidth(frameWidth),
                          m_frameHeight(frameHeight),
                          m_frameRate(frameRate),
                          m_autoGain(autoGain)
    {
    }
};


#endif
