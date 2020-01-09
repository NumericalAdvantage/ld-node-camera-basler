/*
 * This file is part of project link.developers/ld-node-camera-basler.
 * It is copyrighted by the contributors recorded in the version control history of the file,
 * available from its original location https://gitlab.com/link.developers.beta/ld-node-camera-basler.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include <thread>
#include <opencv2/imgproc/imgproc.hpp>
#include <link_dev/Interfaces/OpenCvToImage.h>
#include "baslercamdriver.h"
#include "GenericMatrix3D_generated.h"
#include "Image_generated.h"

using namespace Basler_GigECameraParams;

void setUpCameraForAutoFunctions(Pylon::CBaslerGigEInstantCamera& camera, uint64_t frameWidth,
                                 int64_t frameHeight, uint64_t luminanceControl)
{
    if (IsWritable(camera.OffsetX))
    {
        camera.OffsetX.SetValue(camera.OffsetX.GetMin());
    }
    if (IsWritable(camera.OffsetY))
    {
        camera.OffsetY.SetValue(camera.OffsetY.GetMin());
    }

    camera.Width.SetValue(camera.Width.GetMax());
    camera.Height.SetValue(camera.Height.GetMax());

    // Set the Auto Function AOI for luminance statistics.
    // Currently, AutoFunctionAOISelector_AOI1 is predefined to gather
    // luminance statistics.
                    
    camera.AutoFunctionAOISelector.SetValue(AutoFunctionAOISelector_AOI1);
    camera.AutoFunctionAOIOffsetX.SetValue(camera.OffsetX.GetMin());
    camera.AutoFunctionAOIOffsetY.SetValue(camera.OffsetY.GetMin());
    camera.AutoFunctionAOIWidth.SetValue(camera.Width.GetMax());
    camera.AutoFunctionAOIHeight.SetValue(camera.Height.GetMax());
    
    // Set the target value for luminance control. The value is always expressed
    // as an 8 bit value regardless of the current pixel data output format,
    // i.e., 0 -> black, 255 -> white.
    camera.AutoTargetValue.SetValue(luminanceControl);

    return;
}

void AutoGainOnce(Pylon::CBaslerGigEInstantCamera& camera)
{
    // Check whether the gain auto function is available.
    if(!IsWritable(camera.GainAuto))
    {
        std::cout << "The camera does not support Gain Auto." << std::endl << std::endl;
        return;
    }
    std::cout << "Initial Gain = " << camera.GainRaw.GetValue() << std::endl;
    // Set the gain ranges for luminance control.
    camera.AutoGainRawLowerLimit.SetValue(camera.GainRaw.GetMin());
    camera.AutoGainRawUpperLimit.SetValue(camera.GainRaw.GetMax());

    // When the "once" mode of operation is selected,
    // the parameter values are automatically adjusted until the related image property
    // reaches the target value. After the automatic parameter value adjustment is complete, the auto
    // function will automatically be set to "off" and the new parameter value will be applied to the
    // subsequently grabbed images.
    camera.GainAuto.SetValue(GainAuto_Once);
    std::cout << "Final Gain = " << camera.GainRaw.GetValue() << std::endl << std::endl;
    return;
}

void AutoGainContinuous(Pylon::CBaslerGigEInstantCamera& camera)
{
    // Check whether the Gain Auto feature is available.
    if ( !IsWritable( camera.GainAuto))
    {
        std::cout << "The camera does not support Gain Auto." << std::endl;
        return;
    }

    // When "continuous" mode is selected, the parameter value is adjusted repeatedly while images are acquired.
    // Depending on the current frame rate, the automatic adjustments will usually be carried out for
    // every or every other image unless the camera's micro controller is kept busy by other tasks.
    // The repeated automatic adjustment will proceed until the "once" mode of operation is used or
    // until the auto function is set to "off", in which case the parameter value resulting from the latest
    // automatic adjustment will operate unless the value is manually adjusted.
    camera.GainAuto.SetValue(GainAuto_Continuous);
    return;
}

void AutoExposureContinuous(Pylon::CBaslerGigEInstantCamera& camera)
{
    // Check whether the Exposure Auto feature is available.
    if(!IsWritable(camera.ExposureAuto))
    {
        std::cout << "The camera does not support Exposure Auto." << std::endl;
        return;
    }

    camera.ExposureAuto.SetValue(ExposureAuto_Continuous);
    return;
}

void BaslerCamConfigEvents::OnOpened(Pylon::CInstantCamera& camera)
{
    try
    {
        // Allow all the names in the namespace GenApi to be used without qualification.
        // Get the camera control object.
        GenApi::INodeMap& nodemap = camera.GetNodeMap();
        // Get the parameters for setting the image area of interest (Image AOI).
        Pylon::CIntegerParameter width(nodemap, "Width");
        Pylon::CIntegerParameter height(nodemap, "Height");
        Pylon::CIntegerParameter offsetX(nodemap, "OffsetX");
        Pylon::CIntegerParameter offsetY(nodemap, "OffsetY");
        Pylon::CFloatParameter frameRate(nodemap, "AcquisitionFrameRateAbs");
        Pylon::CFloatParameter resultingFrameRate(nodemap, "ResultingFrameRateAbs");
        
        // Maximize the Image AOI.
        offsetX.TrySetToMinimum(); // Set to minimum if writable.
        offsetY.TrySetToMinimum(); // Set to minimum if writable.
        
        height.SetValue(m_frameHeight);
        width.SetValue(m_frameWidth);

        std::cout << "Frame rate before: " << frameRate.GetValue() << std::endl;
        frameRate.SetValue(m_frameRate);
        std::cout << "Frame rate after: " << frameRate.GetValue() << std::endl;
        std::cout << "Frame rate resultant: " << resultingFrameRate.GetValue() << std::endl;
    
        // Set the pixel data format.
        Pylon::CEnumParameter(nodemap, "PixelFormat").SetValue("Mono8");

        Pylon::CIntegerParameter(nodemap, "GevSCPSPacketSize").SetValue(BASLER_RECOMMENDED_PACKET_SIZE);

        /*
        // Select the Gain parameter
        Pylon::CEnumParameter(nodemap, "RemoveParameterLimitSelector").SetValue("Gain");
        // Remove the limits of the selected parameter
        Pylon::CBooleanParameter(nodemap, "RemoveParameterLimit").SetValue(true);  
        // Select the AutoTargetValue parameter
        Pylon::CEnumParameter(nodemap, "RemoveParameterLimitSelector").SetValue("AutoTargetvalue");
        // Remove the limits of the selected parameter
        Pylon::CBooleanParameter(nodemap, "RemoveParameterLimit").SetValue(true);  
        */
    }
    catch (const Pylon::GenericException& e)
    {
        throw RUNTIME_EXCEPTION( "Could not apply configuration. const GenericException caught in OnOpened method msg=%hs", e.what());
    }
    return;
}

