#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/seq-ts-header.h"

#include <vector>
#include <iostream>
#include <algorithm>

#define u32 uint32_t
#define u16 uint16_t

using namespace ns3;

static std::vector<std::vector<Time>> g_delays_by_reducer;

static void RxTracer(u32 reducer_idx, Ptr<const Packet> p, const Address &from, const Address &local)
{
    Ptr<Packet> copy = p->Copy();
    SeqTsHeader seq_ts;
    copy->RemoveHeader(seq_ts);

    Time tx_time = seq_ts.GetTs();
    Time rx_time = Simulator::Now();
    Time delay = rx_time - tx_time;

    g_delays_by_reducer[reducer_idx].push_back(delay);
}

static std::vector<double> zipf_cfd(u32 m, double alpha)
{
    std::vector<double> cdf(m);
    if (m == 0)
    {
        return cdf;
    }

    double sum = 0.0f;
    for (u32 k = 0; k < m; ++k)
    {
        double w = 1.0f / std::pow(static_cast<double>(k + 1), alpha);
        cdf[k] = w;
        sum += w;
    }

    double acc = 0.0f;
    for (u32 k = 0; k < m; k++)
    {
        acc += cdf[k] / sum;
        cdf[k] = acc;
    }

    cdf[m - 1] = 1.0f;
    return cdf;
}

static u32 zipf_pick(const std::vector<double> &cdf, double u)
{
    u32 low = 0;
    u32 high = static_cast<u32>(cdf.size() - 1);
    while (low < high)
    {
        u32 mid = (low + high) / 2u;
        if (u <= cdf[mid])
        {
            high = mid;
        }
        else
        {
            low = mid + 1;
        }
    }

    return low;
}

