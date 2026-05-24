import { useState } from "react";
import { Link } from "react-router-dom";
import { ChevronLeft, Search, Plus, Check, Wifi, WifiOff } from "lucide-react";

interface Network {
  name: string;
  chainId: string;
  status: "online" | "degraded" | "offline";
  gas: string;
}

interface NetworkGroup {
  category: string;
  networks: Network[];
}

const NETWORKS: NetworkGroup[] = [
  {
    category: "ZK Networks",
    networks: [
      { name: "zkSync Era", chainId: "324", status: "online", gas: "0.25 Gwei" },
      { name: "Scroll", chainId: "534352", status: "online", gas: "0.18 Gwei" },
      { name: "Linea", chainId: "59144", status: "online", gas: "0.32 Gwei" },
      { name: "Polygon zkEVM", chainId: "1101", status: "degraded", gas: "0.45 Gwei" },
    ],
  },
  {
    category: "L1 Networks",
    networks: [
      { name: "Ethereum", chainId: "1", status: "online", gas: "12.5 Gwei" },
      { name: "BNB Chain", chainId: "56", status: "online", gas: "3.2 Gwei" },
    ],
  },
  {
    category: "Testnets",
    networks: [
      { name: "Sepolia", chainId: "11155111", status: "online", gas: "0.01 Gwei" },
      { name: "Goerli", chainId: "5", status: "offline", gas: "—" },
    ],
  },
];

