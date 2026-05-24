import { createFileRoute } from "@tanstack/react-router";
import { useQuery } from "@tanstack/react-query";
import { useState } from "react";
import { Clock, Flame, Users, Zap } from "lucide-react";
import { AppLayout } from "@/components/nautica/AppLayout";
import { getDrops } from "@/lib/nautica.functions";

export const Route = createFileRoute("/drops")(
  {
    head: () => ({
      meta: [
        { title: "Drops — Nautica" },
        { name: "description", content: "Live and upcoming drops on Nautica. Curated editions from working artists." },
        { property: "og:title", content: "Drops — Nautica" },
      ],
    }),
    component: DropsPage,
  },
);

const STATUS_CONFIG = {
  live: { label: "Live", color: "badge-success", icon: Flame },
  upcoming: { label: "Upcoming", color: "badge-warning", icon: Clock },
  ended: { label: "Ended", color: "nautica-pill", icon: Clock },
} as const;

function DropsPage() {
  const { data = [] } = useQuery({ queryKey: ["drops"], queryFn: () => getDrops() });
  const [filter, setFilter] = useState<"all" | "live" | "upcoming" | "ended">("all");

  const filtered = filter === "all" ? data : data.filter((d) => d.status === filter);

  return (
    <AppLayout>
      {/* Header */}
      <header className="flex items-end justify-between gap-4 flex-wrap animate-fade-down">
        <div>
          <p className="text-xs text-primary font-display font-semibold uppercase tracking-widest">This week</p>
          <h1 className="font-display text-4xl md:text-5xl font-bold text-ink mt-1">Drops</h1>
        </div>
        <div className="flex items-center gap-2">
          <span className="stat-card px-3 py-2 text-xs flex items-center gap-1.5">
            <Flame className="w-3 h-3 text-success" />
            <span className="font-display font-semibold text-ink">{data.filter((d) => d.status === "live").length}</span>
            <span className="text-muted-foreground">live now</span>
          </span>
        </div>
      </header>

      {/* Filter pills */}
      <div className="mt-6 flex gap-2 animate-fade-up" style={{ animationDelay: "100ms" }}>
        {(["all", "live", "upcoming", "ended"] as const).map((f) => (
          <button
            key={f}
            onClick={() => setFilter(f)}
            className={
              "px-4 py-2.5 rounded-full text-sm font-display font-semibold capitalize transition-all duration-200 " +
              (filter === f
                ? "bg-primary text-primary-foreground shadow-soft"
                : "bg-muted text-muted-foreground hover:bg-secondary")
            }
          >
            {f}
          </button>
        ))}
      </div>

      {/* Drops list */}
      <div className="mt-6 grid gap-4 stagger-children">
        {filtered.map((d) => {
          const config = STATUS_CONFIG[d.status];
          const mintProgress = d.pieces > 0 ? (d.minted / d.pieces) * 100 : 0;

          return (
            <article
              key={d.id}
              className="nautica-glass overflow-hidden group animate-fade-up hover:border-primary/20 transition-all duration-300"
            >
              <div className="flex flex-col sm:flex-row gap-0">
                {/* Cover */}
                <div className="sm:w-64 h-52 sm:h-auto shrink-0 relative overflow-hidden">
                  <div
                    className="absolute inset-0 transition-transform duration-500 group-hover:scale-105"
                    style={{ backgroundImage: d.cover }}
                  />
                  {d.status === "live" && (
                    <div className="absolute top-3 left-3 px-2.5 py-1 rounded-full bg-success/90 backdrop-blur-sm text-success-foreground text-[10px] font-bold uppercase tracking-wider flex items-center gap-1">
                      <span className="w-1.5 h-1.5 rounded-full bg-success-foreground animate-pulse" />
                      Live
                    </div>
                  )}
                </div>

                {/* Content */}
                <div className="flex-1 min-w-0 p-5 sm:p-6 flex flex-col justify-between gap-4">
                  <div>
                    <div className="flex items-center gap-2 flex-wrap">
                      <span className={config.color + " flex items-center gap-1"}>
                        <config.icon className="w-3 h-3" />
                        {config.label}
                      </span>
                      <span className="text-xs text-muted-foreground">{d.startsAt}</span>
                    </div>
                    <h2 className="font-display text-2xl md:text-3xl font-bold text-ink mt-2 leading-tight">
                      {d.title}
                    </h2>
                    <p className="text-sm text-muted-foreground mt-1">
                      by <span className="text-primary font-medium">{d.artist}</span>
                    </p>
                  </div>

                  <div className="space-y-3">
                    {/* Mint progress */}
                    <div>
                      <div className="flex items-center justify-between text-xs mb-1.5">
                        <span className="text-muted-foreground font-medium">
                          <Users className="w-3 h-3 inline mr-1" />
                          {d.minted} / {d.pieces} minted
                        </span>
                        <span className="text-ink font-display font-semibold">
                          {mintProgress.toFixed(0)}%
                        </span>
                      </div>
                      <div className="h-2 rounded-full bg-muted overflow-hidden">
                        <div
                          className="h-full rounded-full bg-primary transition-all duration-700 ease-out"
                          style={{ width: `${mintProgress}%` }}
                        />
                      </div>
                    </div>

                    {/* Price & action */}
                    <div className="flex items-center justify-between gap-3">
                      <div>
                        <p className="text-[10px] uppercase tracking-wider text-muted-foreground font-semibold">
                          Mint Price
                        </p>
                        <p className="font-display text-xl font-bold text-ink">
                          {d.price} <span className="text-xs text-muted-foreground font-medium">{d.currency}</span>
                        </p>
                      </div>
                      <button
                        className={
                          d.status === "live"
                            ? "btn-cta"
                            : d.status === "upcoming"
                            ? "btn-primary"
                            : "btn-secondary"
                        }
                      >
                        {d.status === "live" && <><Zap className="w-4 h-4" /> Mint Now</>}
                        {d.status === "upcoming" && <><Clock className="w-4 h-4" /> Remind Me</>}
                        {d.status === "ended" && "View Collection"}
                      </button>
                    </div>
                  </div>
                </div>
              </div>
            </article>
          );
        })}
      </div>
    </AppLayout>
  );
}
