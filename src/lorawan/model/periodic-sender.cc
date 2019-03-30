/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "ns3/periodic-sender.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/lora-net-device.h"
#include "ns3/periodic-packet-header.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("PeriodicSender");

NS_OBJECT_ENSURE_REGISTERED (PeriodicSender);

TypeId
PeriodicSender::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PeriodicSender")
    .SetParent<Application> ()
    .AddConstructor<PeriodicSender> ()
    .SetGroupName ("lorawan")
    .AddAttribute ("Interval", "The interval between packet sends of this app",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&PeriodicSender::GetInterval,
                                     &PeriodicSender::SetInterval),
                   MakeTimeChecker ());
  // .AddAttribute ("PacketSizeRandomVariable", "The random variable that determines the shape of the packet size, in bytes",
  //                StringValue ("ns3::UniformRandomVariable[Min=0,Max=10]"),
  //                MakePointerAccessor (&PeriodicSender::m_pktSizeRV),
  //                MakePointerChecker <RandomVariableStream>());
  return tid;
}

PeriodicSender::PeriodicSender ()
  : m_interval (Seconds (10)),
  m_initialDelay (Seconds (1)),
  m_basePktSize (10),
  packet_count(0),
  m_pktSizeRV (0),
  m_lastRound (false)

{
  NS_LOG_FUNCTION_NOARGS ();
}

PeriodicSender::~PeriodicSender ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
PeriodicSender::SetInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_interval = interval;
}

Time
PeriodicSender::GetInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interval;
}

void
PeriodicSender::SetInitialDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_initialDelay = delay;
}


void
PeriodicSender::SetPacketSizeRandomVariable (Ptr <RandomVariableStream> rv)
{
  m_pktSizeRV = rv;
}


void
PeriodicSender::SetPacketSize (uint8_t size)
{
  m_basePktSize = size;
}


uint16_t
PeriodicSender::GetPacketCount (void) const {
  return packet_count;
}


void
PeriodicSender::SetPacketCount (uint16_t count) {
  packet_count = count;
}


void
PeriodicSender::ScheduleCancellation (void)
{
  Simulator::Cancel (m_sendEvent);
}


void
PeriodicSender::SendPacket (void)
{
  // If this is the first packet transmission, save the nodeUID for later use
  if(packet_count == 0) {
    nodeUID = m_mac->GetDevice ()->GetNode ()->GetId ();
  }

  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet;

  // Preparing the Header to be put into the packet's payload
  PeriodicPacketHeader periodicHeader;
  periodicHeader.SetNodeUid(nodeUID);
  periodicHeader.SetPacketId(packet_count);

  packet = Create<Packet> ();
  packet->AddHeader (periodicHeader);

  // Getting the packet's current size in B
  uint32_t packetSize = packet->GetSize ();

  // Filling the packet payload up with zeroes until the specified size.
  packet->AddPaddingAtEnd (m_basePktSize - packetSize);

  m_mac->Send (packet);

  ++packet_count;
  // Checking if the simulation is not yet about to be stopped
  if (!m_lastRound)
  {
    // Schedule the next SendPacket event
    m_sendEvent = Simulator::Schedule (m_interval, &PeriodicSender::SendPacket,
                                       this);
  }

  NS_LOG_DEBUG ("Sent a packet of size " << packet->GetSize ());
}

void
PeriodicSender::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  // Make sure we have a MAC layer
  if (m_mac == 0)
    {
      // Assumes there's only one device
      Ptr<LoraNetDevice> loraNetDevice = m_node->GetDevice (0)->GetObject<LoraNetDevice> ();

      m_mac = loraNetDevice->GetMac ();
      NS_ASSERT (m_mac != 0);
    }

  // Schedule the next SendPacket event
  Simulator::Cancel (m_sendEvent);
  NS_LOG_DEBUG ("Starting up application with a first event with a " <<
                m_initialDelay.GetSeconds () << " seconds delay");
  m_sendEvent = Simulator::Schedule (m_initialDelay,
                                     &PeriodicSender::SendPacket, this);
  NS_LOG_DEBUG ("Event Id: " << m_sendEvent.GetUid ());
}

void
PeriodicSender::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_lastRound = true;
  // Cancel transactions taking too long to finish
  Simulator::Schedule (Minutes (40), &PeriodicSender::ScheduleCancellation,
                       this);
}

}
}
