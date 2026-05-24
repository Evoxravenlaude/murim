// Realistic mock fixtures used while the live API is not yet wired.
// Server fns return these so every screen renders end-to-end.

export type MarketItem = {
  id: string;
  title: string;
  artist: string;
  price: number;
  currency: "ETH" | "USDC" | "SOL";
  edition: string;
  category: "Generative" | "Photography" | "Painting" | "3D" | "Music" | "PFP";
  cover: string;
  floor?: number;
  volume24h?: number;
  change24h?: number;
  owners?: number;
  listed?: number;
};

export type Drop = {
  id: string;
  title: string;
  artist: string;
  startsAt: string;
  status: "live" | "upcoming" | "ended";
  pieces: number;
  minted: number;
  price: number;
  currency: "ETH" | "SOL";
  cover: string;
};

export type Activity = {
  id: string;
  kind: "buy" | "send" | "receive" | "mint" | "list" | "swap";
  title: string;
  amount: number;
  currency: "ETH" | "USDC" | "SOL";
  proof: "verified" | "pending" | "none";
  at: string;
  from?: string;
  to?: string;
  hash?: string;
};

export type Token = {
  symbol: string;
  name: string;
  balance: number;
  price: number;
  change24h: number;
  icon: string; // gradient string for now
};

export type Collection = {
  id: string;
  name: string;
  cover: string;
  floor: number;
  volume: number;
  items: number;
  owners: number;
  change24h: number;
};

const grad = (a: string, b: string) =>
  `linear-gradient(135deg, ${a}, ${b})`;

export const MOCK_MARKET: MarketItem[] = [
  { id: "m1", title: "Tideglass 04", artist: "Anouk Vey", price: 0.42, currency: "ETH", edition: "1 of 12", category: "Generative", cover: grad("oklch(0.65 0.15 220)", "oklch(0.45 0.18 280)"), floor: 0.38, volume24h: 2.4, change24h: 12.5, owners: 8, listed: 4 },
  { id: "m2", title: "Soft Machine", artist: "Hiro Tanaka", price: 1.20, currency: "ETH", edition: "1/1", category: "3D", cover: grad("oklch(0.50 0.20 180)", "oklch(0.35 0.15 220)"), floor: 1.20, volume24h: 1.2, change24h: -3.2, owners: 1, listed: 1 },
  { id: "m3", title: "Grove, August", artist: "Mara Idris", price: 0.32, currency: "ETH", edition: "3 of 25", category: "Photography", cover: grad("oklch(0.55 0.12 130)", "oklch(0.40 0.15 160)"), floor: 0.28, volume24h: 4.6, change24h: 8.1, owners: 18, listed: 7 },
  { id: "m4", title: "Folded Light", artist: "Lev Marin", price: 0.18, currency: "ETH", edition: "open", category: "Generative", cover: grad("oklch(0.60 0.18 320)", "oklch(0.42 0.22 280)"), floor: 0.15, volume24h: 8.2, change24h: 24.3, owners: 124, listed: 32 },
  { id: "m5", title: "Field Notes II", artist: "P. Okafor", price: 0.54, currency: "ETH", edition: "2 of 8", category: "Painting", cover: grad("oklch(0.68 0.14 80)", "oklch(0.50 0.18 40)"), floor: 0.48, volume24h: 1.1, change24h: -1.5, owners: 6, listed: 2 },
  { id: "m6", title: "Reef Memory", artist: "Sela Kim", price: 0.08, currency: "ETH", edition: "5 of 50", category: "Photography", cover: grad("oklch(0.48 0.14 200)", "oklch(0.32 0.18 240)"), floor: 0.06, volume24h: 3.8, change24h: 5.7, owners: 38, listed: 12 },
  { id: "m7", title: "Neural Bloom", artist: "Ada Chen", price: 2.10, currency: "ETH", edition: "1/1", category: "Generative", cover: grad("oklch(0.55 0.22 340)", "oklch(0.38 0.25 300)"), floor: 2.10, volume24h: 2.1, change24h: 0, owners: 1, listed: 1 },
  { id: "m8", title: "Pulse Sequence", artist: "J. Moreau", price: 0.065, currency: "ETH", edition: "17 of 100", category: "Music", cover: grad("oklch(0.62 0.16 155)", "oklch(0.42 0.14 195)"), floor: 0.05, volume24h: 12.4, change24h: 34.1, owners: 78, listed: 22 },
  { id: "m9", title: "Monolith #7", artist: "Kai Stone", price: 0.88, currency: "ETH", edition: "7 of 10", category: "3D", cover: grad("oklch(0.30 0.05 260)", "oklch(0.18 0.03 240)"), floor: 0.72, volume24h: 5.6, change24h: -7.2, owners: 9, listed: 3 },
];

