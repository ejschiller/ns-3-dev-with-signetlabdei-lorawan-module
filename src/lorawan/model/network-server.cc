/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Davide Magrin <magrinda@dei.unipd.it>
 *          Martina Capuzzo <capuzzom@dei.unipd.it>
 */

#include "ns3/network-server.h"
#include "ns3/net-device.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/packet.h"
#include "ns3/lora-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/periodic-packet-header.h"
#include "ns3/transactional-packet-header.h"
#include "ns3/lora-device-address.h"
#include "ns3/network-status.h"
#include "ns3/lora-frame-header.h"
#include "ns3/node-container.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/mac-command.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("NetworkServer");

NS_OBJECT_ENSURE_REGISTERED (NetworkServer);

TypeId
NetworkServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NetworkServer")
    .SetParent<Application> ()
    .AddConstructor<NetworkServer> ()
    .AddTraceSource ("ReceivedPacket",
                     "Trace source that is fired when a packet arrives at the Network Server",
                     MakeTraceSourceAccessor (&NetworkServer::m_receivedPacket),
                     "ns3::Packet::TracedCallback")
    .SetGroupName ("lorawan");
  return tid;
}

NetworkServer::NetworkServer () :
  m_status (Create<NetworkStatus> ()),
  m_controller (Create<NetworkController> (m_status)),
  m_scheduler (Create<NetworkScheduler> (m_status, m_controller)),
  m_collectStats(false),
  m_transactionMode(false),
  m_receivedPackets(0),
  m_packetLossInterference(0),
  m_packetLossUnderSensitivity(0),
  m_packetLossNoMoreReceivers(0),
  m_packetLossBecauseTransmitting(0),
  m_numberOfPacketsPerTransaction(0),
  filename ("output.csv")
{
  NS_LOG_FUNCTION_NOARGS ();
}

NetworkServer::~NetworkServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
NetworkServer::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
NetworkServer::StopApplication (void)
{
  if(m_collectStats) PrintStatistics ();
  NS_LOG_FUNCTION_NOARGS ();
}

void
NetworkServer::AddGateway (Ptr<Node> gateway, Ptr<NetDevice> netDevice)
{
  NS_LOG_FUNCTION (this << gateway);

  // Get the PointToPointNetDevice
  Ptr<PointToPointNetDevice> p2pNetDevice;
  for (uint32_t i = 0; i < gateway->GetNDevices (); i++)
    {
      p2pNetDevice = gateway->GetDevice (i)->GetObject<PointToPointNetDevice> ();
      if (p2pNetDevice != 0)
        {
          // We found a p2pNetDevice on the gateway
          break;
        }
    }

  // Get the gateway's LoRa MAC layer (assumes gateway's MAC is configured as first device)
  Ptr<GatewayLoraMac> gwMac = gateway->GetDevice (0)->GetObject<LoraNetDevice> ()->
    GetMac ()->GetObject<GatewayLoraMac> ();
  NS_ASSERT (gwMac != 0);

  // Get the PHY
  Ptr<GatewayLoraPhy> gwPhy = gateway->GetDevice (0)->GetObject<LoraNetDevice> ()->
    GetPhy ()->GetObject<GatewayLoraPhy> ();

  if(m_collectStats) {

    gwPhy->TraceConnectWithoutContext("ReceivedPacket",
                                        MakeCallback
                                        (&NetworkServer::RegisterPacketReception,
                                        this));

    gwPhy->TraceConnectWithoutContext("LostPacketBecauseInterference",
                                        MakeCallback
                                        (&NetworkServer::RegisterPacketLossInterference,
                                        this));

    gwPhy->TraceConnectWithoutContext("LostPacketBecauseUnderSensitivity",
                                        MakeCallback
                                        (&NetworkServer::RegisterPacketLossUnderSensitivity,
                                        this));

    gwPhy->TraceConnectWithoutContext("LostPacketBecauseNoMoreReceivers",
                                        MakeCallback
                                        (&NetworkServer::RegisterPacketLossNoMoreReceivers,
                                        this));

    gwPhy->TraceConnectWithoutContext("NoReceptionBecauseTransmitting",
                                        MakeCallback
                                        (&NetworkServer::RegisterPacketLossBecauseTransmitting,
                                        this));
  }

  // Get the Address
  Address gatewayAddress = p2pNetDevice->GetAddress ();

  // Create new gatewayStatus
  Ptr<GatewayStatus> gwStatus = Create<GatewayStatus> (gatewayAddress,
                                                       netDevice,
                                                       gwMac);

  m_status->AddGateway (gatewayAddress, gwStatus);
}

