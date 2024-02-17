#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>


using namespace ns3;

double globaltotal = 0;
double globaltotalreceivepackets = 0;
double globalthroughput[50] = {0};//tony
double globalreceivepacket[50] = {0};//tony
//Configuration config;
/*
std::__cxx11::string getWifiMode(std::__cxx11::string dataMode) {
	if (dataMode == "MCS1_0")
		return "OfdmRate300KbpsBW1MHz";
	else if (dataMode == "MCS1_1")
		return "OfdmRate600KbpsBW1MHz";
	else if (dataMode == "MCS1_2")
		return "OfdmRate900KbpsBW1MHz";
	else if (dataMode == "MCS1_3")
		return "OfdmRate1_2MbpsBW1MHz";
	else if (dataMode == "MCS1_4")
		return "OfdmRate1_8MbpsBW1MHz";
	else if (dataMode == "MCS1_5")
		return "OfdmRate2_4MbpsBW1MHz";
	else if (dataMode == "MCS1_6")
		return "OfdmRate2_7MbpsBW1MHz";
	else if (dataMode == "MCS1_7")
		return "OfdmRate3MbpsBW1MHz";
	else if (dataMode == "MCS1_8")
		return "OfdmRate3_6MbpsBW1MHz";
	else if (dataMode == "MCS1_9")
		return "OfdmRate4MbpsBW1MHz";
	else if (dataMode == "MCS1_10")
		return "OfdmRate150KbpsBW1MHz";

	else if (dataMode == "MCS2_0")
		return "OfdmRate650KbpsBW2MHz";
	else if (dataMode == "MCS2_1")
		return "OfdmRate1_3MbpsBW2MHz";
	else if (dataMode == "MCS2_2")
		return "OfdmRate1_95MbpsBW2MHz";
	else if (dataMode == "MCS2_3")
		return "OfdmRate2_6MbpsBW2MHz";
	else if (dataMode == "MCS2_4")
		return "OfdmRate3_9MbpsBW2MHz";
	else if (dataMode == "MCS2_5")
		return "OfdmRate5_2MbpsBW2MHz";
	else if (dataMode == "MCS2_6")
		return "OfdmRate5_85MbpsBW2MHz";
	else if (dataMode == "MCS2_7")
		return "OfdmRate6_5MbpsBW2MHz";
	else if (dataMode == "MCS2_8")
		return "OfdmRate7_8MbpsBW2MHz";
	return "";
}
*/

ApplicationContainer gencbr(NodeContainer server, NodeContainer client, Ipv4Address address, double simulationTime, double starttime)
{
	UdpServerHelper myServer(12345); //創造一個監聽port:12345的udp_server_helper
	ApplicationContainer serverApp = myServer.Install(server); //在給定的server node安裝一個udp_server_app
	serverApp.Start(Seconds(0.0));
	serverApp.Stop(Seconds(simulationTime + 1));

	UdpClientHelper myClient(address, 12345);
	myClient.SetAttribute("MaxPackets", UintegerValue(4294967295u));
	myClient.SetAttribute("Interval", TimeValue(Time("0.00001"))); //packets/s
	myClient.SetAttribute("PacketSize", UintegerValue(1472));

	ApplicationContainer clientApp = myClient.Install(client); //在給定的client node安裝一個udp_client_app
	clientApp.Start(Seconds(1.0 + starttime));
	clientApp.Stop(Seconds(simulationTime + 1));

	UdpEchoClientHelper echoClientHelper(address, 9);
	echoClientHelper.SetAttribute("MaxPackets", UintegerValue(1));
	echoClientHelper.SetAttribute("Interval", TimeValue(Seconds(0.1)));
	echoClientHelper.SetAttribute("PacketSize", UintegerValue(10));
	ApplicationContainer pingApps;

	echoClientHelper.SetAttribute("StartTime", TimeValue(Seconds(starttime / 10)));
	pingApps.Add(echoClientHelper.Install(client)); //1

	return serverApp; //回傳udp_server_app
}

