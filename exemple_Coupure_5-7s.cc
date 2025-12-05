// partie1.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/netanim-module.h"              // Pour utiliser NetAnim
#include "ns3/ndnSIM/helper/ndn-link-control-helper.hpp"

namespace ns3 {

int
main(int argc, char* argv[])
{
  // Définir les paramètres par défaut pour les liens PointToPoint
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("10p"));

  std::string animfile = "AnimPartie1.xml";

  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Créer la topologie : grille 3x4
  PointToPointHelper p2p;
  PointToPointGridHelper grid(3, 4, p2p);
  grid.BoundingBox(100, 100, 200, 200);

  // Installer la pile ndn sur tous les nœuds
  ndn::StackHelper ndnHelper;
  ndnHelper.InstallAll();

  // Choix des nœuds producteur / consommateurs
  Ptr<Node> producer  = grid.GetNode(2, 0);  // nœud 8
  Ptr<Node> consumer1 = grid.GetNode(1, 1);  // nœud 5
  Ptr<Node> consumer2 = grid.GetNode(1, 2);  // nœud 6
  Ptr<Node> consumer3 = grid.GetNode(0, 0);  // nœud 0

  // Préfixe
  std::string prefix = "/ETS";

  // Applications consommateurs (ConsumerBatches)
  ndn::AppHelper cHelper("ns3::ndn::ConsumerBatches");
  cHelper.SetPrefix(prefix);

  // C1 : 1s 1 paquet, 4s 2 paquets, 5s 1 paquet
  cHelper.SetAttribute("Batches", StringValue("1s 1 4s 2 5s 1"));
  cHelper.Install(consumer1);

  // C2 : 2s 1 paquet, 3s 2 paquets
  cHelper.SetAttribute("Batches", StringValue("2s 1 3s 2"));
  cHelper.Install(consumer2);

  // C3 : 1s 1, 2s 1, 4s 1, 5s 1
  cHelper.SetAttribute("Batches", StringValue("1s 1 2s 1 4s 1 5s 1"));
  cHelper.Install(consumer3);

  // Application producteur
  ndn::AppHelper pHelper("ns3::ndn::Producer");
  pHelper.SetPrefix(prefix);
  pHelper.SetAttribute("PayloadSize", StringValue("1024"));
  pHelper.Install(producer);

  // Routage global
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();
  ndnGlobalRoutingHelper.AddOrigins(prefix, producer);
  ndnGlobalRoutingHelper.CalculateRoutes();

  // Coupure de lien entre les nœuds (1,0) et (2,0) = nœuds 4 et 8
  Simulator::Schedule(Seconds(5.0),
                      ndn::LinkControlHelper::FailLink,
                      grid.GetNode(1,0), grid.GetNode(2,0));

  Simulator::Schedule(Seconds(7.0),
                      ndn::LinkControlHelper::UpLink,
                      grid.GetNode(1,0), grid.GetNode(2,0));

  // Arrêt de la simulation
  Simulator::Stop(Seconds(10.0));

  // NetAnim
  AnimationInterface anim(animfile);
  // anim.EnablePacketMetadata();

  // Traces ASCII
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("asciiPartie1.tr"));

  // Lancer la simulation
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
