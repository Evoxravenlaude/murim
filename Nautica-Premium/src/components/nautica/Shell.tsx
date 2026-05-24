import { Link, useLocation } from "@tanstack/react-router";
import {
  Compass,
  Sparkles,
  Send,
  Activity as ActivityIcon,
  User,
  Plus,
  Wallet,
  ArrowLeftRight,
} from "lucide-react";
import type { ComponentType } from "react";
import { ThemeToggle } from "./ThemeToggle";
import logo from "@/assets/nautica-logo.png";

type Item = {
  to: string;
  label: string;
  Icon: ComponentType<{ className?: string }>;
};

const NAV: Item[] = [
  { to: "/market", label: "Market", Icon: Compass },
  { to: "/drops", label: "Drops", Icon: Sparkles },
  { to: "/create", label: "Create", Icon: Plus },
  { to: "/send", label: "Send", Icon: Send },
  { to: "/swap", label: "Swap", Icon: ArrowLeftRight },
  { to: "/activity", label: "Activity", Icon: ActivityIcon },
  { to: "/profile", label: "Profile", Icon: User },
];

export function SideRail() {
  const { pathname } = useLocation();
  return (
    <aside className="hidden md:flex md:w-[260px] lg:w-[280px] shrink-0 flex-col gap-2 p-5 sticky top-0 h-screen border-r border-border/50">
      {/* Logo */}
      <Link to="/" className="flex items-center gap-3 px-3 py-3 mb-2 group">
        <div className="w-10 h-10 rounded-2xl bg-primary/10 border border-primary/20 grid place-items-center overflow-hidden transition-all duration-300 group-hover:bg-primary/20 group-hover:shadow-glow-primary">
          <img src={logo} alt="" width={28} height={28} className="w-7 h-7" />
        </div>
        <div>
          <span className="font-display text-xl font-bold text-ink tracking-tight">
            Nautica
          </span>
          <span className="block text-[10px] text-muted-foreground font-medium tracking-wider uppercase">
            Marketplace
          </span>
        </div>
      </Link>

      {/* Navigation */}
      <nav className="flex flex-col gap-0.5 flex-1">
        {NAV.map(({ to, label, Icon }) => {
          const active = pathname === to || pathname.startsWith(to + "/");
          return (
            <Link
              key={to}
              to={to}
              className={
                "flex items-center gap-3 rounded-2xl px-3 py-2.5 text-sm font-medium transition-all duration-200 " +
                (active
                  ? "bg-primary text-primary-foreground shadow-soft"
                  : "text-muted-foreground hover:bg-muted hover:text-foreground")
              }
            >
              <Icon className="w-[18px] h-[18px]" />
              <span className="font-display">{label}</span>
              {active && (
                <div className="ml-auto w-1.5 h-1.5 rounded-full bg-primary-foreground animate-pulse" />
              )}
            </Link>
          );
        })}
      </nav>

      {/* Bottom section */}
      <div className="flex flex-col gap-3 pt-3 border-t border-border/50">
        {/* Mini wallet card */}
        <div className="nautica-glass p-4 relative overflow-hidden">
          <div
            aria-hidden
            className="absolute -top-8 -right-8 w-20 h-20 rounded-full bg-primary/10 blur-2xl"
          />
          <div className="relative flex items-center gap-3">
            <div className="w-8 h-8 rounded-xl bg-primary/15 grid place-items-center">
              <Wallet className="w-4 h-4 text-primary" />
            </div>
            <div className="min-w-0 flex-1">
              <p className="text-[11px] text-muted-foreground font-medium">
                Connected
              </p>
              <p className="text-sm text-ink font-display font-semibold truncate">
                0x4a7b…f02c
              </p>
            </div>
          </div>
          <div className="flex items-center gap-2 mt-3">
            <span className="text-xs text-muted-foreground">2.41 ETH</span>
            <span className="ml-auto badge-success">
              <span className="w-1 h-1 rounded-full bg-success" />
              Active
            </span>
          </div>
        </div>

        <div className="flex items-center justify-between">
          <ThemeToggle compact />
          <span className="text-[10px] text-muted-foreground/60">v2.0</span>
        </div>
      </div>
    </aside>
  );
}

export function BottomDock() {
  const { pathname } = useLocation();
  const DOCK_NAV = NAV.slice(0, 5); // Show first 5 in dock

  return (
    <nav className="md:hidden fixed bottom-4 left-4 right-4 z-40 nautica-glass-strong p-1.5 flex items-center justify-between animate-fade-up">
      {DOCK_NAV.map(({ to, label, Icon }) => {
        const active = pathname === to || pathname.startsWith(to + "/");
        return (
          <Link
            key={to}
            to={to}
            className={
              "flex-1 flex flex-col items-center gap-1 py-2 px-1 rounded-2xl transition-all duration-200 " +
              (active
                ? "bg-primary text-primary-foreground scale-105"
                : "text-muted-foreground active:scale-95")
            }
          >
            <Icon className="w-[18px] h-[18px]" />
            <span className="text-[9px] font-display font-semibold tracking-wide">
              {label}
            </span>
          </Link>
        );
      })}
    </nav>
  );
}

export function TopBar() {
  return (
    <header className="md:hidden flex items-center justify-between px-5 pt-5 pb-2">
      <Link to="/" className="flex items-center gap-2.5 group">
        <div className="w-9 h-9 rounded-xl bg-primary/10 border border-primary/20 grid place-items-center overflow-hidden">
          <img src={logo} alt="" width={24} height={24} className="w-6 h-6" />
        </div>
        <span className="font-display text-lg font-bold text-ink">
          Nautica
        </span>
      </Link>
      <div className="flex items-center gap-2">
        <span className="nautica-pill text-[10px]">
          <Wallet className="w-3 h-3" />
          0x4a…f02c
        </span>
        <ThemeToggle compact />
      </div>
    </header>
  );
}