double run(double simulationTime, double range, double radius, int nodeNum, double cca)
{
	double a[nodeNum / 2];
	int nodearray[nodeNum];
	double temp;
	int i;
	int c, d;
	double x, y, x1, y1;
	double r = radius;
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue(0));//(999999));
	Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", UintegerValue(999999));

	NodeContainer nodes;
	nodes.Create(nodeNum);

	YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
	YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default();
	YansWifiChannelHelper channel3 = YansWifiChannelHelper::Default();

	YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
	phy.SetErrorRateModel("ns3::YansErrorRateModel");
	//phy.Set("RtsCtsThreshold", UintegerValue(0));
	phy.Set("CcaMode1Threshold", DoubleValue(-82.0));
	//phy.Set("CcaMode1Threshold", DoubleValue(-9999999999999999999999999999.0));
	phy.Set("EnergyDetectionThreshold", DoubleValue(-79.0));
	phy.Set("S1g1MfieldEnabled", BooleanValue(true));
	//phy.Set("ShortGuardEnabled", BooleanValue(false));
	//phy.Set("ChannelWidth", UintegerValue(1)); // changed
	/*
	phy.Set("TxGain", DoubleValue(0.0));
	phy.Set("RxGain", DoubleValue(0.0));
	phy.Set("TxPowerLevels", UintegerValue(1));
	phy.Set("TxPowerEnd", DoubleValue(0.0));
	phy.Set("TxPowerStart", DoubleValue(0.0));
	phy.Set("RxNoiseFigure", DoubleValue(6.8));
	phy.Set("LdpcEnabled", BooleanValue(true));
	*/

	YansWifiPhyHelper phy2 = YansWifiPhyHelper::Default();
	phy2.SetErrorRateModel("ns3::YansErrorRateModel");
	//phy2.Set("RtsCtsThreshold", UintegerValue(9999999999999999));
	phy2.Set("CcaMode1Threshold", DoubleValue(-82.0));
	//phy2.Set("CcaMode1Threshold", DoubleValue(-82.0));
	phy2.Set("EnergyDetectionThreshold", DoubleValue(-79.0));
	phy2.Set("S1g1MfieldEnabled", BooleanValue(true));
	//phy2.Set("ShortGuardEnabled", BooleanValue(false));
	//phy2.Set("ChannelWidth", UintegerValue(1)); // changed
	/*
	phy2.Set("TxGain", DoubleValue(0.0));
	phy2.Set("RxGain", DoubleValue(0.0));
	phy2.Set("TxPowerLevels", UintegerValue(1));
	phy2.Set("TxPowerEnd", DoubleValue(0.0));
	phy2.Set("TxPowerStart", DoubleValue(0.0));
	phy2.Set("RxNoiseFigure", DoubleValue(6.8));
	phy2.Set("LdpcEnabled", BooleanValue(true));
	*/

	YansWifiPhyHelper phy3 = YansWifiPhyHelper::Default();
	phy3.SetErrorRateModel("ns3::YansErrorRateModel");
	//phy3.Set("RtsCtsThreshold", UintegerValue(9999999999999999));
	phy3.Set("CcaMode1Threshold", DoubleValue(-82.0));
	//phy3.Set("CcaMode1Threshold", DoubleValue(-82.0));
	phy3.Set("EnergyDetectionThreshold", DoubleValue(-79.0));
	phy3.Set("S1g1MfieldEnabled", BooleanValue(true));
	//phy3.Set("ShortGuardEnabled", BooleanValue(false));
	//phy3.Set("ChannelWidth", UintegerValue(1)); // changed
	/*
	phy3.Set("TxGain", DoubleValue(0.0));
	phy3.Set("RxGain", DoubleValue(0.0));
	phy3.Set("TxPowerLevels", UintegerValue(1));
	phy3.Set("TxPowerEnd", DoubleValue(0.0));
	phy3.Set("TxPowerStart", DoubleValue(0.0));
	phy3.Set("RxNoiseFigure", DoubleValue(6.8));
	phy3.Set("LdpcEnabled", BooleanValue(true));
	*/

	phy.SetChannel(channel.Create());
	phy2.SetChannel(channel2.Create());
	phy3.SetChannel(channel3.Create());

	WifiHelper wifi;
	wifi.SetStandard(WIFI_PHY_STANDARD_80211ah);
	S1gWifiMacHelper mac = S1gWifiMacHelper::Default();
	S1gWifiMacHelper mac2 = S1gWifiMacHelper::Default();
	S1gWifiMacHelper mac3 = S1gWifiMacHelper::Default();

	StringValue DataRate = StringValue("OfdmRate1_2MbpsBW1MHz"); //Blue
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", DataRate,
		"ControlMode", DataRate);

	mac.SetType("ns3::AdhocWifiMac"); // use ad-hoc MAC
	//wifiMac.SetType("ns3::AdhocWifiMac", "S1gSupported", BooleanValue(true), "HtSupported", BooleanValue(true));
	mac2.SetType("ns3::AdhocWifiMac"); // use ad-hoc MAC
	mac3.SetType("ns3::AdhocWifiMac"); // use ad-hoc MAC

	//NetDeviceContainer devices = wifi.Install (phy, mac, nodes);
	phy2.Set("ChannelNumber", UintegerValue(58));
	phy3.Set("ChannelNumber", UintegerValue(60));
	NetDeviceContainer devices = wifi.Install(phy, mac, phy2, mac2, phy3, mac3, nodes);
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
	int area_size = range / 25;
	int area_y = range / (nodeNum / 50);
	for (i = 0; i < nodeNum / 2; i++)
	{
		int count = i / (nodeNum / 50);
		int upperbound_x = area_size * (count + 1);
		int lowerbound_x = area_size * count;
		int upperbound_y = range, lowerbound_y = 0;

		lowerbound_y = i % (nodeNum / 50) * area_y;
		upperbound_y = (i % (nodeNum / 50) + 1) * area_y;


		x = double(upperbound_x - lowerbound_x) * rand() / (RAND_MAX)+lowerbound_x;
		y = double(upperbound_y - lowerbound_y) * rand() / (RAND_MAX)+lowerbound_y;
		do
		{
			/* code */
			r = r * 0.3;
			x1 = (2 * r) * rand() / (RAND_MAX)+x - r;
			temp = sqrt(r * r - (x1 - x) * (x1 - x));
			y1 = (2 * temp) * rand() / (RAND_MAX)+y - temp;
		} while ((x1<0 || x1>range) || (y1<0 || y1>range) || ((x1 - x) * (x1 - x) + (y1 - y) * (y1 - y) < (r / 2) * (r / 2)));


		///std::cout<<x<<" "<<y<<" "<<x1<<" "<<y1<<" "<<sqrt((y1-y)*(y1-y) + (x1-x)*(x1-x))<<std::endl;
		positionAlloc->Add(Vector(x, y, 0.0));
		positionAlloc->Add(Vector(x1, y1, 0.0));
	}
	mobility.SetPositionAllocator(positionAlloc);
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(nodes);

	InternetStackHelper internet;
	internet.Install(nodes);
	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer NodeInterface = ipv4.Assign(devices);
	ipv4.SetBase("10.1.10.0", "255.255.255.0");
	//Ipv4InterfaceContainer j = ipv4.Assign (devices2);

	for (i = 0; i < nodeNum / 2; ++i)
	{
		a[i] = 0.001 + 0.0005 * i;
	}
	for (i = 0; i < nodeNum / 2; ++i)
	{
		c = rand() % (nodeNum / 2);
		d = rand() % (nodeNum / 2);
		temp = a[c];
		a[c] = a[d];
		a[d] = temp;
	}

	for (i = 0; i < nodeNum; ++i)
	{
		nodearray[i] = i;
	}

	ApplicationContainer cbrApps;

	for (i = 0; i < nodeNum;)
	{
		cbrApps.Add(gencbr(nodes.Get(nodearray[i]), nodes.Get(nodearray[i + 1]), NodeInterface.GetAddress(nodearray[i]), simulationTime, a[i]));
		i = i + 2;
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	Simulator::Stop(Seconds(simulationTime + 1));
	Simulator::Run();

	double throughput = 0, totalthroughput = 0, totalreceivepacket = 0;
	for (size_t i = 0; i < cbrApps.GetN(); ++i)
	{
		double totalPacketsThrough = DynamicCast<UdpServer>(cbrApps.Get(i))->GetReceived();
		//uint32_t totalPacketsLost = DynamicCast<UdpServer> (cbrApps.Get (i))->GetLost ();
		throughput = totalPacketsThrough * 1472 * 8 / (simulationTime * 1000000.0); //Mbit/s
			//std::cout << "Flow " << i << std::endl;
			//std::cout << "  Rx Packets:   " << totalPacketsThrough << "\n";
			//std::cout << "  Rx Bytes:   " << totalPacketsThrough * 1472 << "\n";
		//std::cout << "  Throughput: " << throughput  << " Mbps\n";
			//std::cout << "  Lost Packet "<< totalPacketsLost <<"\n";
		totalthroughput += throughput;
		totalreceivepacket += totalPacketsThrough;
		globalreceivepacket[i] += totalPacketsThrough;
		globalthroughput[i] += throughput;
	}
	globaltotalreceivepackets += totalreceivepacket;
	globaltotal += totalthroughput;
	//std::cout << "  totalThroughput: " << totalthroughput  << " Mbps\n";
	std::cout << "  totalreceivepacket: " << totalreceivepacket << std::endl;;

	Simulator::Destroy();
	return totalthroughput;
}

