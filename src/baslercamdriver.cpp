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
        //width.SetToMaximum();
        //height.SetToMaximum();
        height.SetValue(m_frameHeight);
        width.SetValue(m_frameWidth);
        std::cout << "Frame rate before: " << frameRate.GetValue() << std::endl;
        frameRate.SetValue(m_frameRate);
        std::cout << "Frame rate after: " << frameRate.GetValue() << std::endl;
        std::cout << "Frame rate resultant: " << resultingFrameRate.GetValue() << std::endl;
        // Set the pixel data format.
        Pylon::CEnumParameter(nodemap, "PixelFormat").SetValue("Mono8");
    }
    catch (const Pylon::GenericException& e)
    {
        throw RUNTIME_EXCEPTION( "Could not apply configuration. const GenericException caught in OnOpened method msg=%hs", e.what());
    }
}

/*
    Iterates over list of connected cameras and opens the camera for acquisition if camera 
    with same camera ID is found. 
*/
void createCameraBySerialNrAndGrab(std:: string serialNr, uint64_t frameWidth, uint64_t frameHeight, 
                                   uint64_t frameRate, DRAIVE::Link2::OutputPin outputPin)
{
    Pylon::CInstantCamera cam;
    Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();
    Pylon::PylonAutoInitTerm autoInitTerm;
    Pylon::CTlFactory& TlFactory = Pylon::CTlFactory::GetInstance();
    Pylon::DeviceInfoList_t lstDevices;
    TlFactory.EnumerateDevices( lstDevices );
    // This smart pointer will receive the grab result data.
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
                    Pylon::CInstantCamera camera(tlFactory.CreateDevice(*it));

                    camera.RegisterConfiguration(new BaslerCamConfigEvents(frameWidth,
                                                                           frameHeight,
                                                                           frameRate), 
                                                     Pylon::RegistrationMode_Append,
                                                     Pylon::Cleanup_Delete);

                    std::cout << "FullName " <<  camera.GetDeviceInfo().GetFullName() << std::endl;
                    std::cout << "FriendlyName " <<  camera.GetDeviceInfo().GetFriendlyName() << std::endl;
                    std::cout << "UserDefinedName  " <<  camera.GetDeviceInfo().GetUserDefinedName() << std::endl;
                    std::cout << "InternalName " <<  camera.GetDeviceInfo().GetInternalName() << std::endl;
                    std::cout << "ModelName  " <<  camera.GetDeviceInfo().GetModelName() << std::endl;
                    std::cout << "SerialNumber " <<  camera.GetDeviceInfo().GetSerialNumber() << std::endl;

                    std::cout << std::endl;
                    
                    camera.Open();

                    //camera.AcquisitionFrameRate.SetValue(500.0); // set frame rate
                    //camera.ExposureTime.SetValue(1500);
                    camera.MaxNumBuffer = 188;
                    //camera.Width.SetValue(300);

                    //double d = camera.ResultingFrameRate.GetValue();

                    camera.StartGrabbing(Pylon::GrabStrategy_OneByOne); //Opens the capture in continuous acquisition mode.

                    int results = 0;
                    while(camera.IsGrabbing())
                    {
                        // Check that grab results are waiting.
                        if(camera.GetGrabResultWaitObject().Wait(0))
                        {
                            std::cout << std::endl << "Grab results wait in the output queue." << std::endl << std::endl;
                        }
                        
                        int nBuffersInQueue = 0;
                        
                        // All triggered images are still waiting in the output queue
                        // and are now retrieved.
                        while(camera.RetrieveResult(0, ptrGrabResult, Pylon::TimeoutHandling_Return)) //Don't wait for timeout. We want good FPS. If it works, it works. 
                        {
                            nBuffersInQueue++;
                            // Image grabbed successfully?
                            if (ptrGrabResult->GrabSucceeded())
                            {
                                results++;
                                // Access the image data.
                                //std::cout << "SizeX: " << ptrGrabResult->GetWidth() << std::endl;
                                //std::cout << "SizeY: " << ptrGrabResult->GetHeight() << std::endl;
                                uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
                                cv::Size imageSize(frameWidth, frameHeight);
                                
                                cv::Mat frame(imageSize, CV_8UC1, pImageBuffer);
                                link_dev::ImageT currentImage 
                                                 = link_dev::Interfaces::ImageFromOpenCV(frame, 
                                                              link_dev::Format::Format_GRAY_U8);   
              
                                outputPin.push(currentImage, "BaslerCamImage");
                                //std::cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << std::endl << std::endl;
                            }
                            else
                            {
                                std::cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
                            }
                        }
                        if(nBuffersInQueue > 0)
                        {
                            std::cout << "Retrieved " << nBuffersInQueue << " grab results from output queue." << std::endl << std::endl;
                        }
                    }

                    std::cout << "Total results: " << results << std::endl;
                    //Stop the grabbing.
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
                                                             m_outputPin);

    while(m_signalHandler.receiveSignal() != LINK2_SIGNAL_INTERRUPT); 

    std::cout << "Ending the run() function." << std::endl;
    Pylon::PylonTerminate();
}