void
NetworkServer::AddNodes (NodeContainer nodes)
{
  NS_LOG_FUNCTION_NOARGS ();

  // For each node in the container, call the function to add that single node
  NodeContainer::Iterator it;
  for (it = nodes.Begin (); it != nodes.End (); it++)
    {
      AddNode (*it);
    }
}

void
NetworkServer::AddNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);

  // Get the LoraNetDevice
  Ptr<LoraNetDevice> loraNetDevice;
  for (uint32_t i = 0; i < node->GetNDevices (); i++)
    {
      loraNetDevice = node->GetDevice (i)->GetObject<LoraNetDevice> ();
      if (loraNetDevice != 0)
        {
          // We found a LoraNetDevice on the node
          break;
        }
    }

  // Get the MAC
  Ptr<EndDeviceLoraMac> edLoraMac =
    loraNetDevice->GetMac ()->GetObject<EndDeviceLoraMac> ();

  // Keep track of the total number of transmissions required to deliver this packet
  edLoraMac->TraceConnectWithoutContext("RequiredTransmissions",
                                        MakeCallback
                                        (&NetworkServer::RegisterRequiredTransmissions,
                                        this));

  // Update the NetworkStatus about the existence of this node
  m_status->AddNode (edLoraMac);
}

bool
NetworkServer::Receive (Ptr<NetDevice> device, Ptr<const Packet> packet,
                        uint16_t protocol, const Address& address)
{
  NS_LOG_FUNCTION (this << packet << protocol << address);

  // Create a copy of the packet
  Ptr<Packet> myPacket = packet->Copy ();

  LoraMacHeader macHdr;
  myPacket->RemoveHeader (macHdr);
  LoraFrameHeader frameHdr;
  myPacket->RemoveHeader (frameHdr);
  LoraTag tag;
  myPacket->RemovePacketTag (tag);

  if (m_collectStats && !m_transactionMode)
  {
    RegisterSuccessfulTransmission (myPacket);
  }
  else if (m_collectStats && m_transactionMode)
  {
    RegisterSuccessfulTransactionalPacket (myPacket);
  }

  // Fire the trace source
  m_receivedPacket (packet);

  // Inform the scheduler of the newly arrived packet
  m_scheduler->OnReceivedPacket (packet);

  // Inform the status of the newly arrived packet
  m_status->OnReceivedPacket (packet, address);

  // Inform the controller of the newly arrived packet
  m_controller->OnNewPacket (packet);

  return true;
}

void
NetworkServer::AddComponent (Ptr<NetworkControllerComponent> component)
{
  NS_LOG_FUNCTION (this << component);

  m_controller->Install (component);
}

Ptr<NetworkStatus>
NetworkServer::GetNetworkStatus (void)
{
  return m_status;
}

void
NetworkServer::EnableTransactionMode (void)
{
  m_transactionMode = true;
}

void
NetworkServer::EnableStatsCollection (void)
{
  // initializing the m_requiredTransmissions vector
  for (int i = 0; i<8; i++)
  {
    m_requiredTransmissions.push_back (0);
  }

  m_collectStats = true;
}

void
NetworkServer::SetNumberOfPacketsPerTransaction (int packets)
{
  m_numberOfPacketsPerTransaction = packets;
}

void
NetworkServer::SetFileName (std::string fname)
{
  filename = fname;
}

void
NetworkServer::SetCsvStaticDef (std::string def)
{
  csvStaticDef = def;
}

