import { createFileRoute } from "@tanstack/react-router";
import { useMutation, useQuery } from "@tanstack/react-query";
import { useState } from "react";
import { useServerFn } from "@tanstack/react-start";
import {
  ArrowDownUp,
  Settings,
  Zap,
  Shield,
  TrendingUp,
  ChevronDown,
  Loader2,
  Check,
  ExternalLink,
  Info,
  Fuel,
} from "lucide-react";
import { AppLayout } from "@/components/nautica/AppLayout";
import { getTokens, submitSwap } from "@/lib/nautica.functions";

export const Route = createFileRoute("/swap")(
  {
    head: () => ({
      meta: [
        { title: "Swap — Nautica" },
        { name: "description", content: "Swap tokens instantly on Nautica. Best routes, MEV protection, multi-chain." },
        { property: "og:title", content: "Swap — Nautica" },
      ],
    }),
    component: SwapPage,
  },
);

function SwapPage() {
  const [fromToken, setFromToken] = useState("ETH");
  const [toToken, setToToken] = useState("USDC");
  const [amount, setAmount] = useState("0.5");
  const [slippage, setSlippage] = useState("0.5");
  const [showSettings, setShowSettings] = useState(false);
  const [showFromTokens, setShowFromTokens] = useState(false);
  const [showToTokens, setShowToTokens] = useState(false);

  const { data: tokens = [] } = useQuery({ queryKey: ["tokens"], queryFn: () => getTokens() });

  const fromTokenData = tokens.find((t) => t.symbol === fromToken);
  const toTokenData = tokens.find((t) => t.symbol === toToken);

  // Mock exchange rate
  const rate = fromTokenData && toTokenData ? fromTokenData.price / toTokenData.price : 1;
  const outputAmount = Number(amount) * rate;
  const usdValue = Number(amount) * (fromTokenData?.price ?? 0);

  const swapFn = useServerFn(submitSwap);
  const swapMutation = useMutation({
    mutationFn: () =>
      swapFn({
        data: {
          fromToken,
          toToken,
          amount: Number(amount),
          slippage: Number(slippage),
        },
      }),
  });

  function flipTokens() {
    setFromToken(toToken);
    setToToken(fromToken);
    setAmount(outputAmount.toFixed(6));
  }

  return (
    <AppLayout>
      <header className="animate-fade-down">
        <p className="text-xs text-primary font-display font-semibold uppercase tracking-widest">Trade</p>
        <h1 className="font-display text-4xl md:text-5xl font-bold text-ink mt-1">Swap</h1>
      </header>

      <div className="mt-6 grid lg:grid-cols-[1fr_0.8fr] gap-6">
        {/* Swap card */}
        <div className="animate-fade-up">
          <div className="nautica-glass p-6 md:p-8 relative">
            {/* Header */}
            <div className="flex items-center justify-between mb-6">
              <div className="flex gap-2">
                {(["swap", "limit"] as const).map((mode) => (
                  <button
                    key={mode}
                    className={
                      "px-4 py-2 rounded-full text-sm font-display font-semibold capitalize transition-all " +
                      (mode === "swap"
                        ? "bg-primary text-primary-foreground"
                        : "text-muted-foreground hover:text-foreground")
                    }
                  >
                    {mode}
                  </button>
                ))}
              </div>
              <button
                onClick={() => setShowSettings(!showSettings)}
                className={"btn-icon " + (showSettings ? "!bg-primary !text-primary-foreground !border-primary" : "")}
              >
                <Settings className="w-4 h-4" />
              </button>
            </div>

            {/* Slippage settings */}
            {showSettings && (
              <div className="mb-5 p-4 rounded-2xl bg-muted/50 border border-border/50 animate-fade-down">
                <p className="text-xs font-display font-semibold text-muted-foreground uppercase tracking-wider mb-2">
                  Slippage Tolerance
                </p>
                <div className="flex gap-2">
                  {["0.1", "0.5", "1.0"].map((s) => (
                    <button
                      key={s}
                      onClick={() => setSlippage(s)}
                      className={
                        "px-3 py-2 rounded-xl text-sm font-display font-semibold transition-all " +
                        (slippage === s
                          ? "bg-primary/10 text-primary border border-primary/20"
                          : "bg-muted text-muted-foreground hover:text-foreground")
                      }
                    >
                      {s}%
                    </button>
                  ))}
                  <input
                    value={slippage}
                    onChange={(e) => setSlippage(e.target.value)}
                    className="nautica-input w-20 text-center text-sm"
                    placeholder="Custom"
                  />
                </div>
              </div>
            )}

            {/* From token */}
            <div className="rounded-2xl bg-muted/50 border border-border/50 p-5 relative">
              <div className="flex items-center justify-between mb-2">
                <span className="text-xs text-muted-foreground font-display font-semibold">You pay</span>
                <span className="text-xs text-muted-foreground">
                  Balance: <span className="text-ink font-semibold">{fromTokenData?.balance ?? 0}</span>
                </span>
              </div>
              <div className="flex items-center gap-3">
                <input
                  value={amount}
                  onChange={(e) => setAmount(e.target.value)}
                  inputMode="decimal"
                  className="bg-transparent font-display text-4xl font-bold text-ink outline-none flex-1 min-w-0"
                  placeholder="0.0"
                />
                <div className="relative">
                  <button
                    onClick={() => { setShowFromTokens(!showFromTokens); setShowToTokens(false); }}
                    className="flex items-center gap-2 px-4 py-2.5 rounded-full bg-card border border-border hover:border-primary/20 transition-all"
                  >
                    <div className="w-6 h-6 rounded-full shrink-0" style={{ backgroundImage: fromTokenData?.icon }} />
                    <span className="font-display font-bold text-ink">{fromToken}</span>
                    <ChevronDown className="w-4 h-4 text-muted-foreground" />
                  </button>
                  {showFromTokens && (
                    <div className="absolute top-full mt-2 right-0 w-48 nautica-glass-strong p-2 z-50 animate-scale-in">
                      {tokens.map((t) => (
                        <button
                          key={t.symbol}
                          onClick={() => { setFromToken(t.symbol); setShowFromTokens(false); }}
                          className="w-full flex items-center gap-3 p-2.5 rounded-xl hover:bg-muted/50 transition-colors text-left"
                        >
                          <div className="w-7 h-7 rounded-full shrink-0" style={{ backgroundImage: t.icon }} />
                          <div>
                            <p className="text-sm font-display font-semibold text-ink">{t.symbol}</p>
                            <p className="text-[10px] text-muted-foreground">{t.balance}</p>
                          </div>
                        </button>
                      ))}
                    </div>
                  )}
                </div>
              </div>
              <p className="text-xs text-muted-foreground mt-2">≈ ${usdValue.toLocaleString(undefined, { maximumFractionDigits: 2 })}</p>

              {/* Quick amount buttons */}
              <div className="flex gap-2 mt-3">
                {["25%", "50%", "75%", "Max"].map((pct) => (
                  <button
                    key={pct}
                    onClick={() => {
                      const bal = fromTokenData?.balance ?? 0;
                      const mult = pct === "Max" ? 1 : parseInt(pct) / 100;
                      setAmount((bal * mult).toFixed(6));
                    }}
                    className="px-2.5 py-1 rounded-lg text-[10px] font-display font-semibold text-muted-foreground bg-muted hover:text-primary hover:bg-primary/10 transition-all"
                  >
                    {pct}
                  </button>
                ))}
              </div>
            </div>

            {/* Swap direction button */}
            <div className="flex justify-center -my-3 relative z-10">
              <button
                onClick={flipTokens}
                className="w-11 h-11 rounded-2xl bg-primary text-primary-foreground grid place-items-center shadow-soft hover:shadow-glow-primary transition-all duration-300 hover:rotate-180"
              >
                <ArrowDownUp className="w-5 h-5" />
              </button>
            </div>

            {/* To token */}
            <div className="rounded-2xl bg-muted/50 border border-border/50 p-5">
              <div className="flex items-center justify-between mb-2">
                <span className="text-xs text-muted-foreground font-display font-semibold">You receive</span>
                <span className="text-xs text-muted-foreground">
                  Balance: <span className="text-ink font-semibold">{toTokenData?.balance ?? 0}</span>
                </span>
              </div>
              <div className="flex items-center gap-3">
                <p className="font-display text-4xl font-bold text-ink flex-1 min-w-0 truncate">
                  {outputAmount > 0 ? outputAmount.toLocaleString(undefined, { maximumFractionDigits: 4 }) : "0.0"}
                </p>
                <div className="relative">
                  <button
                    onClick={() => { setShowToTokens(!showToTokens); setShowFromTokens(false); }}
                    className="flex items-center gap-2 px-4 py-2.5 rounded-full bg-card border border-border hover:border-primary/20 transition-all"
                  >
                    <div className="w-6 h-6 rounded-full shrink-0" style={{ backgroundImage: toTokenData?.icon }} />
                    <span className="font-display font-bold text-ink">{toToken}</span>
                    <ChevronDown className="w-4 h-4 text-muted-foreground" />
                  </button>
                  {showToTokens && (
                    <div className="absolute top-full mt-2 right-0 w-48 nautica-glass-strong p-2 z-50 animate-scale-in">
                      {tokens.map((t) => (
                        <button
                          key={t.symbol}
                          onClick={() => { setToToken(t.symbol); setShowToTokens(false); }}
                          className="w-full flex items-center gap-3 p-2.5 rounded-xl hover:bg-muted/50 transition-colors text-left"
                        >
                          <div className="w-7 h-7 rounded-full shrink-0" style={{ backgroundImage: t.icon }} />
                          <div>
                            <p className="text-sm font-display font-semibold text-ink">{t.symbol}</p>
                            <p className="text-[10px] text-muted-foreground">{t.balance}</p>
                          </div>
                        </button>
                      ))}
                    </div>
                  )}
                </div>
              </div>
            </div>

            {/* Rate info */}
            <div className="mt-4 p-3 rounded-xl bg-muted/30 border border-border/30 space-y-2">
              <div className="flex items-center justify-between text-xs">
                <span className="text-muted-foreground flex items-center gap-1">
                  <Info className="w-3 h-3" /> Rate
                </span>
                <span className="text-ink font-display font-semibold">
                  1 {fromToken} = {rate.toLocaleString(undefined, { maximumFractionDigits: 2 })} {toToken}
                </span>
              </div>
              <div className="flex items-center justify-between text-xs">
                <span className="text-muted-foreground flex items-center gap-1">
                  <Fuel className="w-3 h-3" /> Gas
                </span>
                <span className="text-ink font-display font-semibold">~$4.20</span>
              </div>
              <div className="flex items-center justify-between text-xs">
                <span className="text-muted-foreground flex items-center gap-1">
                  <Shield className="w-3 h-3" /> Slippage
                </span>
                <span className="text-ink font-display font-semibold">{slippage}%</span>
              </div>
            </div>

            {/* Swap button */}
            {swapMutation.isSuccess ? (
              <div className="mt-5 p-5 rounded-2xl bg-success/10 border border-success/20 animate-scale-in">
                <div className="flex items-center gap-2 mb-2">
                  <Check className="w-5 h-5 text-success" />
                  <p className="font-display font-bold text-success">Swap Confirmed!</p>
                </div>
                <p className="text-xs text-muted-foreground">
                  {swapMutation.data?.fromAmount} {fromToken} → {swapMutation.data?.toAmount.toLocaleString(undefined, { maximumFractionDigits: 2 })} {toToken}
                </p>
                <p className="text-xs text-muted-foreground font-mono mt-1">
                  Tx: {swapMutation.data?.hash}
                </p>
                <div className="flex gap-2 mt-3">
                  <button className="btn-secondary text-xs">
                    <ExternalLink className="w-3 h-3" /> View TX
                  </button>
                  <button onClick={() => swapMutation.reset()} className="btn-primary text-xs">
                    New Swap
                  </button>
                </div>
              </div>
            ) : (
              <button
                onClick={() => swapMutation.mutate()}
                disabled={swapMutation.isPending || !amount || Number(amount) <= 0}
                className="mt-5 w-full btn-cta py-4"
              >
                {swapMutation.isPending ? (
                  <><Loader2 className="w-4 h-4 animate-spin" /> Swapping…</>
                ) : (
                  <><Zap className="w-4 h-4" /> Swap {fromToken} → {toToken}</>
                )}
              </button>
            )}
          </div>
        </div>

        {/* Right panel — Market info */}
        <div className="space-y-4 animate-fade-up" style={{ animationDelay: "100ms" }}>
          {/* Token list */}
          <div className="nautica-glass p-5">
            <h3 className="font-display text-sm font-bold text-ink mb-3 flex items-center gap-2">
              <TrendingUp className="w-4 h-4 text-primary" /> Your Tokens
            </h3>
            <div className="space-y-1">
              {tokens.map((t) => (
                <div key={t.symbol} className="flex items-center gap-3 p-3 rounded-xl hover:bg-muted/30 transition-colors cursor-pointer">
                  <div className="w-9 h-9 rounded-xl shrink-0" style={{ backgroundImage: t.icon }} />
                  <div className="flex-1 min-w-0">
                    <p className="font-display font-semibold text-ink text-sm">{t.symbol}</p>
                    <p className="text-[10px] text-muted-foreground">{t.name}</p>
                  </div>
                  <div className="text-right">
                    <p className="font-display font-semibold text-ink text-sm">
                      ${(t.balance * t.price).toLocaleString(undefined, { maximumFractionDigits: 2 })}
                    </p>
                    <p className={"text-[10px] font-semibold " + (t.change24h >= 0 ? "text-success" : "text-destructive")}>
                      {t.change24h >= 0 ? "+" : ""}{t.change24h}%
                    </p>
                  </div>
                </div>
              ))}
            </div>
          </div>

          {/* Route info */}
          <div className="nautica-glass p-5">
            <h3 className="font-display text-sm font-bold text-ink mb-3 flex items-center gap-2">
              <Zap className="w-4 h-4 text-primary" /> Route Details
            </h3>
            <div className="space-y-3">
              <div className="flex items-center gap-2">
                <div className="w-2 h-2 rounded-full bg-primary" />
                <div className="flex-1 h-px bg-border" />
                <span className="text-[10px] text-muted-foreground font-medium px-2 py-0.5 rounded bg-muted">Uniswap V3</span>
                <div className="flex-1 h-px bg-border" />
                <div className="w-2 h-2 rounded-full bg-success" />
              </div>
              <div className="flex items-center justify-between text-xs">
                <span className="text-muted-foreground">{fromToken}</span>
                <span className="text-muted-foreground">→</span>
                <span className="text-muted-foreground">{toToken}</span>
              </div>
            </div>

            <div className="mt-4 space-y-2">
              <div className="flex items-center gap-2 p-2.5 rounded-lg bg-muted/30 text-xs">
                <Shield className="w-3.5 h-3.5 text-success shrink-0" />
                <span className="text-muted-foreground">MEV Protection</span>
                <span className="ml-auto badge-success text-[9px]">Active</span>
              </div>
              <div className="flex items-center gap-2 p-2.5 rounded-lg bg-muted/30 text-xs">
                <Zap className="w-3.5 h-3.5 text-primary shrink-0" />
                <span className="text-muted-foreground">Best route</span>
                <span className="ml-auto text-ink font-display font-semibold">Optimized</span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </AppLayout>
  );
}
