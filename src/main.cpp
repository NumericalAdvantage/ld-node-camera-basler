/*
 * This file is part of project link.developers/ld-node-camera-basler.
 * It is copyrighted by the contributors recorded in the version control history of the file,
 * available from its original location https://gitlab.com/link.developers.beta/ld-node-camera-basler.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include <iostream>
#include "baslercamdriver.h"

int main(int argc, char** argv)
{
    try {
        DRAIVE::Link2::NodeResources nodeResources { "l2spec:/link_dev/ld-node-camera-basler", argc, argv };
        DRAIVE::Link2::NodeDiscovery nodeDiscovery { nodeResources };
        
        DRAIVE::Link2::ConfigurationNode rootNode = nodeResources.getUserConfiguration();
        DRAIVE::Link2::OutputPin outputPin{nodeDiscovery, nodeResources, "basler-cam-output-pin"};
       
        DRAIVE::Link2::SignalHandler signalHandler {};
        signalHandler.setReceiveSignalTimeout(-1);

        BaslerCamDriver baslercamdriver{signalHandler,
                                        nodeResources,
                                        nodeDiscovery,
                                        outputPin,
                                        rootNode.getString("CameraID"),
                                        rootNode.getUInt("ImageWidth"),
                                        rootNode.getUInt("ImageHeight"),
                                        rootNode.getUInt("FrameRate"),
                                        rootNode.getUInt("LuminanceControl"),
                                        rootNode.getBoolean("AutoGainOnce"),
                                        rootNode.getBoolean("AutoExposureContinuous"),
                                        rootNode.getBoolean("AutoGainContinuous")};

        baslercamdriver.run();     //Runs until interrupt signal is sent to the program.
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}


/*
    References:
    1. Documentation that this shipped with the proprietary Pylon SDK.
    2. Heavy use of the code samples that also come with the SDK has been made in this project. 
    3. https://www.optophysiology.uni-freiburg.de/Howto/howto_high_framerate_recordings
*/
