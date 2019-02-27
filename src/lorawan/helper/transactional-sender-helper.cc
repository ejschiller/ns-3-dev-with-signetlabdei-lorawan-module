/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/transactional-sender-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/transactional-sender.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("TransactionalSenderHelper");

TransactionalSenderHelper::TransactionalSenderHelper () :
    dataPktSize(0),
    sigPartPktSize(0),
    packetsPerTransaction(0),
    interTransactionDelay(Seconds(0)),
    intraTransactionDelay(Seconds(0))
{
  m_factory.SetTypeId ("ns3::TransactionalSender");

  // m_factory.Set ("PacketSizeRandomVariable", StringValue
  //                  ("ns3::ParetoRandomVariable[Bound=10|Shape=2.5]"));

  m_initialDelayRV = CreateObject<UniformRandomVariable> ();
  m_initialDelayRV->SetAttribute ("Min", DoubleValue (0));

}

TransactionalSenderHelper::~TransactionalSenderHelper ()
{
}

void
TransactionalSenderHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
TransactionalSenderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
TransactionalSenderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
TransactionalSenderHelper::InstallPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  Ptr<TransactionalSender> app = m_factory.Create<TransactionalSender> ();

  app->SetInitialDelay (Seconds (m_initialDelayRV->GetValue (0, interTransactionDelay.GetSeconds ())));

  if(dataPktSize != 0) app->SetDataPacketSize(dataPktSize);
  if(sigPartPktSize != 0) app->SetPartialSignaturePacketSize(sigPartPktSize);
  if(packetsPerTransaction != 0) app->SetPacketsPerTransaction(packetsPerTransaction);
  if(intraTransactionDelay != Seconds(0)) app->SetIntraTransactionDelay(intraTransactionDelay);
  if(interTransactionDelay != Seconds(0)) app->SetInterTransactionDelay(interTransactionDelay);

  app->SetNode (node);
  node->AddApplication (app);

  return app;
}

void
TransactionalSenderHelper::SetDataPacketSize (uint8_t dataSize) {
  dataPktSize = dataSize;
}

uint8_t
TransactionalSenderHelper::GetDataPacketSize (void) const
{
  NS_LOG_FUNCTION (this);
  return dataPktSize;
}

void
TransactionalSenderHelper::SetPartialSignaturePacketSize (uint8_t sigSize)
{
  sigPartPktSize = sigSize;
}

uint8_t
TransactionalSenderHelper::GetPartialSignaturePacketSize (void) const
{
  NS_LOG_FUNCTION (this);
  return sigPartPktSize;
}

void
TransactionalSenderHelper::SetPacketsPerTransaction (uint16_t packets)
{
  packetsPerTransaction = packets;
}

uint16_t
TransactionalSenderHelper::GetPacketsPerTransaction (void) const
{
  NS_LOG_FUNCTION (this);
  return packetsPerTransaction;
}

void
TransactionalSenderHelper::SetInterTransactionDelay (Time interDelay)
{
  NS_LOG_FUNCTION (this << interDelay);
  interTransactionDelay = interDelay;
}


Time
TransactionalSenderHelper::GetInterTransactionDelay (void) const
{
  NS_LOG_FUNCTION (this);
  return interTransactionDelay;
}


void
TransactionalSenderHelper::SetIntraTransactionDelay (Time intraDelay)
{
  NS_LOG_FUNCTION (this << intraDelay);
  intraTransactionDelay = intraDelay;
}


Time
TransactionalSenderHelper::GetIntraTransactionDelay (void) const
{
  NS_LOG_FUNCTION (this);
  return intraTransactionDelay;
}

}
} // namespace ns3
