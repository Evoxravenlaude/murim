import { createServerFn } from "@tanstack/react-start";
import { MOCK_ACTIVITY, MOCK_DROPS, MOCK_MARKET, MOCK_TOKENS, MOCK_COLLECTIONS, MOCK_PORTFOLIO } from "./mock-data";

// All requests proxy through the server so the browser never talks to the
// Node API directly. Until NAUTICA_API_URL is set, mocks are returned.

async function tryFetch<T>(path: string, fallback: T): Promise<T> {
  const base = process.env.NAUTICA_API_URL;
  if (!base) return fallback;
  try {
    const r = await fetch(`${base}${path}`, { headers: { accept: "application/json" } });
    if (!r.ok) return fallback;
    return (await r.json()) as T;
  } catch {
    return fallback;
  }
}

export const getMarket = createServerFn({ method: "GET" }).handler(async () => {
  return tryFetch("/market", MOCK_MARKET);
});

export const getDrops = createServerFn({ method: "GET" }).handler(async () => {
  return tryFetch("/drops", MOCK_DROPS);
});

export const getActivity = createServerFn({ method: "GET" }).handler(async () => {
  return tryFetch("/activity", MOCK_ACTIVITY);
});

export const getTokens = createServerFn({ method: "GET" }).handler(async () => {
  return tryFetch("/tokens", MOCK_TOKENS);
});

export const getCollections = createServerFn({ method: "GET" }).handler(async () => {
  return tryFetch("/collections", MOCK_COLLECTIONS);
});

export const getPortfolio = createServerFn({ method: "GET" }).handler(async () => {
  return tryFetch("/portfolio", MOCK_PORTFOLIO);
});

export const submitPayment = createServerFn({ method: "POST" })
  .inputValidator((d: { to: string; amount: number; currency: "ETH" | "USDC" | "SOL"; note?: string }) => d)
  .handler(async ({ data }) => {
    const base = process.env.NAUTICA_API_URL;
    if (!base) {
      // Mock proof job
      await new Promise((r) => setTimeout(r, 600));
      return { jobId: `mock_${Date.now()}`, status: "pending" as const };
    }
    const r = await fetch(`${base}/payments`, {
      method: "POST",
      headers: { "content-type": "application/json" },
      body: JSON.stringify(data),
    });
    if (!r.ok) throw new Error("Payment failed");
    return (await r.json()) as { jobId: string; status: "pending" };
  });

export const getProofStatus = createServerFn({ method: "GET" })
  .inputValidator((d: { jobId: string }) => d)
  .handler(async ({ data }) => {
    const base = process.env.NAUTICA_PROVER_URL;
    if (!base) {
      // Mock: cycle states based on job age
      const ts = Number(data.jobId.split("_")[1] ?? 0);
      const age = Date.now() - ts;
      const status = age > 6000 ? "verified" : age > 3000 ? "proving" : "queued";
      return { jobId: data.jobId, status, progress: Math.min(100, Math.round(age / 60)) };
    }
    const r = await fetch(`${base}/status/${data.jobId}`);
    if (!r.ok) throw new Error("Status check failed");
    return (await r.json()) as { jobId: string; status: string; progress: number };
  });

export const submitSwap = createServerFn({ method: "POST" })
  .inputValidator((d: { fromToken: string; toToken: string; amount: number; slippage: number }) => d)
  .handler(async ({ data }) => {
    const base = process.env.NAUTICA_API_URL;
    if (!base) {
      await new Promise((r) => setTimeout(r, 800));
      return {
        hash: `0x${Math.random().toString(16).slice(2, 10)}...${Math.random().toString(16).slice(2, 6)}`,
        fromAmount: data.amount,
        toAmount: data.amount * 3847.52, // Mock ETH→USDC rate
        status: "confirmed" as const,
      };
    }
    const r = await fetch(`${base}/swap`, {
      method: "POST",
      headers: { "content-type": "application/json" },
      body: JSON.stringify(data),
    });
    if (!r.ok) throw new Error("Swap failed");
    return (await r.json()) as { hash: string; fromAmount: number; toAmount: number; status: "confirmed" };
  });

export const submitMint = createServerFn({ method: "POST" })
  .inputValidator((d: {
    title: string;
    description: string;
    category: string;
    traits: Array<{ key: string; value: string }>;
    supply: number;
    price: number;
    currency: string;
    royalty: number;
  }) => d)
  .handler(async ({ data }) => {
    const base = process.env.NAUTICA_API_URL;
    if (!base) {
      await new Promise((r) => setTimeout(r, 1200));
      return {
        tokenId: `nft_${Date.now()}`,
        hash: `0x${Math.random().toString(16).slice(2, 10)}...${Math.random().toString(16).slice(2, 6)}`,
        status: "minted" as const,
      };
    }
    const r = await fetch(`${base}/mint`, {
      method: "POST",
      headers: { "content-type": "application/json" },
      body: JSON.stringify(data),
    });
    if (!r.ok) throw new Error("Mint failed");
    return (await r.json()) as { tokenId: string; hash: string; status: "minted" };
  });