int main(int argc, char *argv[])
{
    u32 n_mappers = 4;
    u32 n_reducers = 4;
    u32 ops_per_mapper = 1000;
    double zipf_alpha = 1.2;
    // TODO: make variable
    u32 packet_size = 512;
    std::string link_data_rate = "100.0Gbps";
    std::string link_delay = "100us";
    double stop_time = 10.0f;
    u32 seed = 42;

    CommandLine cmd(__FILE__);
    cmd.AddValue("n_mappers", "number of mapper nodes", n_mappers);
    cmd.AddValue("n_reducers", "number of reducer nodes", n_reducers);
    cmd.AddValue("ops_per_mapper", "operations per mapper node", ops_per_mapper);
    cmd.AddValue("zipf_alpha", "alpha for zipf function", zipf_alpha);
    cmd.AddValue("packet_size", "packet size", packet_size);
    cmd.AddValue("link_data_rate", "data transfer rate for links", link_data_rate);
    cmd.AddValue("link_delay", "csma delay (uniform sending tax)", link_delay);
    cmd.AddValue("stop_time", "stop time (in seconds)", stop_time);
    cmd.AddValue("seed", "seed for random operations", seed);
    cmd.Parse(argc, argv);

    NS_ABORT_IF(n_mappers == 0 || n_reducers == 0);
    g_delays_by_reducer.assign(n_reducers, std::vector<Time>{});

    RngSeedManager::SetSeed(seed);
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
    uv->SetStream(1);

    u32 total_nodes = n_mappers + n_reducers;
    NodeContainer nodes;
    nodes.Create(total_nodes);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue(link_data_rate));
    csma.SetChannelAttribute("Delay", TimeValue(Time(link_delay)));
    csma.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1000p"));

    NetDeviceContainer devices = csma.Install(nodes);

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    u16 base_port = 9000;
    std::vector<Ptr<UdpServer>> reducer_servers;
    reducer_servers.reserve(n_reducers);
    for (u32 i = 0; i < n_reducers; ++i)
    {
        u32 node_idx = n_mappers + i;
        NodeContainer nc(nodes.Get(node_idx));
        UdpServerHelper server_helper(base_port + i);
        ApplicationContainer server_apps = server_helper.Install(nc);
        server_apps.Start(Seconds(0.1 + 0.01 * i));
        server_apps.Stop(Seconds(stop_time));

        Ptr<UdpServer> server = DynamicCast<UdpServer>(server_apps.Get(0));
        server->TraceConnectWithoutContext("RxWithAddresses", MakeBoundCallback(&RxTracer, i));
        reducer_servers.push_back(server);
    }

    if (zipf_alpha < 0.0f)
    {
        zipf_alpha = 0.0f;
    }

    std::vector<double> cdf = zipf_cfd(n_reducers, zipf_alpha);
    std::vector<u32> planned_reducer_ops(n_reducers, 0);
    std::vector<std::vector<u32>> op_mappings(n_mappers, std::vector<u32>(n_reducers, 0));

    for (u32 i = 0; i < n_mappers; ++i)
    {
        for (u32 j = 0; j < ops_per_mapper; ++j)
        {
            double u = uv->GetValue();
            u32 reducer_idx = zipf_pick(cdf, u);
            op_mappings[i][reducer_idx] += 1;
            planned_reducer_ops[reducer_idx] += 1;
        }
    }

    for (u32 i = 0; i < n_mappers; i++)
    {
        Ptr<Node> mapper = nodes.Get(i);
        double start = 1.0 + 0.01 * static_cast<double>(i);
        double end = stop_time;
        double span = std::max(0.1, end - start);

        for (u32 j = 0; j < n_reducers; j++)
        {
            u32 count = op_mappings[i][j];
            // early exit if no ops mapped
            if (count <= 0)
            {
                continue;
            }

            Ipv4Address dst_addr = interfaces.GetAddress(n_mappers + j);
            u16 dst_port = base_port + j;

            UdpClientHelper client(dst_addr, dst_port);
            client.SetAttribute("MaxPackets", UintegerValue(count));

            double interval = span / static_cast<double>(count);
            // floor at 10us
            interval = std::max(1e-5, interval);

            client.SetAttribute("Interval", TimeValue(Seconds(interval)));
            client.SetAttribute("PacketSize", UintegerValue(packet_size));

            ApplicationContainer client_apps = client.Install(mapper);
            client_apps.Start(Seconds(start));
            client_apps.Stop(Seconds(end));
        }
    }

    // end simulation setup
    Simulator::Stop(Seconds(stop_time));
    Simulator::Run();

    std::vector<Time> max_delays;
    std::vector<Time> delays;
    Time overall_delay = Seconds(0);

    for (u32 j = 0; j < n_reducers; ++j)
    {
        const auto &vec = g_delays_by_reducer[j];
        if (vec.empty())
        {
            continue;
        }

        Time sum = Seconds(0);
        Time max_delay = Seconds(0);

        for (auto d : vec)
        {
            sum += d;
            if (d > max_delay)
            {
                max_delay = d;
            }
        }

        max_delays.push_back(max_delay);
        delays.push_back(sum);

        std::cout << "Reducer " << j << " max delay: " << max_delay.GetNanoSeconds() << " ns over " << vec.size() << " packets." << std::endl;
    }

    for (u32 i = 0; i < delays.size(); ++i)
    {
        if (delays[i] > overall_delay)
        {
            overall_delay = delays[i];
        }
    }

    std::vector<u32> received_ops(n_reducers, 0);
    for (u32 i = 0; i < n_reducers; ++i)
    {
        received_ops[i] = reducer_servers[i]->GetReceived();
    }

    Simulator::Destroy();

    std::cout << "Statistics:" << std::endl;
    std::cout << "Mapper count: " << n_mappers << std::endl;
    std::cout << "Reducer count: " << n_reducers << std::endl;
    std::cout << "Ops/mapper count: " << ops_per_mapper << std::endl;
    std::cout << "Total operations" << n_mappers * ops_per_mapper << std::endl;
    std::cout << "Zipf alpha: " << zipf_alpha << std::endl;
    std::cout << "Packet size: " << packet_size << std::endl;
    std::cout << "Link transfer rate: " << link_data_rate << std::endl;
    std::cout << "Runtime: " << overall_delay.GetMilliSeconds() << std::endl;

    std::cout << "Reducer\tPlanned\tReceived" << std::endl;
    for (u32 i = 0; i < n_reducers; ++i)
    {
        std::cout << i << "\t" << planned_reducer_ops[i] << "\t" << received_ops[i] << std::endl;
    }
    std::cout << std::endl;

    return 0;
}
