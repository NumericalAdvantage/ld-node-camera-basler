# ld-node-camera-basler

[![build status](https://gitlab.com/link.developers.beta/ld-node-camera-basler/badges/master/build.svg)](https://gitlab.com/link.developers.beta/ld-node-camera-basler/commits/master)

## Motivation and Context

- This node is desiged to be an interface between Basler GigE cameras and the link2 mesh. This node allows for configuration of the cameras and then the user is able to stream images captured from the camera (with desired framerate, image dimensions, other camera settings like autexposure) to the mesh.
- Internally, this node uses the Pylon SDK provided by the manufacturer to "talk" to the cameras. Then it records images from the camera and writes the same thing on the mesh in "Image" format. This "Image" is the "offer" of this node and the format is defined in [ld-lib-image](https://gitlab.com/link.developers/ld-lib-image).

## Hardware set-up
- The camera used to develop this node was a BASLER acA1300-75gc. The camera can be powered over ethernet or via a hirose connector. In this particular setup I have used the PoE solution. To do this, a D-Link DGS-1008P Gigabit POE network switch was used.  
- Tools called "IpConfigurator" and "PylonViewer" are crucial in setting up the SDK and in troubleshooting. They are part of the SDK download which is available from [here](https://www.baslerweb.com/en/sales-support/downloads/software-downloads/).

## Tips for improving performance
- Adjust the MTU in the node and the host system. You should set the [MTU](https://www.wikiwand.com/en/Maximum_transmission_unit) of your host system to [9012](https://docs.baslerweb.com/network-related-parameters-(gige-cameras).html#packet-size) for best results. Note that Basler actually recommends a value for `9014`, but we found that if that value is used, the camera doesn't accept it and produces the following error :  `The difference between Value = 9014 and Min = 220 must be dividable without rest by Inc = 4`. Hence the value recommended is `9012`.
- If you start the node and don't see any images or the data rate what you expect is not what you observe, it ma well be the case that you need to increase your MTU.
- Note that the default value for this parameter is 1500 bytes, which may be low for higher framerates. 

## The node in action
- When used with ld-node-image-2d-viewer-2:

  ![The node in action](assets/basler_cam_demo.mp4)

## Installation

```
conda install ld-node-camera-basler
```

## Usage example

```
ld-node-camera-basler --instance-file instance.json
```

## Specification

This node implements the following RFCs:
- http://draive.com/link_dev/rfc/rfc048/ld-node-camera-basler

## Contribution

Your help is very much appreciated. For more information, please see our [contribution guide](./CONTRIBUTING.md) and the [Collective Code Construction Contract](https://gitlab.com/link.developers/RFC/blob/master/001/README.md) (C4).

## Maintainers

- Ritwik Ghosh (maintainer, original author)
