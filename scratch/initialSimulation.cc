/*
 * This example creates a simple network in which all LoRaWAN components are
 * simulated: End Devices, some Gateways and a Network Server.
 * Two end devices are already configured to send unconfirmed and confirmed messages respectively.
 */

#include "ns3/point-to-point-module.h"
#include "ns3/forwarder-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/lora-channel.h"
#include "ns3/mobility-helper.h"
#include "ns3/lora-phy-helper.h"
#include "ns3/lora-mac-helper.h"
#include "ns3/lora-helper.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/periodic-sender.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lora-device-address-generator.h"
#include "ns3/one-shot-sender-helper.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("NetworkServerExample");

int main (int argc, char *argv[])
{

  bool verbose = false;

  CommandLine cmd;
  cmd.AddValue ("verbose", "Whether to print output or not", verbose);
  cmd.Parse (argc, argv);

  // Logging
  //////////

  LogComponentEnable ("NetworkServerExample", LOG_LEVEL_ALL);
  LogComponentEnable ("SimpleNetworkServer", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraFrameHeader", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraMacHeader", LOG_LEVEL_ALL);
  // LogComponentEnable("MacCommand", LOG_LEVEL_ALL);
  // LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraChannel", LOG_LEVEL_ALL);
  // LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLoraMac", LOG_LEVEL_ALL);
  // LogComponentEnable ("OneShotSender", LOG_LEVEL_ALL);
  // LogComponentEnable("PointToPointNetDevice", LOG_LEVEL_ALL);
  // LogComponentEnable ("Forwarder", LOG_LEVEL_ALL);
  // LogComponentEnable ("OneShotSender", LOG_LEVEL_ALL);
  // LogComponentEnable ("DeviceStatus", LOG_LEVEL_ALL);
  // LogComponentEnable ("GatewayStatus", LOG_LEVEL_ALL);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  // Create a simple wireless channel
  ///////////////////////////////////

  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  // Helpers
  //////////

  // End Device mobility
  // x/y-coordinates according to running nodeExtractor.py on 2M transmissions
  MobilityHelper mobilityEd, mobilityGw;
  Ptr<ListPositionAllocator> positionAllocEd = CreateObject<ListPositionAllocator> ();
  positionAllocEd->Add (Vector (9782.655684816418, 16407.83287118317, 0.0));
  positionAllocEd->Add (Vector (761.9884992111474, 12798.042369708477, 0.0));
  positionAllocEd->Add (Vector (6515.527137617231, 13605.91745874213, 0.0));
  positionAllocEd->Add (Vector (7930.596178661217, 11784.030245990027, 0.0));
  positionAllocEd->Add (Vector (9795.627437463845, 10606.133177199517, 0.0));
  positionAllocEd->Add (Vector (2547.116770894616, 6258.487048110168, 0.0));

  mobilityEd.SetPositionAllocator (positionAllocEd);
  mobilityEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Gateway mobility
  // x/y-coordinates according to result of gatewayExtractor.py
  Ptr<ListPositionAllocator> positionAllocGw = CreateObject<ListPositionAllocator> ();
  positionAllocGw->Add (Vector (2553.2568212926853, 6247.463313310262, 0.0));
  positionAllocGw->Add (Vector (6013.332195830182, 12224.117915571522, 0.0));
  positionAllocGw->Add (Vector (16102.710063695908, 13928.383597958076, 0.0));

  mobilityGw.SetPositionAllocator (positionAllocGw);
  mobilityGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LoraMacHelper
  LoraMacHelper macHelper = LoraMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();

  // Create EDs
  /////////////

  const uint32_t NO_OF_END_DEVICES = 6;

  NodeContainer endDevices;
  endDevices.Create (NO_OF_END_DEVICES);
  mobilityEd.Install (endDevices);

  // Create a LoraDeviceAddressGenerator
  uint8_t nwkId = 54;
  uint32_t nwkAddr = 1864;
  Ptr<LoraDeviceAddressGenerator> addrGen = CreateObject<LoraDeviceAddressGenerator> (nwkId,nwkAddr);

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LoraMacHelper::ED);
  macHelper.SetAddressGenerator (addrGen);
  macHelper.SetRegion (LoraMacHelper::EU);
  helper.Install (phyHelper, macHelper, endDevices);

  // Set message type (Default is unconfirmed)
  //Ptr<LoraMac> edMac1 = endDevices.Get (1)->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac ();
  //Ptr<EndDeviceLoraMac> edLoraMac1 = edMac1->GetObject<EndDeviceLoraMac> ();
  //edLoraMac1->SetMType (LoraMacHeader::CONFIRMED_DATA_UP);

  // Set message type to CONFIRMED_DATA_UP for all end devices
  for(uint32_t i = 0; i < NO_OF_END_DEVICES; i++) {

      Ptr<LoraMac> edMacTemp = endDevices.Get (i)->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac ();
      Ptr<EndDeviceLoraMac> edLoraMacTemp = edMacTemp->GetObject<EndDeviceLoraMac> ();
      edLoraMacTemp->SetMType (LoraMacHeader::CONFIRMED_DATA_UP);
      NS_LOG_UNCOND(i);

  }

  // Install applications in EDs
  OneShotSenderHelper oneShotHelper = OneShotSenderHelper ();
  oneShotHelper.SetSendTime (Seconds (2));
  oneShotHelper.Install (endDevices.Get (0));

  oneShotHelper.SetSendTime (Seconds (50));
  oneShotHelper.Install (endDevices.Get (1));

  oneShotHelper.SetSendTime (Seconds (300));
  oneShotHelper.Install (endDevices.Get (2));

  oneShotHelper.SetSendTime (Seconds (450));
  oneShotHelper.Install (endDevices.Get (3));

  oneShotHelper.SetSendTime (Seconds (670));
  oneShotHelper.Install (endDevices.Get (4));

  oneShotHelper.SetSendTime (Seconds (750));
  oneShotHelper.Install (endDevices.Get (5));


  ////////////////
  // Create GWs //
  ////////////////

  NodeContainer gateways;
  gateways.Create (3);
  mobilityGw.Install (gateways);

  // Create the LoraNetDevices of the gateways
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LoraMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  // Set spreading factors up
  macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);

  ////////////
  // Create NS
  ////////////

  NodeContainer networkServers;
  networkServers.Create (1);

  // Install the SimpleNetworkServer application on the network server
  NetworkServerHelper networkServerHelper;
  networkServerHelper.SetGateways (gateways);
  networkServerHelper.SetEndDevices (endDevices);
  networkServerHelper.Install (networkServers);

  // Install the Forwarder application on the gateways
  ForwarderHelper forwarderHelper;
  forwarderHelper.Install (gateways);

  // Start simulation
  Simulator::Stop (Seconds (800));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