int main(int argc, char** argv)
{
	double simulationTime = 1;//s
	double traintime = 1, range = 3000.0, radius = 100.0;
	int nodeNum = 50;//tony
	double cca = -112;
	srand(time(NULL));
	CommandLine cmd;
	cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
	cmd.AddValue("traintime", "Simulation time in seconds", traintime);
	cmd.AddValue("range", "Simulation time in seconds", range);
	cmd.AddValue("radius", "Simulation time in seconds", radius);
	cmd.AddValue("nodeNum", "Simulation time in seconds", nodeNum);
	cmd.AddValue("cca", "Simulation time in seconds", cca);
	cmd.Parse(argc, argv);

	clock_t t1, t2;
	t1 = clock();

	int i;
	for (i = 0; i < traintime; i++)
	{
		std::cout << i << std::endl;
		run(simulationTime, range, radius, nodeNum, cca);
	}
	/*for(i=0;i<nodeNum/2;i++)
	{
	std::cout<<"Flow "<<i<<" : "<<std::endl;
	std::cout<<"  AverageThroughput :   "<<globalthroughput[i]/traintime<<" Mbps "<<std::endl;
	std::cout<<"  Average RxPackets :   "<<globalreceivepacket[i]/traintime<<std::endl;
	std::cout<<"  Average RxBytes   :   "<<globalreceivepacket[i]*1472/traintime<<std::endl;
	}*/
	std::cout << std::endl;
	std::cout << "  Totalthrouhput    :   " << globaltotal / traintime << " Mbps " << std::endl;
	std::cout << "  TotalRxpackets    :   " << globaltotalreceivepackets / traintime << std::endl;
	t2 = clock();

	std::ofstream file_out;
	file_out.open("data.txt", std::ios_base::app);
	file_out << globaltotalreceivepackets / traintime << "\n";
	std::cout << "Done !\n";

	printf("%lf sec\n", (t2 - t1) / (double)(CLOCKS_PER_SEC));
	return 0;
}
