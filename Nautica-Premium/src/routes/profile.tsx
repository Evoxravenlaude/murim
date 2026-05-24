import { createFileRoute, Link } from "@tanstack/react-router";
import { useQuery } from "@tanstack/react-query";
import { Copy, ExternalLink, Shield, TrendingUp, Image, Wallet, Settings, ChevronRight } from "lucide-react";
import { AppLayout } from "@/components/nautica/AppLayout";
import { ThemeToggle } from "@/components/nautica/ThemeToggle";
import { getPortfolio, getTokens } from "@/lib/nautica.functions";

export const Route = createFileRoute("/profile")(
  {
    head: () => ({
      meta: [
        { title: "Profile — Nautica" },
        { name: "description", content: "Your wallet, portfolio, collected editions, and Nautica preferences." },
        { property: "og:title", content: "Profile — Nautica" },
      ],
    }),
    component: ProfilePage,
  },
);

function ProfilePage() {
  const { data: portfolio } = useQuery({ queryKey: ["portfolio"], queryFn: () => getPortfolio() });
  const { data: tokens = [] } = useQuery({ queryKey: ["tokens"], queryFn: () => getTokens() });

  return (
    <AppLayout>
      {/* Header */}
      <header className="flex items-end justify-between gap-4 flex-wrap animate-fade-down">
        <div className="flex items-center gap-4">
          {/* Avatar */}
          <div className="w-16 h-16 rounded-3xl bg-primary/10 border-2 border-primary/20 grid place-items-center relative">
            <span className="text-2xl">🌊</span>
            <div className="absolute -bottom-1 -right-1 w-5 h-5 rounded-full bg-success border-2 border-background" />
          </div>
          <div>
            <h1 className="font-display text-3xl md:text-4xl font-bold text-ink">Sajon</h1>
            <p className="text-sm text-muted-foreground font-mono mt-0.5">0x4a7b…f02c</p>
          </div>
        </div>
        <ThemeToggle />
      </header>

      {/* Portfolio overview — Inspired by fintech ref */}
      <div className="mt-6 nautica-glass p-6 md:p-8 relative overflow-hidden animate-fade-up">
        <div aria-hidden className="absolute -top-16 -right-16 w-48 h-48 rounded-full bg-primary/10 blur-3xl" />
        <div className="relative">
          <p className="text-xs text-muted-foreground font-display font-semibold uppercase tracking-widest">
            Portfolio Value
          </p>
          <div className="flex items-end gap-3 mt-2">
            <p className="font-display text-5xl md:text-6xl font-bold text-ink leading-none">
              ${portfolio?.totalValue.toLocaleString() ?? "—"}
            </p>
            {portfolio && (
              <span className={
                "badge-success mb-2 text-sm " +
                (portfolio.change24h >= 0 ? "" : "!bg-destructive/15 !text-destructive")
              }>
                <TrendingUp className="w-3 h-3" />
                {portfolio.change24h >= 0 ? "+" : ""}{portfolio.change24h}%
              </span>
            )}
          </div>
          <p className="text-sm text-muted-foreground mt-1">
            {portfolio?.totalValueETH ?? "—"} ETH · +${portfolio?.change24hValue.toLocaleString() ?? "0"} today
          </p>
        </div>

        {/* Quick stats */}
        <div className="grid grid-cols-3 gap-3 mt-6">
          <div className="rounded-2xl bg-muted/50 p-4 border border-border/50">
            <p className="text-[10px] uppercase tracking-wider text-muted-foreground font-semibold">Collected</p>
            <p className="font-display text-2xl font-bold text-ink mt-1">{portfolio?.collected ?? 0}</p>
          </div>
          <div className="rounded-2xl bg-primary/5 p-4 border border-primary/10">
            <p className="text-[10px] uppercase tracking-wider text-primary font-semibold">Listed</p>
            <p className="font-display text-2xl font-bold text-ink mt-1">{portfolio?.listed ?? 0}</p>
          </div>
          <div className="rounded-2xl bg-muted/50 p-4 border border-border/50">
            <p className="text-[10px] uppercase tracking-wider text-muted-foreground font-semibold">Volume</p>
            <p className="font-display text-2xl font-bold text-ink mt-1">{portfolio?.volume ?? 0} <span className="text-xs text-muted-foreground">ETH</span></p>
          </div>
        </div>
      </div>

      <div className="mt-5 grid md:grid-cols-2 gap-5">
        {/* Wallet & tokens */}
        <section className="nautica-glass p-6 animate-fade-up" style={{ animationDelay: "100ms" }}>
          <div className="flex items-center justify-between mb-4">
            <div className="flex items-center gap-2">
              <Wallet className="w-4 h-4 text-primary" />
              <span className="text-xs uppercase tracking-widest font-display font-semibold text-muted-foreground">Tokens</span>
            </div>
            <Link to="/swap" className="text-xs text-primary font-display font-semibold hover:underline">
              Swap →
            </Link>
          </div>

          <div className="space-y-2">
            {tokens.map((t) => (
              <div key={t.symbol} className="flex items-center gap-3 p-3 rounded-xl hover:bg-muted/50 transition-colors cursor-pointer group">
                <div className="w-10 h-10 rounded-xl shrink-0" style={{ backgroundImage: t.icon }} />
                <div className="flex-1 min-w-0">
                  <p className="font-display font-semibold text-ink text-sm">{t.symbol}</p>
                  <p className="text-[11px] text-muted-foreground">{t.name}</p>
                </div>
                <div className="text-right">
                  <p className="font-display font-semibold text-ink text-sm">{t.balance}</p>
                  <p className={"text-[11px] " + (t.change24h >= 0 ? "text-success" : "text-destructive")}>
                    {t.change24h >= 0 ? "+" : ""}{t.change24h}%
                  </p>
                </div>
              </div>
            ))}
          </div>
        </section>

        {/* Privacy settings */}
        <section className="nautica-glass p-6 animate-fade-up" style={{ animationDelay: "200ms" }}>
          <div className="flex items-center gap-2 mb-4">
            <Shield className="w-4 h-4 text-primary" />
            <span className="text-xs uppercase tracking-widest font-display font-semibold text-muted-foreground">Privacy & Security</span>
          </div>

          <div className="space-y-3">
            {[
              { label: "Auto-prove sends", desc: "Generate ZK proofs automatically", enabled: true },
              { label: "Hide balances", desc: "Mask token amounts on screen", enabled: false },
              { label: "Private mode", desc: "Block activity from public view", enabled: true },
            ].map((setting) => (
              <div key={setting.label} className="flex items-center justify-between p-3 rounded-xl bg-muted/30 border border-border/50">
                <div>
                  <p className="text-sm font-display font-semibold text-ink">{setting.label}</p>
                  <p className="text-[11px] text-muted-foreground">{setting.desc}</p>
                </div>
                <button className={
                  "w-12 h-7 rounded-full relative transition-all duration-300 " +
                  (setting.enabled ? "bg-primary" : "bg-muted border border-border")
                }>
                  <span className={
                    "absolute top-1 w-5 h-5 rounded-full transition-all duration-300 " +
                    (setting.enabled
                      ? "right-1 bg-primary-foreground"
                      : "left-1 bg-muted-foreground")
                  } />
                </button>
              </div>
            ))}
          </div>

          {/* Quick actions */}
          <div className="mt-4 space-y-1">
            {[
              { label: "Copy address", icon: Copy },
              { label: "View on explorer", icon: ExternalLink },
              { label: "Settings", icon: Settings },
            ].map((action) => (
              <button key={action.label} className="w-full flex items-center gap-3 p-3 rounded-xl text-sm text-muted-foreground hover:bg-muted/50 hover:text-foreground transition-colors">
                <action.icon className="w-4 h-4" />
                <span className="font-display font-medium">{action.label}</span>
                <ChevronRight className="w-4 h-4 ml-auto" />
              </button>
            ))}
          </div>
        </section>

        {/* Collection */}
        <section className="nautica-glass p-6 md:col-span-2 animate-fade-up" style={{ animationDelay: "300ms" }}>
          <div className="flex items-center justify-between mb-4">
            <div className="flex items-center gap-2">
              <Image className="w-4 h-4 text-primary" />
              <span className="text-xs uppercase tracking-widest font-display font-semibold text-muted-foreground">Your Collection</span>
            </div>
            <Link to="/market" className="text-xs text-primary font-display font-semibold hover:underline">
              View all →
            </Link>
          </div>
          <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-4 gap-3">
            {[
              { bg: "linear-gradient(135deg, oklch(0.65 0.15 220), oklch(0.45 0.18 280))", name: "Tideglass 04", artist: "Anouk Vey" },
              { bg: "linear-gradient(135deg, oklch(0.50 0.20 180), oklch(0.35 0.15 220))", name: "Soft Machine", artist: "Hiro Tanaka" },
              { bg: "linear-gradient(135deg, oklch(0.55 0.12 130), oklch(0.40 0.15 160))", name: "Grove, August", artist: "Mara Idris" },
              { bg: "linear-gradient(135deg, oklch(0.60 0.18 320), oklch(0.42 0.22 280))", name: "Folded Light", artist: "Lev Marin" },
            ].map((nft, i) => (
              <div key={i} className="group cursor-pointer">
                <div
                  className="aspect-square rounded-2xl overflow-hidden transition-transform duration-300 group-hover:scale-[1.02]"
                  style={{ backgroundImage: nft.bg }}
                />
                <p className="font-display font-semibold text-sm text-ink mt-2 truncate">{nft.name}</p>
                <p className="text-[11px] text-muted-foreground">{nft.artist}</p>
              </div>
            ))}
          </div>
        </section>
      </div>
    </AppLayout>
  );
}