void printCameraDetails(Pylon::CBaslerGigEInstantCamera& camera)
{
    std::cout << "FullName: " <<  camera.GetDeviceInfo().GetFullName() << std::endl;
    std::cout << "FriendlyName: " <<  camera.GetDeviceInfo().GetFriendlyName() << std::endl;
    std::cout << "UserDefinedName:  " <<  camera.GetDeviceInfo().GetUserDefinedName() << std::endl;
    std::cout << "InternalName: " <<  camera.GetDeviceInfo().GetInternalName() << std::endl;
    std::cout << "ModelName: " <<  camera.GetDeviceInfo().GetModelName() << std::endl;
    std::cout << "SerialNumber: " <<  camera.GetDeviceInfo().GetSerialNumber() << std::endl;
    std::cout << std::endl;
    return;
}

/*
    Iterates over list of connected cameras and opens the camera for acquisition if camera 
    with same camera ID is found. 
*/
void createCameraBySerialNrAndGrab(std:: string serialNr, uint64_t frameWidth, 
                                   uint64_t frameHeight, uint64_t frameRate, 
                                   uint64_t luminanceControl, bool autoExposure,
                                   bool autoGain, bool autoGainOnce,
                                   DRAIVE::Link2::OutputPin outputPin,
                                   Pylon::WaitObjects waitObjectsContainer)
{
    Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();
    Pylon::PylonAutoInitTerm autoInitTerm;
    Pylon::CTlFactory& TlFactory = Pylon::CTlFactory::GetInstance();
    Pylon::DeviceInfoList_t lstDevices;
    TlFactory.EnumerateDevices(lstDevices);
    Pylon::CGrabResultPtr ptrGrabResult;
    
    std::cout << "Number of devices found: " << lstDevices.size() << std::endl;

    if(!lstDevices.empty()) 
    {
        Pylon::DeviceInfoList_t::const_iterator it;
        for(it = lstDevices.begin(); it != lstDevices.end(); ++it )
        {
            std::stringstream ss;
            ss << it->GetSerialNumber();
            std::string name = ss.str();
            if(serialNr.compare(name) == 0)
            {
                try
                {        
                    //We found our camera. Create a camera device which we can 
                    //use to grab images and so forth.
                    Pylon::CBaslerGigEInstantCamera camera(tlFactory.CreateDevice(*it));
                    camera.RegisterConfiguration(new BaslerCamConfigEvents(frameWidth,
                                                                           frameHeight,
                                                                           frameRate,
                                                                           autoGain), 
                                                     Pylon::RegistrationMode_Append,
                                                     Pylon::Cleanup_Delete);

                    printCameraDetails(camera);
                    //Initialize wait objects. 
                    waitObjectsContainer.Add(camera.GetGrabResultWaitObject());

                    camera.Open();
                    camera.MaxNumBuffer = NUMBER_OF_BUFFERS_FOR_GRAB_ENGINE;
                    
                    if(autoExposure || autoGain || autoGainOnce)
                    {
                        setUpCameraForAutoFunctions(camera, frameWidth, frameHeight, luminanceControl);

                        if(autoGain)
                        {
                            AutoGainContinuous(camera);        
                        }
                        else if(autoGainOnce)
                        {
                            AutoGainOnce(camera);
                        }

                        if(autoExposure)
                        {
                            AutoExposureContinuous(camera);
                        }
                    }

                    camera.StartGrabbing(Pylon::GrabStrategy_OneByOne); 

                    bool terminate = false;
                    unsigned int index;

                    while(camera.IsGrabbing() && terminate == false)
                    {
                        if(!waitObjectsContainer.WaitForAny(0xFFFFFFFF, &index))
                        {
                            std::cout << "This should not happen. Check wait Objects." << std::endl;
                            break;
                        }

                        switch(index)
                        {
                            case 0:  // Received a termination request
                            {
                                terminate = true;
                                break;
                            }
                            case 1:  // A grabbed buffer is available
                            {
                                // All triggered images are still waiting in the output queue
                                // and will now be retrieved.
                                // Don't wait for timeout. We want good FPS. If it works, it works. 
                                // This smart pointer will receive the grab result data.
                                //camera.GetGrabResultWaitObject().Wait(0);
                                if(camera.RetrieveResult(0, ptrGrabResult, Pylon::TimeoutHandling_Return)) 
                                {
                                    if(ptrGrabResult->GrabSucceeded())
                                    {
                                        uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
                                        cv::Size imageSize(frameWidth, frameHeight);
                                        cv::Mat frame(imageSize, CV_8UC1, pImageBuffer);
                                        
                                        link_dev::ImageT currentImage =
                                        link_dev::Interfaces::ImageFromOpenCV(frame,
                                                                            link_dev::Format::Format_GRAY_U8);   
                                        outputPin.push(currentImage, "BaslerCamImage");
                                    }
                                    else
                                    {
                                        std::cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
                                    }
                                }
                                break;
                            }
                            default: 
                            {
                                break;
                            }
                        } 
                    }

                    ptrGrabResult.Release();
                    camera.StopGrabbing();
                }
                catch(const Pylon::GenericException &e)
                {
                    std::cerr << "An exception occurred." << std::endl
                    << e.GetDescription() << std::endl;
                }
            }
            else
            {
                std::cerr << "No devices found!" << std::endl;
                return;
            }
        }
    }
}


int BaslerCamDriver::run() 
{
    Pylon::PylonInitialize();
    
    std::thread cameraGrabber(createCameraBySerialNrAndGrab, m_cameraID,
                                                             m_frameWidth,
                                                             m_frameHeight,
                                                             m_frameRate,
                                                             m_luminanceControl,
                                                             m_autoExposure,
                                                             m_autoGain,
                                                             m_autoGainOnce,
                                                             m_outputPin,
                                                             m_waitObjectsContainer);

    while(m_signalHandler.receiveSignal() != LINK2_SIGNAL_INTERRUPT); 

    //Allow the cameraGrabber thread to finish.
    m_terminateWaitObj.Signal();

    cameraGrabber.join();

    std::cout << "Ending the run() function." << std::endl;
    Pylon::PylonTerminate();

    return 0;
}
