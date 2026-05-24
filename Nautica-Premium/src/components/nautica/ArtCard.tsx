import { Link } from "@tanstack/react-router";
import { Heart, Eye } from "lucide-react";
import { useState } from "react";
import type { MarketItem } from "@/lib/mock-data";

export function ArtCard({ item, index = 0 }: { item: MarketItem; index?: number }) {
  const [liked, setLiked] = useState(false);
  const [tilt, setTilt] = useState({ x: 0, y: 0 });

  function handleMouseMove(e: React.MouseEvent<HTMLDivElement>) {
    const rect = e.currentTarget.getBoundingClientRect();
    const x = (e.clientX - rect.left) / rect.width;
    const y = (e.clientY - rect.top) / rect.height;
    setTilt({
      x: (y - 0.5) * -8,
      y: (x - 0.5) * 8,
    });
  }

  function handleMouseLeave() {
    setTilt({ x: 0, y: 0 });
  }

  return (
    <div
      className="animate-fade-up"
      style={{ animationDelay: `${index * 60}ms` }}
    >
      <div
        onMouseMove={handleMouseMove}
        onMouseLeave={handleMouseLeave}
        className="group relative"
        style={{
          transform: `perspective(800px) rotateX(${tilt.x}deg) rotateY(${tilt.y}deg)`,
          transition: "transform 0.15s ease-out",
        }}
      >
        <Link
          to="/market/$id"
          params={{ id: item.id }}
          className="block nautica-card overflow-hidden cursor-pointer transition-all duration-300 hover:shadow-elevated hover:border-primary/20"
        >
          {/* Cover with gradient */}
          <div className="relative aspect-[4/5] w-full overflow-hidden">
            <div
              className="absolute inset-0 transition-transform duration-500 group-hover:scale-105"
              style={{ backgroundImage: item.cover }}
            />
            {/* Overlay gradient */}
            <div className="absolute inset-0 bg-gradient-to-t from-black/40 via-transparent to-transparent opacity-0 group-hover:opacity-100 transition-opacity duration-300" />

            {/* Floating actions */}
            <div className="absolute top-3 right-3 flex gap-2 opacity-0 group-hover:opacity-100 transition-all duration-300 translate-y-2 group-hover:translate-y-0">
              <button
                onClick={(e) => {
                  e.preventDefault();
                  e.stopPropagation();
                  setLiked(!liked);
                }}
                className={
                  "w-9 h-9 rounded-full backdrop-blur-xl grid place-items-center transition-all duration-200 " +
                  (liked
                    ? "bg-destructive/90 text-white"
                    : "bg-black/30 text-white/90 hover:bg-black/50")
                }
              >
                <Heart
                  className="w-4 h-4"
                  fill={liked ? "currentColor" : "none"}
                />
              </button>
              <div className="w-9 h-9 rounded-full bg-black/30 backdrop-blur-xl grid place-items-center text-white/90">
                <Eye className="w-4 h-4" />
              </div>
            </div>

            {/* Category tag */}
            <div className="absolute top-3 left-3">
              <span className="px-2.5 py-1 rounded-full bg-black/30 backdrop-blur-xl text-white text-[10px] font-semibold uppercase tracking-wider">
                {item.category}
              </span>
            </div>

            {/* Edition badge */}
            <div className="absolute bottom-3 left-3 opacity-0 group-hover:opacity-100 transition-all duration-300 translate-y-2 group-hover:translate-y-0">
              <span className="px-2.5 py-1 rounded-full bg-white/20 backdrop-blur-xl text-white text-[11px] font-medium">
                {item.edition}
              </span>
            </div>
          </div>

          {/* Info */}
          <div className="p-4">
            <div className="flex items-start justify-between gap-3">
              <div className="min-w-0 flex-1">
                <p className="text-[11px] text-muted-foreground font-medium truncate">
                  {item.artist}
                </p>
                <h3 className="font-display text-lg font-semibold text-ink leading-tight truncate mt-0.5">
                  {item.title}
                </h3>
              </div>
              <div className="text-right shrink-0">
                <p className="text-[10px] uppercase tracking-wider text-muted-foreground font-medium">
                  Price
                </p>
                <p className="font-display text-lg font-bold text-ink leading-none mt-0.5">
                  {item.price}
                  <span className="text-xs ml-1 text-muted-foreground font-medium">
                    {item.currency}
                  </span>
                </p>
              </div>
            </div>

            {/* Quick buy button */}
            <button
              onClick={(e) => e.preventDefault()}
              className="mt-3 w-full btn-primary py-2.5 text-xs opacity-0 group-hover:opacity-100 transition-all duration-300 translate-y-1 group-hover:translate-y-0"
            >
              Buy Now
            </button>
          </div>
        </Link>
      </div>
    </div>
  );
}

/* Skeleton loading card */
export function ArtCardSkeleton() {
  return (
    <div className="nautica-card overflow-hidden">
      <div className="aspect-[4/5] w-full bg-muted animate-shimmer" />
      <div className="p-4 space-y-3">
        <div className="h-3 w-1/3 bg-muted rounded-full animate-shimmer" />
        <div className="h-5 w-2/3 bg-muted rounded-full animate-shimmer" />
        <div className="h-3 w-1/4 bg-muted rounded-full animate-shimmer" />
      </div>
    </div>
  );
}
