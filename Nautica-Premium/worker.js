/**
 * Nautica Edge Worker
 * - Serves the SPA for all non-API routes
 * - Proxies Alchemy + 0x requests server-side so API keys never reach the browser
 *
 * Required Cloudflare secrets (set via `wrangler secret put`):
 *   ALCHEMY_KEY   — your Alchemy API key
 *   ZERO_X_KEY    — your 0x API key
 */

const CORS = {
  'Access-Control-Allow-Origin': '*',
  'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
  'Access-Control-Allow-Headers': 'Content-Type',
};

export default {
  async fetch(request, env) {
    const url = new URL(request.url);

    // ── Preflight ──────────────────────────────────────────────────────────
    if (request.method === 'OPTIONS') {
      return new Response(null, { headers: CORS });
    }

    // ── /api/alchemy/* → https://eth-mainnet.g.alchemy.com/* ──────────────
    if (url.pathname.startsWith('/api/alchemy/')) {
      const path = url.pathname.replace('/api/alchemy/', '');
      const target = `https://eth-mainnet.g.alchemy.com/${path}${url.search}`;
      const upstream = await fetch(target, {
        method: request.method,
        headers: {
          'Content-Type': 'application/json',
          Authorization: `Bearer ${env.ALCHEMY_KEY}`,
        },
        body: request.method !== 'GET' ? request.body : undefined,
      });
      const body = await upstream.text();
      return new Response(body, {
        status: upstream.status,
        headers: { 'Content-Type': 'application/json', ...CORS },
      });
    }

    // ── /api/0x/* → https://api.0x.org/* ──────────────────────────────────
    if (url.pathname.startsWith('/api/0x/')) {
      const path = url.pathname.replace('/api/0x/', '');
      const target = `https://api.0x.org/${path}${url.search}`;
      const upstream = await fetch(target, {
        method: request.method,
        headers: {
          'Content-Type': 'application/json',
          '0x-api-key': env.ZERO_X_KEY,
        },
        body: request.method !== 'GET' ? request.body : undefined,
      });
      const body = await upstream.text();
      return new Response(body, {
        status: upstream.status,
        headers: { 'Content-Type': 'application/json', ...CORS },
      });
    }

    // ── /api/alchemy-nft/* → https://eth-mainnet.g.alchemy.com/nft/v3/* ──
    if (url.pathname.startsWith('/api/alchemy-nft/')) {
      const path = url.pathname.replace('/api/alchemy-nft/', '');
      const target = `https://eth-mainnet.g.alchemy.com/nft/v3/${env.ALCHEMY_KEY}/${path}${url.search}`;
      const upstream = await fetch(target, {
        headers: { 'Content-Type': 'application/json' },
      });
      const body = await upstream.text();
      return new Response(body, {
        status: upstream.status,
        headers: { 'Content-Type': 'application/json', ...CORS },
      });
    }

    // ── Everything else → SPA ─────────────────────────────────────────────
    return env.ASSETS.fetch(request);
  },
};
