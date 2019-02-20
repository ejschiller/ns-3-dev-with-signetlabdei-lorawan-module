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
  m_packetLossBecauseTransmitting(0)
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
    RegisterSuccessfulTransmission(myPacket);
  }
  else if (m_collectStats && m_transactionMode)
  {
    // TODO: Implement a RegisterTransactionPacket-method & call it from here
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
NetworkServer::RegisterPacketReception (Ptr<const Packet> packet, unsigned int index)
{
  ++m_receivedPackets;
  //NS_LOG_UNCOND("Successfully received a packet. Packet count = " << m_receivedPackets);
}

void
NetworkServer::RegisterPacketLossInterference (Ptr<const Packet> packet, unsigned int index)
{
  ++m_packetLossInterference;
  //NS_LOG_UNCOND("Lost a packet due to interference. Interference packet loss count = " << m_packetLossInterference);
}

void
NetworkServer::RegisterPacketLossUnderSensitivity (Ptr<const Packet> packet, unsigned int index)
{
  ++m_packetLossUnderSensitivity;
  //NS_LOG_UNCOND("Lost a packet due to reception under sensitivity. Under sensitivity packet loss count = " << m_packetLossUnderSensitivity);
}

void
NetworkServer::RegisterPacketLossNoMoreReceivers (Ptr<const Packet> packet, unsigned int index)
{
  ++m_packetLossNoMoreReceivers;
  //NS_LOG_UNCOND("Lost a packet because no more receivers were available. No more receivers packet loss count = " << m_packetLossNoMoreReceivers);
}

void
NetworkServer::RegisterPacketLossBecauseTransmitting (Ptr<const Packet> packet, unsigned int index)
{
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
  int packetId = (int) periodicHdr.GetPacketId ();

  auto search = m_successfulTransmissions.find (nodeUidTmp);
  if(search == m_successfulTransmissions.end ())
  {
    std::set<int> transmissionSet;
    // Filling in the first transmission for this node
    transmissionSet.insert (packetId);
    m_successfulTransmissions.insert (std::make_pair (nodeUidTmp, transmissionSet));
  }
  else
  {
    search->second.insert (packetId);
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

  if(m_transactionMode)
  {
    /* TODO: implement analysis of transactions here */
  }
  else
  {
    // Computing the number of individual successful packet transmissions
    int successfulTransmissions = 0;
    for(auto ite = m_successfulTransmissions.begin (); ite != m_successfulTransmissions.end (); ++ite)
    {
      successfulTransmissions += ite->second.size ();
    }

    NS_LOG_UNCOND("Successful Transmissions: " << successfulTransmissions);
  }
}


}
}