void
NetworkServer::SetCsvStaticData (std::string data)
{
  csvStaticData = data;
}

void
NetworkServer::RegisterPacketReception (Ptr<const Packet> packet, unsigned int index)
{
  ++m_receivedPackets;
  //NS_LOG_UNCOND("Successfully received a packet. Packet count = " << m_receivedPackets);
}

void
NetworkServer::RegisterPacketLossInterference (Ptr<const Packet> packet, unsigned int index)
{
  Ptr<Packet> myPacket = packet->Copy ();

  if (m_transactionMode)
  {
    RegisterUnsuccessfulTransactionalPacket (myPacket);
  }
  else RegisterUnsuccessfulTransmission (myPacket);

  ++m_packetLossInterference;
  //NS_LOG_UNCOND("Lost a packet due to interference. Interference packet loss count = " << m_packetLossInterference);
}

void
NetworkServer::RegisterPacketLossUnderSensitivity (Ptr<const Packet> packet, unsigned int index)
{
  Ptr<Packet> myPacket = packet->Copy ();

  if (m_transactionMode)
  {
    RegisterUnsuccessfulTransactionalPacket (myPacket);
  }
  else RegisterUnsuccessfulTransmission (myPacket);

  ++m_packetLossUnderSensitivity;
  //NS_LOG_UNCOND("Lost a packet due to reception under sensitivity. Under sensitivity packet loss count = " << m_packetLossUnderSensitivity);
}

void
NetworkServer::RegisterPacketLossNoMoreReceivers (Ptr<const Packet> packet, unsigned int index)
{
  Ptr<Packet> myPacket = packet->Copy ();

  if (m_transactionMode)
  {
    RegisterUnsuccessfulTransactionalPacket (myPacket);
  }
  else RegisterUnsuccessfulTransmission (myPacket);

  ++m_packetLossNoMoreReceivers;
  //NS_LOG_UNCOND("Lost a packet because no more receivers were available. No more receivers packet loss count = " << m_packetLossNoMoreReceivers);
}

void
NetworkServer::RegisterPacketLossBecauseTransmitting (Ptr<const Packet> packet, unsigned int index)
{
  Ptr<Packet> myPacket = packet->Copy ();

  if (m_transactionMode)
  {
    RegisterUnsuccessfulTransactionalPacket (myPacket);
  }
  else RegisterUnsuccessfulTransmission (myPacket);

  ++m_packetLossBecauseTransmitting;
  //NS_LOG_UNCOND("Lost a packet because GW was transmitting during packet arrival. GW Tx packet loss count = " << m_packetLossBecauseTransmitting);
}

void
NetworkServer::RegisterRequiredTransmissions (unsigned char ch_attempts, bool flag, Time time, Ptr<Packet> packet)
{
  int attempts = (int) ch_attempts;
  NS_ASSERT (attempts <= 8);
  if (attempts > 0)
  {
    m_requiredTransmissions[attempts-1] = m_requiredTransmissions[attempts-1] + 1;
    //NS_LOG_UNCOND ("Total number of transmissions required to deliver this packet: " << (int) attempts);
  }
}

void
NetworkServer::RegisterSuccessfulTransmission (Ptr<Packet> packet)
{
  NS_ASSERT(m_collectStats);
  PeriodicPacketHeader periodicHdr;
  packet->RemoveHeader (periodicHdr);

  int nodeUidTmp = (int) periodicHdr.GetNodeUid ();
  int packetIdTmp = (int) periodicHdr.GetPacketId ();

  auto search = m_successfulTransmissions.find (nodeUidTmp);
  if(search == m_successfulTransmissions.end ())
  {
    std::set<int> transmissionSet;
    // Registering the first successful transmission for this node
    transmissionSet.insert (packetIdTmp);
    m_successfulTransmissions.insert (std::make_pair (nodeUidTmp, transmissionSet));
  }
  else
  {
    search->second.insert (packetIdTmp);
  }
}

