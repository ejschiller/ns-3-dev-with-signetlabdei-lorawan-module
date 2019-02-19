/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/transactional-sender.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/lora-net-device.h"
#include "ns3/transactional-packet-header.h"

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
    .SetGroupName ("lorawan");/*
    .AddAttribute ("InterTransactionDelay", "The interval between two consecutive transactions in this app",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&TransactionalSender::GetInterTransactionDelay,
                                     &TransactionalSender::SetInterTransactionDelay),
                   MakeTimeChecker ())
     .AddAttribute ("IntraTransactionDelay", "The interval between two consecutive packets belonging to a transaction in this app.",
                    TimeValue (Seconds (0)),
                    MakeTimeAccessor (&TransactionalSender::GetIntraTransactionDelay,
                                      &TransactionalSender::SetIntraTransactionDelay),
                    MakeTimeChecker ())
     .AddAttribute ("PacketSizeRandomVariable", "The random variable that determines the shape of the packet size, in bytes",
                    StringValue ("ns3::UniformRandomVariable[Min=0,Max=10]"),
                    MakePointerAccessor (&TransactionalSender::m_pktSizeRV),
                    MakePointerChecker <RandomVariableStream>());*/
  return tid;
}

TransactionalSender::TransactionalSender () :
  m_initialDelay (Seconds (1)),
  interTransactionDelay (Hours (2)),
  intraTransactionDelay (Seconds (10)),
  dataPktSize(42),      // regular data packets, [40 B + 2 B] counter per packet
  sigPartPktSize(34),   // one half of signature, [32 B + 2 B] counter per part
  packetsPerTransaction(10),
  packet_count (0),
  transaction_count(0)

{
  NS_LOG_FUNCTION_NOARGS ();
}

TransactionalSender::~TransactionalSender ()
{
  NS_LOG_FUNCTION_NOARGS ();
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
TransactionalSender::SetPacketsPerTransaction (uint16_t packets)
{
  packetsPerTransaction = packets;
}

uint16_t
TransactionalSender::GetPacketsPerTransaction (void) const
{
  NS_LOG_FUNCTION (this);
  return packetsPerTransaction;
}

uint16_t
TransactionalSender::GetPacketCount (void) const {
  return packet_count;
}

void
TransactionalSender::SetPacketCount (uint16_t count) {
  packet_count = count;
}

uint16_t
TransactionalSender::GetTransactionCount (void) const {
  return transaction_count;
}

void
TransactionalSender::SetTransactionCount (uint16_t count) {
  transaction_count = count;
}


void
TransactionalSender::SendPacket (void)
{
  // If this is the first packet transmission, save the nodeUID for later use
  if(transaction_count == 0 && packet_count == 0) {
    nodeUID = m_mac->GetDevice ()->GetNode ()->GetId ();
  }

  NS_LOG_FUNCTION (this);


  Ptr<Packet> packet;

  // Preparing the Header to be put into the packet's payload
  TransactionalPacketHeader transactionalHeader;
  transactionalHeader.SetNodeUid(nodeUID);
  transactionalHeader.SetTransactionId(transaction_count);
  transactionalHeader.SetPacketId(packet_count);

  packet = Create<Packet> ();
  packet->AddHeader (transactionalHeader);

  // Getting the packet's current size in B
  uint32_t packetSize = packet->GetSize ();

  // Asserting, that the packet size caused by the header is not greater than
  // the default signature- resp. data-packet sizes.
  NS_ASSERT(packetSize <= sigPartPktSize && packetSize <= dataPktSize);

  if (packet_count == packetsPerTransaction) {
    // Filling the packet payload up with zeroes until the specified size.
    packet->AddPaddingAtEnd (sigPartPktSize - packetSize);
    m_mac->Send (packet);
    NS_LOG_DEBUG ("Sent signature packet 1/2 of size " << packet->GetSize () << " B");
    ++packet_count;

    /*TransactionalPacketHeader testHeaderReceive;
    packet->RemoveHeader (testHeaderReceive);

    NS_LOG_UNCOND("testHeaderReceive->GetNodeUid ()  = " << testHeaderReceive.GetNodeUid ());
    NS_LOG_UNCOND("testHeaderReceive->GetPacketId ()  = " << testHeaderReceive.GetPacketId ());
    NS_LOG_UNCOND("testHeaderReceive->GetTransactionId ()  = " << testHeaderReceive.GetTransactionId ());*/

    m_sendEvent = Simulator::Schedule (intraTransactionDelay, &TransactionalSender::SendPacket,
                                     this);

  } else if (packet_count == packetsPerTransaction + 1) {
    // resetting the counter (for the upcoming transaction)
    SetPacketCount(0);
    // incrementing the transaction number (id, counter)
    ++transaction_count;
    // Filling the packet payload up with zeroes until the specified size.
    packet->AddPaddingAtEnd (sigPartPktSize - packetSize);
    m_mac->Send (packet);
    NS_LOG_DEBUG ("Sent signature packet 2/2 of size " << packet->GetSize () << " B");
    // next transaction is scheduled after the inter-transaction delay
    m_sendEvent = Simulator::Schedule (interTransactionDelay, &TransactionalSender::SendPacket,
                                     this);

  } else if (packet_count == 0) {
      // Filling the packet payload up with zeroes until the specified size.
      // For some unknown reason, the packet size of the first packet of a transaction is always 9 B bigger...
      packet->AddPaddingAtEnd (dataPktSize - packetSize - 9);
      m_mac->Send (packet);
      NS_LOG_DEBUG ("Sent a data packet of size " << packet->GetSize () << " B, packet count: " << packet_count);
      ++packet_count;
      m_sendEvent = Simulator::Schedule (intraTransactionDelay, &TransactionalSender::SendPacket,
                                         this);

  } else {
    // Filling the packet payload up with zeroes until the specified size.
    packet->AddPaddingAtEnd (dataPktSize - packetSize);
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
