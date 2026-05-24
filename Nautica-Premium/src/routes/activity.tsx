import { createFileRoute } from "@tanstack/react-router";
import { useQuery } from "@tanstack/react-query";
import { useState } from "react";
import {
  ArrowDownLeft,
  ArrowUpRight,
  ShoppingBag,
  Sparkles,
  ArrowLeftRight,
  Tag,
  ExternalLink,
  Filter,
  TrendingUp,
  TrendingDown,
} from "lucide-react";
import { AppLayout } from "@/components/nautica/AppLayout";
import { getActivity } from "@/lib/nautica.functions";
import type { Activity } from "@/lib/mock-data";

export const Route = createFileRoute("/activity")(
  {
    head: () => ({
      meta: [
        { title: "Activity — Nautica" },
        { name: "description", content: "Your Nautica activity: collected pieces, private payments, swaps, and proof status." },
        { property: "og:title", content: "Activity — Nautica" },
      ],
    }),
    component: ActivityPage,
  },
);

const ICON_MAP: Record<Activity["kind"], typeof ShoppingBag> = {
  buy: ShoppingBag,
  send: ArrowUpRight,
  receive: ArrowDownLeft,
  mint: Sparkles,
  list: Tag,
  swap: ArrowLeftRight,
};

const KIND_COLORS: Record<Activity["kind"], string> = {
  buy: "bg-primary/10 text-primary",
  send: "bg-highlight/10 text-highlight",
  receive: "bg-success/10 text-success",
  mint: "bg-accent text-accent-foreground",
  list: "bg-muted text-muted-foreground",
  swap: "bg-feature/30 text-feature-foreground",
};

const PROOF_BADGES: Record<Activity["proof"], { label: string; class: string }> = {
  verified: { label: "ZK Verified", class: "badge-success" },
  pending: { label: "Proving…", class: "badge-warning" },
  none: { label: "", class: "" },
};

