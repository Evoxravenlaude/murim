import { createFileRoute, Link, useParams, notFound } from "@tanstack/react-router";
import { useQuery } from "@tanstack/react-query";
import { ArrowLeft, ShieldCheck, Sparkles, Heart, Share2, ExternalLink, Eye, Tag } from "lucide-react";
import { useState } from "react";
import { AppLayout } from "@/components/nautica/AppLayout";
import { getMarket } from "@/lib/nautica.functions";

export const Route = createFileRoute("/market/$id")(
  {
    head: ({ params }) => ({
      meta: [
        { title: `Edition ${params.id} — Nautica` },
        { name: "description", content: "On-chain edition details, provenance, and private buy on Nautica." },
        { property: "og:title", content: `Edition ${params.id} — Nautica` },
      ],
    }),
    component: ItemPage,
    notFoundComponent: () => (
      <AppLayout>
        <div className="text-center py-20 animate-fade-up">
          <div className="w-16 h-16 mx-auto rounded-3xl bg-muted grid place-items-center mb-4">
            <span className="text-2xl">🔍</span>
          </div>
          <p className="font-display text-xl font-semibold text-ink">Edition not found</p>
          <Link to="/market" className="btn-primary mt-4 text-xs">Back to market</Link>
        </div>
      </AppLayout>
    ),
  },
);

