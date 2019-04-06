#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/gateway-lora-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/node-container.h"
#include "ns3/mobility-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/double.h"
#include "ns3/transactional-sender-helper.h"
#include "ns3/command-line.h"
#include "ns3/network-server-helper.h"
#include "ns3/forwarder-helper.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("OGscenario");

// Network settings

Time simulationTime = Hours (12);
int dataPacketSize = 42;
int partialSignaturePacketSize = 34;
int packetsPerTransaction = 5;
int signaturePacketsPerTransaction = 2;

int noOfOilfieldEndDevices = 30;
int noOfOilFieldGateways = 9;

int nDevices = noOfOilfieldEndDevices;
const int nGateways = noOfOilFieldGateways;

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.AddValue ("nDevices",
                "Number of end devices to include in the simulation",
                nDevices);
  cmd.AddValue ("simulationTime",
                "The time for which to simulate",
                simulationTime);

  cmd.Parse (argc, argv);

  // Set up logging
  LogComponentEnable ("OGscenario", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraChannel", LOG_LEVEL_INFO);
  // LogComponentEnable("LoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  //LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraInterferenceHelper", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraMac", LOG_LEVEL_ALL);
  //LogComponentEnable("EndDeviceLoraMac", LOG_LEVEL_ALL);
  //LogComponentEnable("GatewayLoraMac", LOG_LEVEL_ALL);
  // LogComponentEnable("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  // LogComponentEnable("LogicalLoraChannel", LOG_LEVEL_ALL);
  //LogComponentEnable("LoraHelper", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraPhyHelper", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraMacHelper", LOG_LEVEL_ALL);
  //LogComponentEnable("TransactionalSenderHelper", LOG_LEVEL_ALL);
  //LogComponentEnable("TransactionalSender", LOG_LEVEL_DEBUG);
  // LogComponentEnable("LoraMacHeader", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraFrameHeader", LOG_LEVEL_ALL);
  // LogComponentEnable("NetworkScheduler", LOG_LEVEL_ALL);
  // LogComponentEnable("NetworkStatus", LOG_LEVEL_ALL);
  // LogComponentEnable("NetworkController", LOG_LEVEL_ALL);
  //LogComponentEnable("LoraPacketTracker", LOG_LEVEL_ALL);
  //LogComponentEnable ("MobilityHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("NetworkServerHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("NetworkServer", LOG_LEVEL_ALL);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  /***********
   *  Setup  *
   ***********/

  // Mobility//
  /////////////

  // End Device mobility

  MobilityHelper mobilityOilFieldEd, mobilityOilFieldGw;
  Ptr<ListPositionAllocator> positionAllocOilfieldEd = CreateObject<ListPositionAllocator> ();

  // x/y-coordinates of oilfield EDs
  std::vector<Vector> oilFieldEDpositions;
  oilFieldEDpositions.push_back (Vector (18613, 4551, 0.0));
  oilFieldEDpositions.push_back (Vector (11659, 18073, 0.0));
  oilFieldEDpositions.push_back (Vector (19868, 8278, 0.0));
  oilFieldEDpositions.push_back (Vector (11588, 10670, 0.0));
  oilFieldEDpositions.push_back (Vector (800, 11127, 0.0));
  oilFieldEDpositions.push_back (Vector (12619, 4147, 0.0));
  oilFieldEDpositions.push_back (Vector (5466, 7336, 0.0));
  oilFieldEDpositions.push_back (Vector (1398, 15341, 0.0));
  oilFieldEDpositions.push_back (Vector (12571, 7702, 0.0));
  oilFieldEDpositions.push_back (Vector (11040, 14957, 0.0));
  oilFieldEDpositions.push_back (Vector (15868, 8975, 0.0));
  oilFieldEDpositions.push_back (Vector (8875, 9358, 0.0));
  oilFieldEDpositions.push_back (Vector (1223, 3746, 0.0));
  oilFieldEDpositions.push_back (Vector (12878, 16870, 0.0));
  oilFieldEDpositions.push_back (Vector (15346, 18604, 0.0));
  oilFieldEDpositions.push_back (Vector (14599, 6730, 0.0));
  oilFieldEDpositions.push_back (Vector (15206, 3815, 0.0));
  oilFieldEDpositions.push_back (Vector (2433, 10177, 0.0));
  oilFieldEDpositions.push_back (Vector (13165, 1393, 0.0));
  oilFieldEDpositions.push_back (Vector (18932, 8631, 0.0));
  oilFieldEDpositions.push_back (Vector (3853, 6245, 0.0));
  oilFieldEDpositions.push_back (Vector (8603, 2086, 0.0));
  oilFieldEDpositions.push_back (Vector (602, 8408, 0.0));
  oilFieldEDpositions.push_back (Vector (9416, 15295, 0.0));
  oilFieldEDpositions.push_back (Vector (4520, 18961, 0.0));
  oilFieldEDpositions.push_back (Vector (14174, 19862, 0.0));
  oilFieldEDpositions.push_back (Vector (748, 14159, 0.0));
  oilFieldEDpositions.push_back (Vector (2016, 16825, 0.0));
  oilFieldEDpositions.push_back (Vector (3450, 232, 0.0));
  oilFieldEDpositions.push_back (Vector (7293, 11223, 0.0));

  for (auto iteOilFieldEDpos = oilFieldEDpositions.begin ();
           iteOilFieldEDpos != oilFieldEDpositions.end();
           ++ iteOilFieldEDpos)
  {
    positionAllocOilfieldEd->Add (*iteOilFieldEDpos);
  }

  mobilityOilFieldEd.SetPositionAllocator (positionAllocOilfieldEd);
  mobilityOilFieldEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Gateway mobility

  // x/y-coordinates of oilfield GWs
  std::vector<Vector> oilFieldGWpositions;
  oilFieldGWpositions.push_back(Vector (10000, 10000, 0.0));
  oilFieldGWpositions.push_back(Vector (5000, 5000, 0.0));
  oilFieldGWpositions.push_back(Vector (15000, 15000, 0.0));
  oilFieldGWpositions.push_back(Vector (5000, 15000, 0.0));
  oilFieldGWpositions.push_back(Vector (15000, 5000, 0.0));
  oilFieldGWpositions.push_back(Vector (0, 20000, 0.0));
  oilFieldGWpositions.push_back(Vector (20000, 0, 0.0));
  oilFieldGWpositions.push_back(Vector (0, 0, 0.0));
  oilFieldGWpositions.push_back(Vector (20000, 20000, 0.0));

  Ptr<ListPositionAllocator> positionAllocOilfieldGw = CreateObject<ListPositionAllocator> ();

  for (auto iteOilFieldGWpos = oilFieldGWpositions.begin();
      iteOilFieldGWpos != oilFieldGWpositions.end(); ++iteOilFieldGWpos)
  {
    positionAllocOilfieldGw->Add (*iteOilFieldGWpos);
  }

  mobilityOilFieldGw.SetPositionAllocator (positionAllocOilfieldGw);
  mobilityOilFieldGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  /************************
   *  Create the channel  *
   ************************/

  // Create the lora channel object
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  /************************
   *  Create the helpers  *
   ************************/

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LoraMacHelper
  LoraMacHelper macHelper = LoraMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();

  /************************
   *  Create End Devices  *
   ************************/

  // Create a set of oilfield nodes
  NodeContainer oilfieldEndDevices;
  oilfieldEndDevices.Create (noOfOilfieldEndDevices);

  /*****************************************************
  *  Install applications on the oilfield end devices  *
  ******************************************************/

  TransactionalSenderHelper oilfieldAppHelper = TransactionalSenderHelper ();
  oilfieldAppHelper.SetDataPacketSize (dataPacketSize);
  oilfieldAppHelper.SetPartialSignaturePacketSize (partialSignaturePacketSize);
  oilfieldAppHelper.SetIntraTransactionDelay (Minutes (5));
  oilfieldAppHelper.SetInterTransactionDelay (Minutes (5));
  oilfieldAppHelper.SetPacketsPerTransaction (packetsPerTransaction);
  ApplicationContainer oilfieldAppContainer = oilfieldAppHelper.Install (oilfieldEndDevices);
  oilfieldAppContainer.Start (Seconds (0));
  oilfieldAppContainer.Stop (simulationTime);

  // Assign a mobility model to each oilfield node
  mobilityOilFieldEd.Install (oilfieldEndDevices);

  /*****************************************************
  *  Concatenation of all enddevice NodeContainers     *
  ******************************************************/
  NodeContainer endDevices;
  // endDevices.Create (0);
  endDevices.Add (oilfieldEndDevices);

  // Create the LoraNetDevices of the end devices
  uint8_t nwkId = 54;
  uint32_t nwkAddr = 1864;
  Ptr<LoraDeviceAddressGenerator> addrGen = CreateObject<LoraDeviceAddressGenerator> (nwkId,nwkAddr);

  // Create the LoraNetDevices of the end devices
  macHelper.SetAddressGenerator (addrGen);
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LoraMacHelper::ED);
  helper.Install (phyHelper, macHelper, endDevices);

  // Now end devices are connected to the channel

  // Connect trace sources
  for (NodeContainer::Iterator j = endDevices.Begin ();
       j != endDevices.End (); ++j)
    {
      Ptr<Node> node = *j;
      Ptr<LoraNetDevice> loraNetDevice = node->GetDevice (0)->GetObject<LoraNetDevice> ();
      Ptr<LoraPhy> phy = loraNetDevice->GetPhy ();
      // Set message type to CONFIRMED_DATA_UP (ACK) for all end devices
      Ptr<EndDeviceLoraMac> mac = loraNetDevice->GetMac ()->GetObject<EndDeviceLoraMac>();
      mac->SetMType (LoraMacHeader::CONFIRMED_DATA_UP);
  }

  /*********************
   *  Create Gateways  *
   *********************/

  // Create a set of oilfield gateways
  NodeContainer oilfieldGateways;
  oilfieldGateways.Create (noOfOilFieldGateways);

  // Assign a mobility model to each oilfield node
  mobilityOilFieldGw.Install (oilfieldGateways);

  /*****************************************************
  *  Concatenation of all gateway NodeContainers     *
  ******************************************************/
  NodeContainer gateways;
  // gateways.Create (0);
  gateways.Add (oilfieldGateways);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LoraMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  /**********************************************
   *  Set up the end device's spreading factor  *
   **********************************************/
  macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);

  NS_LOG_DEBUG ("Completed configuration");

  // Install the SimpleNetworkServer application on the network server
  NodeContainer networkServers;
  networkServers.Create (1);
  NetworkServerHelper networkServerHelper;
  networkServerHelper.EnableStatsCollection ();
  networkServerHelper.EnableTransactionMode ();
  networkServerHelper.SetSimulationTime (simulationTime);
  networkServerHelper.SetFileName ("OGscenario.csv");
  networkServerHelper.SetCsvStaticDef ("def_n-3,def_n-2,");
  std::stringstream streamDef;
  std::stringstream streamData;
  streamDef << "NEndDevices,NGateways,SimulationTime," <<
               "PacketsPerTransaction,SignaturePacketsPerTransaction," <<
               "DataPacketSize,PartialSignaturePacketSize,";
  streamData << nDevices << "," << nGateways << "," <<
                simulationTime.GetSeconds () << "," <<
                packetsPerTransaction << "," <<
                signaturePacketsPerTransaction << "," <<
                dataPacketSize << "," <<
                partialSignaturePacketSize << ",";
  networkServerHelper.SetCsvStaticDef(streamDef.str ());
  networkServerHelper.SetCsvStaticData(streamData.str ());
  networkServerHelper.SetNumberOfPacketsPerTransaction (packetsPerTransaction
                                           + signaturePacketsPerTransaction);
  networkServerHelper.SetGateways (gateways);
  networkServerHelper.SetEndDevices (endDevices);
  networkServerHelper.Install (networkServers);
  // Install the Forwarder application on the gateways
  ForwarderHelper forwarderHelper;
  forwarderHelper.Install (gateways);

  /****************
   *  Simulation  *
   ****************/

  Simulator::Stop (simulationTime + Hours (1));

  NS_LOG_INFO ("Running simulation...");
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
