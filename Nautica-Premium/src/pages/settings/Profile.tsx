import { useState } from "react";
import { Link } from "react-router-dom";
import { ChevronLeft, Copy, Check, X } from "lucide-react";

const ACTIVITIES = [
  { type: "swap", desc: "Swapped 0.5 ETH for NAUT", time: "2 min ago" },
  { type: "send", desc: "Sent 1.2 ETH to 0x9b...1c4e", time: "1 hr ago" },
  { type: "receive", desc: "Received 500 USDC", time: "3 hr ago" },
  { type: "nft", desc: "Bought Node #2847 for 2.4 ETH", time: "1 day ago" },
];

const CONNECTED_DAPPS = [
  { name: "Uniswap", url: "app.uniswap.org", permissions: ["View Balance", "Request Transactions"] },
  { name: "OpenSea", url: "opensea.io", permissions: ["View NFTs", "List Items"] },
  { name: "Aave", url: "app.aave.com", permissions: ["View Balance", "Deposit/Withdraw"] },
];

export default function Profile() {
  const [copied, setCopied] = useState(false);
  const [dapps, setDapps] = useState(CONNECTED_DAPPS);
  const address = "0x7aB243890DDe3c23F2a9B2c8f8e3f1a2B3c4D5e6";
  const ens = "nautica.eth";

  const handleCopy = () => {
    navigator.clipboard.writeText(address);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  const revokeDapp = (name: string) => {
    setDapps(dapps.filter((d) => d.name !== name));
  };

  return (
    <div className="min-h-screen bg-void px-4 py-6">
      <div className="max-w-lg mx-auto">
        {/* Header */}
        <div className="flex items-center gap-3 mb-8">
          <Link to="/wallet-home" className="text-text-secondary hover:text-text-primary">
            <ChevronLeft size={20} />
          </Link>
          <h1 className="font-heading text-xl text-text-primary">Profile</h1>
        </div>

        {/* Profile Header */}
        <div className="flex flex-col items-center mb-8">
          <div className="w-20 h-20 rounded-full bg-gradient-to-br from-cyan to-blue flex items-center justify-center mb-4">
            <span className="font-heading text-2xl text-white">
              {ens.slice(0, 2).toUpperCase()}
            </span>
          </div>
          <h2 className="font-heading text-xl text-text-primary">{ens}</h2>
          <div className="flex items-center gap-2 mt-2">
            <code className="font-mono text-xs text-text-secondary">
              {address.slice(0, 6)}...{address.slice(-4)}
            </code>
            <button onClick={handleCopy} className="text-text-tertiary hover:text-cyan">
              {copied ? <Check size={12} className="text-cyan" /> : <Copy size={12} />}
            </button>
          </div>
        </div>

        {/* Stats */}
        <div className="grid grid-cols-4 gap-3 mb-8">
          {[
            { label: "Volume", value: "$45.2K" },
            { label: "Transactions", value: "284" },
            { label: "First Active", value: "Mar 2024" },
            { label: "Net Worth", value: "$33.4K" },
          ].map((stat) => (
            <div key={stat.label} className="p-3 bg-obsidian border border-white/5 text-center">
              <p className="font-mono text-[10px] text-text-secondary">{stat.label}</p>
              <p className="font-heading text-sm text-text-primary mt-1">{stat.value}</p>
            </div>
          ))}
        </div>

        {/* Recent Activity */}
        <div className="mb-8">
          <h3 className="font-heading text-lg text-text-primary mb-4">Recent Activity</h3>
          <div className="space-y-1">
            {ACTIVITIES.map((act, i) => (
              <div key={i} className="flex items-center justify-between p-3 bg-obsidian/50">
                <p className="font-mono text-xs text-text-primary">{act.desc}</p>
                <span className="font-mono text-[10px] text-text-tertiary">{act.time}</span>
              </div>
            ))}
          </div>
        </div>

        {/* Connected dApps */}
        <div>
          <h3 className="font-heading text-lg text-text-primary mb-4">Connected dApps</h3>
          {dapps.length === 0 ? (
            <p className="text-text-secondary text-sm text-center py-8">No connected dApps</p>
          ) : (
            <div className="space-y-2">
              {dapps.map((dapp) => (
                <div key={dapp.name} className="p-4 bg-obsidian border border-white/5">
                  <div className="flex items-center justify-between mb-2">
                    <div className="flex items-center gap-3">
                      <div className="w-8 h-8 rounded-full bg-cyan/10 flex items-center justify-center">
                        <span className="font-mono text-xs text-cyan">{dapp.name[0]}</span>
                      </div>
                      <div>
                        <p className="font-body text-sm text-text-primary">{dapp.name}</p>
                        <p className="font-mono text-[10px] text-text-tertiary">{dapp.url}</p>
                      </div>
                    </div>
                    <button
                      onClick={() => revokeDapp(dapp.name)}
                      className="text-text-tertiary hover:text-red transition-colors"
                    >
                      <X size={14} />
                    </button>
                  </div>
                  <div className="flex flex-wrap gap-1">
                    {dapp.permissions.map((perm) => (
                      <span key={perm} className="px-2 py-0.5 bg-white/5 font-mono text-[10px] text-text-secondary">
                        {perm}
                      </span>
                    ))}
                  </div>
                </div>
              ))}
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
