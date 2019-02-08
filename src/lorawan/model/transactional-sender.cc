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
  interTransactionDelay (Hours (2)),
  intraTransactionDelay (Seconds (10)),
  m_basePktSize (10),
  dataPktSize(42),      // regular data packets, [40 B + 2 B] counter per packet
  sigPartPktSize(34),   // one half of signature, [32 B + 2 B] counter per part
  packetsPerTransaction(10),
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
TransactionalSender::SetInterTransactionDelay (Time interDelay)
{
  NS_LOG_FUNCTION (this << interDelay);
  interTransactionDelay = interDelay;
}


Time
TransactionalSender::GetInterTransactionDelay (void) const
{
  NS_LOG_FUNCTION (this);
  return interTransactionDelay;
}


void
TransactionalSender::SetIntraTransactionDelay (Time intraDelay)
{
  NS_LOG_FUNCTION (this << intraDelay);
  intraTransactionDelay = intraDelay;
}


Time
TransactionalSender::GetIntraTransactionDelay (void) const
{
  NS_LOG_FUNCTION (this);
  return intraTransactionDelay;
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

void
TransactionalSender::SetDataPacketSize (uint8_t dataSize) {
  dataPktSize = dataSize;
}

uint8_t
TransactionalSender::GetDataPacketSize (void) const
{
  NS_LOG_FUNCTION (this);
  return dataPktSize;
}

void
TransactionalSender::SetPartialSignaturePacketSize (uint8_t sigSize)
{
  sigPartPktSize = sigSize;
}

uint8_t
TransactionalSender::GetPartialSignaturePacketSize (void) const
{
  NS_LOG_FUNCTION (this);
  return sigPartPktSize;
}

void
TransactionalSender::SetPacketsPerTransaction (uint32_t packets)
{
  packetsPerTransaction = packets;
}

uint32_t
TransactionalSender::GetPacketsPerTransaction (void) const
{
  NS_LOG_FUNCTION (this);
  return packetsPerTransaction;
}

uint32_t
TransactionalSender::GetPacketCount (void) const {
  return packet_count;
}


void
TransactionalSender::SetPacketCount (uint32_t count) {
  packet_count = count;
}


void
TransactionalSender::SendPacket (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet;

  if (packet_count == packetsPerTransaction) {

    packet = Create<Packet> (sigPartPktSize);
    m_mac->Send (packet);
    NS_LOG_DEBUG ("Sent signature packet 1/2 of size " << packet->GetSize () << " B");
    ++packet_count;
    m_sendEvent = Simulator::Schedule (intraTransactionDelay, &TransactionalSender::SendPacket,
                                     this);

  } else if (packet_count == packetsPerTransaction + 1) {

    SetPacketCount(0);
    packet = Create<Packet> (sigPartPktSize);
    m_mac->Send (packet);
    NS_LOG_DEBUG ("Sent signature packet 2/2 of size " << packet->GetSize () << " B");
    // resetting the counter (for the upcoming transaction)
    // next transaction is scheduled after the inter-transaction delay
    m_sendEvent = Simulator::Schedule (interTransactionDelay, &TransactionalSender::SendPacket,
                                     this);

  } else if (packet_count == 0) {

      // For some unknown reason, the packet size of the first packet of a transaction is always 9 B bigger...
      packet = Create<Packet> (dataPktSize-9);
      m_mac->Send (packet);
      NS_LOG_DEBUG ("Sent a data packet of size " << packet->GetSize () << " B, packet count: " << packet_count);
      ++packet_count;
      m_sendEvent = Simulator::Schedule (intraTransactionDelay, &TransactionalSender::SendPacket,
                                         this);

  } else {

    packet = Create<Packet> (dataPktSize);
    m_mac->Send (packet);
    NS_LOG_DEBUG ("Sent a data packet of size " << packet->GetSize () << " B, packet count: " << packet_count);
    ++packet_count;
    m_sendEvent = Simulator::Schedule (intraTransactionDelay, &TransactionalSender::SendPacket,
                                       this);
  }

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