void
NetworkServer::RegisterUnsuccessfulTransmission (Ptr<Packet> packet)
{
  NS_ASSERT(m_collectStats);
  LoraFrameHeader loraFrameHeader;
  LoraMacHeader loraMacHeader;
  if(packet->GetSize () > (loraFrameHeader.GetSerializedSize () + loraMacHeader.GetSerializedSize ()))
  {
    PeriodicPacketHeader periodicHdr;
    packet->RemoveHeader (loraMacHeader);
    packet->RemoveHeader (loraFrameHeader);
    packet->RemoveHeader (periodicHdr);

    int nodeUidTmp = (int) periodicHdr.GetNodeUid ();
    int packetIdTmp = (int) periodicHdr.GetPacketId ();

    auto search = m_unsuccessfulTransmissions.find (nodeUidTmp);
    if(search == m_unsuccessfulTransmissions.end ())
    {
      std::set<int> transmissionSet;
      // Registering the first unsuccessful transmission for this node
      transmissionSet.insert (packetIdTmp);
      m_unsuccessfulTransmissions.insert (std::make_pair (nodeUidTmp, transmissionSet));
    }
    else
    {
      search->second.insert (packetIdTmp);
    }
  }
}

void
NetworkServer::RegisterSuccessfulTransactionalPacket (Ptr<Packet> packet)
{
  NS_ASSERT(m_transactionMode);
  TransactionalPacketHeader transactionalHdr;
  packet->RemoveHeader (transactionalHdr);

  int nodeUidTmp = (int) transactionalHdr.GetNodeUid ();
  int transactionIdTmp = (int) transactionalHdr.GetTransactionId ();
  int packetIdTmp = (int) transactionalHdr.GetPacketId ();

  auto search = m_successfulTransactionalPackets.find (nodeUidTmp);
  if (search == m_successfulTransactionalPackets.end ())
  {
    std::map<int, std::set<int>> transactionMap;
    // Registering the first transaction for this node;
    std::set<int> transaction;
    transaction.insert (packetIdTmp);
    transactionMap.insert (std::make_pair (transactionIdTmp, transaction));
    m_successfulTransactionalPackets.insert (std::make_pair (nodeUidTmp, transactionMap));
  }
  else
  {
    // Checking, if for this node the packet's transaction is already present
    auto searchTransaction = search->second.find (transactionIdTmp);
    if (searchTransaction == search->second.end ())
    {
      std::set<int> transaction;
      transaction.insert (packetIdTmp);
      search->second.insert (std::make_pair (transactionIdTmp, transaction));
    }
    else
    {
      searchTransaction->second.insert(packetIdTmp);
    }
  }
}

void
NetworkServer::RegisterUnsuccessfulTransactionalPacket (Ptr<Packet> packet)
{
  NS_ASSERT(m_transactionMode);
  LoraFrameHeader loraFrameHeader;
  LoraMacHeader loraMacHeader;
  if(packet->GetSize () > (loraFrameHeader.GetSerializedSize () + loraMacHeader.GetSerializedSize ()))
  {
    TransactionalPacketHeader transactionalHdr;
    packet->RemoveHeader (loraMacHeader);
    packet->RemoveHeader (loraFrameHeader);
    packet->RemoveHeader (transactionalHdr);

    int nodeUidTmp = (int) transactionalHdr.GetNodeUid ();
    int transactionIdTmp = (int) transactionalHdr.GetTransactionId ();
    int packetIdTmp = (int) transactionalHdr.GetPacketId ();

    auto search = m_unsuccessfulTransactionalPackets.find (nodeUidTmp);
    if (search == m_unsuccessfulTransactionalPackets.end ())
    {
      std::map<int, std::set<int>> transactionMap;
      // Registering the first transaction for this node;
      std::set<int> transaction;
      transaction.insert (packetIdTmp);
      transactionMap.insert (std::make_pair (transactionIdTmp, transaction));
      m_unsuccessfulTransactionalPackets.insert (std::make_pair (nodeUidTmp, transactionMap));
    }
    else
    {
      // Checking, if for this node the packet's transaction is already present
      auto searchTransaction = search->second.find (transactionIdTmp);
      if (searchTransaction == search->second.end ())
      {
        std::set<int> transaction;
        transaction.insert (packetIdTmp);
        search->second.insert (std::make_pair (transactionIdTmp, transaction));
      }
      else
      {
        searchTransaction->second.insert(packetIdTmp);
      }
    }
  }
}

