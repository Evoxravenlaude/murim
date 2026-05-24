import { createFileRoute } from "@tanstack/react-router";
import { useQuery } from "@tanstack/react-query";
import { useState, useEffect } from "react";
import { Search, SlidersHorizontal, Grid3X3, LayoutList, TrendingUp, ArrowUpDown, Flame } from "lucide-react";
import { AppLayout } from "@/components/nautica/AppLayout";
import { ArtCard, ArtCardSkeleton } from "@/components/nautica/ArtCard";
import { getMarket, getCollections } from "@/lib/nautica.functions";
import type { MarketItem } from "@/lib/mock-data";

const CATS: Array<MarketItem["category"] | "All"> = ["All", "Generative", "Photography", "Painting", "3D", "Music", "PFP"];
const SORTS = ["Trending", "Price: Low", "Price: High", "Recent"] as const;

const LIVE_FEED = [
  { text: "Tideglass 04 sold for 0.42 ETH", time: "2m ago" },
  { text: "Neural Bloom listed at 2.10 ETH", time: "8m ago" },
  { text: "Pulse Sequence minted by J. Moreau", time: "15m ago" },
  { text: "Reef Memory offer received: 0.12 ETH", time: "22m ago" },
];

export const Route = createFileRoute("/market")({
  head: () => ({
    meta: [
      { title: "Market — Nautica" },
      { name: "description", content: "Browse curated on-chain art on Nautica." },
      { property: "og:title", content: "Market — Nautica" },
    ],
  }),
  component: MarketPage,
});