function ActivityPage() {
  const { data = [] } = useQuery({ queryKey: ["activity"], queryFn: () => getActivity() });
  const [filter, setFilter] = useState<Activity["kind"] | "all">("all");

  const filtered = filter === "all" ? data : data.filter((a) => a.kind === filter);

  // Quick stats
  const totalSpent = data.filter((a) => a.kind === "buy" || a.kind === "send").reduce((sum, a) => sum + a.amount, 0);
  const totalReceived = data.filter((a) => a.kind === "receive").reduce((sum, a) => sum + a.amount, 0);
  const zkCount = data.filter((a) => a.proof === "verified").length;

  return (
    <AppLayout>
      <header className="flex items-end justify-between gap-4 flex-wrap animate-fade-down">
        <div>
          <p className="text-xs text-primary font-display font-semibold uppercase tracking-widest">History</p>
          <h1 className="font-display text-4xl md:text-5xl font-bold text-ink mt-1">Activity</h1>
        </div>
        <span className="stat-card px-3 py-2 text-xs">
          <span className="font-display font-semibold text-ink">{data.length}</span>
          <span className="text-muted-foreground ml-1">transactions</span>
        </span>
      </header>

      {/* Stats row */}
      <div className="mt-6 grid grid-cols-2 md:grid-cols-4 gap-3 stagger-children">
        <div className="stat-card animate-fade-up">
          <div className="flex items-center gap-2 mb-2">
            <TrendingDown className="w-4 h-4 text-destructive" />
            <p className="text-[10px] uppercase tracking-widest text-muted-foreground font-semibold">Spent</p>
          </div>
          <p className="font-display text-2xl font-bold text-ink leading-none">
            {totalSpent.toFixed(2)} <span className="text-xs text-muted-foreground font-medium">ETH</span>
          </p>
        </div>
        <div className="stat-card animate-fade-up" style={{ borderColor: "oklch(0.72 0.18 155 / 0.2)" }}>
          <div className="flex items-center gap-2 mb-2">
            <TrendingUp className="w-4 h-4 text-success" />
            <p className="text-[10px] uppercase tracking-widest text-muted-foreground font-semibold">Received</p>
          </div>
          <p className="font-display text-2xl font-bold text-ink leading-none">
            {totalReceived.toFixed(2)} <span className="text-xs text-muted-foreground font-medium">ETH</span>
          </p>
        </div>
        <div className="stat-card animate-fade-up">
          <div className="flex items-center gap-2 mb-2">
            <Sparkles className="w-4 h-4 text-primary" />
            <p className="text-[10px] uppercase tracking-widest text-muted-foreground font-semibold">Minted</p>
          </div>
          <p className="font-display text-2xl font-bold text-ink leading-none">
            {data.filter((a) => a.kind === "mint").length}
          </p>
        </div>
        <div className="stat-card animate-fade-up" style={{ borderColor: "oklch(0.72 0.16 180 / 0.2)" }}>
          <div className="flex items-center gap-2 mb-2">
            <ShoppingBag className="w-4 h-4 text-primary" />
            <p className="text-[10px] uppercase tracking-widest text-muted-foreground font-semibold">ZK Proofs</p>
          </div>
          <p className="font-display text-2xl font-bold text-ink leading-none">
            {zkCount} / {data.length}
          </p>
        </div>
      </div>

      {/* Filter pills */}
      <div className="mt-5 flex gap-2 overflow-x-auto pb-1 animate-fade-up" style={{ animationDelay: "200ms" }}>
        <div className="flex items-center gap-1.5 text-xs text-muted-foreground mr-1 shrink-0">
          <Filter className="w-3 h-3" />
        </div>
        {(["all", "buy", "send", "receive", "mint", "swap", "list"] as const).map((f) => (
          <button
            key={f}
            onClick={() => setFilter(f)}
            className={
              "shrink-0 px-3 py-2 rounded-full text-xs font-display font-semibold capitalize transition-all duration-200 " +
              (filter === f
                ? "bg-primary text-primary-foreground"
                : "bg-muted text-muted-foreground hover:bg-secondary")
            }
          >
            {f}
          </button>
        ))}
      </div>

      {/* Transaction list */}
      <div className="mt-5 nautica-glass overflow-hidden divide-y divide-border/50 animate-fade-up" style={{ animationDelay: "300ms" }}>
        {filtered.map((a) => {
          const Icon = ICON_MAP[a.kind];
          const colorClass = KIND_COLORS[a.kind];
          const proof = PROOF_BADGES[a.proof];

          return (
            <div
              key={a.id}
              className="flex items-center gap-4 p-4 md:p-5 hover:bg-muted/30 transition-colors group cursor-pointer"
            >
              <div className={"w-11 h-11 rounded-2xl grid place-items-center shrink-0 " + colorClass}>
                <Icon className="w-5 h-5" />
              </div>
              <div className="flex-1 min-w-0">
                <p className="font-display font-semibold text-ink text-sm truncate">{a.title}</p>
                <div className="flex items-center gap-2 mt-0.5 flex-wrap">
                  <span className="text-[11px] text-muted-foreground">{a.at}</span>
                  {proof.label && (
                    <span className={proof.class + " text-[10px]"}>
                      {proof.label}
                    </span>
                  )}
                  {a.hash && (
                    <span className="text-[10px] text-muted-foreground font-mono hidden sm:inline">
                      {a.hash}
                    </span>
                  )}
                </div>
              </div>
              {a.amount > 0 && (
                <div className="text-right shrink-0">
                  <p className="font-display text-lg font-bold text-ink leading-none">
                    {a.kind === "send" ? "−" : a.kind === "receive" ? "+" : ""}
                    {a.amount}
                  </p>
                  <p className="text-[10px] text-muted-foreground mt-0.5">{a.currency}</p>
                </div>
              )}
              <ExternalLink className="w-4 h-4 text-muted-foreground opacity-0 group-hover:opacity-100 transition-opacity shrink-0" />
            </div>
          );
        })}
      </div>
    </AppLayout>
  );
}
