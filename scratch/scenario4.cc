/*
 * This script simulates a LoRa network using geographical device positions
 * of regularly transmitting TTN devices in the Zurich area using six gateways.
 *
 * Scenario #4: Transmission of signed, confirmed multi-packet transactions.
 */

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

 NS_LOG_COMPONENT_DEFINE ("Scenario4");

 // Network settings
 int nDevices = 50;
 const int nGateways = 6;
 Time simulationTime = Hours (0.5);
 int dataPacketSize = 42;
 int partialSignaturePacketSize = 34;
 Time intraTransactionDelay = Seconds (120);
 Time interTransactionDelay = intraTransactionDelay;
 int packetsPerTransaction = 10;
 int signaturePacketsPerTransaction = 2;


 int main (int argc, char *argv[])
 {

   CommandLine cmd;
   cmd.AddValue ("nDevices",
                 "Number of end devices to include in the simulation",
                 nDevices);
   cmd.AddValue ("simulationTime",
                 "The time for which to simulate",
                 simulationTime);
   cmd.AddValue ("intraDelay",
                 "The delay between two succeeding transmissions",
                 intraTransactionDelay);
   cmd.AddValue ("interDelay",
                 "The delay between two succeeding transactions",
                 interTransactionDelay);

   cmd.Parse (argc, argv);

   // Set up logging
   LogComponentEnable ("Scenario4", LOG_LEVEL_ALL);
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

   double min = -1000.0;
   double max = 1000.0;

   Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
   rand->SetAttribute ("Min", DoubleValue (min));
   rand->SetAttribute ("Max", DoubleValue (max));

   MobilityHelper mobilityEd, mobilityGw;
   Ptr<ListPositionAllocator> positionAllocEd = CreateObject<ListPositionAllocator> ();

   // x/y-coordinates according to running nodeExtractor.py on 2M transmissions
   std::vector<Vector> extractedPositions;
   extractedPositions.push_back (Vector (9782.655684816418, 16407.83287118317, 0.0));
   extractedPositions.push_back (Vector (761.9884992111474, 12798.042369708477, 0.0));
   extractedPositions.push_back (Vector (6515.527137617231, 13605.91745874213, 0.0));
   extractedPositions.push_back (Vector (7930.596178661217, 11784.030245990027, 0.0));
   extractedPositions.push_back (Vector (9795.627437463845, 10606.133177199517, 0.0));
   extractedPositions.push_back (Vector (2547.116770894616, 6258.487048110168, 0.0));

   // Add extracted end devices' geographic positions
   int countED = 0;
   auto iteED = extractedPositions.begin();

   while(countED != nDevices && iteED != extractedPositions.end()) {
       positionAllocEd->Add (*iteED);

       ++iteED;
       ++countED;
   }

   // Add additional end devices around real nodes with random position offset
   while(countED < nDevices) {
       if(iteED == extractedPositions.end()) iteED = extractedPositions.begin();

       // Add random offset between min & max to x and y coordinates
       Vector tempV(iteED->x + rand->GetValue(), iteED->y + rand->GetValue(), 0.0);
       positionAllocEd->Add (tempV);

       ++iteED;
       ++countED;
   }

   mobilityEd.SetPositionAllocator (positionAllocEd);
   mobilityEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

   // Gateway mobility

   // x/y-coordinates according to result of gatewayExtractor.py
   std::vector<Vector> gatewayPositions;

   gatewayPositions.push_back(Vector (9799.650541180512, 16411.96338876558, 0.0));
   gatewayPositions.push_back(Vector (1066.7192320141476, 12001.422228587267, 0.0));
   gatewayPositions.push_back(Vector (7593.089246468502, 13988.325700527115, 0.0));
   gatewayPositions.push_back(Vector (7969.250789985876, 11903.682904455607, 0.0));
   gatewayPositions.push_back(Vector (9757.048948329408, 10692.272563352395, 0.0));
   gatewayPositions.push_back(Vector (2553.2568212926853, 6247.463313310262, 0.0));

   Ptr<ListPositionAllocator> positionAllocGw = CreateObject<ListPositionAllocator> ();

   for(auto iteGW = gatewayPositions.begin(); iteGW != gatewayPositions.end(); ++iteGW)
   {
     positionAllocGw->Add (*iteGW);
   }

   mobilityGw.SetPositionAllocator (positionAllocGw);
   mobilityGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

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

   // Create a set of nodes
   NodeContainer endDevices;
   endDevices.Create (nDevices);

   // Assign a mobility model to each node
   mobilityEd.Install (endDevices);

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
   NodeContainer gateways;
   gateways.Create (nGateways);

   mobilityGw.Install (gateways);

   // Create a netdevice for each gateway
   phyHelper.SetDeviceType (LoraPhyHelper::GW);
   macHelper.SetDeviceType (LoraMacHelper::GW);
   helper.Install (phyHelper, macHelper, gateways);

   /**********************************************
    *  Set up the end device's spreading factor  *
    **********************************************/
   macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);

   NS_LOG_DEBUG ("Completed configuration");

   /*********************************************
   *  Install applications on the end devices  *
   ********************************************/

   TransactionalSenderHelper appHelper = TransactionalSenderHelper ();
   appHelper.SetDataPacketSize (dataPacketSize);
   appHelper.SetPartialSignaturePacketSize (partialSignaturePacketSize);
   appHelper.SetIntraTransactionDelay (intraTransactionDelay);
   appHelper.SetInterTransactionDelay (interTransactionDelay);
   appHelper.SetPacketsPerTransaction (packetsPerTransaction);
   ApplicationContainer appContainer = appHelper.Install (endDevices);
   appContainer.Start (Seconds (0));
   appContainer.Stop (simulationTime);
   NodeContainer networkServers;
   networkServers.Create (1);
   // Install the SimpleNetworkServer application on the network server
   NetworkServerHelper networkServerHelper;
   networkServerHelper.EnableStatsCollection ();
   networkServerHelper.EnableTransactionMode ();
   networkServerHelper.SetSimulationTime (simulationTime);
   networkServerHelper.SetFileName ("scenario4.csv");
   networkServerHelper.SetCsvStaticDef ("def_n-3,def_n-2,");
   std::stringstream streamDef;
   std::stringstream streamData;
   streamDef << "NEndDevices,NGateways,SimulationTime," <<
                "InterTransactionDelay,IntraTransactionDelay," <<
                "PacketsPerTransaction,SignaturePacketsPerTransaction," <<
                "DataPacketSize,PartialSignaturePacketSize,";
   streamData << nDevices << "," << nGateways << "," <<
                 simulationTime.GetSeconds () << "," <<
                 interTransactionDelay.GetSeconds () << "," <<
                 intraTransactionDelay.GetSeconds () << "," <<
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
