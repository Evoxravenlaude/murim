import { useState } from "react";
import { Link } from "react-router-dom";
import { ChevronLeft, Plus, AlertCircle } from "lucide-react";

export default function AddLiquidity() {
  const [amountA, setAmountA] = useState("");
  const [amountB, setAmountB] = useState("");
  const [feeTier, setFeeTier] = useState<"0.05" | "0.3" | "1">("0.3");
  const [fullRange, setFullRange] = useState(true);

  return (
    <div className="min-h-screen bg-void px-4 py-6">
      <div className="max-w-lg mx-auto">
        {/* Header */}
        <div className="flex items-center gap-3 mb-8">
          <Link to="/pools" className="text-text-secondary hover:text-text-primary">
            <ChevronLeft size={20} />
          </Link>
          <h1 className="font-heading text-xl text-text-primary">Add Liquidity</h1>
        </div>

        {/* Token Pair */}
        <div className="space-y-3 mb-6">
          <div className="p-4 bg-obsidian border border-white/5">
            <div className="flex items-center justify-between mb-2">
              <span className="font-mono text-xs text-text-secondary">Token A</span>
              <span className="font-mono text-xs text-text-tertiary">Balance: 4.28 ETH</span>
            </div>
            <div className="flex items-center gap-4">
              <input
                type="number"
                value={amountA}
                onChange={(e) => setAmountA(e.target.value)}
                className="flex-1 bg-transparent text-2xl font-heading text-text-primary placeholder:text-text-tertiary focus:outline-none"
                placeholder="0.0"
              />
              <button className="flex items-center gap-2 px-3 py-2 bg-white/5">
                <span className="font-mono text-sm text-text-primary">ETH</span>
              </button>
            </div>
          </div>

          <div className="flex justify-center">
            <div className="w-8 h-8 bg-obsidian border border-white/10 flex items-center justify-center">
              <Plus size={16} className="text-text-tertiary" />
            </div>
          </div>

          <div className="p-4 bg-obsidian border border-white/5">
            <div className="flex items-center justify-between mb-2">
              <span className="font-mono text-xs text-text-secondary">Token B</span>
              <span className="font-mono text-xs text-text-tertiary">Balance: 8,420 USDC</span>
            </div>
            <div className="flex items-center gap-4">
              <input
                type="number"
                value={amountB}
                onChange={(e) => setAmountB(e.target.value)}
                className="flex-1 bg-transparent text-2xl font-heading text-text-primary placeholder:text-text-tertiary focus:outline-none"
                placeholder="0.0"
              />
              <button className="flex items-center gap-2 px-3 py-2 bg-white/5">
                <span className="font-mono text-sm text-text-primary">USDC</span>
              </button>
            </div>
          </div>
        </div>

        {/* Fee Tier */}
        <div className="mb-6">
          <label className="block font-mono text-xs text-text-secondary mb-2">Fee Tier</label>
          <div className="grid grid-cols-3 gap-2">
            {[
              { value: "0.05" as const, label: "0.05%", desc: "Best for stable pairs" },
              { value: "0.3" as const, label: "0.3%", desc: "Best for most pairs" },
              { value: "1" as const, label: "1%", desc: "Best for exotic pairs" },
            ].map((tier) => (
              <button
                key={tier.value}
                onClick={() => setFeeTier(tier.value)}
                className={`p-3 border text-center transition-colors ${
                  feeTier === tier.value
                    ? "border-cyan bg-cyan/10"
                    : "border-white/10 hover:border-white/20"
                }`}
              >
                <p className={`font-mono text-sm ${feeTier === tier.value ? "text-cyan" : "text-text-primary"}`}>
                  {tier.label}
                </p>
                <p className="font-mono text-[10px] text-text-tertiary mt-1">{tier.desc}</p>
              </button>
            ))}
          </div>
        </div>

        {/* Price Range */}
        <div className="mb-6">
          <div className="flex items-center justify-between mb-2">
            <label className="font-mono text-xs text-text-secondary">Price Range</label>
            <button
              onClick={() => setFullRange(!fullRange)}
              className={`font-mono text-xs ${fullRange ? "text-cyan" : "text-text-secondary"}`}
            >
              {fullRange ? "Full Range" : "Custom Range"}
            </button>
          </div>
          {fullRange ? (
            <div className="p-4 bg-obsidian border border-white/5 text-center">
              <p className="font-mono text-sm text-text-primary">Full Range</p>
              <p className="font-mono text-xs text-text-secondary mt-1">
                Your liquidity will be active across all price ranges
              </p>
            </div>
          ) : (
            <div className="grid grid-cols-2 gap-2">
              <div className="p-3 bg-obsidian border border-white/5">
                <p className="font-mono text-[10px] text-text-secondary mb-1">Min Price</p>
                <input
                  type="number"
                  className="w-full bg-transparent font-mono text-sm text-text-primary focus:outline-none"
                  placeholder="0.0"
                />
              </div>
              <div className="p-3 bg-obsidian border border-white/5">
                <p className="font-mono text-[10px] text-text-secondary mb-1">Max Price</p>
                <input
                  type="number"
                  className="w-full bg-transparent font-mono text-sm text-text-primary focus:outline-none"
                  placeholder="∞"
                />
              </div>
            </div>
          )}
        </div>

        {/* Preview */}
        <div className="mb-8 p-4 bg-obsidian border border-white/5 space-y-2">
          <div className="flex items-center justify-between">
            <span className="font-mono text-xs text-text-secondary">Pool Share</span>
            <span className="font-mono text-xs text-text-primary">0.0012%</span>
          </div>
          <div className="flex items-center justify-between">
            <span className="font-mono text-xs text-text-secondary">Estimated APR</span>
            <span className="font-mono text-xs text-cyan">12.4%</span>
          </div>
        </div>

        {/* Add Button */}
        <button className="w-full py-4 bg-cyan text-void font-mono text-sm tracking-wider font-medium hover:bg-opacity-90 transition-all">
          Add Liquidity
        </button>

        {/* Warning */}
        <div className="flex items-start gap-2 mt-4 p-3 bg-amber/5 border border-amber/20">
          <AlertCircle size={14} className="text-amber shrink-0 mt-0.5" />
          <p className="font-mono text-xs text-amber">
            By adding liquidity, you will earn fees proportional to your share of the pool.
            Impermanent loss may occur.
          </p>
        </div>
      </div>
    </div>
  );
}
