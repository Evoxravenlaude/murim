import { useState, useEffect } from "react";
import { Link, useLocation } from "react-router-dom";
import {
  Search, Bell, Menu, X, Sun, Moon,
} from "lucide-react";
import { ConnectButton } from "@rainbow-me/rainbowkit";

interface TopNavProps {
  theme: "dark" | "light";
  setTheme: (t: "dark" | "light") => void;
}

const navLinks = [
  { label: "Home", path: "/" },
  { label: "DEX", path: "/dex" },
  { label: "Marketplace", path: "/nft" },
  { label: "ZK Proofs", path: "/zk-send" },
  { label: "Pools", path: "/pools" },
];

// Inline N-lettermark — matches favicon.svg exactly, no broken image request
function NauticaLogo({ size = 32 }: { size?: number }) {
  return (
    <svg
      width={size}
      height={size}
      viewBox="0 0 64 64"
      xmlns="http://www.w3.org/2000/svg"
      aria-hidden="true"
    >
      <defs>
        <linearGradient id="nav-g" x1="0" y1="0" x2="1" y2="1">
          <stop offset="0%" stopColor="#06F5D6" />
          <stop offset="100%" stopColor="#0EA5E9" />
        </linearGradient>
      </defs>
      <circle cx="32" cy="32" r="30" fill="#050A12" />
      <circle cx="32" cy="32" r="29" fill="none" stroke="url(#nav-g)" strokeWidth="1.5" opacity="0.6" />
      <text
        x="32"
        y="42"
        textAnchor="middle"
        fontFamily="Arial Black, sans-serif"
        fontWeight="900"
        fontSize="32"
        fill="url(#nav-g)"
      >
        N
      </text>
    </svg>
  );
}

export default function TopNav({ theme, setTheme }: TopNavProps) {
  const [scrolled, setScrolled] = useState(false);
  const [mobileOpen, setMobileOpen] = useState(false);
  const [searchOpen, setSearchOpen] = useState(false);
  const location = useLocation();

  useEffect(() => {
    const onScroll = () => setScrolled(window.scrollY > 20);
    window.addEventListener("scroll", onScroll, { passive: true });
    return () => window.removeEventListener("scroll", onScroll);
  }, []);

  useEffect(() => {
    setMobileOpen(false);
  }, [location.pathname]);

  return (
    <nav
      className={`fixed top-0 left-0 right-0 z-50 transition-all duration-300 ${
        scrolled
          ? "liquid-glass border-b border-white/5"
          : "bg-transparent"
      }`}
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-16">
          {/* Logo — inline SVG, no broken image risk */}
          <Link to="/" className="flex items-center gap-3">
            <NauticaLogo size={32} />
            <span className="font-heading text-lg font-medium tracking-wider text-text-primary">
              Nautica
            </span>
          </Link>

          {/* Desktop Nav */}
          <div className="hidden md:flex items-center gap-1">
            {navLinks.map((link) => (
              <Link
                key={link.path}
                to={link.path}
                className={`px-4 py-2 text-sm font-body transition-colors duration-200 ${
                  location.pathname === link.path
                    ? "text-cyan"
                    : "text-text-secondary hover:text-text-primary"
                }`}
              >
                {link.label}
              </Link>
            ))}
          </div>

          {/* Right Side */}
          <div className="flex items-center gap-2">
            <button
              onClick={() => setSearchOpen(!searchOpen)}
              className="p-2 text-text-secondary hover:text-cyan transition-colors"
              aria-label="Search"
            >
              <Search size={18} />
            </button>

            <button
              onClick={() => setTheme(theme === "dark" ? "light" : "dark")}
              className="p-2 text-text-secondary hover:text-cyan transition-colors"
              aria-label="Toggle theme"
            >
              {theme === "dark" ? <Sun size={18} /> : <Moon size={18} />}
            </button>

            <Link
              to="/notifications"
              className="p-2 text-text-secondary hover:text-cyan transition-colors relative"
            >
              <Bell size={18} />
              <span className="absolute top-1 right-1 w-2 h-2 bg-amber rounded-full" />
            </Link>

            <div className="hidden sm:block">
              <ConnectButton
                chainStatus="icon"
                showBalance={false}
                accountStatus="address"
              />
            </div>

            <button
              onClick={() => setMobileOpen(!mobileOpen)}
              className="md:hidden p-2 text-text-secondary"
              aria-label="Menu"
            >
              {mobileOpen ? <X size={20} /> : <Menu size={20} />}
            </button>
          </div>
        </div>
      </div>

      {/* Search Overlay */}
      {searchOpen && (
        <div className="absolute top-full left-0 right-0 liquid-glass border-t border-white/5 p-4">
          <input
            type="text"
            placeholder="Search tokens, NFTs, addresses..."
            className="w-full bg-obsidian border border-white/10 px-4 py-3 text-text-primary font-mono text-sm placeholder:text-text-tertiary focus:outline-none focus:border-cyan"
            autoFocus
          />
        </div>
      )}

      {/* Mobile Menu */}
      {mobileOpen && (
        <div className="md:hidden liquid-glass border-t border-white/5">
          <div className="px-4 py-3 space-y-1">
            {navLinks.map((link) => (
              <Link
                key={link.path}
                to={link.path}
                className={`block px-4 py-3 text-sm font-body ${
                  location.pathname === link.path
                    ? "text-cyan"
                    : "text-text-secondary"
                }`}
              >
                {link.label}
              </Link>
            ))}
            <div className="px-4 py-3">
              <ConnectButton
                chainStatus="icon"
                showBalance={false}
                accountStatus="address"
              />
            </div>
          </div>
        </div>
      )}
    </nav>
  );
}
