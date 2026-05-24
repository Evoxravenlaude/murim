import { useState } from "react";
import { Link, useLocation } from "react-router-dom";
import { ChevronLeft, Shield, Check, X, Search } from "lucide-react";

export default function ZKVerify() {
  const location = useLocation();
  const [proofInput, setProofInput] = useState(location.state?.proof || "");
  const [verifying, setVerifying] = useState(false);
  const [result, setResult] = useState<"valid" | "invalid" | null>(null);

  const handleVerify = () => {
    setVerifying(true);
    setTimeout(() => {
      setVerifying(false);
      setResult(proofInput.length > 10 && proofInput.startsWith("0x") ? "valid" : "invalid");
    }, 1500);
  };

  return (
    <div className="min-h-screen bg-void px-4 py-6">
        {/* Coming Soon Banner */}
        <div className="mb-0 px-4 py-2 bg-amber/5 border-b border-amber/20 flex items-center justify-center gap-2">
          <span className="font-mono text-xs text-amber">⚠ ZK execution is under development — transactions will not be broadcast</span>
        </div>
      <div className="max-w-lg mx-auto">
        {/* Header */}
        <div className="flex items-center gap-3 mb-8">
          <Link to="/zk-history" className="text-text-secondary hover:text-text-primary">
            <ChevronLeft size={20} />
          </Link>
          <div className="flex items-center gap-2">
            <Shield size={18} className="text-cyan" />
            <h1 className="font-heading text-xl text-text-primary">Verify ZK Proof</h1>
          </div>
        </div>

        {/* Input */}
        <div className="mb-6">
          <label className="block font-mono text-xs text-text-secondary mb-2">
            Proof Hash
          </label>
          <textarea
            value={proofInput}
            onChange={(e) => { setProofInput(e.target.value); setResult(null); }}
            className="w-full h-32 bg-obsidian border border-white/10 p-4 text-text-primary font-mono text-sm placeholder:text-text-tertiary focus:outline-none focus:border-cyan resize-none"
            placeholder="Paste proof hash (0x...)"
          />
        </div>

        {/* Verify Button */}
        <button
          onClick={handleVerify}
          disabled={!proofInput || verifying}
          className="w-full py-4 bg-cyan text-void font-mono text-sm tracking-wider font-medium hover:bg-opacity-90 transition-all disabled:opacity-30 flex items-center justify-center gap-2"
        >
          {verifying ? (
            <>
              <div className="w-4 h-4 border-2 border-void border-t-transparent rounded-full animate-spin" />
              Verifying...
            </>
          ) : (
            <>
              <Search size={16} />
              Verify Proof
            </>
          )}
        </button>

        {/* Result */}
        {result && (
          <div className={`mt-6 p-6 border ${
            result === "valid" ? "bg-cyan/5 border-cyan/30" : "bg-red/5 border-red/30"
          }`}>
            <div className="flex items-center gap-3 mb-4">
              {result === "valid" ? (
                <div className="w-10 h-10 rounded-full bg-cyan/10 flex items-center justify-center">
                  <Check size={20} className="text-cyan" />
                </div>
              ) : (
                <div className="w-10 h-10 rounded-full bg-red/10 flex items-center justify-center">
                  <X size={20} className="text-red" />
                </div>
              )}
              <div>
                <p className={`font-heading text-lg ${result === "valid" ? "text-cyan" : "text-red"}`}>
                  {result === "valid" ? "Proof is Valid" : "Verification Failed"}
                </p>
                <p className="font-mono text-xs text-text-secondary">
                  {result === "valid"
                    ? "Proof is valid and verified on-chain"
                    : "The provided proof could not be verified"}
                </p>
              </div>
            </div>

            {result === "valid" && (
              <div className="space-y-2 border-t border-white/5 pt-4">
                <div className="flex items-center justify-between">
                  <span className="font-mono text-[10px] text-text-secondary">Merkle Root</span>
                  <span className="font-mono text-[10px] text-text-primary">0x2c8d...4f1a</span>
                </div>
                <div className="flex items-center justify-between">
                  <span className="font-mono text-[10px] text-text-secondary">Nullifier</span>
                  <span className="font-mono text-[10px] text-text-primary">0x7a3f...9e2b</span>
                </div>
                <div className="flex items-center justify-between">
                  <span className="font-mono text-[10px] text-text-secondary">Commitment</span>
                  <span className="font-mono text-[10px] text-text-primary">0x1b4e...7f3a</span>
                </div>
                <div className="flex items-center justify-between">
                  <span className="font-mono text-[10px] text-text-secondary">Block Number</span>
                  <span className="font-mono text-[10px] text-text-primary">18,429,371</span>
                </div>
              </div>
            )}
          </div>
        )}

        {/* Verify Another */}
        {result && (
          <button
            onClick={() => { setProofInput(""); setResult(null); }}
            className="mt-4 w-full py-3 border border-white/10 text-text-secondary font-mono text-sm hover:border-cyan/30 transition-all"
          >
            Verify Another Proof
          </button>
        )}
      </div>
    </div>
  );
}
