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

int noOfPipelineEndDevices = 21;
int noOfPipelineGateways = 10;

int noOfShipEndDevices = 5;
int noOfShipGateways = 1;

int noOfTankEndDevices = 3;
int noOfTankGateways = 1;

int nDevices = noOfOilfieldEndDevices + noOfPipelineEndDevices +
                noOfShipEndDevices + noOfTankEndDevices;
const int nGateways = noOfOilFieldGateways + noOfPipelineGateways +
                        noOfShipGateways + noOfTankGateways;

int main (int argc, char *argv[])
{

  CommandLine cmd;
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



  // Oilfield mobility

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



  // Pipeline mobility

  MobilityHelper mobilityPipelineEd, mobilityPipelineGw;
  Ptr<ListPositionAllocator> positionAllocPipelineEd = CreateObject<ListPositionAllocator> ();

  // x/y-coordinates of pipeline EDs
  std::vector<Vector> pipelineEDpositions;
  pipelineEDpositions.push_back (Vector (20000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (25000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (30000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (35000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (40000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (45000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (50000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (55000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (60000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (65000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (70000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (75000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (80000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (85000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (90000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (95000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (100000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (105000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (110000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (115000.0, 10000, 0.0));
  pipelineEDpositions.push_back (Vector (120000.0, 10000, 0.0));

  for (auto itePipelineEDpos = pipelineEDpositions.begin ();
           itePipelineEDpos != pipelineEDpositions.end();
           ++ itePipelineEDpos)
  {
    positionAllocPipelineEd->Add (*itePipelineEDpos);
  }

  mobilityPipelineEd.SetPositionAllocator (positionAllocPipelineEd);
  mobilityPipelineEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");


  // x/y-coordinates of pipeline GWs
  std::vector<Vector> pipelineGWpositions;
  pipelineGWpositions.push_back (Vector (25000.0, 10100, 0.0));
  pipelineGWpositions.push_back (Vector (35000.0, 9900, 0.0));
  pipelineGWpositions.push_back (Vector (45000.0, 10100, 0.0));
  pipelineGWpositions.push_back (Vector (55000.0, 9900, 0.0));
  pipelineGWpositions.push_back (Vector (65000.0, 10100, 0.0));
  pipelineGWpositions.push_back (Vector (75000.0, 9900, 0.0));
  pipelineGWpositions.push_back (Vector (85000.0, 10100, 0.0));
  pipelineGWpositions.push_back (Vector (95000.0, 9900, 0.0));
  pipelineGWpositions.push_back (Vector (105000.0, 10100, 0.0));
  pipelineGWpositions.push_back (Vector (115000.0, 9900, 0.0));

  Ptr<ListPositionAllocator> positionAllocPipelineGw = CreateObject<ListPositionAllocator> ();

  for (auto itePipelineGWpos = pipelineGWpositions.begin();
      itePipelineGWpos != pipelineGWpositions.end(); ++itePipelineGWpos)
  {
    positionAllocPipelineGw->Add (*itePipelineGWpos);
  }

  mobilityPipelineGw.SetPositionAllocator (positionAllocPipelineGw);
  mobilityPipelineGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");



  // Ship mobility

  MobilityHelper mobilityShipEd, mobilityShipGw;
  Ptr<ListPositionAllocator> positionAllocShipEd = CreateObject<ListPositionAllocator> ();

  // x/y-coordinates of ship EDs
  std::vector<Vector> shipEDpositions;
  shipEDpositions.push_back (Vector (69905, 70042, 0.0));
  shipEDpositions.push_back (Vector (70017, 70062, 0.0));
  shipEDpositions.push_back (Vector (70052, 70031, 0.0));
  shipEDpositions.push_back (Vector (70086, 69913, 0.0));
  shipEDpositions.push_back (Vector (69959, 69952, 0.0));

  for (auto iteShipEDpos = shipEDpositions.begin ();
           iteShipEDpos != shipEDpositions.end();
           ++ iteShipEDpos)
  {
    positionAllocShipEd->Add (*iteShipEDpos);
  }

  mobilityShipEd.SetPositionAllocator (positionAllocShipEd);
  mobilityShipEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");


  // x/y-coordinates of ship GWs
  std::vector<Vector> shipGWpositions;
  shipGWpositions.push_back (Vector (70000, 70000, 0.0));


  Ptr<ListPositionAllocator> positionAllocShipGw = CreateObject<ListPositionAllocator> ();

  for (auto iteShipGWpos = shipGWpositions.begin();
      iteShipGWpos != shipGWpositions.end(); ++iteShipGWpos)
  {
    positionAllocShipGw->Add (*iteShipGWpos);
  }

  mobilityShipGw.SetPositionAllocator (positionAllocShipGw);
  mobilityShipGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");



  // Tank mobility

  MobilityHelper mobilityTankEd, mobilityTankGw;
  Ptr<ListPositionAllocator> positionAllocTankEd = CreateObject<ListPositionAllocator> ();

  // x/y-coordinates of tank EDs
  std::vector<Vector> tankEDpositions;
  tankEDpositions.push_back (Vector (39950, 99950, 0.0));
  tankEDpositions.push_back (Vector (40050, 99950, 0.0));
  tankEDpositions.push_back (Vector (40000, 100050, 0.0));


  for (auto iteTankEDpos = tankEDpositions.begin ();
           iteTankEDpos != tankEDpositions.end();
           ++ iteTankEDpos)
  {
    positionAllocTankEd->Add (*iteTankEDpos);
  }

  mobilityTankEd.SetPositionAllocator (positionAllocTankEd);
  mobilityTankEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");


  // x/y-coordinates of tank GWs
  std::vector<Vector> tankGWpositions;
  tankGWpositions.push_back (Vector (40000, 100000, 0.0));


  Ptr<ListPositionAllocator> positionAllocTankGw = CreateObject<ListPositionAllocator> ();

  for (auto iteTankGWpos = tankGWpositions.begin();
      iteTankGWpos != tankGWpositions.end(); ++iteTankGWpos)
  {
    positionAllocTankGw->Add (*iteTankGWpos);
  }

  mobilityTankGw.SetPositionAllocator (positionAllocTankGw);
  mobilityTankGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");


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

  // Create a set of pipeline nodes
  NodeContainer pipelineEndDevices;
  pipelineEndDevices.Create (noOfPipelineEndDevices);

  // Create a set of ship nodes
  NodeContainer shipEndDevices;
  shipEndDevices.Create (noOfShipEndDevices);

  // Create a set of tank nodes
  NodeContainer tankEndDevices;
  tankEndDevices.Create (noOfTankEndDevices);

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

  /*****************************************************
  *  Install applications on the pipeline end devices  *
  ******************************************************/

  TransactionalSenderHelper pipelineAppHelper = TransactionalSenderHelper ();
  pipelineAppHelper.SetDataPacketSize (dataPacketSize);
  pipelineAppHelper.SetPartialSignaturePacketSize (partialSignaturePacketSize);
  pipelineAppHelper.SetIntraTransactionDelay (Minutes (1.2));
  pipelineAppHelper.SetInterTransactionDelay (Minutes (1.2));
  pipelineAppHelper.SetPacketsPerTransaction (packetsPerTransaction);
  ApplicationContainer pipelineAppContainer = pipelineAppHelper.Install (pipelineEndDevices);
  pipelineAppContainer.Start (Seconds (0));
  pipelineAppContainer.Stop (simulationTime);

  /*****************************************************
  *  Install applications on the ship end devices  *
  ******************************************************/

  TransactionalSenderHelper shipAppHelper = TransactionalSenderHelper ();
  shipAppHelper.SetDataPacketSize (dataPacketSize);
  shipAppHelper.SetPartialSignaturePacketSize (partialSignaturePacketSize);
  shipAppHelper.SetIntraTransactionDelay (Minutes (0.4));
  shipAppHelper.SetInterTransactionDelay (Minutes (0.4));
  shipAppHelper.SetPacketsPerTransaction (packetsPerTransaction);
  ApplicationContainer shipAppContainer = shipAppHelper.Install (shipEndDevices);
  shipAppContainer.Start (Seconds (0));
  shipAppContainer.Stop (simulationTime);

  /*****************************************************
  *  Install applications on the tank end devices  *
  ******************************************************/

  TransactionalSenderHelper tankAppHelper = TransactionalSenderHelper ();
  tankAppHelper.SetDataPacketSize (dataPacketSize);
  tankAppHelper.SetPartialSignaturePacketSize (partialSignaturePacketSize);
  tankAppHelper.SetIntraTransactionDelay (Hours (0.25));
  tankAppHelper.SetInterTransactionDelay (Hours (0.25));
  tankAppHelper.SetPacketsPerTransaction (packetsPerTransaction);
  ApplicationContainer tankAppContainer = tankAppHelper.Install (tankEndDevices);
  tankAppContainer.Start (Seconds (0));
  tankAppContainer.Stop (simulationTime);

  // Assign a mobility model to each oilfield node
  mobilityOilFieldEd.Install (oilfieldEndDevices);

  // Assign a mobility model to each pipeline node
  mobilityPipelineEd.Install (pipelineEndDevices);

  // Assign a mobility model to each ship node
  mobilityShipEd.Install (shipEndDevices);

  // Assign a mobility model to each tank node
  mobilityTankEd.Install (tankEndDevices);

  /*****************************************************
  *  Concatenation of all enddevice NodeContainers     *
  ******************************************************/
  NodeContainer endDevices;
  endDevices.Add (oilfieldEndDevices);
  endDevices.Add (pipelineEndDevices);
  endDevices.Add (shipEndDevices);
  endDevices.Add (tankEndDevices);

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

  // Create a set of pipeline gateways
  NodeContainer pipelineGateways;
  pipelineGateways.Create (noOfPipelineGateways);

  // Create a set of ship gateways
  NodeContainer shipGateways;
  shipGateways.Create (noOfShipGateways);

  // Create a set of tank gateways
  NodeContainer tankGateways;
  tankGateways.Create (noOfTankGateways);

  // Assign a mobility model to each oilfield gateway
  mobilityOilFieldGw.Install (oilfieldGateways);

  // Assign a mobility model to each pipeline gateway
  mobilityPipelineGw.Install (pipelineGateways);

  // Assign a mobility model to each ship gateway
  mobilityShipGw.Install (shipGateways);

  // Assign a mobility model to each tank gateway
  mobilityTankGw.Install (tankGateways);

  /*****************************************************
  *  Concatenation of all gateway NodeContainers     *
  ******************************************************/
  NodeContainer gateways;
  gateways.Add (oilfieldGateways);
  gateways.Add (pipelineGateways);
  gateways.Add (shipGateways);
  gateways.Add (tankGateways);

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