export default function Networks() {
  const [search, setSearch] = useState("");
  const [activeNetwork, setActiveNetwork] = useState("zkSync Era");
  const [showAddCustom, setShowAddCustom] = useState(false);

  const allNetworks: Network[] = NETWORKS.flatMap((g) => g.networks);
  const filteredGroups: NetworkGroup[] = NETWORKS.map((group) => ({
    category: group.category,
    networks: group.networks.filter((n: Network) =>
      n.name.toLowerCase().includes(search.toLowerCase())
    ),
  })).filter((group) => group.networks.length > 0);

  return (
    <div className="min-h-screen bg-void px-4 py-6">
      <div className="max-w-lg mx-auto">
        {/* Header */}
        <div className="flex items-center justify-between mb-8">
          <div className="flex items-center gap-3">
            <Link to="/settings" className="text-text-secondary hover:text-text-primary">
              <ChevronLeft size={20} />
            </Link>
            <h1 className="font-heading text-xl text-text-primary">Select Network</h1>
          </div>
          <button
            onClick={() => setShowAddCustom(!showAddCustom)}
            className="flex items-center gap-1 text-cyan font-mono text-xs hover:opacity-80"
          >
            <Plus size={12} />
            Custom
          </button>
        </div>

        {/* Search */}
        <div className="relative mb-6">
          <Search size={16} className="absolute left-3 top-1/2 -translate-y-1/2 text-text-tertiary" />
          <input
            type="text"
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            className="w-full bg-obsidian border border-white/10 pl-10 pr-4 py-2 font-mono text-sm text-text-primary placeholder:text-text-tertiary focus:outline-none focus:border-cyan"
            placeholder="Search networks..."
          />
        </div>

        {/* Active Network */}
        <div className="mb-6 p-4 bg-cyan/5 border border-cyan/20">
          <p className="font-mono text-[10px] text-cyan mb-1">Active Network</p>
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <div className="w-8 h-8 rounded-full bg-cyan/20 flex items-center justify-center">
                <Wifi size={14} className="text-cyan" />
              </div>
              <div>
                <p className="font-body text-sm text-text-primary">{activeNetwork}</p>
                <p className="font-mono text-[10px] text-text-secondary">
                  Chain ID: {allNetworks.find((n: Network) => n.name === activeNetwork)?.chainId}
                </p>
              </div>
            </div>
            <Check size={16} className="text-cyan" />
          </div>
        </div>

        {/* Network List */}
        {filteredGroups.map((group) => (
          <div key={group.category} className="mb-6">
            <h3 className="font-mono text-xs text-text-secondary uppercase tracking-wider mb-3">
              {group.category}
            </h3>
            <div className="space-y-1">
              {group.networks.map((network) => (
                <button
                  key={network.name}
                  onClick={() => network.status !== "offline" && setActiveNetwork(network.name)}
                  className={`w-full flex items-center justify-between p-4 bg-obsidian border transition-colors ${
                    activeNetwork === network.name ? "border-cyan/30" : "border-white/5 hover:border-white/10"
                  }`}
                >
                  <div className="flex items-center gap-3">
                    <div className={`w-8 h-8 rounded-full flex items-center justify-center ${
                      network.status === "online" ? "bg-cyan/10" : network.status === "degraded" ? "bg-amber/10" : "bg-red/10"
                    }`}>
                      {network.status === "offline" ? (
                        <WifiOff size={14} className="text-red" />
                      ) : (
                        <Wifi size={14} className={network.status === "degraded" ? "text-amber" : "text-cyan"} />
                      )}
                    </div>
                    <div className="text-left">
                      <p className="font-body text-sm text-text-primary">{network.name}</p>
                      <p className="font-mono text-[10px] text-text-tertiary">
                        Chain ID: {network.chainId}
                      </p>
                    </div>
                  </div>
                  <div className="text-right">
                    <span className={`font-mono text-[10px] ${
                      network.status === "online" ? "text-cyan" : network.status === "degraded" ? "text-amber" : "text-red"
                    }`}>
                      {network.status}
                    </span>
                    <p className="font-mono text-[10px] text-text-tertiary">{network.gas}</p>
                  </div>
                </button>
              ))}
            </div>
          </div>
        ))}

        {/* Add Custom RPC */}
        {showAddCustom && (
          <div className="fixed inset-0 bg-black/70 flex items-center justify-center z-50 px-4">
            <div className="bg-obsidian border border-white/10 p-6 max-w-sm w-full">
              <h3 className="font-heading text-lg text-text-primary mb-4">Add Custom Network</h3>
              <div className="space-y-3">
                <div>
                  <label className="block font-mono text-xs text-text-secondary mb-1">Network Name</label>
                  <input className="w-full bg-obsidian border border-white/10 px-3 py-2 font-mono text-sm text-text-primary focus:outline-none focus:border-cyan" placeholder="My Network" />
                </div>
                <div>
                  <label className="block font-mono text-xs text-text-secondary mb-1">RPC URL</label>
                  <input className="w-full bg-obsidian border border-white/10 px-3 py-2 font-mono text-sm text-text-primary focus:outline-none focus:border-cyan" placeholder="https://..." />
                </div>
                <div>
                  <label className="block font-mono text-xs text-text-secondary mb-1">Chain ID</label>
                  <input className="w-full bg-obsidian border border-white/10 px-3 py-2 font-mono text-sm text-primary focus:outline-none focus:border-cyan" placeholder="1" />
                </div>
                <div>
                  <label className="block font-mono text-xs text-text-secondary mb-1">Symbol</label>
                  <input className="w-full bg-obsidian border border-white/10 px-3 py-2 font-mono text-sm text-text-primary focus:outline-none focus:border-cyan" placeholder="ETH" />
                </div>
                <div>
                  <label className="block font-mono text-xs text-text-secondary mb-1">Block Explorer</label>
                  <input className="w-full bg-obsidian border border-white/10 px-3 py-2 font-mono text-sm text-text-primary focus:outline-none focus:border-cyan" placeholder="https://etherscan.io" />
                </div>
              </div>
              <div className="flex gap-3 mt-6">
                <button
                  onClick={() => setShowAddCustom(false)}
                  className="flex-1 py-3 border border-white/10 text-text-secondary font-mono text-sm"
                >
                  Cancel
                </button>
                <button className="flex-1 py-3 bg-cyan text-void font-mono text-sm font-medium">
                  Add Network
                </button>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
