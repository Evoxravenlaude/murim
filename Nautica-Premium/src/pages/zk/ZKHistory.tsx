import { useState } from "react";
import { Link } from "react-router-dom";
import { ChevronLeft, Shield, ArrowUpRight, ArrowDownLeft, Download, Check, X, Clock } from "lucide-react";

const TRANSACTIONS = [
  { id: 1, type: "send", amount: "0.5", token: "ETH", shield: "Stealth #2847", time: "2 min ago", status: "verified", proof: "0x7a3f...9e2b", nullifier: "0x2c8d...4f1a" },
  { id: 2, type: "receive", amount: "1.2", token: "ETH", shield: "Stealth #1024", time: "1 hr ago", status: "verified", proof: "0x1b4e...7f3a", nullifier: "0x9e5c...2d1b" },
  { id: 3, type: "send", amount: "500", token: "USDC", shield: "Stealth #8912", time: "3 hr ago", status: "pending", proof: "0x3f7a...1e5b", nullifier: "0x5d2c...8f4a" },
  { id: 4, type: "receive", amount: "0.8", token: "ETH", shield: "Stealth #4567", time: "1 day ago", status: "verified", proof: "0x8e2b...4c7d", nullifier: "0x1a9f...3e5b" },
  { id: 5, type: "send", amount: "2500", token: "NAUT", shield: "Stealth #3210", time: "2 days ago", status: "failed", proof: "—", nullifier: "—" },
];

export default function ZKHistory() {
  const [filter, setFilter] = useState<"all" | "sent" | "received">("all");
  const [expanded, setExpanded] = useState<number | null>(null);

  const filtered = TRANSACTIONS.filter((tx) => {
    if (filter === "sent") return tx.type === "send";
    if (filter === "received") return tx.type === "receive";
    return true;
  });

  const handleExport = () => {
    const csv = "Type,Amount,Token,Shield,Time,Status,Proof,Nullifier\n" +
      filtered.map(tx => `${tx.type},${tx.amount},${tx.token},${tx.shield},${tx.time},${tx.status},${tx.proof},${tx.nullifier}`).join("\n");
    const blob = new Blob([csv], { type: "text/csv" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = "zk-transactions.csv";
    a.click();
  };

  return (
    <div className="min-h-screen bg-void px-4 py-6">
        {/* Coming Soon Banner */}
        <div className="mb-0 px-4 py-2 bg-amber/5 border-b border-amber/20 flex items-center justify-center gap-2">
          <span className="font-mono text-xs text-amber">⚠ ZK execution is under development — transactions will not be broadcast</span>
        </div>
      <div className="max-w-2xl mx-auto">
        {/* Header */}
        <div className="flex items-center justify-between mb-8">
          <div className="flex items-center gap-3">
            <Link to="/" className="text-text-secondary hover:text-text-primary">
              <ChevronLeft size={20} />
            </Link>
            <div className="flex items-center gap-2">
              <Shield size={18} className="text-cyan" />
              <h1 className="font-heading text-xl text-text-primary">Private Transaction History</h1>
            </div>
          </div>
          <button
            onClick={handleExport}
            className="flex items-center gap-1 text-cyan font-mono text-xs hover:opacity-80"
          >
            <Download size={12} />
            Export
          </button>
        </div>

        {/* Filter Tabs */}
        <div className="flex gap-4 mb-6 border-b border-white/5">
          {(["all", "sent", "received"] as const).map((f) => (
            <button
              key={f}
              onClick={() => setFilter(f)}
              className={`pb-3 font-mono text-xs capitalize transition-colors ${
                filter === f ? "text-cyan border-b-2 border-cyan" : "text-text-secondary hover:text-text-primary"
              }`}
            >
              {f}
            </button>
          ))}
        </div>

        {/* Transaction List */}
        <div className="space-y-1">
          {filtered.map((tx) => (
            <div key={tx.id}>
              <button
                onClick={() => setExpanded(expanded === tx.id ? null : tx.id)}
                className="w-full flex items-center gap-3 p-3 bg-obsidian/50 hover:bg-obsidian transition-colors text-left"
              >
                <div className={`w-8 h-8 rounded-full flex items-center justify-center ${
                  tx.type === "send" ? "bg-red/10" : "bg-blue/10"
                }`}>
                  {tx.type === "send" ? (
                    <ArrowUpRight size={14} className="text-red" />
                  ) : (
                    <ArrowDownLeft size={14} className="text-blue" />
                  )}
                </div>
                <div className="flex-1">
                  <p className="font-mono text-xs text-text-primary capitalize">
                    {tx.type} {tx.amount} {tx.token}
                  </p>
                  <p className="font-mono text-[10px] text-text-tertiary">{tx.shield}</p>
                </div>
                <div className="flex items-center gap-3">
                  {tx.status === "verified" && <Check size={12} className="text-cyan" />}
                  {tx.status === "pending" && <Clock size={12} className="text-amber" />}
                  {tx.status === "failed" && <X size={12} className="text-red" />}
                  <span className={`font-mono text-[10px] ${
                    tx.status === "verified" ? "text-cyan" : tx.status === "pending" ? "text-amber" : "text-red"
                  }`}>
                    {tx.status}
                  </span>
                  <span className="font-mono text-[10px] text-text-tertiary">{tx.time}</span>
                </div>
              </button>

              {/* Expanded Details */}
              {expanded === tx.id && tx.status !== "failed" && (
                <div className="p-4 bg-obsidian border-t border-white/5 space-y-2">
                  <div className="flex items-center justify-between">
                    <span className="font-mono text-[10px] text-text-secondary">Proof Hash</span>
                    <span className="font-mono text-[10px] text-text-primary">{tx.proof}</span>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="font-mono text-[10px] text-text-secondary">Nullifier</span>
                    <span className="font-mono text-[10px] text-text-primary">{tx.nullifier}</span>
                  </div>
                  <Link
                    to="/zk-verify"
                    state={{ proof: tx.proof }}
                    className="inline-block mt-2 text-cyan font-mono text-[10px] hover:opacity-80"
                  >
                    Verify Proof →
                  </Link>
                </div>
              )}
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
