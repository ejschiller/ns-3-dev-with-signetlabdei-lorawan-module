/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/transactional-sender.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/lora-net-device.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("TransactionalSender");

NS_OBJECT_ENSURE_REGISTERED (TransactionalSender);

TypeId
TransactionalSender::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TransactionalSender")
    .SetParent<Application> ()
    .AddConstructor<TransactionalSender> ()
    .SetGroupName ("lorawan")
    .AddAttribute ("Interval", "The interval between packet sends of this app",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&TransactionalSender::GetInterval,
                                     &TransactionalSender::SetInterval),
                   MakeTimeChecker ());
  // .AddAttribute ("PacketSizeRandomVariable", "The random variable that determines the shape of the packet size, in bytes",
  //                StringValue ("ns3::UniformRandomVariable[Min=0,Max=10]"),
  //                MakePointerAccessor (&TransactionalSender::m_pktSizeRV),
  //                MakePointerChecker <RandomVariableStream>());
  return tid;
}

TransactionalSender::TransactionalSender ()
  : m_interval (Seconds (10)),
  m_initialDelay (Seconds (1)),
  m_basePktSize (10),
  packet_count (0),
  m_pktSizeRV (0)

{
  NS_LOG_FUNCTION_NOARGS ();
}

TransactionalSender::~TransactionalSender ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
TransactionalSender::SetInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_interval = interval;
}

Time
TransactionalSender::GetInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interval;
}

void
TransactionalSender::SetInitialDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_initialDelay = delay;
}


void
TransactionalSender::SetPacketSizeRandomVariable (Ptr <RandomVariableStream> rv)
{
  m_pktSizeRV = rv;
}


void
TransactionalSender::SetPacketSize (uint8_t size)
{
  m_basePktSize = size;
}


uint32_t
TransactionalSender::GetPacketCount () {
  return packet_count;
}


void
TransactionalSender::SetPacketCount (uint32_t count) {
  packet_count = count;
}

void
TransactionalSender::IncrementPacketCount () {
  ++packet_count;
}


void
TransactionalSender::SendPacket (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet;


  if(packet_count == 0) {

    packet = Create<Packet> (32);   // sending public key, 32 B
    m_mac->Send (packet);
    NS_LOG_UNCOND ("Sent a public key packet of size " << packet->GetSize () << "B");
    ++packet_count;
    m_sendEvent = Simulator::Schedule (m_interval, &TransactionalSender::SendPacket,
                                   this);

  } else if (packet_count == 11) {

    packet = Create<Packet> (34);   // sending 1st part of signature, 32 B + 2 B counter
    m_mac->Send (packet);
    NS_LOG_UNCOND ("Sent signature packet 1/2 of size " << packet->GetSize () << "B");
    ++packet_count;
    m_sendEvent = Simulator::Schedule (m_interval, &TransactionalSender::SendPacket,
                                     this);

  } else if (packet_count == 12) {

    packet = Create<Packet> (34);   // sending 2nd part of signature, 32 B + 2 B counter
    m_mac->Send (packet);
    NS_LOG_UNCOND ("Sent signature packet 2/2 of size " << packet->GetSize () << "B");
    packet_count = 0;
    m_sendEvent = Simulator::Schedule (m_interval, &TransactionalSender::SendPacket,
                                     this);

  } else {

    packet = Create<Packet> (42);   // sending data, 40 B + 2 B counter
    m_mac->Send (packet);
    NS_LOG_UNCOND ("Sent a data packet of size " << packet->GetSize () << "B");
    ++packet_count;
    m_sendEvent = Simulator::Schedule (m_interval, &TransactionalSender::SendPacket,
                                       this);
  }

  /*packet = Create<Packet> (32);

  m_mac->Send (packet);

  ++packet_count;

  NS_LOG_UNCOND("packet_count: " << (int)packet_count);

  // Schedule the next SendPacket event
  m_sendEvent = Simulator::Schedule (m_interval, &TransactionalSender::SendPacket,
                                     this);

  NS_LOG_DEBUG ("Sent a packet of size " << packet->GetSize ());*/
}

void
TransactionalSender::StartApplication (void)
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
                                     &TransactionalSender::SendPacket, this);
  NS_LOG_DEBUG ("Event Id: " << m_sendEvent.GetUid ());
}

void
TransactionalSender::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_sendEvent);
}

}
}