export const MOCK_DROPS: Drop[] = [
  { id: "d1", title: "Saltwater Index", artist: "Anouk Vey", startsAt: "Live now", status: "live", pieces: 24, minted: 18, price: 0.42, currency: "ETH", cover: grad("oklch(0.50 0.20 180)", "oklch(0.35 0.15 220)") },
  { id: "d2", title: "Quiet Algorithms", artist: "Hiro Tanaka", startsAt: "In 2h 14m", status: "upcoming", pieces: 12, minted: 0, price: 1.2, currency: "ETH", cover: grad("oklch(0.65 0.15 220)", "oklch(0.45 0.18 280)") },
  { id: "d3", title: "Notes from August", artist: "Mara Idris", startsAt: "Tomorrow, 18:00", status: "upcoming", pieces: 8, minted: 0, price: 0.32, currency: "ETH", cover: grad("oklch(0.55 0.12 130)", "oklch(0.40 0.15 160)") },
  { id: "d4", title: "Folded Light Vol I", artist: "Lev Marin", startsAt: "Ended 3d ago", status: "ended", pieces: 50, minted: 50, price: 0.18, currency: "ETH", cover: grad("oklch(0.60 0.18 320)", "oklch(0.42 0.22 280)") },
];

export const MOCK_ACTIVITY: Activity[] = [
  { id: "a1", kind: "buy", title: "Tideglass 04 — Anouk Vey", amount: 0.42, currency: "ETH", proof: "verified", at: "2m ago", hash: "0x7af3...c8b1" },
  { id: "a2", kind: "send", title: "Private payment to 0x91…2c", amount: 0.12, currency: "ETH", proof: "verified", at: "1h ago", hash: "0x9b2c...a7f4" },
  { id: "a3", kind: "receive", title: "From 0x4a…f0", amount: 0.05, currency: "ETH", proof: "verified", at: "5h ago", hash: "0x3d1e...b928" },
  { id: "a4", kind: "swap", title: "ETH → USDC on Uniswap", amount: 0.5, currency: "ETH", proof: "none", at: "8h ago", hash: "0x6c4a...d193" },
  { id: "a5", kind: "send", title: "Private payment to 0x77…ab", amount: 0.08, currency: "ETH", proof: "pending", at: "Today", hash: "0x1f8b...e472" },
  { id: "a6", kind: "mint", title: "Field Notes II — minted", amount: 0, currency: "ETH", proof: "verified", at: "Yesterday", hash: "0x5a9f...c381" },
  { id: "a7", kind: "list", title: "Reef Memory — listed", amount: 0.08, currency: "ETH", proof: "none", at: "2d ago", hash: "0x8e2d...a156" },
  { id: "a8", kind: "buy", title: "Pulse Sequence — J. Moreau", amount: 0.065, currency: "ETH", proof: "verified", at: "3d ago", hash: "0x2c7f...b934" },
];

export const MOCK_TOKENS: Token[] = [
  { symbol: "ETH", name: "Ethereum", balance: 2.41, price: 3847.52, change24h: 2.34, icon: grad("oklch(0.50 0.15 260)", "oklch(0.35 0.10 280)") },
  { symbol: "USDC", name: "USD Coin", balance: 1240.50, price: 1.00, change24h: 0.01, icon: grad("oklch(0.55 0.16 240)", "oklch(0.40 0.12 220)") },
  { symbol: "SOL", name: "Solana", balance: 12.8, price: 178.90, change24h: -1.23, icon: grad("oklch(0.60 0.20 300)", "oklch(0.45 0.22 280)") },
  { symbol: "MATIC", name: "Polygon", balance: 450, price: 0.89, change24h: 5.67, icon: grad("oklch(0.55 0.22 280)", "oklch(0.40 0.18 300)") },
  { symbol: "WETH", name: "Wrapped ETH", balance: 0.5, price: 3847.52, change24h: 2.34, icon: grad("oklch(0.50 0.15 260)", "oklch(0.35 0.10 280)") },
];

export const MOCK_COLLECTIONS: Collection[] = [
  { id: "c1", name: "Tideglass Series", cover: grad("oklch(0.65 0.15 220)", "oklch(0.45 0.18 280)"), floor: 0.38, volume: 48.2, items: 12, owners: 8, change24h: 12.5 },
  { id: "c2", name: "Neural Landscapes", cover: grad("oklch(0.55 0.22 340)", "oklch(0.38 0.25 300)"), floor: 1.80, volume: 124.6, items: 25, owners: 18, change24h: -3.2 },
  { id: "c3", name: "Sound Fragments", cover: grad("oklch(0.62 0.16 155)", "oklch(0.42 0.14 195)"), floor: 0.05, volume: 86.4, items: 100, owners: 78, change24h: 34.1 },
  { id: "c4", name: "Monolith Collection", cover: grad("oklch(0.30 0.05 260)", "oklch(0.18 0.03 240)"), floor: 0.72, volume: 56.8, items: 10, owners: 9, change24h: -7.2 },
];

// Portfolio stats
export const MOCK_PORTFOLIO = {
  totalValue: 12847.32,
  totalValueETH: 3.34,
  change24h: 4.28,
  change24hValue: 527.14,
  collected: 7,
  listed: 2,
  volume: 4.86,
};