void
NetworkServer::PrintStatistics (void)
{
  NS_ASSERT(m_collectStats);
  NS_LOG_UNCOND("Simulation statistics:");
  NS_LOG_UNCOND("Matrix 1: [ # of successful packet receptions at gateways ||" <<
                " # of packets lost due to interference |" <<
                " # of packets lost due to reception under sensitivity |" <<
                " # of packets lost because no more receivers |"<<
                " # of packets lost because GW was transmitting during packet arrival ]");
  NS_LOG_UNCOND("[ " << m_receivedPackets << " || " << m_packetLossInterference <<
                " | " << m_packetLossUnderSensitivity << " | " <<
                 m_packetLossNoMoreReceivers << " | " <<
                  m_packetLossBecauseTransmitting << " ]");
  NS_LOG_UNCOND("Matrix 2, # of transmissions required to deliver packets successfully:" <<
                " [ 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 ]");
  NS_LOG_UNCOND("[ " << m_requiredTransmissions[0] << " | " << m_requiredTransmissions[1] <<
                " | " << m_requiredTransmissions[2] << " | " << m_requiredTransmissions[3] <<
                " | " << m_requiredTransmissions[4] << " | " << m_requiredTransmissions[5] <<
                " | " << m_requiredTransmissions[6] << " | " << m_requiredTransmissions[7] << " ] ");

  double conversion = (Hours(1).GetSeconds () / m_stopTime.GetSeconds ());

  if(m_transactionMode)
  {
    NS_ASSERT(m_numberOfPacketsPerTransaction > 0);
    // Computing the number of entirely received transactions (all packets + signature packets received)
    int successfulTransactions = 0;
    int incompleteTransactions = 0;
    for (auto edIte = m_successfulTransactionalPackets.begin (); edIte != m_successfulTransactionalPackets.end (); ++edIte)
    {
      int highestTransactionId = 0;
      for (auto transIte = edIte->second.begin (); transIte != edIte->second.end (); ++transIte)
      {
        // Recording the highest transaction id present in the m_successfulTransactionalPackets map
        if (transIte->first > highestTransactionId) highestTransactionId = transIte->first;

        // Counting this transaction to the no. of successful transactions, if its size fulfils the requirement
        if (transIte->second.size () == (unsigned int) m_numberOfPacketsPerTransaction)
        {
          NS_LOG_DEBUG ("Successful transaction nr.: " << transIte->first <<
           ", size: " << transIte->second.size () << ", node :" << edIte->first);

          ++successfulTransactions;

          // Removing the transaction from the m_unsuccessfulTransactionalPackets map, if present there
          auto edIte2 = m_unsuccessfulTransactionalPackets.find (edIte->first);
          if (edIte2 != m_unsuccessfulTransactionalPackets.end ())
          {
            edIte2->second.erase (transIte->first);
          }
        }
      }

      auto edIte2 = m_unsuccessfulTransactionalPackets.find (edIte->first);
      // Checking, if there is a transaction map for the current node
      if (edIte2 != m_unsuccessfulTransactionalPackets.end ())
      {
        for (auto transIte2 = edIte2->second.begin (); transIte2 != edIte2->second.end (); ++transIte2)
        {
          NS_LOG_DEBUG ("Unsuccessful transaction nr.: " << transIte2->first <<
           ", size: " << transIte2->second.size () << ", node :" << edIte2->first);

          ++incompleteTransactions;
        }

        if (edIte2->second.size () > 0)
        {
          auto lastTrans = std::prev (edIte2->second.end (), 1);
          /* If the last unsuccessful transaction's ID is bigger than the greatest successful transaction
             ID, it is truly the node's last transaction. If its size is smaller than the no. of packets
             per transaction, it is assumed, that its failure is due to short simulation time.*/
          if (lastTrans->first > highestTransactionId &&
              lastTrans->second.size () < (unsigned int) m_numberOfPacketsPerTransaction)
          {
            --incompleteTransactions;
          }
        }
      }
    }

    double successRate = (double) successfulTransactions
                            / (successfulTransactions + incompleteTransactions);
    NS_LOG_UNCOND("# of successful transactions: " << successfulTransactions);
    NS_LOG_UNCOND("# of incomplete transactions: " << incompleteTransactions);
    NS_LOG_UNCOND("Success rate: " << successRate * 100 << "%");
    NS_LOG_UNCOND("Throughput: " << successfulTransactions * conversion
                    << " transactions per hour.");

    std::ofstream outfile;
    std::ifstream file(filename);
    if(!file)
    {
        outfile.open (filename, std::ios_base::app);
        outfile << csvStaticDef << "SuccessfulTransactions," <<
                                   "IncompleteTransactions," <<
                                   "SuccessRate," <<
                                   "Throughput" <<  std::endl;
    }
    else
    {
      outfile.open (filename, std::ios_base::app);
    }

    outfile << csvStaticData << successfulTransactions << "," <<
                                incompleteTransactions << "," <<
                                successRate << "," <<
                                successfulTransactions * conversion << std::endl;
  }
  else
  {
    // Computing the number of individual un-/successful packet transmissions
    int successfulTransmissions = 0;
    int unsuccessfulTransmissions = 0;

    for (auto ite = m_successfulTransmissions.begin (); ite != m_successfulTransmissions.end (); ++ite)
    {
      // successfulTransmissions += (size of set)
      successfulTransmissions += ite->second.size ();

      // search for the current end device in the m_unsuccessfulTransmissions map
      auto search = m_unsuccessfulTransmissions.find (ite->first);
      if(search != m_unsuccessfulTransmissions.end ())
      {
        // building a temp set of transmissions which did not reach any GW
        std::set<int> actualMissing;
        std::set_difference (search->second.begin (), search->second.end (),
                             ite->second.begin (), ite->second.end (),
                             std::inserter (actualMissing, actualMissing.begin ()));

        // recording the number of globally unsuccessful transmissions for this ED
        unsuccessfulTransmissions += actualMissing.size ();

        // remove entry for this ED, such that only entries for EDs with only unsuccessful transmissions remain.
        m_unsuccessfulTransmissions.erase (search);
      }
    }

    // adding the unsuccessful transmissions for EDs, which did not have any successful transmissions
    for (auto ite = m_unsuccessfulTransmissions.begin (); ite != m_unsuccessfulTransmissions.end (); ++ ite)
    {
      unsuccessfulTransmissions += ite->second.size ();
    }

    double successRate = (double) successfulTransmissions
                            / (successfulTransmissions + unsuccessfulTransmissions);
    NS_LOG_UNCOND("# of successful transmissions: " << successfulTransmissions);
    NS_LOG_UNCOND("# of unsuccessful transmissions: " << unsuccessfulTransmissions);
    NS_LOG_UNCOND("Success rate: " << successRate * 100 << "%");
    NS_LOG_UNCOND("Throughput: " << successfulTransmissions * conversion
                    << " transmissions per hour.");

    std::ofstream outfile;
    std::ifstream file(filename);
    if(!file)
    {
        outfile.open (filename, std::ios_base::app);
        outfile << csvStaticDef << "SuccessfulTransmissions," <<
                                   "UnsuccessfulTransmissions," <<
                                   "SuccessRate," <<
                                   "Throughput" <<  std::endl;
    }
    else
    {
      outfile.open (filename, std::ios_base::app);
    }

    outfile << csvStaticData << successfulTransmissions << "," <<
                                unsuccessfulTransmissions << "," <<
                                successRate << "," <<
                                successfulTransmissions * conversion << std::endl;
  }
}
}
}