function ItemPage() {
  const { id } = useParams({ from: "/market/$id" });
  const [liked, setLiked] = useState(false);
  const [tab, setTab] = useState<"details" | "provenance" | "traits">("details");
  const { data = [], isLoading } = useQuery({ queryKey: ["market"], queryFn: () => getMarket() });
  const item = data.find((x) => x.id === id);

  if (!isLoading && !item) throw notFound();

  const mockTraits = [
    { key: "Background", value: "Deep Ocean", rarity: "12%" },
    { key: "Palette", value: "Chromatic", rarity: "8%" },
    { key: "Complexity", value: "High", rarity: "15%" },
    { key: "Resolution", value: "4K", rarity: "45%" },
    { key: "Style", value: "Abstract", rarity: "22%" },
    { key: "Generation", value: "v3.2", rarity: "5%" },
  ];

  return (
    <AppLayout>
      {/* Back link */}
      <Link
        to="/market"
        className="inline-flex items-center gap-2 text-sm text-muted-foreground hover:text-foreground transition-colors font-display font-medium animate-fade-down"
      >
        <ArrowLeft className="w-4 h-4" /> Market
      </Link>

      <div className="mt-5 grid lg:grid-cols-[1.1fr_1fr] gap-6 animate-fade-up">
        {/* Artwork */}
        <div className="relative group">
          <div
            className="nautica-card overflow-hidden aspect-[4/5] w-full relative"
            style={{ backgroundImage: item?.cover }}
          >
            {/* Floating actions on image */}
            <div className="absolute top-4 right-4 flex gap-2">
              <button
                onClick={() => setLiked(!liked)}
                className={
                  "w-10 h-10 rounded-full backdrop-blur-xl grid place-items-center transition-all " +
                  (liked ? "bg-destructive/80 text-white" : "bg-black/20 text-white/80 hover:bg-black/40")
                }
              >
                <Heart className="w-4 h-4" fill={liked ? "currentColor" : "none"} />
              </button>
              <button className="w-10 h-10 rounded-full bg-black/20 backdrop-blur-xl text-white/80 grid place-items-center hover:bg-black/40 transition">
                <Share2 className="w-4 h-4" />
              </button>
            </div>

            {/* Views count */}
            <div className="absolute bottom-4 left-4 flex items-center gap-1.5 px-3 py-1.5 rounded-full bg-black/20 backdrop-blur-xl text-white/80 text-xs font-medium">
              <Eye className="w-3 h-3" /> 1.2k views
            </div>
          </div>
        </div>

        {/* Info panel */}
        <div className="flex flex-col gap-4">
          {/* Title card */}
          <div className="nautica-glass p-6">
            <p className="text-sm text-primary font-display font-semibold">{item?.artist ?? "—"}</p>
            <h1 className="font-display text-3xl md:text-4xl font-bold text-ink leading-tight mt-1">
              {item?.title ?? "Loading…"}
            </h1>
            <div className="flex flex-wrap gap-2 mt-4">
              <span className="nautica-pill border border-border">{item?.edition}</span>
              <span className="nautica-pill border border-border">{item?.category}</span>
              <span className="badge-success"><ShieldCheck className="w-3 h-3" /> Verified</span>
            </div>

            {/* Price & stats */}
            <div className="mt-6 grid grid-cols-2 gap-3">
              <div className="stat-card">
                <p className="text-[10px] uppercase tracking-widest text-muted-foreground font-semibold">Price</p>
                <p className="font-display text-3xl font-bold text-ink leading-none mt-2">
                  {item?.price}
                  <span className="text-sm text-muted-foreground ml-1 font-medium">{item?.currency}</span>
                </p>
              </div>
              <div className="rounded-2xl bg-primary/10 border border-primary/20 p-4">
                <p className="text-[10px] uppercase tracking-widest text-primary font-semibold">Royalty</p>
                <p className="font-display text-3xl font-bold text-primary leading-none mt-2">7.5%</p>
              </div>
            </div>

            {/* Buy buttons */}
            <div className="mt-6 flex flex-wrap gap-3">
              <button className="btn-cta flex-1">
                <Tag className="w-4 h-4" /> Buy Edition
              </button>
              <Link to="/send" className="btn-primary flex-1">
                <ShieldCheck className="w-4 h-4" /> Pay Privately
              </Link>
            </div>
          </div>

          {/* Tabs */}
          <div className="nautica-glass overflow-hidden">
            <div className="flex border-b border-border">
              {(["details", "provenance", "traits"] as const).map((t) => (
                <button
                  key={t}
                  onClick={() => setTab(t)}
                  className={
                    "flex-1 px-4 py-3 text-sm font-display font-semibold capitalize transition-all " +
                    (tab === t
                      ? "text-primary border-b-2 border-primary bg-primary/5"
                      : "text-muted-foreground hover:text-foreground")
                  }
                >
                  {t}
                </button>
              ))}
            </div>

            <div className="p-5">
              {tab === "details" && (
                <div className="space-y-4 animate-fade-in">
                  <div className="flex items-center justify-between">
                    <span className="text-sm text-muted-foreground">Contract</span>
                    <span className="text-sm text-ink font-mono">0x7af3…c8b1</span>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="text-sm text-muted-foreground">Token ID</span>
                    <span className="text-sm text-ink font-mono">#{item?.id}</span>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="text-sm text-muted-foreground">Chain</span>
                    <span className="text-sm text-ink">Ethereum</span>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="text-sm text-muted-foreground">Standard</span>
                    <span className="text-sm text-ink">ERC-721</span>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="text-sm text-muted-foreground">Owners</span>
                    <span className="text-sm text-ink">{item?.owners ?? "—"}</span>
                  </div>
                </div>
              )}

              {tab === "provenance" && (
                <div className="space-y-3 animate-fade-in">
                  {[
                    { action: `Minted by ${item?.artist}`, time: "3d ago", icon: Sparkles },
                    { action: "Listed on Nautica", time: "2d ago", icon: Tag },
                    { action: "2 transfers · ZK verified", time: "1d ago", icon: ShieldCheck },
                  ].map((p, i) => (
                    <div key={i} className="flex items-center gap-3 p-3 rounded-xl bg-muted/50">
                      <div className="w-8 h-8 rounded-xl bg-primary/10 grid place-items-center shrink-0">
                        <p.icon className="w-4 h-4 text-primary" />
                      </div>
                      <div className="flex-1 min-w-0">
                        <p className="text-sm text-ink font-medium">{p.action}</p>
                      </div>
                      <span className="text-xs text-muted-foreground shrink-0">{p.time}</span>
                    </div>
                  ))}
                </div>
              )}

              {tab === "traits" && (
                <div className="grid grid-cols-2 gap-2 animate-fade-in">
                  {mockTraits.map((t) => (
                    <div key={t.key} className="p-3 rounded-xl bg-muted/50 border border-border/50">
                      <p className="text-[10px] uppercase tracking-wider text-primary font-semibold">{t.key}</p>
                      <p className="text-sm text-ink font-medium mt-0.5">{t.value}</p>
                      <p className="text-[10px] text-muted-foreground mt-1">{t.rarity} have this</p>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </div>

          {/* Drop info */}
          <div className="nautica-glass p-5 flex items-center gap-3 relative overflow-hidden">
            <div aria-hidden className="absolute -top-6 -right-6 w-20 h-20 rounded-full bg-primary/10 blur-2xl" />
            <div className="w-10 h-10 rounded-xl bg-primary/10 grid place-items-center shrink-0">
              <Sparkles className="w-5 h-5 text-primary" />
            </div>
            <div className="relative">
              <p className="text-sm text-ink font-medium">
                Part of the <strong className="text-primary">Saltwater Index</strong> drop
              </p>
              <p className="text-xs text-muted-foreground mt-0.5">12 pieces remain · Ends in 4h</p>
            </div>
          </div>
        </div>
      </div>
    </AppLayout>
  );
}
