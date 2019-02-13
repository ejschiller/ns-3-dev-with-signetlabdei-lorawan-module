/*
 * This script simulates a Lora network with device positions based on extracted
 * geographical positions of real devices in the Zurich area.
 * Scenario #4: Transactional transmission of ACKnowledged packets.
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
#include "ns3/random-variable-stream.h"
#include "ns3/transactional-sender-helper.h"
#include "ns3/command-line.h"
#include "ns3/correlated-shadowing-propagation-loss-model.h"
#include "ns3/building-penetration-loss.h"
#include "ns3/building-allocator.h"
#include "ns3/buildings-helper.h"
#include <algorithm>
#include <ctime>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("ComplexLorawanNetworkExample");

// Network settings
int nDevices = 30;
int nGateways = 15;
double simulationTime = 10000;

// Output control
bool print = true;

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.AddValue ("nDevices",
                "Number of end devices to include in the simulation",
                nDevices);
  cmd.AddValue ("simulationTime",
                "The time for which to simulate",
                simulationTime);
  cmd.AddValue ("print",
                "Whether or not to print various informations",
                print);
  cmd.Parse (argc, argv);

  // Set up logging
  //LogComponentEnable ("ComplexLorawanNetworkExample", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraChannel", LOG_LEVEL_INFO);
  // LogComponentEnable("LoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraInterferenceHelper", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraMac", LOG_LEVEL_ALL);
  // LogComponentEnable("EndDeviceLoraMac", LOG_LEVEL_ALL);
  //LogComponentEnable("GatewayLoraMac", LOG_LEVEL_ALL);
  // LogComponentEnable("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  // LogComponentEnable("LogicalLoraChannel", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraHelper", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraPhyHelper", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraMacHelper", LOG_LEVEL_ALL);
  LogComponentEnable("TransactionalSenderHelper", LOG_LEVEL_ALL);
  LogComponentEnable("TransactionalSender", LOG_LEVEL_DEBUG);
  // LogComponentEnable("LoraMacHeader", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraFrameHeader", LOG_LEVEL_ALL);
  // LogComponentEnable("NetworkScheduler", LOG_LEVEL_ALL);
  // LogComponentEnable("NetworkStatus", LOG_LEVEL_ALL);
  // LogComponentEnable("NetworkController", LOG_LEVEL_ALL);
  //LogComponentEnable("LoraPacketTracker", LOG_LEVEL_ALL);
  //LogComponentEnable ("MobilityHelper", LOG_LEVEL_ALL);
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
  extractedPositions.push_back(Vector (9782.655684816418, 16407.83287118317, 0.0));
  extractedPositions.push_back(Vector (761.9884992111474, 12798.042369708477, 0.0));
  extractedPositions.push_back(Vector (6515.527137617231, 13605.91745874213, 0.0));
  extractedPositions.push_back(Vector (7930.596178661217, 11784.030245990027, 0.0));
  extractedPositions.push_back(Vector (9795.627437463845, 10606.133177199517, 0.0));
  extractedPositions.push_back(Vector (2547.116770894616, 6258.487048110168, 0.0));

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

  // nDevices should at least be larger than nGateways
  NS_ASSERT(nGateways < nDevices);

  // x/y-coordinates according to result of gatewayExtractor.py
  std::vector<Vector> gatewayPositions;

  gatewayPositions.push_back(Vector (8685.550585162477, 3512.3997478022648, 0.0));
  gatewayPositions.push_back(Vector (4516.319102031179, 4865.475907711138, 0.0));
  gatewayPositions.push_back(Vector (6861.287421084475, 15531.468585852708, 0.0));
  gatewayPositions.push_back(Vector (5559.748275220627, 12511.135662783403, 0.0));
  gatewayPositions.push_back(Vector (7401.995269967127, 12425.073237299308, 0.0));
  gatewayPositions.push_back(Vector (10766.53554378869, 10684.974758624507, 0.0));
  gatewayPositions.push_back(Vector (10655.866523780744, 10566.872982699686, 0.0));
  gatewayPositions.push_back(Vector (9869.66170600825, 10728.706601473037, 0.0));
  gatewayPositions.push_back(Vector (10655.866523780744, 10566.872982699686, 0.0));
  gatewayPositions.push_back(Vector (9757.048948329408, 10692.272563352395, 0.0));
  gatewayPositions.push_back(Vector (7236.711447313079, 14208.324453553883, 0.0));
  gatewayPositions.push_back(Vector (9094.048935775994, 10074.613378230628, 0.0));
  gatewayPositions.push_back(Vector (14733.273650575778, 2959.646779379429, 0.0));
  gatewayPositions.push_back(Vector (4872.662050584215, 12464.951393250958, 0.0));
  gatewayPositions.push_back(Vector (5535.832890966674, 12550.799783112278, 0.0));
  gatewayPositions.push_back(Vector (5557.728674735874, 12504.835272178752, 0.0));
  gatewayPositions.push_back(Vector (4030.2577847668435, 9141.32729968711, 0.0));
  gatewayPositions.push_back(Vector (5245.990583991981, 11578.832547668018, 0.0));
  gatewayPositions.push_back(Vector (9923.271503742319, 11122.104255760903, 0.0));
  gatewayPositions.push_back(Vector (16102.710063695908, 13928.383597958076, 0.0));
  gatewayPositions.push_back(Vector (10476.50623490091, 10322.927904981654, 0.0));
  gatewayPositions.push_back(Vector (10103.883493548376, 10958.17126916995, 0.0));
  gatewayPositions.push_back(Vector (9704.61431639106, 11504.677970921533, 0.0));
  gatewayPositions.push_back(Vector (10763.189965545083, 14926.593545413169, 0.0));
  gatewayPositions.push_back(Vector (8101.7095359608065, 10223.787945662334, 0.0));
  gatewayPositions.push_back(Vector (9558.133872383507, 6860.177873340959, 0.0));
  gatewayPositions.push_back(Vector (11684.699918592814, 8793.377278830361, 0.0));
  gatewayPositions.push_back(Vector (10491.287685745978, 12975.252744754252, 0.0));
  gatewayPositions.push_back(Vector (11163.206232463708, 9052.93065886994, 0.0));
  gatewayPositions.push_back(Vector (15498.545741137932, 16448.159839435917, 0.0));
  gatewayPositions.push_back(Vector (7593.089246468502, 13988.325700527115, 0.0));
  gatewayPositions.push_back(Vector (11359.999955139589, 8602.684260609269, 0.0));
  gatewayPositions.push_back(Vector (9729.15838289389, 14536.232096235733, 0.0));
  gatewayPositions.push_back(Vector (9833.321313925553, 16341.966639570514, 0.0));
  gatewayPositions.push_back(Vector (13201.022074749111, 7565.245094465121, 0.0));
  gatewayPositions.push_back(Vector (10626.646428018925, 9264.96559980791, 0.0));
  gatewayPositions.push_back(Vector (11549.884018216631, 690.3172453085426, 0.0));
  gatewayPositions.push_back(Vector (9704.05680557515, 11504.422194130428, 0.0));
  gatewayPositions.push_back(Vector (10496.0453135313, 10327.772159880784, 0.0));
  gatewayPositions.push_back(Vector (3872.5714019192383, 10033.394565044611, 0.0));
  gatewayPositions.push_back(Vector (10482.170753213577, 10864.077915647416, 0.0));
  gatewayPositions.push_back(Vector (11074.317690260359, 2787.4050105146016, 0.0));
  gatewayPositions.push_back(Vector (7670.559946893598, 14844.625232111779, 0.0));
  gatewayPositions.push_back(Vector (10100.666593686095, 10957.763470206119, 0.0));
  gatewayPositions.push_back(Vector (14459.921123372857, 13703.479859293817, 0.0));
  gatewayPositions.push_back(Vector (9704.052566521801, 11504.724580663256, 0.0));
  gatewayPositions.push_back(Vector (9799.650541180512, 16411.96338876558, 0.0));
  gatewayPositions.push_back(Vector (10679.27087547048, 11006.251976159081, 0.0));
  gatewayPositions.push_back(Vector (4122.352615268552, 16542.762861510157, 0.0));
  gatewayPositions.push_back(Vector (11256.429709248943, 17286.03420078481, 0.0));
  gatewayPositions.push_back(Vector (5559.072105840081, 12506.632249587448, 0.0));
  gatewayPositions.push_back(Vector (15385.773455846938, 13586.74466004668, 0.0));
  gatewayPositions.push_back(Vector (15197.974589264602, 13594.245988164359, 0.0));
  gatewayPositions.push_back(Vector (8568.43242397951, 11425.544730221678, 0.0));
  gatewayPositions.push_back(Vector (9283.78174269793, 9290.900175155664, 0.0));
  gatewayPositions.push_back(Vector (6013.332195830182, 12224.117915571522, 0.0));
  gatewayPositions.push_back(Vector (7650.555621664622, 10060.03770199616, 0.0));
  gatewayPositions.push_back(Vector (2553.2568212926853, 6247.463313310262, 0.0));

  Ptr<ListPositionAllocator> positionAllocGw = CreateObject<ListPositionAllocator> ();

  int countGW = 0;
  auto iteGW = gatewayPositions.begin();

  while(countGW != nGateways && iteGW != gatewayPositions.end()){
      positionAllocGw->Add (*iteGW);

      ++iteGW;
      ++countGW;
  }

  // Add additional gateways around real nodes with random position offset
  while(countGW < nGateways) {
      if(iteGW == gatewayPositions.end()) iteGW = gatewayPositions.begin();

      // Add random offset between min & max to x and y coordinates
      Vector tempV(iteGW->x + rand->GetValue(), iteGW->y + rand->GetValue(), 0.0);
      positionAllocGw->Add (tempV);

      ++iteGW;
      ++countGW;
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
  helper.EnablePacketTracking ("performance"); // Output filename
  // helper.EnableSimulationTimePrinting ();

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
    }

  /*********************
   *  Create Gateways  *
   *********************/

  // Create the gateway nodes (allocate them uniformely on the disc)
  NodeContainer gateways;
  gateways.Create (nGateways);

  mobilityGw.Install(gateways);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LoraMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  /**********************************************
   *  Set up the end device's spreading factor  *
   **********************************************/

  macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);

  NS_LOG_DEBUG ("Completed configuration");

  /*********************************************************************
   *  Set message type to CONFIRMED_DATA_UP (ACK) for all end devices  *
   *********************************************************************/
  for(int i = 0; i < nDevices; i++) {
      Ptr<LoraMac> edMacTemp = endDevices.Get (i)->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac ();
      Ptr<EndDeviceLoraMac> edLoraMacTemp = edMacTemp->GetObject<EndDeviceLoraMac> ();
      edLoraMacTemp->SetMType (LoraMacHeader::CONFIRMED_DATA_UP);
  }

  /*********************************************
   *  Install applications on the end devices  *
   *********************************************/

   TransactionalSenderHelper appHelper = TransactionalSenderHelper ();
   //appHelper.SetDataPacketSize(30);
   //appHelper.SetPartialSignaturePacketSize(20);
   //appHelper.SetIntraTransactionDelay(Seconds(20));
   //appHelper.SetInterTransactionDelay(Hours(1));
   //appHelper.SetPacketsPerTransaction(30);

   ApplicationContainer appContainer = appHelper.Install (endDevices);

   appContainer.Start (Seconds (0));
   appContainer.Stop (Seconds (simulationTime));

  /**********************
   * Print output files *
   *********************/
  if (print)
    {
      helper.PrintEndDevices (endDevices, gateways,
                              "endDevices.dat");
    }

  ////////////////
  // Simulation //
  ////////////////

  Simulator::Stop (Seconds (simulationTime) + Hours (1));

  NS_LOG_INFO ("Running simulation...");
  Simulator::Run ();

  Simulator::Destroy ();

  ///////////////////////////
  // Print results to file //
  ///////////////////////////
  NS_LOG_INFO ("Computing performance metrics...");
  helper.PrintPerformance (Seconds(0), Seconds (simulationTime));

  return 0;
}