function MarketPage() {
  const [cat, setCat] = useState<(typeof CATS)[number]>("All");
  const [q, setQ] = useState("");
  const [sort, setSort] = useState<(typeof SORTS)[number]>("Trending");
  const [view, setView] = useState<"grid" | "list">("grid");
  const [showFilters, setShowFilters] = useState(false);
  const [feedIdx, setFeedIdx] = useState(0);

  const { data = [], isLoading } = useQuery({ queryKey: ["market"], queryFn: () => getMarket() });
  const { data: collections = [] } = useQuery({ queryKey: ["collections"], queryFn: () => getCollections() });

  useEffect(() => {
    const t = setInterval(() => setFeedIdx((i) => (i + 1) % LIVE_FEED.length), 4000);
    return () => clearInterval(t);
  }, []);

  let items = data.filter(
    (i) => (cat === "All" || i.category === cat) &&
      (q === "" || `${i.title} ${i.artist}`.toLowerCase().includes(q.toLowerCase())),
  );
  if (sort === "Price: Low") items = [...items].sort((a, b) => a.price - b.price);
  if (sort === "Price: High") items = [...items].sort((a, b) => b.price - a.price);

  return (
    <AppLayout>
      {/* Header */}
      <div className="flex items-end justify-between gap-4 flex-wrap animate-fade-down">
        <div>
          <p className="text-xs text-primary font-display font-semibold uppercase tracking-widest">Explore</p>
          <h1 className="font-display text-4xl md:text-5xl font-bold text-ink mt-1">Market</h1>
        </div>
        <span className="stat-card px-3 py-2 text-xs">
          <TrendingUp className="w-3 h-3 text-primary inline mr-1" />
          <span className="font-display font-semibold text-ink">{data.length}</span>
          <span className="text-muted-foreground ml-1">pieces</span>
        </span>
      </div>

      {/* Live ticker */}
      <div className="mt-4 nautica-glass px-4 py-2.5 flex items-center gap-3 overflow-hidden animate-fade-up" style={{ animationDelay: "50ms" }}>
        <div className="flex items-center gap-1.5 shrink-0">
          <span className="w-2 h-2 rounded-full bg-success animate-pulse" />
          <span className="text-[10px] text-success font-display font-bold uppercase tracking-wider">Live</span>
        </div>
        <div className="flex-1 min-w-0 relative h-5 overflow-hidden">
          {LIVE_FEED.map((item, i) => (
            <div key={i} className={"absolute inset-0 flex items-center gap-2 transition-all duration-500 " + (i === feedIdx ? "opacity-100 translate-y-0" : "opacity-0 translate-y-4")}>
              <span className="text-xs text-foreground font-medium truncate">{item.text}</span>
              <span className="text-[10px] text-muted-foreground shrink-0">{item.time}</span>
            </div>
          ))}
        </div>
      </div>

      {/* Trending collections */}
      {collections.length > 0 && (
        <div className="mt-4 animate-fade-up" style={{ animationDelay: "80ms" }}>
          <div className="flex items-center gap-2 mb-3">
            <Flame className="w-4 h-4 text-highlight" />
            <span className="text-xs uppercase tracking-widest font-display font-semibold text-muted-foreground">Trending Collections</span>
          </div>
          <div className="flex gap-3 overflow-x-auto pb-2 scrollbar-none">
            {collections.map((c, i) => (
              <div key={c.id} className="shrink-0 w-56 nautica-glass p-3 cursor-pointer group hover:border-primary/20 transition-all duration-300 animate-fade-up" style={{ animationDelay: `${i * 80 + 100}ms` }}>
                <div className="flex items-center gap-3">
                  <div className="w-12 h-12 rounded-xl shrink-0 group-hover:scale-105 transition-transform duration-300" style={{ backgroundImage: c.cover }} />
                  <div className="min-w-0 flex-1">
                    <p className="font-display font-semibold text-ink text-sm truncate">{c.name}</p>
                    <div className="flex items-center gap-2 mt-0.5">
                      <span className="text-[10px] text-muted-foreground">Floor: <span className="text-ink font-semibold">{c.floor}</span></span>
                      <span className={"text-[10px] font-semibold " + (c.change24h >= 0 ? "text-success" : "text-destructive")}>{c.change24h >= 0 ? "+" : ""}{c.change24h}%</span>
                    </div>
                  </div>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Search */}
      <div className="mt-5 flex gap-3 animate-fade-up" style={{ animationDelay: "100ms" }}>
        <div className="flex-1 nautica-glass flex items-center gap-2 px-4 py-1">
          <Search className="w-4 h-4 text-muted-foreground shrink-0" />
          <input value={q} onChange={(e) => setQ(e.target.value)} placeholder="Search artist, title, or collection…" className="flex-1 bg-transparent outline-none text-sm py-2.5 text-foreground placeholder:text-muted-foreground" />
          {q && <button onClick={() => setQ("")} className="text-muted-foreground hover:text-foreground text-xs">✕</button>}
        </div>
        <button onClick={() => setShowFilters(!showFilters)} className={"btn-icon " + (showFilters ? "!bg-primary !text-primary-foreground !border-primary" : "")}>
          <SlidersHorizontal className="w-4 h-4" />
        </button>
        <button onClick={() => setView(view === "grid" ? "list" : "grid")} className="btn-icon">
          {view === "grid" ? <LayoutList className="w-4 h-4" /> : <Grid3X3 className="w-4 h-4" />}
        </button>
      </div>

      {/* Category pills */}
      <div className="mt-4 flex gap-2 overflow-x-auto pb-1 scrollbar-none animate-fade-up" style={{ animationDelay: "150ms" }}>
        {CATS.map((c) => (
          <button key={c} onClick={() => setCat(c)} className={"shrink-0 px-4 py-2.5 rounded-full text-sm font-display font-semibold transition-all duration-200 " + (cat === c ? "bg-primary text-primary-foreground shadow-soft" : "bg-muted text-muted-foreground hover:bg-secondary hover:text-foreground")}>{c}</button>
        ))}
      </div>

      {/* Sort */}
      {showFilters && (
        <div className="mt-3 nautica-glass p-3 flex flex-wrap gap-2 animate-fade-down">
          <div className="flex items-center gap-1.5 text-xs text-muted-foreground mr-2">
            <ArrowUpDown className="w-3 h-3" /><span className="font-display font-semibold">Sort by</span>
          </div>
          {SORTS.map((s) => (
            <button key={s} onClick={() => setSort(s)} className={"px-3 py-1.5 rounded-full text-xs font-medium transition-all " + (sort === s ? "bg-primary/10 text-primary border border-primary/20" : "bg-muted text-muted-foreground hover:text-foreground")}>{s}</button>
          ))}
        </div>
      )}

      {/* Results count */}
      {!isLoading && (
        <div className="mt-4 flex items-center gap-2 text-xs text-muted-foreground animate-fade-in">
          <span className="font-display font-semibold text-ink">{items.length}</span>
          <span>result{items.length !== 1 ? "s" : ""}</span>
          {cat !== "All" && <span>in <span className="text-primary">{cat}</span></span>}
          {q && <span>matching "<span className="text-ink">{q}</span>"</span>}
        </div>
      )}

      {/* Grid */}
      <div className={"mt-4 gap-4 stagger-children " + (view === "grid" ? "grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3" : "grid grid-cols-1")}>
        {isLoading ? Array.from({ length: 6 }).map((_, i) => <ArtCardSkeleton key={i} />) : items.map((item, i) => <ArtCard key={item.id} item={item} index={i} />)}
      </div>

      {/* Empty */}
      {!isLoading && items.length === 0 && (
        <div className="mt-12 text-center animate-fade-up">
          <div className="w-16 h-16 mx-auto rounded-3xl bg-muted grid place-items-center mb-4"><Search className="w-6 h-6 text-muted-foreground" /></div>
          <p className="font-display text-xl font-semibold text-ink">No pieces found</p>
          <p className="text-sm text-muted-foreground mt-1">Try adjusting your search or filters.</p>
          <button onClick={() => { setCat("All"); setQ(""); }} className="btn-secondary mt-4 text-xs">Clear filters</button>
        </div>
      )}
    </AppLayout>
  );
}
